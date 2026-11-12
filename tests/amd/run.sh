#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# gfx942 vadd: Omega is the producer; LLVM 18 tools are independent ORACLES only.
# Usage: sh tests/amd/run.sh /absolute/path/to/pinned/omc
# AMD_OUT keeps outputs (default mktemp). Missing mandatory tools => NOT_RUN exit 3,
# unsupported host => NOT_RUN exit 77; FAIL => 1; pin refusal => 69; usage => 64; I/O => 74.
# Hardware rows are always NOT_RUN; PASS is host byte/format evidence, never hardware qualification.
set -u
LC_ALL=C
export LC_ALL
for tool in uname sha256sum; do
  command -v "$tool" >/dev/null 2>&1 || { echo "NOT_RUN missing tool $tool"; exit 3; }
done
if [ "$(uname -s)" != Linux ] || [ "$(uname -m)" != aarch64 ]; then
  echo 'NOT_RUN gfx942 host gate requires Linux AArch64 and the pinned seeded omc'; exit 77
fi
[ "$#" -eq 1 ] || { echo 'usage: run.sh /absolute/path/to/pinned/omc' >&2; exit 64; }
omc=$1
case $omc in /*) ;; *) echo 'usage: omc path must be absolute' >&2; exit 64 ;; esac
[ -f "$omc" ] && [ -x "$omc" ] || { echo 'NOT_RUN pinned omc is missing or not executable'; exit 3; }
# Same source pin as tests/multitarget/run.sh and AGENTS.md (omega-mv e5b32567/369dbc96).
omc_pin=2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f
digest() { d=$(sha256sum < "$1") || return 1; printf '%s\n' "${d%% *}"; }
[ "$(digest "$omc")" = "$omc_pin" ] || { echo "REFUSED: compiler digest differs from pin $omc_pin" >&2; exit 69; }
# Pin check deliberately precedes mkdir, source reads, every compilation, and every producer run.
for tool in llvm-mc-18 llvm-objdump-18 llvm-readelf-18 llvm-objcopy-18 timeout cmp sed grep awk dd head tail wc tr mkdir chmod mktemp cat dirname sh od; do
  command -v "$tool" >/dev/null 2>&1 || { echo "NOT_RUN missing tool $tool"; exit 3; }
done
root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd) || exit 74
out=${AMD_OUT:-$(mktemp -d "${TMPDIR:-/tmp}/cudapp-gfx942.XXXXXX")} || exit 74
mkdir -p "$out" || exit 74
# Canonicalize only after mkdir, so recursive wrong-pin control can prove it created no output dir.
out=$(CDPATH= cd -- "$out" && pwd) || exit 74
src=$root/backend/amd/gfx942
printf 'PIN omc %s\nOutputs: AMD_OUT (or the fresh temporary directory)\n' "$omc_pin"
for tool in llvm-mc-18 llvm-objdump-18 llvm-readelf-18 llvm-objcopy-18; do
  "$tool" --version > "$out/$tool.version" 2>&1 || { echo "FAIL cannot run $tool"; exit 1; }
  printf 'ORACLE %s %s\n' "$tool" "$(head -n 1 "$out/$tool.version")"
done
pass=0; fail=0
ok() { pass=$((pass + 1)); echo "PASS $*"; }
bad() { fail=$((fail + 1)); echo "FAIL $*"; }
expect() {
  want=$1; name=$2; shift 2
  "$@" > "$out/$name.log" 2>&1; got=$?
  if [ "$got" -eq "$want" ]; then ok "$name exit=$got $(tail -n 1 "$out/$name.log")";
  else bad "$name exit=$got want=$want $(tail -n 1 "$out/$name.log")"; fi
}
compare() {
  if cmp -s "$1" "$2"; then ok "$3"; else bad "$3"; fi
}
poke() { printf "\\$3" | dd of="$1" bs=1 seek="$2" conv=notrunc status=none; }
build() {
  name=$1; unit=$2
  timeout 120 "$omc" < "$unit" > "$out/$name" 2> "$out/$name.compile.err"; rc=$?
  if [ "$rc" -ne 0 ] || [ -s "$out/$name.compile.err" ] || [ ! -s "$out/$name" ]; then
    cat "$out/$name.compile.err"; echo "FAIL compile $name exit=$rc"; exit 1
  fi
  chmod 700 "$out/$name" || exit 74
  echo "COMPILE $name src=$(digest "$unit") bin=$(digest "$out/$name")"
}
build targets "$root/backend/targets.om"
build hsaco "$src/hsaco.om"
build hsaco-again "$src/hsaco.om"
compare "$out/hsaco" "$out/hsaco-again" 'two producer compilations byte-identical'
for build in hsaco hsaco-again; do
  timeout 30 "$out/$build" "$src/vadd.isa" > "$out/$build.hsaco" 2> "$out/$build.emit.err"; rc=$?
  if [ "$rc" -ne 0 ] || [ -s "$out/$build.emit.err" ] || [ ! -s "$out/$build.hsaco" ]; then
    echo "FAIL producer $build exit=$rc"; cat "$out/$build.emit.err"; exit 1
  fi
done
compare "$out/hsaco.hsaco" "$out/hsaco-again.hsaco" 'two code-object builds byte-identical'
echo "OBJECT sha256=$(digest "$out/hsaco.hsaco") bytes=$(wc -c < "$out/hsaco.hsaco" | tr -d ' ')"
T=$out/targets
expect 0 load-gfx942 "$T" load AMDGPU_HSACO "$out/hsaco.hsaco" gfx942
grep -q '^LOAD ACCEPT AMDGPU_HSACO target=amdgcn-amd-amdhsa--gfx942 ' "$out/load-gfx942.log" || bad 'loader did not print LOAD ACCEPT'
# The oracle is ET_REL. No linker is required or invoked, and no oracle bytes are producer input.
if ! llvm-mc-18 -triple=amdgcn-amd-amdhsa -mcpu=gfx942 -filetype=obj "$src/vadd-gfx942.s" -o "$out/oracle.o" > "$out/assemble.log" 2>&1; then
  echo 'FAIL oracle assembly'; cat "$out/assemble.log"; exit 1
fi
llvm-mc-18 -triple=amdgcn-amd-amdhsa -mcpu=gfx942 -show-encoding "$src/vadd-gfx942.s" > "$out/oracle.encodings.txt" 2>&1 || { echo 'FAIL encoding oracle'; exit 1; }
for pair in hsaco.hsaco:omega oracle.o:oracle; do
  file=${pair%:*}; prefix=${pair#*:}
  # llvm-objcopy rewrites its input when no explicit output is supplied. Supply a throwaway output
  # even for --dump-section: the original emitted/assembled objects must NEVER be modified.
  llvm-objcopy-18 --dump-section=.text="$out/$prefix.text" --dump-section=.rodata="$out/$prefix.rodata" --dump-section=.note="$out/$prefix.note" \
    "$out/$file" "$out/$prefix.objcopy-rewrite" > "$out/$prefix.extract.log" 2>&1 || { echo "FAIL extract $prefix"; exit 1; }
  llvm-readelf-18 -h -l -S -s -r -d --notes "$out/$file" > "$out/$prefix.readelf.txt" 2>&1 || { echo "FAIL readelf $prefix"; exit 1; }
done
compare "$out/omega.text" "$out/oracle.text" 'code section bytes equal llvm-mc-18 gfx942 oracle'
compare "$out/omega.note" "$out/oracle.note" 'AMDGPU metadata note bytes equal independent oracle'
# Descriptor REL64 relocation only affects byte offsets 16..23. All other bytes must agree.
dd if="$out/omega.rodata" of="$out/omega.descriptor-head" bs=1 count=16 status=none || exit 74
dd if="$out/oracle.rodata" of="$out/oracle.descriptor-head" bs=1 count=16 status=none || exit 74
dd if="$out/omega.rodata" of="$out/omega.descriptor-tail" bs=1 skip=24 status=none || exit 74
dd if="$out/oracle.rodata" of="$out/oracle.descriptor-tail" bs=1 skip=24 status=none || exit 74
compare "$out/omega.descriptor-head" "$out/oracle.descriptor-head" 'descriptor sizes/reserved prefix match oracle'
compare "$out/omega.descriptor-tail" "$out/oracle.descriptor-tail" 'descriptor resources/properties match oracle'
# The ET_REL oracle has a REL64 relocation here; independently check the resolved displacement.
entryva=$(awk '$8 == "vadd" { print "0x" $2; exit }' "$out/omega.readelf.txt")
descva=$(awk '$8 == "vadd.kd" { print "0x" $2; exit }' "$out/omega.readelf.txt")
disp=$(od -An -tu8 -j 16 -N 8 "$out/omega.rodata" | tr -d ' ')
if [ -n "$entryva" ] && [ -n "$descva" ] && [ "$disp" = "$((entryva - descva))" ]; then
  ok 'descriptor entry displacement resolves to vadd (no relocation remains)'
else bad 'descriptor entry displacement'; fi
# ELF addresses differ (ET_DYN has code VA, ET_REL starts at zero); rebase ORACLE by emitted VA.
vma=$(awk '$3 == ".text" { print "0x" $5; exit }' "$out/omega.readelf.txt")
[ -n "$vma" ] || { echo 'FAIL .text VMA not found'; exit 1; }
(cd "$out" && llvm-objdump-18 -d hsaco.hsaco) > "$out/omega.objdump.txt" 2>&1 || exit 1
(cd "$out" && llvm-objdump-18 -d --adjust-vma="$vma" oracle.o) > "$out/oracle.objdump.txt" 2>&1 || exit 1
# Strip ONLY the input-filename heading, retain addresses, instruction words, operands and labels.
sed '1,2d' "$out/omega.objdump.txt" > "$out/omega.listing"
sed '1,2d' "$out/oracle.objdump.txt" > "$out/oracle.listing"
compare "$out/omega.listing" "$out/oracle.listing" 'objdump -d listings equal (oracle rebased to emitted .text VA)'
if grep -q 'NT_AMDGPU_METADATA' "$out/omega.readelf.txt" && grep -q 'amdhsa.target: *amdgcn-amd-amdhsa--gfx942' "$out/omega.readelf.txt" \
 && grep -q '\.name: *vadd$' "$out/omega.readelf.txt" && grep -q ' vadd.kd$' "$out/omega.readelf.txt" \
 && grep -q 'Flags: *0x54C, gfx942' "$out/omega.readelf.txt" && ! grep -q 'warning:\|error:' "$out/omega.readelf.txt"; then
  ok 'llvm-readelf parses HSA v5 gfx942 flags, metadata kernel vadd and symbol vadd.kd'
else bad 'llvm-readelf metadata/header/symbol check'; fi
# Negative 1: one bit in a TABLE operand (v_add_f32 src0 v4=260 -> v5=261).
sed 's/^I 5 260 8 4 0x01 /I 5 261 8 4 0x01 /' "$src/vadd.isa" > "$out/flipped.isa"
if cmp -s "$src/vadd.isa" "$out/flipped.isa"; then bad 'instruction-table mutation did not apply'; fi
timeout 30 "$out/hsaco" "$out/flipped.isa" > "$out/flipped.hsaco" 2> "$out/flipped.emit.err"; rc=$?
if [ "$rc" -ne 0 ] || [ -s "$out/flipped.emit.err" ]; then bad "instruction-table mutant did not produce object exit=$rc";
else
  llvm-objcopy-18 --dump-section=.text="$out/flipped.text" "$out/flipped.hsaco" "$out/flipped.objcopy-rewrite" > "$out/flipped.extract.log" 2>&1 || exit 1
  if cmp -s "$out/flipped.text" "$out/oracle.text"; then bad 'CONTROL instruction-table flipped bit escaped';
  else ok 'CONTROL instruction-table flipped bit CAUGHT code mismatch'; fi
fi
# Negative 2: gfx942 mach 0x4c -> gfx1100 mach 0x41; documented targets.om refusal 33.
dd if="$out/hsaco.hsaco" of="$out/wrong-mach.hsaco" status=none || exit 74
poke "$out/wrong-mach.hsaco" 48 101 || exit 74
expect 33 control-wrong-mach "$T" load AMDGPU_HSACO "$out/wrong-mach.hsaco" gfx942
# Negative 3: retains ELF/program headers but ends before PT_NOTE: documented refusal 34.
head -c 600 "$out/hsaco.hsaco" > "$out/truncated.hsaco" || exit 74
expect 34 control-truncated "$T" load AMDGPU_HSACO "$out/truncated.hsaco" gfx942
# Negative 4: an executable with the wrong digest must not run, compile, or create output.
cat > "$out/wrong-omc" <<'WRONG'
#!/bin/sh
printf 'WRONG COMPILER EXECUTED\n'
exit 99
WRONG
chmod 700 "$out/wrong-omc" || exit 74
AMD_OUT="$out/wrong-pin-must-not-exist" sh "$root/tests/amd/run.sh" "$out/wrong-omc" > "$out/wrong-pin.log" 2>&1; rc=$?
if [ "$rc" -eq 69 ] && grep -q '^REFUSED: compiler digest differs from pin ' "$out/wrong-pin.log" \
  && ! grep -q 'WRONG COMPILER EXECUTED' "$out/wrong-pin.log" && [ ! -e "$out/wrong-pin-must-not-exist" ]; then
  ok 'CONTROL wrong compiler digest CAUGHT exit=69 before compiler/producer/output setup'
else bad "CONTROL wrong compiler digest exit=$rc or effect escaped"; fi
# The loader's ACCEPT is NOT a kernel execution check or proof of runtime-loader interoperability.
echo 'hardware NOT_RUN: gfx942 MI300X dispatch, FP32 outputs, n=0/tail/boundary vectors, ROCr loading and all other GPU rows'
echo 'NOT_RUN linked-oracle whole-object parity: gate uses ET_REL section parity; Spark has no ld.lld'
if [ "$fail" -ne 0 ]; then echo "AMD-GFX942-HOST FAIL pass=$pass fail=$fail hardware=NOT_RUN"; exit 1; fi
echo "AMD-GFX942-HOST PASS pass=$pass fail=0 hardware=NOT_RUN"
exit 0
