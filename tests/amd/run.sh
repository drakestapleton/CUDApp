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
for tool in python3 llvm-mc-18 llvm-objdump-18 llvm-readelf-18 llvm-objcopy-18 timeout cmp sed grep awk dd head tail wc tr mkdir chmod mktemp cat dirname sh od; do
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
# MessagePack encoders at every width boundary, both sides, and the uint64 maximum (the vadd metadata only uses small values, so the object
# comparisons above cannot see a width defect). The producer's own mpuint/mpstr are linked under a test
# main; expected bytes are the msgpack spec formats written here by hand, not producer output.
mptest() {
  sed '/^fn main to int/,$d' "$1" > "$out/$2.om" || exit 74
  cat >> "$out/$2.om" <<'MPMAIN'
fn main to int
mpuint 0
mpuint 127
mpuint 128
mpuint 255
mpuint 256
mpuint 65535
mpuint 65536
mpuint 4294967295
mpuint 4294967296
mpuint 18446744073709551615
a = new uchar 31 65
mpstr a
b = new uchar 32 66
mpstr b
c = new uchar 255 67
mpstr c
d = new uchar 256 68
mpstr d
f = new uchar 65535 70
mpstr f
e = new uchar 65536 69
mpstr e
flush
ret 0
end
MPMAIN
  timeout 120 "$omc" < "$out/$2.om" > "$out/$2" 2> "$out/$2.compile.err" && chmod 700 "$out/$2" \
    && timeout 30 "$out/$2" > "$out/$2.bytes" 2> "$out/$2.err"
}
{ printf '\000\177\314\200\314\377\315\001\000\315\377\377\316\000\001\000\000\316\377\377\377\377\317\000\000\000\001\000\000\000\000\317\377\377\377\377\377\377\377\377'
  printf '\277'; head -c 31 /dev/zero | tr '\000' A
  printf '\331\040'; head -c 32 /dev/zero | tr '\000' B
  printf '\331\377'; head -c 255 /dev/zero | tr '\000' C
  printf '\332\001\000'; head -c 256 /dev/zero | tr '\000' D
  printf '\332\377\377'; head -c 65535 /dev/zero | tr '\000' F
  printf '\333\000\001\000\000'; head -c 65536 /dev/zero | tr '\000' E; } > "$out/msgpack.expected" || exit 74
if mptest "$src/hsaco.om" msgpack && cmp -s "$out/msgpack.bytes" "$out/msgpack.expected"; then
  ok 'MessagePack uint and str encodings at every width boundary equal the spec bytes'
else bad 'MessagePack width encodings'; fi
# Control: the old single-byte value encoder (octet v) must be caught by the same comparison.
# It must compile and run, so the comparison (not a build failure) is what catches it.
sed 's/^if le v 127$/if le v 18446744073709551615/' "$src/hsaco.om" > "$out/mp-mutant-src.om" || exit 74
if cmp -s "$src/hsaco.om" "$out/mp-mutant-src.om"; then bad 'MessagePack mutant did not apply'
elif ! mptest "$out/mp-mutant-src.om" mp-mutant; then bad 'MessagePack mutant did not build and run'
elif cmp -s "$out/mp-mutant.bytes" "$out/msgpack.expected"; then bad 'CONTROL single-byte MessagePack value escaped'
else ok 'CONTROL single-byte MessagePack value CAUGHT by byte comparison'; fi
# Control: a str16/str32 cutoff one below the spec (65534) must be caught at length 65535.
sed 's/^if le n 65535$/if le n 65534/' "$src/hsaco.om" > "$out/mp-mutant2-src.om" || exit 74
if cmp -s "$src/hsaco.om" "$out/mp-mutant2-src.om"; then bad 'MessagePack str16 mutant did not apply'
elif ! mptest "$out/mp-mutant2-src.om" mp-mutant2; then bad 'MessagePack str16 mutant did not build and run'
elif cmp -s "$out/mp-mutant2.bytes" "$out/msgpack.expected"; then bad 'CONTROL str16 cutoff 65534 escaped'
else ok 'CONTROL str16 cutoff 65534 CAUGHT by byte comparison'; fi
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
# The generic table metadata must retain the existing complete vadd object, not just its .text.
if [ "$(digest "$out/hsaco.hsaco")" = 10eb12e019ff2117ddc0190650899d4254589375c62e5ebb970b1bbb7e67e55f ]; then
  ok 'vadd entire object unchanged from pre-generalization'
else bad 'vadd object changed during metadata generalization'; fi
for producer in hsaco hsaco-again; do
  timeout 30 "$out/$producer" "$src/fsum16.isa" > "$out/fsum16-$producer.hsaco" 2> "$out/fsum16-$producer.err"; rc=$?
  if [ "$rc" -ne 0 ] || [ -s "$out/fsum16-$producer.err" ]; then
    echo "FAIL fsum16 producer exit=$rc"; cat "$out/fsum16-$producer.err"; exit 1
  fi
done
compare "$out/fsum16-hsaco.hsaco" "$out/fsum16-hsaco-again.hsaco" 'fsum16 two code-object builds byte-identical'
if ! llvm-mc-18 -triple=amdgcn-amd-amdhsa -mcpu=gfx942 -filetype=obj "$src/fsum16-gfx942.s" -o "$out/fsum16-oracle.o" > "$out/fsum16-assemble.log" 2>&1; then
  echo 'FAIL fsum16 oracle assembly'; cat "$out/fsum16-assemble.log"; exit 1
fi
for pair in fsum16-hsaco.hsaco:fsum16-omega fsum16-oracle.o:fsum16-oracle; do
  file=${pair%:*}; prefix=${pair#*:}
  llvm-objcopy-18 --dump-section=.text="$out/$prefix.text" --dump-section=.rodata="$out/$prefix.rodata" --dump-section=.note="$out/$prefix.note" \
    "$out/$file" "$out/$prefix.objcopy-rewrite" > "$out/$prefix.extract.log" 2>&1 || exit 1
  llvm-readelf-18 -h -l -S -s -r -d --notes "$out/$file" > "$out/$prefix.readelf.txt" 2>&1 || exit 1
  dd if="$out/$prefix.rodata" of="$out/$prefix.descriptor-head" bs=1 count=16 status=none || exit 74
  dd if="$out/$prefix.rodata" of="$out/$prefix.descriptor-tail" bs=1 skip=24 status=none || exit 74
done
compare "$out/fsum16-omega.text" "$out/fsum16-oracle.text" 'fsum16 .text bytes equal independent llvm-mc-18 oracle'
compare "$out/fsum16-omega.note" "$out/fsum16-oracle.note" 'fsum16 metadata note bytes equal independent oracle'
compare "$out/fsum16-omega.descriptor-head" "$out/fsum16-oracle.descriptor-head" 'fsum16 descriptor sizes match oracle'
compare "$out/fsum16-omega.descriptor-tail" "$out/fsum16-oracle.descriptor-tail" 'fsum16 descriptor resources/properties match oracle'
entryva=$(awk '$8 == "fsum16" { print "0x" $2; exit }' "$out/fsum16-omega.readelf.txt")
descva=$(awk '$8 == "fsum16.kd" { print "0x" $2; exit }' "$out/fsum16-omega.readelf.txt")
disp=$(od -An -tu8 -j 16 -N 8 "$out/fsum16-omega.rodata" | tr -d ' ')
if [ -n "$entryva" ] && [ -n "$descva" ] && [ "$disp" = "$((entryva - descva))" ]; then
  ok 'fsum16 descriptor entry displacement resolves to kernel symbol'
else bad 'fsum16 descriptor entry displacement'; fi
if grep -q '\.name: *fsum16$' "$out/fsum16-omega.readelf.txt" \
  && grep -q ' fsum16.kd$' "$out/fsum16-omega.readelf.txt" \
  && grep -q '\.kernarg_segment_size: *20$' "$out/fsum16-omega.readelf.txt" \
  && grep -q '\.sgpr_count: *18$' "$out/fsum16-omega.readelf.txt" \
  && ! grep -q 'warning:\|error:' "$out/fsum16-omega.readelf.txt"; then
  ok 'llvm-readelf parses fsum16 symbol, 20-byte kernarg and table register counts'
else bad 'fsum16 llvm-readelf metadata/header/symbol check'; fi
# Reject broken metadata before emitting any object; do not infer a kernel from its filename.
for control in missing-kernel duplicate-kernel argument-gap; do
  case $control in
    missing-kernel) sed '/^K /d' "$src/fsum16.isa" ;;
    duplicate-kernel) sed '/^K /p' "$src/fsum16.isa" ;;
    argument-gap) sed 's/^A n 16 4 1$/A n 20 4 1/' "$src/fsum16.isa" ;;
  esac > "$out/$control.isa"
  timeout 30 "$out/hsaco" "$out/$control.isa" > "$out/$control.hsaco" 2> "$out/$control.err"; rc=$?
  if [ "$rc" -eq 4 ] && [ ! -s "$out/$control.hsaco" ]; then
    ok "CONTROL $control CAUGHT exit=4 before object emission"
  else bad "CONTROL $control exit=$rc or partial object escaped"; fi
done
echo "FSUM16 OBJECT sha256=$(digest "$out/fsum16-hsaco.hsaco") bytes=$(wc -c < "$out/fsum16-hsaco.hsaco" | tr -d ' ')"
if [ "$(digest "$out/fsum16-hsaco.hsaco")" = 15cf499fa114e801d931fb73cbee13fcb6e8764c08a470368306625a8df37478 ]; then
  ok 'fsum16 entire object unchanged from pre-SGEMM'
else bad 'fsum16 object changed'; fi
for producer in hsaco hsaco-again; do
  timeout 30 "$out/$producer" "$src/sgemm.isa" > "$out/sgemm-$producer.hsaco" 2> "$out/sgemm-$producer.emit.err"; rc=$?
  if [ "$rc" -ne 0 ] || [ -s "$out/sgemm-$producer.emit.err" ]; then
    echo "FAIL sgemm emit exit=$rc"; cat "$out/sgemm-$producer.emit.err"; exit 1
  fi
done
compare "$out/sgemm-hsaco.hsaco" "$out/sgemm-hsaco-again.hsaco" 'sgemm two code-object builds byte-identical'
if ! llvm-mc-18 -triple=amdgcn-amd-amdhsa -mcpu=gfx942 -filetype=obj "$src/sgemm-gfx942.s" -o "$out/sgemm-oracle.o" > "$out/sgemm-assemble.log" 2>&1; then
  echo 'FAIL sgemm oracle assembly'; cat "$out/sgemm-assemble.log"; exit 1
fi
llvm-mc-18 -triple=amdgcn-amd-amdhsa -mcpu=gfx942 -show-encoding "$src/sgemm-gfx942.s" > "$out/sgemm-encodings.txt" 2>&1 || exit 1
for pair in sgemm-hsaco.hsaco:sgemm-omega sgemm-oracle.o:sgemm-oracle; do
  file=${pair%:*}; name=${pair#*:}
  llvm-objcopy-18 --dump-section .text="$out/$name.text" --dump-section .note="$out/$name.note" --dump-section .rodata="$out/$name.rodata" "$out/$file" "$out/$name.objcopy-rewrite" || exit 1
  llvm-readelf-18 -h -S -s -n "$out/$file" > "$out/$name.readelf.txt" || exit 1
  dd if="$out/$name.rodata" of="$out/$name.descriptor-head" bs=1 count=16 status=none || exit 74
  dd if="$out/$name.rodata" of="$out/$name.descriptor-tail" bs=1 skip=24 status=none || exit 74
done
compare "$out/sgemm-omega.text" "$out/sgemm-oracle.text" 'sgemm .text including v_fmac_f32 and backward branch equals LLVM18 gfx942'
compare "$out/sgemm-omega.note" "$out/sgemm-oracle.note" 'sgemm metadata note equals independent oracle'
compare "$out/sgemm-omega.descriptor-head" "$out/sgemm-oracle.descriptor-head" 'sgemm descriptor sizes equal oracle'
compare "$out/sgemm-omega.descriptor-tail" "$out/sgemm-oracle.descriptor-tail" 'sgemm descriptor resources including workgroup Y equal oracle'
rsrc2=$(od -An -tu4 -j 52 -N 4 "$out/sgemm-omega.rodata" | tr -d ' ')
if [ "$((rsrc2 & 256))" -eq 256 ] && grep -q 'v_fmac_f32' "$out/sgemm-encodings.txt"; then
  ok 'sgemm workgroup-id-Y enabled in rsrc2 and v_fmac_f32 accepted by gfx942 oracle'
else bad 'sgemm Y descriptor bit or fmac encoding'; fi
entryva=$(awk '$8 == "sgemm" { print "0x" $2; exit }' "$out/sgemm-omega.readelf.txt")
descva=$(awk '$8 == "sgemm.kd" { print "0x" $2; exit }' "$out/sgemm-omega.readelf.txt")
disp=$(od -An -tu8 -j 16 -N 8 "$out/sgemm-omega.rodata" | tr -d ' ')
if [ -n "$entryva" ] && [ -n "$descva" ] && [ "$disp" = "$((entryva - descva))" ]; then
  ok 'sgemm descriptor entry displacement resolves to kernel symbol'
else bad 'sgemm descriptor entry displacement'; fi
if grep -q '\.name: *sgemm$' "$out/sgemm-omega.readelf.txt" \
  && grep -q '\.kernarg_segment_size: *36$' "$out/sgemm-omega.readelf.txt" \
  && grep -q '\.sgpr_count: *28$' "$out/sgemm-omega.readelf.txt" \
  && grep -q '\.vgpr_count: *15$' "$out/sgemm-omega.readelf.txt" \
  && grep -q '\.name: *K$' "$out/sgemm-omega.readelf.txt"; then
  ok 'sgemm llvm-readelf metadata reports six arguments and descriptor resources'
else bad 'sgemm llvm-readelf metadata'; fi
if python3 -I "$root/tests/amd/branches.py" "$out/hsaco" "$src/sgemm.isa" "$out"; then
  ok 'signed SOPP offset boundaries, undefined and duplicate labels'
else bad 'signed SOPP offset controls'; fi
compare "$out/sgemm-hsaco.hsaco" "$out/sgemm-hsaco-again.hsaco" 'sgemm original object preserved by all oracle tools'
echo "SGEMM OBJECT sha256=$(digest "$out/sgemm-hsaco.hsaco") bytes=$(wc -c < "$out/sgemm-hsaco.hsaco" | tr -d ' ')"
# The loader's ACCEPT is NOT a kernel execution check or proof of runtime-loader interoperability.
echo 'hardware NOT_RUN: gfx942 MI300X dispatch, FP32 outputs, n=0/tail/boundary vectors, ROCr loading and all other GPU rows'
echo 'NOT_RUN linked-oracle whole-object parity: gate uses ET_REL section parity; Spark has no ld.lld'
if [ "$fail" -ne 0 ]; then echo "AMD-GFX942-HOST FAIL pass=$pass fail=$fail hardware=NOT_RUN"; exit 1; fi
echo "AMD-GFX942-HOST PASS pass=$pass fail=0 kernels=vadd,fsum16,sgemm hardware=NOT_RUN"
exit 0
