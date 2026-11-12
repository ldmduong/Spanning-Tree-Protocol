from collections import Counter, defaultdict
from decimal import Decimal as D
from itertools import pairwise
from pathlib import Path
import heapq
import sys

root_dir = Path(__file__).parent
ring = [(0, 1), (1, 2), (2, 3), (3, 4), (4, 0)]
extra_ring = [(4, 5), (5, 6), (6, 7), (7, 4)]

def topology(config):
    links = ring + ([] if config == "K5" else [(1, 4)])
    return links + (extra_ring if config == "K6" else [])

def fields(line):
    return dict(token.split("=", 1) for token in line.split()[1:])

def bridge_num(name):
    return int(name.removeprefix("b"))

def graph(links):
    neighbors = defaultdict(list)
    for a, b in links:
        neighbors[a].append(b)
        neighbors[b].append(a)
    return dict(neighbors)

def solve(config, down):
    links = topology(config)
    neighbors = graph(links)
    bid = {node: (4096 if config == "K2" and node == 3 else 32768, node)
           for node in neighbors}
    root = min(bid, key=bid.get)
    cost = defaultdict(lambda: float("inf"), {root: 0})
    outgoing = lambda a, b: 3 if config == "K4" and {a, b} == {0, 1} else 1
    pending = [(0, root)]
    while pending:
        distance, node = heapq.heappop(pending)
        if distance != cost[node]:
            continue
        for peer in neighbors[node]:
            trial = distance + outgoing(peer, node)
            if (node, peer) not in down and trial < cost[peer]:
                cost[peer] = trial
                heapq.heappush(pending, (trial, peer))

    expected = {}
    for node, peers in neighbors.items():
        parent = min((cost[peer] + outgoing(node, peer), bid[peer],
                      neighbors[peer].index(node), index)
                     for index, peer in enumerate(peers)
                     if (node, peer) not in down)[-1] if node != root else -1
        ports = []
        for index, peer in enumerate(peers):
            if (node, peer) in down:
                role, state = "disabled", "down"
            elif index == parent:
                role, state = "root", "forwarding"
            elif (cost[node], bid[node], index) < (cost[peer], bid[peer], neighbors[peer].index(node)):
                role, state = "designated", "forwarding"
            else:
                role, state = "alternate", "blocking"
            ports.append((peer, role, state))
        expected[node] = ((root, bid[root][0], cost[node], parent), tuple(ports))
    return expected

def snapshots(folder):
    results = []
    for line in (root_dir / folder / "cmdenv.log").read_text().splitlines():
        line = line.split("\t")[-1]
        if not line.startswith("STP_STATE "):
            continue
        tokens = line.split()
        attr = fields(" ".join(token for token in tokens if not token.startswith("port=")))
        ports = []
        for token in tokens:
            if token.startswith("port="):
                index, peer, role, state = token[5:].split(":")
                assert int(index) == len(ports)
                ports.append((bridge_num(peer), role, state))
        state = tuple(int(attr[key]) for key in ("root", "priority", "cost", "rootPort")), tuple(ports)
        results.append((D(attr["t"]), bridge_num(attr["bridge"]), state))
    return results

def forwarding(states):
    ends = {(node, peer) for node, (_, ports) in states.items()
            for peer, _, state in ports if state == "forwarding"}
    return sorted((a, b) for a, b in ends if a < b and (b, a) in ends)

def tree_path(links, src, dst):
    neighbors = graph(links)
    pending = [(src,)]
    visited = {src}
    while pending:
        path = pending.pop()
        if path[-1] == dst:
            return path
        for peer in neighbors.get(path[-1], []):
            if peer not in visited:
                visited.add(peer)
                pending.append(path + (peer,))
    return None

def verify(config, snapshot, start, end, down):
    changes = [change for change in snapshot if change[0] < end]
    states = {node: state for _, node, state in changes}
    expected = solve(config, down)
    assert states == expected, (config, start, states, expected)
    last = changes[-1][0]
    assert end - last >= 6, (config, start, last, end)
    tree = forwarding(states)
    assert len(tree) == len(states) - 1
    assert all(tree_path(tree, 0, node) for node in states)
    return last, tree

def packet_trace(folder):
    modules = {}
    packets, deliveries, scheduled = {}, {}, {}
    sent, drops = {}, []
    for line in next((root_dir / folder).glob("*.elog")).open():
        if line.startswith(("MC ", "E ", "CM ", "CL ", "ES ")):
            tokens = line.split()
            attr = dict(zip(tokens[1::2], tokens[2::2]))
            kind = tokens[0]
            if kind == "MC" and attr["c"] == "StpBridge":
                modules[attr["id"]] = bridge_num(attr["n"])
            elif kind == "E":
                time, module, msg = D(attr["t"]), modules.get(attr["m"]), attr["msg"]
                if msg in scheduled:
                    key, path = packets[msg]
                    assert scheduled.pop(msg) == (module, time, path[-1][0]), (folder, attr, path)
                    packets[msg] = key, path + ((module, time),)
            elif kind == "CM" and attr["n"] == "probe":
                origin = attr["id"]
            elif kind == "CL" and attr["n"] == "probe":
                packets[attr["id"]] = packets[attr["cid"]]
            elif kind == "ES" and attr["n"] == "probe":
                key, path = packets[attr["id"]]
                assert modules[attr["sm"]] == module == path[-1][0]
                assert D(attr["st"]) == time == path[-1][1]
                assert D(attr["at"]) - time == D(".01")
                scheduled[attr["id"]] = (modules[attr["am"]], D(attr["at"]), module)
        elif "\tPROBE_" in line:
            event = line.split("\t")[-1]
            attr = fields(event)
            src, dst, seq = (int(attr[key]) for key in ("src", "dst", "seq"))
            key = src, seq
            assert time == D(attr["t"])
            if event.startswith("PROBE_SEND"):
                assert module == src and key not in sent
                sent[key] = time, dst
                packets[origin] = key, ((src, time),)
            else:
                traced_key, path = packets[msg]
                assert traced_key == key and sent[key][1] == dst
                assert path[0] == (src, sent[key][0]) and path[-1] == (module, time)
                if event.startswith("PROBE_RECV"):
                    assert module == dst and key not in deliveries, (folder, key)
                    deliveries[key] = path
                else:
                    drops.append((key, attr["reason"], path))
    assert not scheduled
    return sent, deliveries, drops

def format_path(path):
    return " ".join(f"b{node}@{time}" for node, time in path)

def audit_packets(folder, before_tree=None, event=None, last=None, after_tree=None, n=5, end=23):
    sent, deliveries, drops = packet_trace(folder)
    rounds = {D(8) + D(".1") * step for step in range((end - 8) * 10)}
    expected_sends = {(t, a, b) for t in rounds for a in range(n) for b in range(n) if a != b}
    assert {(t, a, b) for (a, _), (t, b) in sent.items()} == expected_sends
    assert len(sent) == len(expected_sends)
    assert not any(reason == "hop-limit" for _, reason, _ in drops)
    reasons = Counter(reason for _, reason, _ in drops)
    print(folder, "total", len(deliveries), "/", len(sent), "copies dropped", dict(reasons))
    for label, start, stop, tree in (
        ("before", D(8), event or D(end), before_tree),
        ("event round", event, event + D(".1") if event is not None else None, None),
        ("remaining transition", event + D(".1") if event is not None else None, last, None),
        ("after", max(event + D(".1"), last) if event is not None else None, D(end), after_tree),
    ):
        if start is None or start >= stop:
            continue
        keys = [key for key, (time, _) in sent.items() if start <= time < stop]
        received = [key for key in keys if key in deliveries]
        if tree:
            assert len(keys) == len(received), (folder, label)
            for key in received:
                actual = tuple(node for node, _ in deliveries[key])
                assert actual == tree_path(tree, key[0], sent[key][1]), (folder, key, actual)
        print(f"  {label} [{start},{stop}) {len(received)}/{len(keys)}")
    missing = sorted(set(sent) - set(deliveries), key=lambda k: (sent[k][0], k))
    print("  missing by send time:", dict(Counter(sent[k][0] for k in missing)))
    for key in missing[:3] if folder == "probe-K3" else missing:
        print(f"  missing src={key[0]} dst={sent[key][1]} seq={key[1]}")
        for drop_key, reason, path in drops:
            if drop_key == key:
                print("   ", reason, format_path(path))
    for src, dst, timestamp in [(0, 3, D(8)), (0, 3, D(12)), (0, 3, D(18)), (0, 1, D(8)), (0, 1, D(18))]:
        for key, (time, target) in sent.items():
            if (key[0], target, time) == (src, dst, timestamp) and key in deliveries:
                print(f"  received src={src} dst={dst} seq={key[1]}", format_path(deliveries[key]))
    for key, reason, path in [drop for drop in drops if drop[1] == "duplicate"][:8]:
        print(f"  duplicate src={key[0]} dst={sent[key][1]} seq={key[1]}", format_path(path))

def audit():
    for config in ("K1", "K2", "K3", "K4", "K5", "K6", "K7_STP", "K7_RSTP"):
        log = snapshots(config)
        event = D("10.5") if config == "K3" else D(12) if config.startswith("K7") else None
        initial_down = {(0, 4), (4, 0)} if config.startswith("K7") else set()
        initial, before = verify(config, log, D(0), event or D(25), initial_down)
        print(config, "initial last", initial, "tree", before)
        if event is not None:
            final_down = {(0, 1), (1, 0)} if config == "K3" else set()
            last, after = verify(config, log, event, D(25), final_down)
            print(config, "post-event last", last, "tree", after)
        else:
            last, after = None, None
        folder = config if config.startswith("K7") else "probe-" + config
        assert snapshots(folder) == log
        audit_packets(folder, before, event, last, after, 8 if config == "K6" else 5)
    audit_packets("restore-in-flight-probe", end=10)

if __name__ == "__main__":
    audit()
