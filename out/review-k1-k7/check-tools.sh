#!/usr/bin/env bash
set -euo pipefail
repo=$PWD
logs="$repo/out/review-k1-k7"
work=$(sed -n 's/^Build workspace: //p' "$logs/toolchain.log")
tmp=$(mktemp -d "${TMPDIR:-/tmp}/stp-review-checker.XXXXXXXX")
for config in K1 K2 K3 K4 K5 K6 K7_STP K7_RSTP; do
    mkdir "$tmp/$config"
    if [[ $config == K7_STP ]]; then
        sed '/PROBE_RECV t=12\.11 src=0 dst=1 seq=164$/d' "$logs/$config/cmdenv.log" > "$tmp/$config/cmdenv.log"
    else
        ln -s "$logs/$config/cmdenv.log" "$tmp/$config/cmdenv.log"
    fi
done
diff -u "$logs/K7_STP/cmdenv.log" "$tmp/K7_STP/cmdenv.log" || test "$?" -eq 1
python3 "$repo/simulations/check_stp.py" "$tmp"
echo 'checker exit=0 after removing one receipt in remaining transition'
for file in Makefile src/Makefile src/*.ned src/*.msg src/StpBridge.cc simulations/*.ned simulations/omnetpp.ini; do
    cmp "$repo/$file" "$work/$file"
done
echo 'Build inputs still byte-for-byte identical'
cd "$work"
cp "$repo/simulations/run" simulations/
set +e
sh simulations/run -u Cmdenv -c K1
status=$?
echo "simulations/run exit=$status"
test "$status" -eq 127
