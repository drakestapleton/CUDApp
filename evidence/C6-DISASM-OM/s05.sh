#!/bin/sh
# t_81083ab1 step 05: the test-sm121dis differential (every corpus word, .om vs C), its red check, then the
# flipped-bit control. Same commands as mk/sm121dis.mk test-sm121dis.
set -u
K=build/sm121dis-kernels
ulimit -c 0
n=0
for f in "$K"/*.bin; do
    n=$((n + 1)); rc=0
    build/sm121dis_diff --pairs "$f" > /dev/null 2> build/sm121dis-diff.err || rc=$?
    if [ $rc != 0 ]; then echo "FAIL .om vs C decoder differential on $f (exit $rc)"; head -3 build/sm121dis-diff.err; exit 1; fi
done
echo "decoder differential: .om and migration-pending C decoders agree on every word of $n kernels"
rc=0
build/sm121dis_diff_red --pairs "$(ls "$K"/*.bin | head -1)" > /dev/null 2> build/sm121dis-diff-red.err || rc=$?
if [ $rc -le 128 ] || ! grep -q '^DISASM DIFFER' build/sm121dis-diff-red.err; then
    echo "RED CHECK BROKEN: the differential did not abort on a flipped bit (exit $rc)"; exit 1; fi
echo "red check: one flipped bit aborts the decoder differential, as it must (exit $rc)"
head -1 build/sm121dis-diff-red.err
time sh tools/sm121dis/negctl.sh build/sm121dis "$K" build/sm121dis-work
