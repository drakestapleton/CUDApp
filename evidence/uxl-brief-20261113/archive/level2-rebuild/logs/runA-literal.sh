echo "=== Level 1 block ==="
git clone https://www.aienos.com/rad/z4RxD8vXigmWg61nM8miQ8DVPAXEP.git CUDApp
echo "### rc=$? pwd=$(pwd | sed s,/home/drakestapleton/work/hive/uxl-brief-20261113/PublicRebuild,SCRATCH,)"
cd CUDApp
echo "### rc=$? pwd=$(pwd | sed s,/home/drakestapleton/work/hive/uxl-brief-20261113/PublicRebuild,SCRATCH,)"
git checkout dc536882c06a692991be97dc65cd2d037d82b129
echo "### rc=$? pwd=$(pwd | sed s,/home/drakestapleton/work/hive/uxl-brief-20261113/PublicRebuild,SCRATCH,)"
python3 -I evidence/amd-qwen38-native-refusal-20261113/verify.py
echo "### rc=$? pwd=$(pwd | sed s,/home/drakestapleton/work/hive/uxl-brief-20261113/PublicRebuild,SCRATCH,)"
echo "=== Level 2 block ==="
git clone https://www.aienos.com/rad/z6do9WDBhuSnvopDkgFdFr1yWA62.git omega-mv
echo "### rc=$? pwd=$(pwd | sed s,/home/drakestapleton/work/hive/uxl-brief-20261113/PublicRebuild,SCRATCH,)"
git -C omega-mv checkout 2ee0a70d1830581b706f288e722f3329d51f3583
echo "### rc=$? pwd=$(pwd | sed s,/home/drakestapleton/work/hive/uxl-brief-20261113/PublicRebuild,SCRATCH,)"
sha256sum omega-mv/seed/out/omc
echo "### rc=$? pwd=$(pwd | sed s,/home/drakestapleton/work/hive/uxl-brief-20261113/PublicRebuild,SCRATCH,)"
#   26540e6de62402d5784b30b24d7c6faae488137c1f8a4b3e784fd0b5d5528991

cd CUDApp
echo "### rc=$? pwd=$(pwd | sed s,/home/drakestapleton/work/hive/uxl-brief-20261113/PublicRebuild,SCRATCH,)"
python3 -I tests/amd-kfd-launch/check.py "$(realpath ../omega-mv/seed/out/omc)"
echo "### rc=$? pwd=$(pwd | sed s,/home/drakestapleton/work/hive/uxl-brief-20261113/PublicRebuild,SCRATCH,)"
#   ends with: AMD-KFD-LAUNCH CPU PASS {...}
sha256sum build/amd-kfd-launch/gate/launch-x86
echo "### rc=$? pwd=$(pwd | sed s,/home/drakestapleton/work/hive/uxl-brief-20261113/PublicRebuild,SCRATCH,)"
#   08a504911ec6e87a9746632a14d22245155597e3a8ed384c93c0e5260bb691e1
