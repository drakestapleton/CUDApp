D=/home/drakestapleton/work/hive/uxl-brief-20261113/PublicRebuild
echo "=== D1: same symlink-farm PATH as the qemu-absent run, plus qemu-x86_64 (shows the farm itself is not the cause) ==="
cd $D/C/CUDApp
git checkout -q -- evidence/amd-qwen38-native-refusal-20261113/native/sample1.stdout; git status --short; echo "### tree restored rc=$?"
rm -rf build
cp -a ../noqemu-bin ../withqemu-bin && ln -s /usr/bin/qemu-x86_64 ../withqemu-bin/qemu-x86_64
WQ=$(realpath ../withqemu-bin)
t0=$(date +%s.%N)
env PATH="$WQ" python3 -I tests/amd-kfd-launch/check.py "$(realpath ../../B/omega-mv/seed/out/omc)" > ../withqemu-gate-stdout.log 2> ../withqemu-gate-stderr.log; echo "### rc=$? gate with farm PATH + qemu-x86_64"
t1=$(date +%s.%N); echo "seconds $(echo "$t1 - $t0" | bc)"
grep -c '^PASS ' ../withqemu-gate-stdout.log; grep -c '^FAIL ' ../withqemu-gate-stdout.log; grep -o '^AMD-KFD-LAUNCH CPU PASS {"binaries": {"a64": "[0-9a-f]*", "x86": "[0-9a-f]*"}, "checks": [0-9]*' ../withqemu-gate-stdout.log
python3 -c "
import json; s=json.load(open('build/amd-kfd-launch/gate/summary.json')); print('summary checks',s['checks'],'failures',s['failures'],'qemu',s['qemu'])"
echo "=== D2: seed/omega-from-seed.sh in a local copy of the omega-mv clone, bounded by timeout 600 ==="
cd $D/S
git clone -q --no-hardlinks ../B/omega-mv omega-mv; echo "### rc=$? local clone"
git -C omega-mv checkout -q 2ee0a70d1830581b706f288e722f3329d51f3583; git -C omega-mv rev-parse HEAD
cd omega-mv
sha256sum seed/out/omc
mv seed/out/omc ../omc.checked-in; echo "### moved checked-in omc aside rc=$? (so a pass cannot come from the committed file)"
t0=$(date +%s.%N)
timeout 600 sh seed/omega-from-seed.sh > ../from-seed.log 2>&1; echo "### rc=$? sh seed/omega-from-seed.sh (124 = hit the 600 s bound)"
t1=$(date +%s.%N); echo "seconds $(echo "$t1 - $t0" | bc)"
tail -25 ../from-seed.log | cut -c1-200
sha256sum seed/out/omc ../omc.checked-in; cmp seed/out/omc ../omc.checked-in && echo "seed-built omc is byte-identical to the checked-in omc"
git status --short | head -10; echo "### git status lines above (none = tracked files unchanged)"
