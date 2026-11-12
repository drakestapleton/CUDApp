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
cat "$src/exec/fp.om" "$src/exec/main.om" > "$out/exec.om"
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
for iteration in 1 2; do
    timeout 30 "$out/hsaco.$iteration" "$src/vadd.isa" > "$out/vadd.$iteration.hsaco"
done
cmp "$out/vadd.1.hsaco" "$out/vadd.2.hsaco"
echo 'PASS code object two builds byte-identical'
python3 "$root/tests/amd-exec/oracle.py" extract "$out/vadd.1.hsaco" "$out/vadd.text"
sed 's/^I 5 260 8 4 0x01 /I 5 261 8 4 0x01 /' "$src/vadd.isa" > "$out/flipped.isa"
if cmp -s "$src/vadd.isa" "$out/flipped.isa"; then echo 'FAIL mutant did not apply'; exit 1; fi
timeout 30 "$out/hsaco.1" "$out/flipped.isa" > "$out/flipped.hsaco"
python3 "$root/tests/amd-exec/oracle.py" extract "$out/flipped.hsaco" "$out/flipped.text"
python3 "$root/tests/amd-exec/oracle.py" suite "$out/exec.1" "$out/vadd.text" "$out/flipped.text" "$out"
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
sha256sum "$out/hsaco.1" "$out/exec.1" "$out/vadd.1.hsaco"
echo 'AMD-GFX942-EXEC PASS counts=0,1,63,64,65,200 hardware=NOT_RUN'
