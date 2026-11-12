#!/usr/bin/env bash
set -euo pipefail

repo=$PWD
logs="$repo/out/fix-k1-k7"
set -x
make MODE=debug 2>&1 | tee "$logs/build.log"
cd simulations

run() {
    local config=$1 label=$2
    shift 2
    local log_dir="$logs/$label"
    mkdir -p "$log_dir"
    ../src/stp_dbg -u Cmdenv -n .:../src -c "$config" \
        --cmdenv-express-mode=false --result-dir="$log_dir" "$@" \
        > "$log_dir/cmdenv.log" 2>&1
}

for config in K1 K2 K3 K4 K5 K6 K7_STP K7_RSTP; do
    run "$config" "$config"
done
python3 check_stp.py "$logs" | tee "$logs/summary.txt"
python3 check_stp.py "$repo/out/review-k1-k7" > "$logs/archived-summary.txt"
run K1 restore-in-flight '--**.link01.failAt=10.002s' '--**.link01.restoreAt=10.004s' '--**.probeEnd=10s'
run K3 in-flight '--**.link01.failAt=10.01s'
run General General

python3 - "$logs" <<'PY' | tee "$logs/regression.txt"
from decimal import Decimal
from pathlib import Path
import sys

root = Path(sys.argv[1])

def trace(label):
    return (root / label / "cmdenv.log").read_text().splitlines()

def states_before(lines, end):
    states = [line.split("\t")[-1] for line in lines if "\tSTP_STATE " in line]
    return [line for line in states if Decimal(line.split()[1][2:]) < end]

restored = trace("restore-in-flight")
assert states_before(restored, Decimal("10.002")) == states_before(trace("K1"), Decimal("10.002"))
for bridge, peer in (("b0", "b1"), ("b1", "b0")):
    assert any(f"STP_BPDU t=0.01 bridge={bridge} port=0 " in line for line in restored)
    assert any(f"STP_LINK_DOWN t=10.002 bridge={bridge} peer={peer} port=0" in line for line in restored)
    assert any(f"STP_LINK_UP t=10.004 bridge={bridge} peer={peer} port=0" in line for line in restored)
    assert any(f"STP_STATE t=12.01 bridge={bridge} " in line and f"port=0:{peer}:" in line
               and line.split(f"port=0:{peer}:")[1].split()[0].endswith(":forwarding") for line in restored)
    assert any(f"STP_DROP t=10.01 bridge={bridge} port=0 reason=down" in line for line in trace("in-flight"))

assert {line.split("\t")[-1] for line in restored if "reason=before-restore" in line} == {
    f"STP_DROP t=10.01 bridge={bridge} port=0 reason=before-restore" for bridge in ("b0", "b1")
}
assert sum("PROBE_SEND " in line for line in restored) == 400
assert sum("PROBE_RECV " in line for line in restored) == 400
assert not any("reason=duplicate" in line for line in restored)
print("Restore regression passed: K1 tree before failure, 400/400 probes, no duplicates, two stale BPDUs discarded after restore.")
print("Failure boundary passed: two BPDUs in flight discarded at the same time as link down.")

for label in ("K1", "K2", "K3", "K4", "K5", "K6"):
    expected = 8400 if label == "K6" else 3000
    assert sum("PROBE_SEND " in line for line in trace(label)) == expected, label
print("K1–K6 generated all expected probes without command-line overrides.")

general = trace("General")
assert any("Simulation time limit reached -- at t=10s," in line for line in general)
assert not any("PROBE_SEND " in line for line in general)
print("General smoke test passed.")
PY
