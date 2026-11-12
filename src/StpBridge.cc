/*
 * StpBridge.cc
 *
 *  Created on: Nov 6, 2026
 *      Author: duongldm
 */

#include <omnetpp.h>
#include <algorithm>
#include <limits>
#include <optional>
#include <set>
#include <tuple>
#include <vector>
#include "Bpdu_m.h"

using namespace omnetpp;

class StpBridge : public cSimpleModule {
  private:
    using BridgeId = std::pair<int64_t, int64_t>;
    using PriorityVector = std::tuple<BridgeId, int64_t, BridgeId, int>;
    enum class Role { Unknown, Root, Designated, Alternate, Down };

    struct Port {
        cDelayChannel *channel;
        int64_t cost;
        simtime_t failAt;
        simtime_t restoreAt;
        bool awaitingBpdu = false;
        Role role = Role::Unknown;
        std::optional<PriorityVector> received;
        simtime_t expires;

        bool forwarding() const {
            return !awaitingBpdu && (role == Role::Root || role == Role::Designated);
        }
    };

    BridgeId self;
    BridgeId root{-1, -1};
    int64_t rootCost = 0;
    int rootPort = -1;
    simtime_t helloTime;
    simtime_t maxAge;
    bool rapid = false;
    simtime_t probeInterval;
    simtime_t probeEnd;
    cMessage *probeTimer = nullptr;
    std::vector<int64_t> destinations;
    int64_t probeSequence = 0;
    std::set<std::pair<int64_t, int64_t>> seenProbes;
    cMessage *helloTimer = nullptr;
    cMessage *portTimer = nullptr;
    std::vector<Port> ports;

    void recompute();
    void sendConfig();
    void logState() const;
    void originateProbes();
    void forwardProbe(StpProbe *probe, int ingress);

  protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;

  public:
    ~StpBridge() override;
};

Define_Module(StpBridge);

StpBridge::~StpBridge() {
    cancelAndDelete(helloTimer);
    cancelAndDelete(portTimer);
    cancelAndDelete(probeTimer);
}

void StpBridge::initialize() {
    self = {par("priority").intValue(), par("bridgeId").intValue()};
    helloTime = par("helloTime").doubleValue();
    maxAge = par("maxAge").doubleValue();
    rapid = par("rapid").boolValue();
    if (self.first < 0 || self.second < 0 || helloTime <= SIMTIME_ZERO || maxAge <= SIMTIME_ZERO)
        throw cRuntimeError("Bridge identity must be nonnegative and timers must be positive");

    for (int i = 0; i < gateSize("ethg"); ++i) {
        auto *channel = check_and_cast<cDelayChannel *>(gate("ethg$o", i)->getChannel());
        int64_t cost = channel->par("pathCost").intValue();
        if (cost <= 0)
            throw cRuntimeError("Path cost must be positive on port %d", i);
        simtime_t failAt = channel->par("failAt").doubleValue();
        simtime_t restoreAt = channel->par("restoreAt").doubleValue();
        if (failAt >= SIMTIME_ZERO && restoreAt >= SIMTIME_ZERO && restoreAt <= failAt)
            throw cRuntimeError("Link restoration must follow failure on port %d", i);
        ports.push_back({channel, cost, failAt, restoreAt});
    }

    helloTimer = new cMessage("hello");
    portTimer = new cMessage("portChange");
    portTimer->setSchedulingPriority(-1);
    recompute();
    scheduleAt(simTime(), helloTimer);
    WATCH(root.second);
    WATCH(rootCost);
    WATCH(rootPort);

    probeInterval = par("probeInterval").doubleValue();
    if (probeInterval > SIMTIME_ZERO) {
        simtime_t start = par("probeStart").doubleValue();
        probeEnd = par("probeEnd").doubleValue();
        if (start < SIMTIME_ZERO || start >= probeEnd)
            throw cRuntimeError("Probe window must satisfy 0 <= probeStart < probeEnd");
        for (cModule::SubmoduleIterator it(getParentModule()); !it.end(); ++it) {
            auto *bridge = dynamic_cast<StpBridge *>(*it);
            if (bridge && bridge != this)
                destinations.push_back(bridge->par("bridgeId").intValue());
        }
        probeTimer = new cMessage("probeRound");
        scheduleAt(start, probeTimer);
    }
}

void StpBridge::recompute() {
    const auto now = simTime();
    std::optional<simtime_t> nextChange;
    auto wakeAt = [&nextChange](simtime_t time) {
        if (!nextChange || time < *nextChange)
            nextChange = time;
    };

    PriorityVector best{self, 0, self, -1};
    int bestPort = -1;
    bool linkChanged = false;
    for (int i = 0; i < gateSize("ethg"); ++i) {
        auto& port = ports[i];
        bool willRestore = now < port.restoreAt;
        bool down = port.failAt < SIMTIME_ZERO ? willRestore :
                    now >= port.failAt && (port.restoreAt < SIMTIME_ZERO || willRestore);
        if (now < port.failAt)
            wakeAt(port.failAt);
        if (willRestore)
            wakeAt(port.restoreAt);
        if (down != (port.role == Role::Down)) {
            port.channel->setDisabled(down);
            port.received.reset();
            port.role = down ? Role::Down : Role::Unknown;
            port.awaitingBpdu = !down;
            linkChanged = true;
            EV_INFO << (down ? "STP_LINK_DOWN t=" : "STP_LINK_UP t=") << now
                    << " bridge=" << getName()
                    << " peer=" << gate("ethg$o", i)->getNextGate()->getOwnerModule()->getName()
                    << " port=" << i << "\n";
        }
        if (down)
            continue;
        if (!port.received)
            continue;
        if (port.expires <= now) {
            port.received.reset();
            EV_INFO << "STP_EXPIRE t=" << now << " bridge=" << getName() << " port=" << i << "\n";
            continue;
        }
        wakeAt(port.expires);
        auto candidate = *port.received;
        std::get<1>(candidate) += port.cost;
        // Ascending local port index breaks ties after the sender's port ID.
        if (candidate < best) {
            best = candidate;
            bestPort = i;
        }
    }

    const auto& bestRoot = std::get<0>(best);
    auto bestCost = std::get<1>(best);
    bool routeChanged = root != bestRoot || rootCost != bestCost || rootPort != bestPort;
    bool changed = linkChanged || routeChanged;
    root = bestRoot;
    rootCost = bestCost;
    rootPort = bestPort;

    for (int i = 0; i < gateSize("ethg"); ++i) {
        auto& port = ports[i];
        if (port.role == Role::Down)
            continue;
        // shortcut: a restored port waits for its first valid BPDU, add proposal/agreement for 802.1w.
        if (port.awaitingBpdu && port.received) {
            port.awaitingBpdu = false;
            changed = true;
        }
        auto role = Role::Designated;
        if (i == rootPort)
            role = Role::Root;
        else if (port.received && *port.received < PriorityVector{root, rootCost, self, i})
            role = Role::Alternate;
        changed |= port.role != role;
        port.role = role;
    }
    if (changed)
        logState();

    // shortcut: triggered BPDUs model faster propagation, add protocol state machines for interoperability.
    if (rapid && (routeChanged || linkChanged) && helloTimer->isScheduled() &&
        helloTimer->getArrivalTime() != now) {
        EV_DETAIL << "STP_TRIGGER t=" << now << " bridge=" << getName() << "\n";
        sendConfig();
    }

    if (nextChange)
        rescheduleAt(*nextChange, portTimer);
    else
        cancelEvent(portTimer);
}

void StpBridge::logState() const {
    EV_INFO << "STP_STATE t=" << simTime() << " bridge=" << getName()
            << " root=" << root.second << " priority=" << root.first
            << " cost=" << rootCost << " rootPort=" << rootPort;
    for (int i = 0; i < gateSize("ethg"); ++i) {
        auto role = ports[i].role;
        const char *roleName = role == Role::Root ? "root" :
                               role == Role::Designated ? "designated" :
                               role == Role::Alternate ? "alternate" : "disabled";
        const char *state = role == Role::Down ? "down" :
                            ports[i].forwarding() ? "forwarding" : "blocking";
        EV_INFO << " port=" << i << ":" << gate("ethg$o", i)->getNextGate()->getOwnerModule()->getName()
                << ":" << roleName << ":" << state;
    }
    EV_INFO << "\n";
}

void StpBridge::sendConfig() {
    // Preserve the root's deadline so a cycle cannot renew stale root information.
    auto validUntil = rootPort == -1 ? simTime() + maxAge : ports[rootPort].expires;
    for (int i = 0; i < gateSize("ethg"); ++i) {
        if (ports[i].role == Role::Down)
            continue;
        auto *bpdu = new Bpdu("BPDU");
        bpdu->setRootId(root.second);
        bpdu->setRootPriority(root.first);
        bpdu->setRootPathCost(rootCost);
        bpdu->setBridgeId(self.second);
        bpdu->setBridgePriority(self.first);
        bpdu->setPortId(i);
        bpdu->setValidUntil(validUntil);
        send(bpdu, "ethg$o", i);
    }
}

void StpBridge::handleMessage(cMessage *msg) {
    if (msg == helloTimer) {
        sendConfig();
        scheduleAt(simTime() + helloTime, helloTimer);
        return;
    }
    if (msg == portTimer) {
        recompute();
        return;
    }

    if (msg == probeTimer) {
        originateProbes();
        if (simTime() + probeInterval < probeEnd)
            scheduleAt(simTime() + probeInterval, probeTimer);
        return;
    }
    if (auto *probe = dynamic_cast<StpProbe *>(msg)) {
        forwardProbe(probe, msg->getArrivalGate()->getIndex());
        return;
    }

    auto *bpdu = check_and_cast<Bpdu *>(msg);
    int i = bpdu->getArrivalGate()->getIndex();
    auto& port = ports[i];
    BridgeId advertisedRoot{bpdu->getRootPriority(), bpdu->getRootId()};
    BridgeId sender{bpdu->getBridgePriority(), bpdu->getBridgeId()};
    auto cost = bpdu->getRootPathCost();
    const char *dropReason = nullptr;
    if (port.role == Role::Down)
        dropReason = "down";
    else if (port.restoreAt <= simTime() && bpdu->getSendingTime() < port.restoreAt)
        dropReason = "before-restore";
    else if (bpdu->getValidUntil() <= simTime())
        dropReason = "expired";
    else if (advertisedRoot.first < 0 || advertisedRoot.second < 0 ||
             sender.first < 0 || sender.second < 0 || advertisedRoot > sender ||
             bpdu->getPortId() < 0 || cost < 0 || (cost == 0) != (advertisedRoot == sender) ||
             cost > std::numeric_limits<int64_t>::max() - port.cost)
        dropReason = "invalid";

    if (dropReason) {
        EV_INFO << "STP_DROP t=" << simTime() << " bridge=" << getName()
                << " port=" << i << " reason=" << dropReason << "\n";
    }
    else {
        port.received = PriorityVector{advertisedRoot, cost, sender, bpdu->getPortId()};
        port.expires = std::min(bpdu->getValidUntil(), simTime() + maxAge);
        EV_DETAIL << "STP_BPDU t=" << simTime() << " bridge=" << getName() << " port=" << i
                  << " root=" << advertisedRoot.second << " priority=" << advertisedRoot.first
                  << " cost=" << cost << " sender=" << sender.second << " senderPriority=" << sender.first
                  << " senderPort=" << bpdu->getPortId() << " expires=" << port.expires << "\n";
        recompute();
    }
    delete bpdu;
}

void StpBridge::originateProbes() {
    for (auto destination : destinations) {
        auto *probe = new StpProbe("probe");
        probe->setSource(self.second);
        probe->setDestination(destination);
        probe->setSequence(probeSequence++);
        probe->setHopsLeft(destinations.size());
        EV_INFO << "PROBE_SEND t=" << simTime() << " src=" << self.second
                << " dst=" << destination << " seq=" << probe->getSequence() << "\n";
        forwardProbe(probe, -1);
    }
}

void StpBridge::forwardProbe(StpProbe *probe, int ingress) {
    const auto source = probe->getSource();
    const auto sequence = probe->getSequence();
    const char *dropReason = nullptr;
    // shortcut: retain IDs for the bounded probe run, expire IDs before using sustained traffic.
    if (ingress >= 0 && !ports[ingress].forwarding())
        dropReason = "blocked";
    else if (!seenProbes.emplace(source, sequence).second)
        dropReason = "duplicate";
    else if (probe->getDestination() == self.second)
        EV_INFO << "PROBE_RECV t=" << simTime() << " src=" << source
                << " dst=" << self.second << " seq=" << sequence << "\n";
    else if (probe->getHopsLeft() == 0)
        dropReason = "hop-limit";
    else {
        probe->setHopsLeft(probe->getHopsLeft() - 1);
        for (int i = 0; i < gateSize("ethg"); ++i)
            if (i != ingress && ports[i].forwarding())
                send(probe->dup(), "ethg$o", i);
    }
    if (dropReason)
        EV_DETAIL << "PROBE_DROP t=" << simTime() << " bridge=" << getName()
                  << " src=" << source << " dst=" << probe->getDestination()
                  << " seq=" << sequence << " reason=" << dropReason << "\n";
    delete probe;
}
