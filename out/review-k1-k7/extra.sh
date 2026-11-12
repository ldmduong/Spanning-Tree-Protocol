#!/usr/bin/env bash
set -euo pipefail
repo=$PWD
logs="$repo/out/review-k1-k7"
work=$(sed -n 's/^Build workspace: //p' "$logs/toolchain.log")
cd "$work/simulations"
set -x
run() {
    local config=$1 label=$2
    shift 2
    local log_dir="$logs/$label"
    mkdir -p "$log_dir"
    ../src/stp_dbg -u Cmdenv -n .:../src -c "$config" \
        --cmdenv-express-mode=false --result-dir="$log_dir" "$@" \
        > "$log_dir/cmdenv.log" 2>&1
}

run K4 cost-b1-first '--*.b1.ethg$o[0].link01.pathCost=3' '--**.link01.pathCost=1'
run K4 cost-b0-first '--*.b0.ethg$o[0].link01.pathCost=3' '--**.link01.pathCost=1'
run K1 restore-in-flight-probe '--**.link01.failAt=10.002s' '--**.link01.restoreAt=10.004s' '--**.probeInterval=0.1s' '--**.probeEnd=10s' --record-eventlog=true
run K7_STP phase-STP '--**.link04.restoreAt=12.5s'
run K7_RSTP phase-RSTP '--**.link04.restoreAt=12.5s'
