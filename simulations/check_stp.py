"""Verify K1–K7 snapshots and probe delivery from the Cmdenv logs."""
from collections import Counter
from decimal import Decimal
from pathlib import Path
import sys


BRIDGES = ("b0", "b1", "b2", "b3", "b4")
RING = (("b0", "b1"), ("b1", "b2"), ("b2", "b3"), ("b3", "b4"), ("b4", "b0"))
DIAGONAL = ("b1", "b4")
EXTRA_RING = (("b4", "b5"), ("b5", "b6"), ("b6", "b7"), ("b7", "b4"))
FAILURE = Decimal("10.5")
RESTORE = Decimal("12")
END = Decimal("25")
STABLE_WINDOW = Decimal("6")

# Each pair is (root-port peer, path cost), in b0–b4 order.
BASE = ((None, 0), ("b0", 1), ("b1", 2), ("b4", 2), ("b0", 1))
VIA_B4 = ((None, 0), ("b4", 2), ("b1", 3), ("b4", 2), ("b0", 1))
ROOT_B3 = (("b4", 2), ("b2", 2), ("b3", 1), (None, 0), ("b3", 1))
BASE_BLOCKED = {("b3", "b2"), ("b4", "b1")}
FAILED = {("b0", "b1"), ("b1", "b0")}
RESTORED = {("b0", "b4"), ("b4", "b0")}
BEFORE_RESTORE = ((None, 0), ("b0", 1), ("b1", 2), ("b2", 3), ("b1", 2))


def verify(label, snapshot, last_change, start, end, root, priority, routes, blocked, down):
    links = RING if label == "K5" else RING + (DIAGONAL,)
    bridges = BRIDGES
    if label == "K6":
        bridges += ("b5", "b6", "b7")
        links += EXTRA_RING
    assert set(snapshot) == set(bridges), (label, snapshot.keys())
    assert end - last_change >= STABLE_WINDOW, (label, last_change, end)

    counts = Counter()
    forwarding = set()
    for bridge, (parent, cost) in zip(bridges, routes):
        fields, ports = snapshot[bridge]
        neighbors = [b if a == bridge else a for a, b in links if bridge in (a, b)]
        assert (fields["root"], fields["priority"], fields["cost"], fields["rootPort"]) == (
            root, priority, cost, neighbors.index(parent) if parent else -1
        ), (label, bridge, fields)
        assert len(ports) == len(neighbors), (label, bridge, ports)
        for i, peer in enumerate(neighbors):
            if (bridge, peer) in down:
                role, state = "disabled", "down"
            elif (bridge, peer) in blocked:
                role, state = "alternate", "blocking"
            else:
                role, state = ("root" if peer == parent else "designated"), "forwarding"
            assert ports[i] == (i, peer, role, state), (label, bridge, ports[i])
            counts[state] += 1
            if state == "forwarding":
                forwarding.add((bridge, peer))

    tree = [(a, b) for a, b in links if (a, b) in forwarding and (b, a) in forwarding]
    reached = {f"b{root}"}
    for _ in tree:
        for a, b in tree:
            if a in reached or b in reached:
                reached.update((a, b))
    assert len(tree) == len(bridges) - 1 and reached == set(bridges), (label, tree, reached)
    edges = " ".join(f"{a}-{b}" for a, b in tree)
    print(f"{label}: root=b{root}, forwarding/blocking/down="
          f"{counts['forwarding']}/{counts['blocking']}/{counts['down']}, tree={edges}, "
          f"last change={last_change}s, convergence={last_change - start}s, "
          f"stable={end - last_change}s")


def verify_probes(config, sends, receives, drops, last_change):
    bridges = range(8 if config == "K6" else 5)
    pairs = {(src, dst) for src in bridges for dst in bridges if src != dst}
    rounds = {Decimal(8) + Decimal("0.1") * i for i in range(150)}
    assert set(sends.values()) == {(t, src, dst) for t in rounds for src, dst in pairs}, config
    assert len(sends) == len(rounds) * len(pairs), config
    assert set(receives) <= set(sends), config
    for key, (time, src, dst) in receives.items():
        start, expected_src, expected_dst = sends[key]
        assert time > start and (src, dst) == (expected_src, expected_dst), (config, key)
    assert not drops["hop-limit"], (config, drops)

    start, end = Decimal(8), Decimal(23)
    windows = (("stable", start, end, True),)
    if config == "K3":
        windows = (
            ("before", start, FAILURE, True),
            ("during reconvergence", FAILURE, last_change, False),
            ("after convergence", last_change, end, True),
        )
    elif config.startswith("K7"):
        next_round = RESTORE + Decimal("0.1")
        windows = (
            ("before", start, RESTORE, True),
            ("restore round", RESTORE, next_round, False),
            ("during reconvergence", next_round, last_change, False),
            ("after convergence", max(last_change, next_round), end, True),
        )
    for label, start, end, require_all in windows:
        if start >= end:
            continue
        sent = {key for key, (time, _, _) in sends.items() if start <= time < end}
        delivered = len(sent & receives.keys())
        assert sent, (config, label)
        if require_all:
            assert delivered == len(sent), (config, label, delivered, len(sent))
        print(f"{config} probes {label}: {delivered}/{len(sent)}")
    print(f"{config} probes total: {len(receives)}/{len(sends)}, drops={dict(drops)}")


def check_logs(log_dir):
    recovery_times = {}
    for config in ("K1", "K2", "K3", "K4", "K5", "K6", "K7_STP", "K7_RSTP"):
        k7 = config.startswith("K7")
        log = (log_dir / config / "cmdenv.log").read_text(encoding="utf-8")
        assert f"Simulation time limit reached -- at t={END}s," in log, config
        snapshot = {}
        last_change = Decimal(0)
        down = set()
        link_events = set()
        sends, receives = {}, {}
        drops = Counter()
        ready = {}
        for line in log.splitlines():
            event, _, payload = line.split("\t")[-1].partition(" ")
            if event in {"STP_LINK_DOWN", "STP_LINK_UP"}:
                fields = dict(field.split("=") for field in payload.split())
                pair = (fields["bridge"], fields["peer"])
                time = Decimal(fields["t"])
                port = int(fields["port"])
                link_events.add((event, time, pair, port))
                if event == "STP_LINK_DOWN":
                    down.add(pair)
                else:
                    down.remove(pair)
            if event in {"PROBE_SEND", "PROBE_RECV", "PROBE_DROP"}:
                fields = dict(field.split("=") for field in payload.split())
                if event == "PROBE_DROP":
                    drops[fields["reason"]] += 1
                else:
                    records = sends if event == "PROBE_SEND" else receives
                    key = int(fields["src"]), int(fields["seq"])
                    assert key not in records, (config, event, key)
                    records[key] = (Decimal(fields["t"]), int(fields["src"]), int(fields["dst"]))
            if "STP_STATE " not in line:
                continue
            fields = dict(field.split("=") for field in line.split("STP_STATE ")[1].split()
                          if not field.startswith("port="))
            time = Decimal(fields.pop("t"))
            if config == "K3" and last_change < FAILURE <= time:
                verify("K3 before", snapshot, last_change, Decimal(0), FAILURE,
                       0, 32768, BASE, BASE_BLOCKED, set())
            if k7 and last_change < RESTORE <= time:
                verify(config + " before", snapshot, last_change, Decimal(0), RESTORE,
                       0, 32768, BEFORE_RESTORE, {("b3", "b4")}, RESTORED)
            last_change = time
            bridge = fields.pop("bridge")
            ports = []
            for field in line.split():
                if field.startswith("port="):
                    index, peer, role, state = field[5:].split(":")
                    ports.append((int(index), peer, role, state))
                    pair = (bridge, peer)
                    if pair in down:
                        assert state == "down", (config, time, bridge, field)
                    if k7 and pair in RESTORED and state == "forwarding":
                        assert time > RESTORE, (config, time, pair)
                        ready.setdefault(pair, time)
            snapshot[bridge] = ({key: int(value) for key, value in fields.items()}, ports)

        expected_events = set()
        if config == "K3":
            expected_events = {("STP_LINK_DOWN", FAILURE, pair, 0) for pair in FAILED}
        if k7:
            expected_events = {("STP_LINK_DOWN", Decimal(0), pair, 1) for pair in RESTORED}
            expected_events |= {("STP_LINK_UP", RESTORE, pair, 1) for pair in RESTORED}
        assert link_events == expected_events, (config, link_events)
        if config == "K2":
            verify(config, snapshot, last_change, Decimal(0), END,
                   3, 4096, ROOT_B3, {("b1", "b0"), ("b1", "b4")}, set())
        elif config == "K3":
            verify("K3 after", snapshot, last_change, FAILURE, END,
                   0, 32768, VIA_B4, {("b2", "b3")}, FAILED)
        elif config == "K4":
            verify(config, snapshot, last_change, Decimal(0), END,
                   0, 32768, VIA_B4, {("b1", "b0"), ("b2", "b3")}, set())
        elif config == "K6":
            verify(config, snapshot, last_change, Decimal(0), END,
                   0, 32768, BASE + (("b4", 2), ("b5", 3), ("b4", 2)),
                   BASE_BLOCKED | {("b6", "b7")}, set())
        elif k7:
            verify(config + " after", snapshot, last_change, RESTORE, END,
                   0, 32768, BASE, BASE_BLOCKED, set())
            assert set(ready) == RESTORED, (config, ready)
            print(f"{config} restored link forwarding at {max(ready.values())}s")
            recovery_times[config] = last_change - RESTORE
        else:
            blocked = BASE_BLOCKED if config == "K1" else {("b3", "b2")}
            verify(config, snapshot, last_change, Decimal(0), END,
                   0, 32768, BASE, blocked, set())
        if k7 or sends or receives or drops:
            verify_probes(config, sends, receives, drops, last_change)

    assert recovery_times["K7_RSTP"] < recovery_times["K7_STP"], recovery_times


if __name__ == "__main__":
    check_logs(Path(sys.argv[1]))
