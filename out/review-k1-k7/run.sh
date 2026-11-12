#!/usr/bin/env bash
set -euo pipefail

repo=$PWD
logs="$repo/out/review-k1-k7"
work=$(mktemp -d "${TMPDIR:-/tmp}/stp-review-k1-k7.XXXXXXXX")
printf 'Build workspace: %s\n' "$work"
mkdir "$work/src" "$work/simulations"
cp Makefile "$work/"
cp src/Makefile src/*.ned src/*.msg src/StpBridge.cc "$work/src/"
cp simulations/*.ned simulations/omnetpp.ini "$work/simulations/"

uname -sr
cat /etc/os-release
make --version
g++ --version
python3 --version
opp_run -v
cd "$work"
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

for config in K1 K2 K3 K4 K5 K6; do
    run "$config" "$config"
done
for config in K7_STP K7_RSTP; do
    run "$config" "$config" --record-eventlog=true
done
python3 "$repo/simulations/check_stp.py" "$logs" | tee "$logs/summary.txt"

for config in K1 K2 K3 K4 K5 K6; do
    run "$config" "probe-$config" '--**.probeInterval=0.1s' --record-eventlog=true
done
run K3 in-flight '--**.link01.failAt=10.01s'
run K1 expiry '--**.maxAge=0.5s'
run K1 restore-in-flight '--**.link01.failAt=10.002s' '--**.link01.restoreAt=10.004s'
run K4 cost-b1-only '--**.link01.pathCost=1' '--*.b1.ethg$o[0].link01.pathCost=3'
run K4 cost-b0-only '--**.link01.pathCost=1' '--*.b0.ethg$o[0].link01.pathCost=3'
run K3 isolate-root '--**.link04.failAt=10.5s' --sim-time-limit=45s
