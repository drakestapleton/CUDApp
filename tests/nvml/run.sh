#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# CUDA++ NVML host tier: the libnvidia-ml C ABI subset in compat/nvml against the resource-manager
# fake tests/nvml/rmfake.om, plus an ABI comparison with the CUDA toolkit's nvml.h, the thin-shell
# guard, checker negative controls and mutants of the Omega sources.
# Device-free: tests/nvml/transport.c replaces the #281 omega2_nvrm_* transport with the fake; nothing
# opens /dev/nvidia*. libnvidia-ml is never linked.
# Usage: tests/nvml/run.sh   (needs OMEGA_OMC, the omega-mv e5b32567 pin, and OMEGA_BRIDGE)
set -u
omc=${OMEGA_OMC:-}
br=${OMEGA_BRIDGE:-}
if [ -z "$omc" ] || [ ! -x "$omc" ] || [ -z "$br" ] || [ ! -f "$br/tools/bridge-seeded" ]; then
  echo 'NOT_RUN: set OMEGA_OMC (omega-mv e5b32567 omc) and OMEGA_BRIDGE (omega-bridge checkout)' >&2
  exit 77
fi
pin=2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f
got=$(sha256sum < "$omc" | cut -c1-64) || exit 74
if [ "$got" != "$pin" ]; then
  echo "REFUSED: omc digest $got is not the omega-mv e5b32567 pin" >&2
  exit 69
fi
fpin=23ff7a2356bb5d980060a4685a27ac46587013069b7d24b37c2873d7ee659b37
got=$(sha256sum < "$br/tools/bridge-seeded" | cut -c1-64) || exit 74
if [ "$got" != "$fpin" ]; then
  echo "REFUSED: bridge-seeded digest $got is not the om/units.sh frontend pin" >&2
  exit 69
fi
root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd) || exit 74
t=$root/tests/nvml
nv=$root/native/nvml
out=$(mktemp -d "${TMPDIR:-/tmp}/cudapp-nvml.XXXXXX") || exit 74
trap 'rm -rf "$out"' EXIT
CC=${CC:-cc}
CFLAGS="-std=gnu11 -O1 -Wall -Wextra -Werror"
fails=0
fail() { echo "FAIL: $*" >&2; fails=$((fails + 1)); }
bridge() { OMEGA_SEEDED_OMC="$omc" sh "$br/tools/bridge-seeded" "$@"; }

# Composes a unit exactly as om/units.sh does: each part followed by one newline.
compose() { # dir out
  : > "$2"
  for f in nvml query step; do cat "$1/$f.om" >> "$2" && printf '\n' >> "$2" || return 1; done
}

compose "$nv" "$out/nvml_step.om" || exit 74
{ cat "$nv/errstr.om"; printf '\n'; } > "$out/nvml_errstr.om"
for u in nvml_step nvml_errstr; do
  bridge c "$out/$u.om" > "$out/$u.c" 2> "$out/$u.log" || { cat "$out/$u.log" >&2; fail "$u did not bridge"; }
  if cmp -s "$out/$u.c" "$root/src/cudapp_om_$u.c"; then
    echo "PASS committed src/cudapp_om_$u.c is the bridge output of native/nvml"
  else
    fail "src/cudapp_om_$u.c is stale; rerun om/units.sh"
  fi
  bridge h "$out/$u.om" > "$out/$u.h" 2>/dev/null
  if grep -qxF -f "$out/$u.h" "$root/src/cudapp_native.h"; then :; else fail "src/cudapp_native.h lacks the $u prototype"; fi
done
bridge c "$t/rmfake.om" > "$out/fake.c" 2> "$out/fake.log" || { cat "$out/fake.log" >&2; fail 'rmfake.om did not bridge'; exit 1; }

link() { # step.c errstr.c exe
  $CC $CFLAGS -I"$root/src" -I"$root/compat/nvml" -I"$t" "$root/compat/nvml/cudapp_nvml.c" "$1" "$2" \
    "$out/fake.c" "$t/transport.c" "$t/nvml_test.c" -lpthread -o "$3"
}
link "$root/src/cudapp_om_nvml_step.c" "$root/src/cudapp_om_nvml_errstr.c" "$out/nvml_test" \
  || { fail 'host test did not build'; exit 1; }
"$out/nvml_test" "$t/errstr-oracle.tsv" > "$out/test.out" 2>&1; rc=$?
last=$(tail -n 1 "$out/test.out")
case "$rc:$last" in
  0:'CUDAPP-NVML HOST PASS ('*' checks)') echo "PASS fake tier: $last" ;;
  *) cat "$out/test.out" >&2; fail "fake tier rc=$rc: $last" ;;
esac

# Checker negative control: an oracle with one altered string must fail.
sed '2s/	.*/	Successful/' "$t/errstr-oracle.tsv" > "$out/bad.tsv"
if cmp -s "$out/bad.tsv" "$t/errstr-oracle.tsv"; then
  fail 'oracle negative control: the edit changed nothing'
elif "$out/nvml_test" "$out/bad.tsv" > "$out/bad.out" 2>&1; then
  fail 'oracle negative control: altered oracle passed'
else
  echo "PASS oracle negative control (altered string rejected: $(tail -n 1 "$out/bad.out"))"
fi

# ABI: compat/nvml/nvml.h must describe the same sizes, offsets and values as NVIDIA's header, and the
# shell must compile against NVIDIA's prototypes. Header only; nothing NVIDIA is linked.
cuda=${CUDA_NVML_INCLUDE:-/usr/local/cuda/include}
if [ -f "$cuda/nvml.h" ]; then
  $CC $CFLAGS -I"$root/compat/nvml" "$t/abi.c" -o "$out/abi-ours" && "$out/abi-ours" > "$out/abi-ours.txt" \
    && $CC $CFLAGS -I"$cuda" "$t/abi.c" -o "$out/abi-cuda" && "$out/abi-cuda" > "$out/abi-cuda.txt" || fail 'abi probe did not build'
  if [ -s "$out/abi-ours.txt" ] && cmp -s "$out/abi-ours.txt" "$out/abi-cuda.txt"; then
    echo "PASS abi: $(wc -l < "$out/abi-ours.txt") facts equal to $cuda/nvml.h"
  else
    diff "$out/abi-ours.txt" "$out/abi-cuda.txt" >&2; fail 'abi differs from the CUDA nvml.h'
  fi
  if $CC $CFLAGS -c -I"$cuda" -I"$root/src" "$root/compat/nvml/cudapp_nvml.c" -o "$out/shell-cuda.o"; then
    echo 'PASS shell compiles against the CUDA nvml.h prototypes'
  else
    fail 'shell does not compile against the CUDA nvml.h prototypes'
  fi
else
  echo "NOT_RUN abi comparison ($cuda/nvml.h absent; set CUDA_NVML_INCLUDE)"
fi

# Thin shell: the C reaches the OS only through omega2_nvrm_* and makes no decisions about RM replies.
thin() {
  awk '/^[ \t]*(\/\*|\*)/ { next }
       /[^_a-z](open|ioctl|close|mmap|read|write)\(|\/dev\/|NV0|NV2080|0x2080/ { print FILENAME ":" NR ": " $0; bad = 1 }
       END { exit bad }' "$1"
}
if thin "$root/compat/nvml/cudapp_nvml.c"; then echo 'PASS thin-shell guard'; else fail 'shell holds OS calls or RM knowledge'; fi
printf 'static int x(void) { return ioctl(3, 0x20800110); }\n' > "$out/fat.c"
if thin "$out/fat.c" > /dev/null; then fail 'thin-shell guard negative control passed'; else echo 'PASS thin-shell guard negative control'; fi

# Mutants: each edits one line that must occur exactly once in its file, and must be caught.
# file|search|replacement
mutants='
nvml|if eq v 86|if eq v 87
nvml|if eq v 27|if eq v 28
nvml|if or or eq x 18446744073709551614 eq x 18446744073709551610 eq x 18446744073709551597|if false
nvml|put b n uchar add 48 rem minor 10|put b n uchar add 49 rem minor 10
nvml|if gt get s 16 0|if false
nvml|if gt get s 16 1|if false
nvml|for t ulong 26|for t ulong 2
nvml|if eq g32 b add add PO 4 mul i 4 52928|if eq g32 b add add PO 4 mul i 4 52929
query|if ge get s 1 get s 21|if gt get s 1 get s 21
query|put b add OO 3 45|put b add OO 3 46
query|put b n 45|put b n 95
query|if eq g32 b add add add PO 24 mul tg 44 16 1|if eq g32 b add add add PO 24 mul tg 44 16 2
query|if ge v 2147483648|if false
query|if ne g32 b add PO 8 0|if false
query|p64 b add OO 16 mul sub t f 1024|p64 b add OO 16 mul t 1024
query|o = add add PO 16 mul rem add r 71 72 776|o = add add PO 16 mul rem r 72 776
query|p32 b OO div g 100|p32 b OO g
query|p64 b add q 8 18446744073709551615|p64 b add q 8 0
query|p32 b add q 16 4294967295|p32 b add q 16 0
query|if or gt c 72 ge r 72|if false
query|if gt f t|if false
errstr|n = w8 o n 32496501618079059|n = w8 o n 32496501618079058
'
mk=0; caught=0
printf '%s\n' "$mutants" | { while IFS='|' read -r file from to; do
  [ -n "$file" ] || continue
  mk=$((mk + 1))
  n=$(grep -cxF "$from" "$nv/$file.om")
  if [ "$n" != 1 ]; then echo "FAIL: mutant site '$from' found $n times in $file.om" >&2; continue; fi
  rm -rf "$out/m" && mkdir "$out/m" && cp "$nv"/*.om "$out/m/" || exit 74
  awk -v f="$from" -v r="$to" '$0 == f { print r; next } { print }' "$nv/$file.om" > "$out/m/$file.om"
  compose "$out/m" "$out/m/step.u.om" && { cat "$out/m/errstr.om"; printf '\n'; } > "$out/m/errstr.u.om"
  if ! bridge c "$out/m/step.u.om" > "$out/m/step.c" 2>/dev/null || ! bridge c "$out/m/errstr.u.om" > "$out/m/errstr.c" 2>/dev/null \
     || ! link "$out/m/step.c" "$out/m/errstr.c" "$out/m/t" 2>/dev/null; then
    echo "FAIL: mutant '$from' did not build; it proves nothing" >&2; continue
  fi
  if "$out/m/t" "$t/errstr-oracle.tsv" > "$out/m/out" 2>&1; then
    echo "FAIL: mutant survived: $file: $from -> $to" >&2
  else
    caught=$((caught + 1)); echo "  mutant $file '$from': caught ($(tail -n 1 "$out/m/out"))"
  fi
done
echo "mutants caught $caught/$mk"
[ "$caught" -eq "$mk" ]; } || fail 'mutants'

if [ "$fails" -ne 0 ]; then
  echo "CUDAPP-NVML HOST FAIL ($fails)"
  exit 1
fi
echo 'CUDAPP-NVML HOST PASS'
