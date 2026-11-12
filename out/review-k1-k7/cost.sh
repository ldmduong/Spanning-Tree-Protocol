#!/usr/bin/env bash
set -euo pipefail
repo=$PWD
logs="$repo/out/review-k1-k7"
work=$(sed -n 's/^Build workspace: //p' "$logs/toolchain.log")
cd "$work/simulations"
set -x
for bridge in b0 b1; do
    log_dir="$logs/cost-${bridge}-out"
    mkdir -p "$log_dir"
    ../src/stp_dbg -u Cmdenv -n .:../src -c K1 \
        --cmdenv-express-mode=false --result-dir="$log_dir" "--**.$bridge.**.link01.pathCost=3" \
        > "$log_dir/cmdenv.log" 2>&1
done
