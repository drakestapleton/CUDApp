#!/bin/sh
# t_81083ab1 step 04: the test-sm121dis red check: the C oracle with the planted fault must FAIL check.sh
set -u
K=build/sm121dis-kernels
if sh tools/sm121dis/check.sh build/sm121dis_red "$K" build/sm121dis-red > build/sm121dis-red.log 2>&1; then
    echo "RED CHECK BROKEN: the faulty decoder passed"; exit 1
elif ! grep -q '^VERDICT FAIL' build/sm121dis-red.log; then
    echo "RED CHECK BROKEN: the faulty decoder's run did not reach a verdict"; tail -3 build/sm121dis-red.log; exit 1
fi
echo "red check: the faulty decoder FAILS the comparison, as it must ($(grep -c '^\[FAIL\]' build/sm121dis-red.log) kernels)"
sed -n '/^kernels:/,$p' build/sm121dis-red.log
