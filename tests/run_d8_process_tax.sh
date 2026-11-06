#!/bin/bash
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# tests/run_d8_process_tax.sh -- developer bar D8, no per-process CUDA tax (docs/issues/D8-process-tax.md).
#
#   tests/run_d8_process_tax.sh --dry-run [OUTDIR]   host only: no GPU, no lease. Runs in bwrap with the GPU
#                                                    devices hidden (it re-executes itself there). Default OUTDIR
#                                                    build/d8-process-tax-dry-run.
#   tests/run_d8_process_tax.sh --gpu [OUTDIR]       GB10: builds, takes ONE spark-window lease (gb10 exclusive,
#                                                    20 minutes, ledger DRA-158), runs the three measurements below
#                                                    inside it, releases the lease on every exit path, then judges.
#                                                    Default OUTDIR build/d8-process-tax/<UTC stamp>.
#   tests/run_d8_process_tax.sh --check-gpu OUTDIR   judges a recorded --gpu run again; opens no device.
#
# What is measured. hello_gpu prints its own overhead line (examples/hello_gpu/hello_gpu.c, "D8"): resident memory
# (VmRSS) before the GPU opens, after it opens and with its buffers allocated and uploaded, its own buffer bytes, and
# what the session asked the driver for. With --hold-ms it then prints its pid and holds its buffers; during the hold
# this script reads the same process from outside: VmRSS from /proc/<pid>/status and, on the GPU, the memory the
# driver reports for the pid (nvidia-smi --query-compute-apps, a read-only oracle). No other program is substituted:
# the measured process is build/hello_gpu itself.
#
# GPU runs, all in the one lease, each a separate process:
#   main        build/hello_gpu --hold-ms=3000                                  must PASS the bar
#   n1-short    build/hello_gpu_control --bug=short --hold-ms=3000              negative control: must FAIL B1
#   n2-ballast  build/hello_gpu_control --d8-ballast-mib=512 --hold-ms=3000     negative control: the open cost must
#               show the 512 MiB of touched host memory held across the open, and the run must FAIL B5
# The D8 verdict is PASS only when main passes AND both controls are caught. --bug=past-end never runs on the chip.
#
# Dry run (host simulator, hello_gpu --sim, which prints the same line without the open): the same three runs, a
# device-hidden run of the GPU path that must stop at NOT_RUN, and a self-test of the GPU judge on synthetic logs
# (labelled synthetic; they are not measurements). The dry run says nothing about the chip: D8 stays NOT_RUN there
# until --gpu runs in a window.
#
# Exit: 0 PASS, 1 FAIL, 2 NOT_RUN.
set -u
ROOT=$(cd "$(dirname "$0")/.." && pwd)
SELF="$ROOT/tests/run_d8_process_tax.sh"
CORES=${D8_CORES-16,17} # spark cores for the hive; D8_CORES= (empty) runs unpinned

# ---- The bar, registered before any chip run (2026-11-06, on CUDApp#280 commit 074adab1) ---------------------------
# B1 run      hello_gpu exits 0 and prints "checked 1048576 of 1048576 elements: 0 wrong, 0 never written" and PASS.
# B2 line     exactly one well-formed D8 line; its open cost and "beyond own buffers" equal the differences of the
#             numbers it prints; own buffers 12288 KiB (sim) or 12288..12352 KiB (a, b, c plus the code buffer).
# B3 outside  the hold line names the pid this script started, and VmRSS read from outside during the hold is within
#             RSS_TOL_KIB of the line's "with buffers" number.
# B4 driver   (GPU) the driver's per-process report lists the pid with a whole number of MiB. Missing is FAIL.
# B5 tax      TAX = max(0, open cost, resident beyond own buffers) + max(0, driver MiB x 1024 - own buffers KiB)
#             (GPU) or max(0, resident beyond own buffers) (sim), and TAX <= BAR_TAX_KIB.
BAR_TAX_KIB=262144 # 256 MiB. WolframLanguageEvaluator: 256*2^10 -> 262144. The pain D8 answers is "4.6 GB
                   # approximately per each model": N[4.6*10^9/2^20] -> 4386.90 MiB, N[4.6*10^9/(256*2^20)] -> 17.136,
                   # so the bar is 17.1 times below the tax it answers.
RSS_TOL_KIB=4096   # outside VmRSS against the line's own "with buffers" reading (the stdio buffer, the hold line)
BALLAST_MIB=512    # negative control; WolframLanguageEvaluator: 512*2^10 -> 524288 KiB the measurement must see
BALLAST_KIB=524288
HOLD_MS=3000
SIM_HOLD_MS=1000

RE_GPU='^overhead \(D8\): resident ([0-9]+) KiB before open, ([0-9]+) KiB after open, ([0-9]+) KiB with buffers; own buffers ([0-9]+) KiB; session driver allocations ([0-9]+) KiB in ([0-9]+); open cost ([+-][0-9]+) KiB, beyond own buffers ([+-][0-9]+) KiB$'
RE_SIM='^overhead \(D8, simulator, no device opened\): resident ([0-9]+) KiB before buffers, ([0-9]+) KiB with buffers; own buffers ([0-9]+) KiB; beyond own buffers ([+-][0-9]+) KiB$'

die_not_run() { echo "NOT_RUN: $*"; exit 2; }
pin() { if [ -n "$CORES" ]; then echo nice -n 10 taskset -c "$CORES"; else echo nice -n 10; fi; }
build() { # the two binaries, from this checkout, with the GPU devices hidden
    local hide=()
    if [ -z "${D8_DEVICE_HIDDEN:-}" ] && command -v bwrap > /dev/null; then hide=(bwrap --dev-bind / / --dev /dev --proc /proc --); fi
    (cd "$ROOT" && "${hide[@]}" ${MAKE:-make} -s build/hello_gpu build/hello_gpu_control) > "$OUT/build.log" 2>&1 ||
        { echo "FAIL: build (see $OUT/build.log)"; tail -n 20 "$OUT/build.log"; exit 1; }
}

# measure NAME KIND(gpu|sim) CMD... : runs CMD, reads it from outside during its hold. Writes NAME.log (headed by
# the command and UTC start, followed by the exit code and UTC end) and NAME.outside (pid, VmRSS, driver MiB).
measure() {
    local name=$1 kind=$2; shift 2
    local log="$OUT/$name.log" outside="$OUT/$name.outside" pid rc i vm drv
    { echo "# command: $*"; echo "# utc start: $(date -u +%Y-%m-%dT%H:%M:%S.%NZ)"; } > "$log"
    "$@" >> "$log" 2>&1 &
    pid=$!
    for i in $(seq 1 600); do
        grep -q '^hold (D8): pid ' "$log" && break
        kill -0 "$pid" 2> /dev/null || break
        sleep 0.1
    done
    vm=missing; drv=missing
    if grep -q '^hold (D8): pid ' "$log" && kill -0 "$pid" 2> /dev/null; then
        vm=$(sed -n 's/^VmRSS:[[:space:]]*\([0-9]*\) kB$/\1/p' "/proc/$pid/status" 2> /dev/null)
        [ -n "$vm" ] || vm=missing
        if [ "$kind" = gpu ]; then
            for i in 1 2 3; do
                drv=$(nvidia-smi --query-compute-apps=pid,used_memory --format=csv,noheader,nounits 2> /dev/null |
                    awk -F', *' -v p="$pid" '$1 == p { print $2; exit }')
                [ -n "$drv" ] && break
                drv=missing; sleep 0.3
            done
        fi
    fi
    wait "$pid"; rc=$?
    { echo "# exit: $rc"; echo "# utc end: $(date -u +%Y-%m-%dT%H:%M:%S.%NZ)"; } >> "$log"
    printf 'pid=%s\nvmrss_kib=%s\ndriver_mib=%s\n' "$pid" "$vm" "$drv" > "$outside"
    echo "ran $name: exit $rc, outside VmRSS ${vm} KiB$([ "$kind" = gpu ] && echo ", driver ${drv} MiB")"
}

# judge KIND(gpu|sim) LOG OUTSIDE : prints one row per bar; sets J_VERDICT (PASS|FAIL|NOT_RUN), J_FAILED (failed bar
# names, space separated), J_OPEN, J_BEYOND, J_TAX (KiB).
row() { printf '  %-3s %-74s %s\n' "$1" "$2" "$3"; [ "$3" = PASS ] || J_FAILED="$J_FAILED $1"; }
judge() {
    local kind=$1 log=$2 outside=$3 rc line n total before open_kib with own drv_alloc open beyond o_pid vm drv hold_pid
    J_FAILED=""; J_OPEN=""; J_BEYOND=""; J_TAX=""
    rc=$(sed -n 's/^# exit: \([0-9]*\)$/\1/p' "$log" | tail -n 1)
    if [ "$rc" = 2 ] || grep -qx 'NOT_RUN' "$log"; then
        echo "  run exited ${rc:-?} without a device (NOT_RUN): $(grep -m1 -E 'not available|NOT_RUN' "$log")"
        J_VERDICT=NOT_RUN; return
    fi
    if [ "$rc" = 0 ] && grep -qx 'checked 1048576 of 1048576 elements: 0 wrong, 0 never written' "$log" &&
        grep -qx PASS "$log" && ! grep -qx FAIL "$log"; then row B1 "run: exit 0, 1048576 of 1048576 right, PASS" PASS
    else row B1 "run: exit ${rc:-?}, $(grep -m1 '^checked ' "$log" || echo 'no check line')" FAIL; fi

    local re; [ "$kind" = gpu ] && re=$RE_GPU || re=$RE_SIM
    n=$(grep -cE "$re" "$log"); total=$(grep -c '^overhead (D8' "$log")
    if [ "$n" -ne 1 ] || [ "$total" -ne 1 ]; then
        row B2 "line: $total D8 lines, $n well-formed (exactly one required)" FAIL
        row B3 "outside: not judged without the line" FAIL; [ "$kind" = gpu ] && row B4 "driver: not judged without the line" FAIL
        row B5 "tax: not judged without the line" FAIL; J_VERDICT=FAIL; return
    fi
    line=$(grep -E "$re" "$log"); [[ $line =~ $re ]]
    if [ "$kind" = gpu ]; then
        before=${BASH_REMATCH[1]}; open_kib=${BASH_REMATCH[2]}; with=${BASH_REMATCH[3]}; own=${BASH_REMATCH[4]}
        drv_alloc=${BASH_REMATCH[5]}; open=$((BASH_REMATCH[7])); beyond=$((BASH_REMATCH[8]))
    else
        before=${BASH_REMATCH[1]}; open_kib=$before; with=${BASH_REMATCH[2]}; own=${BASH_REMATCH[3]}
        drv_alloc=0; open=0; beyond=$((BASH_REMATCH[4]))
    fi
    local own_ok=0
    if [ "$kind" = gpu ]; then [ "$own" -ge 12288 ] && [ "$own" -le 12352 ] && own_ok=1; else [ "$own" -eq 12288 ] && own_ok=1; fi
    if [ "$before" -gt 0 ] && [ "$with" -gt 0 ] && [ $own_ok = 1 ] && [ "$open" -eq $((open_kib - before)) ] &&
        [ "$beyond" -eq $((with - before - own)) ]; then
        row B2 "line: resident $before/$open_kib/$with KiB, own $own KiB, open $open, beyond own $beyond KiB (sums agree)" PASS
    else row B2 "line: printed open $open / beyond $beyond KiB disagree with its numbers, or own $own KiB is off" FAIL; fi
    J_OPEN=$open; J_BEYOND=$beyond

    o_pid=$(sed -n 's/^pid=//p' "$outside"); vm=$(sed -n 's/^vmrss_kib=//p' "$outside"); drv=$(sed -n 's/^driver_mib=//p' "$outside")
    hold_pid=$(sed -n 's/^hold (D8): pid \([0-9]*\),.*/\1/p' "$log")
    if [ -n "$hold_pid" ] && [ "$hold_pid" = "$o_pid" ] && [[ $vm =~ ^[0-9]+$ ]] && [ $((vm > with ? vm - with : with - vm)) -le $RSS_TOL_KIB ]; then
        row B3 "outside: pid $o_pid VmRSS $vm KiB during the hold, line says $with KiB (tolerance $RSS_TOL_KIB)" PASS
    else row B3 "outside: hold pid '${hold_pid}', started pid '${o_pid}', VmRSS '${vm}' KiB vs line $with KiB" FAIL; fi

    local tax rss_part drv_part=0
    rss_part=$beyond; [ "$open" -gt "$rss_part" ] && rss_part=$open; [ "$rss_part" -lt 0 ] && rss_part=0
    if [ "$kind" = gpu ]; then
        if [[ $drv =~ ^[0-9]+$ ]]; then
            row B4 "driver: pid $o_pid holds $drv MiB by the driver's report (session asked for $drv_alloc KiB)" PASS
            drv_part=$((drv * 1024 - own)); [ "$drv_part" -lt 0 ] && drv_part=0
        else row B4 "driver: no per-process report for pid $o_pid ('${drv}')" FAIL; fi
    fi
    tax=$((rss_part + drv_part)); J_TAX=$tax
    if [ "$kind" = gpu ] && ! [[ $drv =~ ^[0-9]+$ ]]; then
        row B5 "tax: not judged without the driver's report (resident part $rss_part KiB)" FAIL
    elif [ "$tax" -le $BAR_TAX_KIB ]; then
        row B5 "tax: $tax KiB = resident $rss_part + driver beyond own buffers $drv_part; bar <= $BAR_TAX_KIB KiB" PASS
    else row B5 "tax: $tax KiB = resident $rss_part + driver beyond own buffers $drv_part; bar <= $BAR_TAX_KIB KiB" FAIL; fi
    if [ -z "$J_FAILED" ]; then J_VERDICT=PASS; else J_VERDICT=FAIL; fi
}

# expect NAME KIND WANT(PASS|FAIL|NOT_RUN) [BAR ...] : judges NAME and records whether the expectation held. For a
# FAIL expectation, every BAR named must be among the failed bars.
EXPECT_BAD=0
expect() {
    local name=$1 kind=$2 want=$3; shift 3
    echo "$name ($kind), expected $want${*:+ by $*}:"
    judge "$kind" "$OUT/$name.log" "$OUT/$name.outside"
    local ok=1 b
    [ "$J_VERDICT" = "$want" ] || ok=0
    for b in "$@"; do [[ " $J_FAILED " == *" $b "* ]] || ok=0; done
    if [ $ok = 1 ]; then echo "  -> $J_VERDICT, as expected"; else echo "  -> $J_VERDICT${J_FAILED:+ (failed:$J_FAILED)}: NOT the expected $want${*:+ by $*}"; EXPECT_BAD=$((EXPECT_BAD + 1)); fi
}

check_gpu() { # judge a recorded --gpu run
    [ -f "$OUT/main.log" ] || die_not_run "no main.log in $OUT: the GPU run did not happen"
    EXPECT_BAD=0
    echo "main (gpu), the bar:"; judge gpu "$OUT/main.log" "$OUT/main.outside"; local main=$J_VERDICT tax=$J_TAX
    echo "  -> $main"
    [ "$main" = NOT_RUN ] && { echo "D8 PROCESS TAX: NOT_RUN (the GPU was not available inside the window)"; exit 2; }
    expect n1-short gpu FAIL B1
    expect n2-ballast gpu FAIL B5
    if [ -n "$J_OPEN" ] && [ "$J_OPEN" -ge $BALLAST_KIB ]; then echo "  ballast seen: open cost +$J_OPEN KiB >= $BALLAST_KIB KiB"
    else echo "  ballast NOT seen: open cost '${J_OPEN}' KiB < $BALLAST_KIB KiB, the measurement is blind"; EXPECT_BAD=$((EXPECT_BAD + 1)); fi
    if [ "$main" = PASS ] && [ $EXPECT_BAD -eq 0 ]; then echo "D8 PROCESS TAX: PASS (tax $tax KiB <= $BAR_TAX_KIB KiB; both negative controls caught)"; exit 0; fi
    echo "D8 PROCESS TAX: FAIL (main $main${tax:+, tax $tax KiB}; $EXPECT_BAD control expectation(s) not met)"; exit 1
}

synthetic() { # NAME LINE DRIVER_MIB [EXIT] : a synthetic GPU log for the judge's self-test
    local name=$1 line=$2 drv=$3 rc=${4:-0}
    {
        echo "# command: synthetic judge self-test (not a measurement)"
        echo "hello_gpu: c = a + b over 1048576 floats on the GPU, built with Omega's own kernel builder"
        [ -n "$line" ] && printf '%s\n' "$line"
        echo "hold (D8): pid 4242, 3000 ms with buffers allocated, for an outside reading of this process"
        echo "checked 1048576 of 1048576 elements: 0 wrong, 0 never written"; echo PASS; echo "# exit: $rc"
    } > "$OUT/$name.log"
    printf 'pid=4242\nvmrss_kib=15310\ndriver_mib=%s\n' "$drv" > "$OUT/$name.outside"
}

dry_run() {
    if [ -z "${D8_DEVICE_HIDDEN:-}" ]; then
        command -v bwrap > /dev/null || die_not_run "bwrap is needed to hide the GPU devices for the dry run"
        D8_DEVICE_HIDDEN=1 exec $(pin) bwrap --dev-bind / / --dev /dev --proc /proc -- bash "$SELF" --dry-run "$OUT"
    fi
    if compgen -G '/dev/nvidia*' > /dev/null; then die_not_run "GPU device nodes are visible; the dry run must not see them"; fi
    mkdir -p "$OUT"; rm -f "$OUT"/*.log "$OUT"/*.outside
    build
    local H="$ROOT/build/hello_gpu" HC="$ROOT/build/hello_gpu_control"
    measure sim-main sim "$H" --sim --hold-ms=$SIM_HOLD_MS
    measure sim-short sim "$HC" --sim --bug=short --hold-ms=$SIM_HOLD_MS
    measure sim-ballast sim "$HC" --sim --d8-ballast-mib=$BALLAST_MIB --hold-ms=$SIM_HOLD_MS
    measure hidden-gpu gpu "$H" --hold-ms=$SIM_HOLD_MS
    EXPECT_BAD=0
    expect sim-main sim PASS
    expect sim-short sim FAIL B1
    expect sim-ballast sim FAIL B5
    if [ -n "$J_BEYOND" ] && [ "$J_BEYOND" -ge $BALLAST_KIB ]; then echo "  ballast seen: beyond own buffers +$J_BEYOND KiB >= $BALLAST_KIB KiB"
    else echo "  ballast NOT seen: beyond own buffers '${J_BEYOND}' KiB"; EXPECT_BAD=$((EXPECT_BAD + 1)); fi
    expect hidden-gpu gpu NOT_RUN

    echo "GPU judge self-test on synthetic logs (not measurements):"
    local good='overhead (D8): resident 2000 KiB before open, 3000 KiB after open, 15300 KiB with buffers; own buffers 12289 KiB; session driver allocations 12320 KiB in 4; open cost +1000 KiB, beyond own buffers +1011 KiB'
    synthetic syn-good "$good" 40
    synthetic syn-no-driver "$good" missing
    synthetic syn-driver-tax "$good" 5000
    synthetic syn-bad-sum "${good/beyond own buffers +1011/beyond own buffers +11}" 40
    synthetic syn-two-lines "$good"$'\n'"$good" 40
    synthetic syn-no-line "" 40
    synthetic syn-exit-1 "$good" 40 1
    synthetic syn-no-device "" missing 2
    expect syn-good gpu PASS
    expect syn-no-driver gpu FAIL B4 B5
    expect syn-driver-tax gpu FAIL B5
    expect syn-bad-sum gpu FAIL B2
    expect syn-two-lines gpu FAIL B2
    expect syn-no-line gpu FAIL B2
    expect syn-exit-1 gpu FAIL B1
    expect syn-no-device gpu NOT_RUN

    echo "GPU verdict self-test on a synthetic recorded run (not a measurement):"
    local V="$OUT/syn-window" vrc
    rm -rf "$V"; mkdir -p "$V"
    cp "$OUT/syn-good.log" "$V/main.log"; cp "$OUT/syn-good.outside" "$V/main.outside"
    cp "$OUT/syn-good.outside" "$V/n1-short.outside"
    sed -e 's/^checked .*/checked 1048576 of 1048576 elements: 1 wrong, 1 never written/' -e 's/^PASS$/FAIL/' \
        -e 's/^# exit: 0$/# exit: 1/' "$OUT/syn-good.log" > "$V/n1-short.log"
    sed -e 's/3000 KiB after open, 15300 KiB with buffers/527300 KiB after open, 539600 KiB with buffers/' \
        -e 's/open cost +1000 KiB, beyond own buffers +1011 KiB/open cost +525300 KiB, beyond own buffers +525311 KiB/' \
        "$OUT/syn-good.log" > "$V/n2-ballast.log"
    printf 'pid=4242\nvmrss_kib=539610\ndriver_mib=40\n' > "$V/n2-ballast.outside"
    bash "$SELF" --check-gpu "$V" > "$V/verdict.txt"; vrc=$?
    if [ $vrc -eq 0 ]; then echo "  good run with both controls caught -> exit 0, as expected"
    else echo "  good run with both controls caught -> exit $vrc, NOT the expected 0"; EXPECT_BAD=$((EXPECT_BAD + 1)); fi
    cp "$OUT/syn-good.log" "$V/n2-ballast.log"; cp "$OUT/syn-good.outside" "$V/n2-ballast.outside"
    bash "$SELF" --check-gpu "$V" > "$V/verdict-blind.txt"; vrc=$?
    if [ $vrc -eq 1 ]; then echo "  ballast control not seen (a blind measurement) -> exit 1, as expected"
    else echo "  ballast control not seen (a blind measurement) -> exit $vrc, NOT the expected 1"; EXPECT_BAD=$((EXPECT_BAD + 1)); fi
    if [ $EXPECT_BAD -eq 0 ]; then
        echo "D8 DRY RUN: PASS (host simulator, device-hidden refusal, judge and verdict self-tests; the chip row stays NOT_RUN until --gpu runs in a window)"
        exit 0
    fi
    echo "D8 DRY RUN: FAIL ($EXPECT_BAD expectation(s) not met)"; exit 1
}

inside_window() { # runs under `spark-window run`, which holds the chip lock
    [ -n "${SPARK_WINDOW:-}" ] && [ "${AIEN_GB10_LOCK_HELD:-}" = 1 ] || die_not_run "not inside an exclusive spark-window lease"
    measure main gpu "$ROOT/build/hello_gpu" --hold-ms=$HOLD_MS
    measure n1-short gpu "$ROOT/build/hello_gpu_control" --bug=short --hold-ms=$HOLD_MS
    measure n2-ballast gpu "$ROOT/build/hello_gpu_control" --d8-ballast-mib=$BALLAST_MIB --hold-ms=$HOLD_MS
}

gpu() {
    [ -z "${D8_DEVICE_HIDDEN:-}" ] && [ -e /dev/nvidiactl ] || die_not_run "no /dev/nvidiactl here (device-hidden sandbox?)"
    command -v spark-window > /dev/null || die_not_run "spark-window is not installed: no lease, no run"
    command -v nvidia-smi > /dev/null || die_not_run "nvidia-smi is not installed: B4 cannot be read"
    mkdir -p "$OUT"
    build
    {
        echo "git HEAD: $(git -C "$ROOT" rev-parse HEAD)"
        echo "git status (outside evidence/): $(git -C "$ROOT" status --porcelain -- . ':!evidence' | wc -l) changed files"
        echo "hello_gpu sha256: $(sha256sum "$ROOT/build/hello_gpu" | cut -d' ' -f1)"
        echo "hello_gpu_control sha256: $(sha256sum "$ROOT/build/hello_gpu_control" | cut -d' ' -f1)"
        echo "driver: $(nvidia-smi --query-gpu=name,driver_version --format=csv,noheader 2>&1)"
        echo "kernel: $(uname -r)"
        echo "cores: ${CORES:-unpinned}"
        echo "bar: tax <= $BAR_TAX_KIB KiB, outside VmRSS tolerance $RSS_TOL_KIB KiB, ballast $BALLAST_MIB MiB, hold $HOLD_MS ms"
    } > "$OUT/env.txt"
    local wid="" rc
    close_lease() { if [ -n "$wid" ]; then spark-window close "$wid" >> "$OUT/lease.txt" 2>&1; echo "closed $wid $(date -u +%FT%TZ)" >> "$OUT/lease.txt"; wid=""; fi; }
    trap close_lease EXIT
    trap 'exit 130' INT TERM HUP
    echo "open-request $(date -u +%FT%TZ)" > "$OUT/lease.txt"
    spark-window open d8-process-tax --ledger DRA-158 --lease 20 --gb10 exclusive --quiet none --wait >> "$OUT/lease.txt" 2>&1
    wid=$(spark-window id 2> /dev/null) || { wid=""; cat "$OUT/lease.txt"; die_not_run "no spark-window lease"; }
    echo "lease $wid start $(date -u +%FT%TZ)" >> "$OUT/lease.txt"
    timeout 1080 spark-window run $(pin) bash "$SELF" --inside-window "$OUT" > "$OUT/window.log" 2>&1; rc=$?
    echo "window run rc=$rc end $(date -u +%FT%TZ)" >> "$OUT/lease.txt"
    close_lease
    cat "$OUT/window.log"
    bash "$SELF" --check-gpu "$OUT" | tee "$OUT/verdict.txt"
    exit "${PIPESTATUS[0]}"
}

MODE=${1:-}
case "$MODE" in
    --dry-run) OUT=${2:-$ROOT/build/d8-process-tax-dry-run}; dry_run ;;
    --gpu) OUT=${2:-$ROOT/build/d8-process-tax/$(date -u +%Y%m%dT%H%M%SZ)}; gpu ;;
    --check-gpu) OUT=${2:?usage: run_d8_process_tax.sh --check-gpu OUTDIR}; check_gpu ;;
    --inside-window) OUT=${2:?OUTDIR}; inside_window ;;
    *) echo "usage: tests/run_d8_process_tax.sh --dry-run [OUTDIR] | --gpu [OUTDIR] | --check-gpu OUTDIR" >&2; exit 2 ;;
esac
