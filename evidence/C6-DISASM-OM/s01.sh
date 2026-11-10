#!/bin/sh
# t_81083ab1 step 01: binaries, the corpus/check controls, then the corpus into a fresh directory
set -u
K=${1:-build/sm121dis-kernels}
make -s OUT_DIR=build build/sm121dis build/sm121dis_red build/sm121dis_diff build/sm121dis_diff_red \
    test-sm121dis-corpus-controls test-sm121dis-check-controls || exit $?
[ -e "$K" ] && { echo "refusing: $K exists (use a fresh name)"; exit 2; }
t0=$(date +%s)
sh tools/sm121dis/corpus.sh build "$K"; rc=$?
echo "corpus.sh exit $rc in $(( $(date +%s) - t0 )) s"
exit $rc
