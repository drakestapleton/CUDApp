# Controls, run from SCRATCH/C. Separate fresh clone so the Run B tree stays untouched.
set -x
date -u +%FT%TZ
git clone -q https://www.aienos.com/rad/z4RxD8vXigmWg61nM8miQ8DVPAXEP.git CUDApp; echo "### rc=$?"
git -C CUDApp checkout -q dc536882c06a692991be97dc65cd2d037d82b129; echo "### rc=$?"
git -C CUDApp rev-parse HEAD
# --- control 1: compiler with exactly one byte changed (last byte XOR 0x01) ---
cp ../B/omega-mv/seed/out/omc omc-onebyte
python3 -c "
import pathlib; p=pathlib.Path('omc-onebyte'); b=bytearray(p.read_bytes()); b[-1]^=1; p.write_bytes(b)"
cmp -l ../B/omega-mv/seed/out/omc omc-onebyte | wc -l
sha256sum ../B/omega-mv/seed/out/omc omc-onebyte
cd CUDApp
ls -d build 2>&1
python3 -I tests/amd-kfd-launch/check.py "$(realpath ../omc-onebyte)"; echo "### rc=$? wrong-compiler (one byte changed)"
ls -d build 2>&1; echo "### ls-build rc=$? (2 = no build directory was created)"
git status --short --ignored | head -5; echo "### git status lines above (none = tree untouched)"
# --- control 2: the path form the brief's realpath protects against (relative path) ---
python3 -I tests/amd-kfd-launch/check.py ../../B/omega-mv/seed/out/omc; echo "### rc=$? relative compiler path"
ls -d build 2>&1
# --- qemu-absent run: PATH is a symlink farm of /usr/bin without qemu-x86_64* ---
mkdir -p ../noqemu-bin
for f in /usr/bin/*; do b=$(basename "$f"); case "$b" in qemu-x86_64*) ;; *) ln -sf "$f" ../noqemu-bin/"$b";; esac; done
NQ=$(realpath ../noqemu-bin)
env PATH="$NQ" sh -c 'command -v qemu-x86_64; echo "### command -v qemu rc=$?"; command -v python3 cc'
env PATH="$NQ" python3 -I tests/amd-kfd-launch/check.py "$(realpath ../../B/omega-mv/seed/out/omc)" > ../noqemu-gate-stdout.log 2> ../noqemu-gate-stderr.log; echo "### rc=$? gate without qemu-x86_64 on PATH"
tail -2 ../noqemu-gate-stdout.log | cut -c1-240; tail -5 ../noqemu-gate-stderr.log
grep -c '^PASS ' ../noqemu-gate-stdout.log; grep -c '^FAIL ' ../noqemu-gate-stdout.log
python3 -c "
import json; s=json.load(open('build/amd-kfd-launch/gate/summary.json')); print('summary checks',s['checks'],'failures',s['failures'],'qemu',s['qemu'])"
sha256sum build/amd-kfd-launch/gate/launch-x86 build/amd-kfd-launch/gate/launch-a64 build/amd-kfd-launch/gate/vadd.hsaco
# --- Level 1 tamper control (this throwaway clone only): change one byte of one published raw file ---
python3 -I evidence/amd-qwen38-native-refusal-20261113/verify.py; echo "### rc=$? verify.py clean"
python3 -c "
import pathlib; p=pathlib.Path('evidence/amd-qwen38-native-refusal-20261113/native/sample1.stdout'); b=bytearray(p.read_bytes()); b[-2]^=1; p.write_bytes(b)"
git status --short
python3 -I evidence/amd-qwen38-native-refusal-20261113/verify.py; echo "### rc=$? verify.py after one-byte tamper of native/sample1.stdout"
date -u +%FT%TZ
