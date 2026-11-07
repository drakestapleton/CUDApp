# ORACLE harness: NVIDIA nvdisasm 13.0.85 is the offline reference here; nothing it prints enters a kernel.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# negctl.sh <sm121dis> <kernel-dir> <work-dir>: the flipped-bit negative control of the C6 parity check (DRA-475).
# Run after tools/sm121dis/check.sh <sm121dis> <kernel-dir> <work-dir>, whose <name>.want files (nvdisasm's normalised
# text of every kernel) it reads. For every kernel: copy it, flip bit 16 of word 0 (the lowest destination-register
# bit, or a fixed bit of forms without one), decode the copy with our disassembler and compare with nvdisasm's text of
# the ORIGINAL kernel. The control holds only when every kernel's text changes and parity breaks in exactly word 0.
# It also decodes the flipped copy with nvdisasm: "tracked" counts kernels where we still agree with the oracle on the
# flipped word, "reported" those where the flipped word is a form we do not know (printed ?? and counted, never guessed).
# Kernels run NEGCTL_JOBS at a time (default: every CPU this process may use); each writes <work-dir>/negctl/<name>.neg.
# Exit 0 when the control holds for every kernel, 1 otherwise, 2 when nvdisasm or a .want file is missing.
set -u
NVDISASM=${NVDISASM:-/usr/local/cuda/bin/nvdisasm}
HERE=$(cd "$(dirname "$0")" && pwd)

if [ "${1:-}" = --one ]; then
    # negctl.sh --one <sm121dis> <work-dir> <kernel.bin>: first line of <name>.neg: BROKE|FAIL TRACKED|REPORTED|-
    DIS=$2; WORK=$3; f=$4; F=$WORK/negctl
    b=$(basename "$f" .bin)
    out=$F/$b.neg.tmp
    if [ ! -f "$WORK/$b.want" ]; then echo "FAIL - $b: $WORK/$b.want missing" > "$out"; mv "$out" "$F/$b.neg"; exit 0; fi
    cp "$f" "$F/$b.bin"
    v=$(od -An -tu1 -j2 -N1 "$f" | tr -d ' ')
    printf "\\$(printf '%03o' $((v ^ 1)))" | dd of="$F/$b.bin" bs=1 seek=2 conv=notrunc status=none
    if cmp -s "$f" "$F/$b.bin"; then echo "FAIL - $b: the flip changed no byte" > "$out"; mv "$out" "$F/$b.neg"; exit 0; fi
    "$DIS" --pairs "$F/$b.bin" 2> /dev/null | sed 's/[[:space:]]\{1,\}/ /g' > "$F/$b.got"
    d=$(diff "$WORK/$b.want" "$F/$b.got" | grep -c '^<')
    w0=$(diff "$WORK/$b.want" "$F/$b.got" | grep '^[0-9]' | head -1)
    if [ "$d" != 1 ] || [ "$w0" != 1c1 ]; then
        echo "FAIL - $b: flipped bit broke parity in $d words ($w0), want exactly word 0" > "$out"; mv "$out" "$F/$b.neg"; exit 0
    fi
    t=REPORTED
    if "$NVDISASM" -b SM121 -hex "$F/$b.bin" > "$F/$b.nv" 2>&1; then
        sh "$HERE/oracle_pairs.sh" "$F/$b.nv" > "$F/$b.fwant"
        if cmp -s "$F/$b.fwant" "$F/$b.got"; then t=TRACKED
        elif ! head -1 "$F/$b.got" | grep -q ' ?? 0x'; then
            echo "FAIL - $b: on the flipped word we print '$(head -1 "$F/$b.got" | cut -d' ' -f4-)', nvdisasm '$(head -1 "$F/$b.fwant" | cut -d' ' -f4-)'" > "$out"
            mv "$out" "$F/$b.neg"; exit 0
        fi
    fi
    echo "BROKE $t $b" > "$out"; mv "$out" "$F/$b.neg"
    exit 0
fi

DIS=$1; DIR=$2; WORK=$3
[ -x "$NVDISASM" ] || { echo "VERDICT NOT_RUN: $NVDISASM missing"; exit 2; }
ls "$WORK"/*.want > /dev/null 2>&1 || { echo "VERDICT NOT_RUN: no .want files in $WORK (run check.sh first)"; exit 2; }
F=$WORK/negctl
mkdir -p "$F"
find "$F/." -maxdepth 1 \( -name '*.neg' -o -name '*.neg.tmp' \) -exec rm -f {} +
JOBS=${NEGCTL_JOBS:-$(nproc 2>/dev/null || echo 1)}
for f in "$DIR"/*.bin; do printf '%s\0' "$f"; done | xargs -0 -n 1 -P "$JOBS" sh "$HERE/negctl.sh" --one "$DIS" "$WORK"
k=0; broke=0; tracked=0; reported=0; bad=0
for f in "$DIR"/*.bin; do
    b=$(basename "$f" .bin); k=$((k + 1))
    if [ -f "$F/$b.neg" ] && read -r st t rest < "$F/$b.neg"; then
        case "$st $t" in
            "BROKE TRACKED") broke=$((broke + 1)); tracked=$((tracked + 1)) ;;
            "BROKE REPORTED") broke=$((broke + 1)); reported=$((reported + 1)) ;;
            *) echo "[FAIL] $rest"; bad=$((bad + 1)) ;;
        esac
    else echo "[FAIL] $b: no result"; bad=$((bad + 1)); fi
done
echo "flipped-bit control: kernels $k, parity broken in exactly word 0: $broke, of those tracking nvdisasm on the flipped word: $tracked, flipped word reported unknown (??): $reported, failures: $bad"
[ "$bad" = 0 ] && [ "$broke" = "$k" ] && { echo "VERDICT PASS: one flipped bit changes our text and breaks parity on every kernel"; exit 0; }
echo "VERDICT FAIL"; exit 1
