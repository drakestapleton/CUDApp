#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# gemm_ex_bench.sh OMEGA_BIN C_BIN OUT_DIR: the host-path cost of omega_gpu_gemm_ex on the Omega path against the C
# oracle (CUDApp#144). Each binary runs `--bench` (tests/gpu_gemm_ex_test.c: every edge and sweep case plus the fp8
# block-rows shapes, a null core, 5 timed calls per case, median/min/max ns) on one core; the lines are joined by case
# id. Prints one line per case "ratio <omega/c median> <id> <omega med> <c med> <name>" into OUT_DIR/ratio.txt, and a
# summary: cases, sum of medians each side, the ratio of the sums, the worst case, the cases above 2x. Pinning
# (taskset/nice) is the caller's. Exit 1 when the two runs disagree on the case list or a return code.
set -eu
o=$1 c=$2 d=$3
mkdir -p "$d"
"$o" --bench > "$d/omega.txt"
"$c" --bench > "$d/c.txt"
awk -v out="$d/ratio.txt" '
FNR == NR { if ($1 == "bench") { orc[$2] = $3; om[$2] = $4; omin[$2] = $5; omax[$2] = $6 } next }
$1 == "bench" {
    id = $2; n++
    if (!(id in om)) { print "FAIL gemm_ex bench: case " id " only in the C run"; bad = 1; next }
    if (orc[id] != $3) { print "FAIL gemm_ex bench: case " id " rc omega " orc[id] " c " $3; bad = 1 }
    name = $0; sub(/^bench [^ ]+ [^ ]+ [^ ]+ [^ ]+ [^ ]+ /, "", name)
    r = $4 > 0 ? om[id] / $4 : 0
    printf "ratio %.3f %s %d %d %s\n", r, id, om[id], $4, name > out
    so += om[id]; sc += $4
    if (r > worst) { worst = r; wid = id; wname = name }
    if (r > 2) over++
    seen[id] = 1
}
END {
    for (id in om) if (!(id in seen)) { print "FAIL gemm_ex bench: case " id " only in the Omega run"; bad = 1 }
    printf "gemm_ex bench: %d cases, sum of medians omega %.3f ms, c %.3f ms, ratio %.3f; worst %.3f (%s %s); %d cases above 2x\n",
        n, so / 1e6, sc / 1e6, (sc > 0 ? so / sc : 0), worst, wid, wname, over
    exit bad
}' "$d/omega.txt" "$d/c.txt"
