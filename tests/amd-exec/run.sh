#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Host-only gfx942 instruction execution. Usage: sh tests/amd-exec/run.sh /absolute/pinned/omc
# AMD_EXEC_OUT optionally preserves artifacts. Production = seeded Omega; Python is test only.
# Pin refusal=69, usage=64, missing tool=3, unsupported host=77; hardware always NOT_RUN.
set -eu
LC_ALL=C
export LC_ALL
[ "$#" -eq 1 ] || { echo 'usage: run.sh /absolute/pinned/omc' >&2; exit 64; }
omc=$1
case $omc in /*) ;; *) exit 64 ;; esac
command -v sha256sum >/dev/null 2>&1 || exit 3
[ -f "$omc" ] && [ -x "$omc" ] || exit 3
pin=2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f
digest=$(sha256sum < "$omc")
[ "${digest%% *}" = "$pin" ] || { echo 'REFUSED compiler digest' >&2; exit 69; }
# No source read, output-directory creation, compilation, or producer run precedes this check.
[ "$(uname -s)" = Linux ] && [ "$(uname -m)" = aarch64 ] || exit 77
for tool in python3 cat cmp chmod mkdir mktemp dirname sed grep timeout; do
    command -v "$tool" >/dev/null 2>&1 || { echo "NOT_RUN missing $tool"; exit 3; }
done
root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
out=${AMD_EXEC_OUT:-$(mktemp -d "${TMPDIR:-/tmp}/gfx942-exec.XXXXXX")}
mkdir -p "$out"
out=$(CDPATH= cd -- "$out" && pwd)
src=$root/backend/amd/gfx942
printf 'PIN omc=%s\n' "$pin"
python3 --version
cat "$src/exec/fp.om" "$src/exec/fma.om" "$src/exec/main.om" > "$out/exec.om"
for name in hsaco exec; do
    input=$src/hsaco.om
    [ "$name" != exec ] || input=$out/exec.om
    for iteration in 1 2; do
        timeout 120 "$omc" < "$input" > "$out/$name.$iteration" 2> "$out/$name.$iteration.compile.log"
        [ ! -s "$out/$name.$iteration.compile.log" ] || { cat "$out/$name.$iteration.compile.log"; exit 1; }
        chmod 700 "$out/$name.$iteration"
    done
    cmp "$out/$name.1" "$out/$name.2"
    echo "PASS $name two builds byte-identical"
done
{
    cat "$src/exec/fp.om" "$src/exec/fma.om"
    sed '/^fn main to int/,$d' "$src/exec/main.om"
    cat "$root/tests/amd-exec/semantics-probe.om"
} > "$out/semantics.om"
mutants=$(python3 -I "$root/tests/amd-exec/semantics.py" mutate "$out/semantics.om" "$out")
for variant in semantics $mutants; do
    timeout 120 "$omc" < "$out/$variant.om" > "$out/$variant" 2> "$out/$variant.compile.log"
    [ ! -s "$out/$variant.compile.log" ] || { cat "$out/$variant.compile.log"; exit 1; }
    chmod 700 "$out/$variant"
    if [ "$variant" = semantics ]; then
        python3 -I "$root/tests/amd-exec/semantics.py" suite "$out/$variant" "$out"
    else
        python3 -I "$root/tests/amd-exec/semantics.py" control "$out/$variant" "$out" "$variant"
    fi
done
{ printf 'om2\n'; cat "$src/exec/fma.om" "$root/tests/amd-exec/fma-probe.om"; } > "$out/fma-probe.om"
timeout 120 "$omc" < "$out/fma-probe.om" > "$out/fma-probe" 2> "$out/fma-probe.compile.log"
[ ! -s "$out/fma-probe.compile.log" ] || { cat "$out/fma-probe.compile.log"; exit 1; }
chmod 700 "$out/fma-probe"
python3 -I "$root/tests/amd-exec/sgemm.py" probe "$out/fma-probe"
python3 -I "$root/tests/amd-exec/fp8gemv.py" mutate "$out/exec.om" "$out/exec-ocp.om"
timeout 120 "$omc" < "$out/exec-ocp.om" > "$out/exec-ocp" 2> "$out/exec-ocp.compile.log"
[ ! -s "$out/exec-ocp.compile.log" ] || { cat "$out/exec-ocp.compile.log"; exit 1; }
chmod 700 "$out/exec-ocp"
kernels='vadd fsum16 sgemm fp8gemv'
for kernel in $kernels; do
    for iteration in 1 2; do
        timeout 30 "$out/hsaco.$iteration" "$src/$kernel.isa" > "$out/$kernel.$iteration.hsaco"
    done
    cmp "$out/$kernel.1.hsaco" "$out/$kernel.2.hsaco"
    echo "PASS $kernel code object two builds byte-identical"
    python3 -I "$root/tests/amd-exec/oracle.py" extract "$out/$kernel.1.hsaco" "$out/$kernel.text"
done
digest=$(sha256sum < "$out/vadd.1.hsaco")
[ "${digest%% *}" = 10eb12e019ff2117ddc0190650899d4254589375c62e5ebb970b1bbb7e67e55f ]
echo 'PASS vadd code object unchanged from pre-generalization'
digest=$(sha256sum < "$out/fsum16.1.hsaco")
[ "${digest%% *}" = 15cf499fa114e801d931fb73cbee13fcb6e8764c08a470368306625a8df37478 ]
echo 'PASS fsum16 code object unchanged from pre-SGEMM'
digest=$(sha256sum < "$out/sgemm.1.hsaco")
[ "${digest%% *}" = 466d2a9e8ed61768ca1d09c283933dbae12f768a5bae22f5cf50f13c527d7e70 ]
echo 'PASS corrected sgemm code object unchanged from pre-FP8'
sed 's/^I 5 260 8 4 0x01 /I 5 261 8 4 0x01 /' "$src/vadd.isa" > "$out/flipped.isa"
if cmp -s "$src/vadd.isa" "$out/flipped.isa"; then echo 'FAIL mutant did not apply'; exit 1; fi
timeout 30 "$out/hsaco.1" "$out/flipped.isa" > "$out/flipped.hsaco"
python3 -I "$root/tests/amd-exec/oracle.py" extract "$out/flipped.hsaco" "$out/flipped.text"
python3 -I "$root/tests/amd-exec/oracle.py" suite "$out/exec.1" "$out/vadd.text" "$out/flipped.text" "$out"
python3 -I "$root/tests/amd-exec/oracle.py" reverse-table "$src/fsum16.isa" "$out/reversed.isa"
timeout 30 "$out/hsaco.1" "$out/reversed.isa" > "$out/reversed.hsaco"
python3 -I "$root/tests/amd-exec/oracle.py" extract "$out/reversed.hsaco" "$out/reversed.text"
python3 -I "$root/tests/amd-exec/oracle.py" fsum-suite "$out/exec.1" "$out/fsum16.text" "$out/reversed.text" "$out"
python3 -I "$root/tests/amd-exec/sgemm.py" mutant "$src/sgemm.isa" "$out/unfused.isa"
timeout 30 "$out/hsaco.1" "$out/unfused.isa" > "$out/unfused.hsaco"
python3 -I "$root/tests/amd-exec/oracle.py" extract "$out/unfused.hsaco" "$out/unfused.text"
python3 -I "$root/tests/amd-exec/sgemm.py" suite "$out/exec.1" "$out/sgemm.text" "$out/unfused.text" "$out"
python3 -I "$root/tests/amd-exec/fp8gemv.py" scale-mutant "$src/fp8gemv.isa" "$out/no-scale.isa"
timeout 30 "$out/hsaco.1" "$out/no-scale.isa" > "$out/no-scale.hsaco"
python3 -I "$root/tests/amd-exec/oracle.py" extract "$out/no-scale.hsaco" "$out/no-scale.text"
python3 -I "$root/tests/amd-exec/fp8gemv.py" suite "$out/exec.1" "$out/fp8gemv.text" "$out/exec-ocp" "$out/no-scale.text" "$out"
python3 -I "$root/tests/amd-exec/fp8gemv.py" flat-mutant "$out/exec.om" "$out/exec-flat.om"
timeout 120 "$omc" < "$out/exec-flat.om" > "$out/exec-flat" 2> "$out/exec-flat.compile.log"
[ ! -s "$out/exec-flat.compile.log" ] || { cat "$out/exec-flat.compile.log"; exit 1; }
chmod 700 "$out/exec-flat"
python3 -I "$root/tests/amd-exec/fp8gemv.py" long-wave "$out/exec.1" "$out/exec-flat" "$out/fp8gemv.text" "$out"
cat > "$out/wrong-omc" <<'WRONG'
#!/bin/sh
printf 'WRONG_COMPILER_EXECUTED\n' >&2
exit 99
WRONG
chmod 700 "$out/wrong-omc"
rc=0
AMD_EXEC_OUT="$out/wrong-pin-must-not-exist" sh "$root/tests/amd-exec/run.sh" "$out/wrong-omc" > "$out/wrong-pin.log" 2>&1 || rc=$?
[ "$rc" -eq 69 ] && grep -qx 'REFUSED compiler digest' "$out/wrong-pin.log" 
[ ! -e "$out/wrong-pin-must-not-exist" ]
if grep -q WRONG_COMPILER_EXECUTED "$out/wrong-pin.log"; then exit 1; fi
echo 'CONTROL wrong-compiler-digest CAUGHT exit=69 before compiler execution or output creation'
sha256sum "$out/hsaco.1" "$out/exec.1" "$out/vadd.1.hsaco" "$out/fsum16.1.hsaco" "$out/sgemm.1.hsaco" "$out/fp8gemv.1.hsaco"
python3 -I "$root/tests/amd-exec/sgemm.py" summary "$kernels"
