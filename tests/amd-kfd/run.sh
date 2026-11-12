#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Host data-layer gate. Production decisions/serialization are seeded Omega; C and Python here
# are independent test oracles only (PROVENANCE.md). No device is opened, not even for discovery.
# usage: sh tests/amd-kfd/run.sh /absolute/path/to/pinned/omc
# Optional: AMD_KFD_OUT (keep output), KFD_CC (native C oracle), KFD_X86_CC (cross C oracle),
# KFD_OBJCOPY (defaults llvm-objcopy-18), KFD_MC (llvm-mc-18), KFD_READELF (llvm-readelf-18).
# Same exit meanings as multitarget/run.sh: 0 PASS (NOT_RUN listed); 1 FAIL; 64 usage;
# 69 pin refused; 74 I/O; 77 NOT_RUN required tool or Linux AArch64 unavailable.
set -u
LC_ALL=C
export LC_ALL
[ "$#" -eq 1 ] || { echo 'usage: run.sh /absolute/path/to/pinned-omc' >&2; exit 64; }
omc=$1
case $omc in /*) ;; *) echo 'usage: omc path must be absolute' >&2; exit 64 ;; esac
for tool in dirname sed uname basename sha256sum; do
  command -v "$tool" >/dev/null 2>&1 || { echo "NOT_RUN required tool missing: $tool"; exit 77; }
done
root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd) || exit 74
# Use the existing gate as the one source of this repository's compiler digest.
omc_pin=$(sed -n 's/^omc_pin=//p' "$root/tests/multitarget/run.sh")
[ "${#omc_pin}" -eq 64 ] || { echo 'FAIL: cannot read repository compiler pin'; exit 1; }
digest() { d=$(sha256sum < "$1") || return 1; printf '%s\n' "${d%% *}"; }
[ -f "$omc" ] && [ -x "$omc" ] || { echo 'NOT_RUN pinned omc missing/not executable'; exit 77; }
[ "$(digest "$omc")" = "$omc_pin" ] || { echo "REFUSED: compiler digest differs from pin $omc_pin" >&2; exit 69; }
# Digest is verified before any compiler invocation, other oracle, or output-directory creation.
if [ "$(uname -s)" != Linux ] || [ "$(uname -m)" != aarch64 ]; then
  echo 'NOT_RUN: the amd-kfd host gate needs Linux AArch64 and the pinned seeded omc'; exit 77
fi
cc=${KFD_CC:-cc}; objcopy=${KFD_OBJCOPY:-llvm-objcopy-18}
for tool in "$cc" "$objcopy" python3 cat cmp chmod mkdir mktemp timeout od head tail grep; do
  command -v "$tool" >/dev/null 2>&1 || { echo "NOT_RUN required tool missing: $tool"; exit 77; }
done
[ -f /usr/include/linux/kfd_ioctl.h ] || { echo 'NOT_RUN native KFD header missing'; exit 77; }
cd "$root" || exit 74
out=${AMD_KFD_OUT:-$(mktemp -d "${TMPDIR:-/tmp}/amd-kfd.XXXXXX")} || exit 74
mkdir -p "$out" || exit 74
npass=0; nfail=0; nnr=0
ok() { npass=$((npass + 1)); echo "PASS $*"; }
bad() { nfail=$((nfail + 1)); echo "FAIL $*"; }
notrun() { nnr=$((nnr + 1)); echo "NOT_RUN $*"; }
echo "PIN omc $omc_pin (from tests/multitarget/run.sh)"
echo "Outputs: $out"
echo "ORACLE native-header sha256=$(digest /usr/include/linux/kfd_ioctl.h)"

# Python oracles run with -I: PYTHON* variables (PYTHONOPTIMIZE strips asserts) are ignored.
# A fake compiler leaves a marker if it is ever executed. The recursive gate must refuse its
# digest before running it or creating its requested outputs. No alternate pin is injectable.
printf '#!/bin/sh\nprintf executed > "%s"\nexit 0\n' "$out/WRONG-COMPILER-EXECUTED" > "$out/wrong-omc" || exit 74
chmod 700 "$out/wrong-omc" || exit 74
wrong=$(CDPATH= cd -- "$out" && pwd)/wrong-omc
AMD_KFD_OUT="$out/wrong-pin-output" sh tests/amd-kfd/run.sh "$wrong" > "$out/wrong-pin.log" 2>&1
rc=$?
if [ "$rc" -eq 69 ] && [ ! -e "$out/WRONG-COMPILER-EXECUTED" ] && [ ! -e "$out/wrong-pin-output" ]; then
  ok 'CONTROL CAUGHT wrong-compiler-digest: exit=69 compiler not executed; output directory not created'
else bad "wrong-compiler-digest exit=$rc"; fi

for u in aql kernarg records ring; do
  cat "backend/amd/kfd/$u.om" backend/amd/kfd/cli.om > "$out/$u.source.om" || exit 74
  for suffix in '' .repeat; do
    timeout 120 "$omc" < "$out/$u.source.om" > "$out/$u$suffix" 2> "$out/$u$suffix.compile.err"
    rc=$?
    if [ "$rc" -ne 0 ] || [ -s "$out/$u$suffix.compile.err" ] || [ ! -s "$out/$u$suffix" ]; then
      cat "$out/$u$suffix.compile.err"; echo "AMD-KFD-HOST FAIL compile=$u$suffix exit=$rc"; exit 1
    fi
    chmod 700 "$out/$u$suffix" || exit 74
  done
  if cmp -s "$out/$u" "$out/$u.repeat"; then
    ok "deterministic build $u sha256=$(digest "$out/$u")"
  else bad "nonidentical builds $u"; fi
done

# Live C system-header oracle: sizeof, offsetof, request macros. Test-only, no device calls.
"$cc" -std=c11 -O2 -Wall -Wextra -Werror tests/amd-kfd/layout-oracle.c -o "$out/native-layout" > "$out/native-build.log" 2>&1
rc=$?
if [ "$rc" -ne 0 ]; then cat "$out/native-build.log"; echo "AMD-KFD-HOST FAIL native oracle build exit=$rc"; exit 1; fi
"$out/native-layout" > "$out/native-table.tsv" || exit 1
if cmp -s backend/amd/kfd/requests.tsv "$out/native-table.tsv"; then
  ok 'native aarch64: every KFD request/record size/field offset/width equals installed kfd_ioctl.h'
else bad 'native KFD table differs from installed header'; fi
"$objcopy" --dump-section ".kfdlayout=$out/native-layout.bin" "$out/native-layout" || exit 1
xc=${KFD_X86_CC:-x86_64-linux-gnu-gcc}
if [ -d /usr/x86_64-linux-gnu/include ] && [ -f /usr/x86_64-linux-gnu/include/linux/kfd_ioctl.h ] && command -v "$xc" >/dev/null 2>&1; then
  echo "ORACLE x86-64-header sha256=$(digest /usr/x86_64-linux-gnu/include/linux/kfd_ioctl.h)"
  "$xc" -std=c11 -O2 -Wall -Wextra -Werror -I/usr/x86_64-linux-gnu/include -c tests/amd-kfd/layout-oracle.c -o "$out/x86-layout.o" > "$out/x86-build.log" 2>&1
  rc=$?
  if [ "$rc" -ne 0 ]; then cat "$out/x86-build.log"; bad "x86-64 header oracle build exit=$rc"
  elif python3 -I -c 'import pathlib,sys; assert pathlib.Path(sys.argv[1]).read_bytes()[18:20] == bytes([62,0])' "$out/x86-layout.o" \
       && "$objcopy" --dump-section ".kfdlayout=$out/x86-layout.bin" "$out/x86-layout.o" \
       && cmp -s "$out/native-layout.bin" "$out/x86-layout.bin"; then
    ok 'x86-64 cross-compiled header layout: all requests/sizes/offsets/widths equal native table (no x86 execution claimed)'
  else bad 'x86-64 header values/layout differ or compiler not targeting x86-64'; fi
else notrun 'x86-64 layout: cross header tree or x86-64 compiler missing'; fi

# Establish the assembly oracle's code-object version without linking or launching it.
mc=${KFD_MC:-llvm-mc-18}; re=${KFD_READELF:-llvm-readelf-18}
if command -v "$mc" >/dev/null 2>&1 && command -v "$re" >/dev/null 2>&1; then
  "$mc" --version | head -1
  "$mc" -triple amdgcn-amd-amdhsa -mcpu=gfx1151 -filetype=obj backend/amd/vadd-gfx1151.s -o "$out/vadd-oracle.o" > "$out/asm.log" 2>&1
  rc=$?
  if [ "$rc" -eq 0 ] && "$re" -h --notes "$out/vadd-oracle.o" > "$out/vadd-oracle.readelf" \
    && grep -q 'ABI Version: *3$' "$out/vadd-oracle.readelf" \
    && grep -q 'gfx1151' "$out/vadd-oracle.readelf" \
    && grep -q '.kernarg_segment_size: *28$' "$out/vadd-oracle.readelf"; then
    ok 'assembly oracle ELF ABI 3 = code object v5; metadata declares kernarg size 28'
  else cat "$out/asm.log"; bad "assembly oracle code-object version/metadata exit=$rc"; fi
else notrun 'assembly ELF version: llvm-mc/llvm-readelf missing (source metadata still checked)'; fi

# KFD cannot currently be *issued* by the pinned compiler: retain compile-only minimal
# reproductions. The data layer itself uses no devctl. Never run a refused compiler output.
python3 -I - "$out/native-table.tsv" "$out" <<'PY'
import pathlib, sys
rows = pathlib.Path(sys.argv[1]).read_text().splitlines()
out = pathlib.Path(sys.argv[2])
i = 0
while i < len(rows):
    nums, name = rows[i].split('#', 1)
    req, size, count = map(int, nums.split())
    if req:
        (out / ('allowlist-' + name.strip() + '.om')).write_text(
            f'global b uchar {size}\nfn main to int\nr = devctl neg int 1 uint {req} b 0 {size}\nret 0\nend\n')
    i += count + 1
PY
for src in "$out"/allowlist-*.om; do
  timeout 120 "$omc" < "$src" > "$src.refused-output" 2> "$src.err"
  rc=$?
  if [ "$rc" -eq 9 ]; then ok "allowlist compile-only refusal $(basename "$src" .om) exit=9"
  else bad "allowlist expected refusal $(basename "$src" .om) exit=$rc"; fi
done
python3 -I tests/amd-kfd/oracle.py . "$out" > "$out/byte-oracle.log" 2>&1
rc=$?
cat "$out/byte-oracle.log"
if [ "$rc" -eq 0 ]; then ok 'independent byte-oracle suite'; else bad "independent byte-oracle exit=$rc"; fi
notrun 'AMD gfx942 hardware: KFD device calls, queue creation, mapped memory, AQL dispatch, completion and vadd execution'
notrun 'MI300X model loading, Qwen 27B inference, speed and hardware coherency'
if [ "$nfail" -ne 0 ]; then echo "AMD-KFD-HOST FAIL pass=$npass fail=$nfail not_run=$nnr"; exit 1; fi
echo "AMD-KFD-HOST PASS pass=$npass fail=0 not_run=$nnr"
exit 0
