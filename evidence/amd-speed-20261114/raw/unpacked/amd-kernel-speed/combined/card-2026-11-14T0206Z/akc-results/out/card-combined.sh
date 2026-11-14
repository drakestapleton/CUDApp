#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Combined MI300X run (uxl-brief/AmdKernelSpeed; measuring script, not Omega). cwd /workspace/akc after pre.sh.
# Arms, each a full 256-token lighthouse decode, interleaved over 3 repetitions:
#   A  published launcher 08a50491, mode plan,  real2.plan, v1 objects (baseline)
#   B  AmdLaunchGap launcher de67f27e, mode plan3, real2.plan, v1 objects
#   C  B + rmsnorm5120 v2 + gdnstate_step v2
#   D1 C + fp8gemv_b128f v2b1      D2 C + fp8gemv_b128f v2b2
#   E1 D1 on cap286.plan            E2 D2 on cap286.plan
# Every launch runs under timeout 150 s (two launcher hangs were seen on 2026-11-14), exit 124 marks a hang.
# then the altered-weights controls plan under E2's launcher and objects (must not print the published ids).
cd /workspace/akc || exit 74
exec > combined.log 2>&1
while ! grep -q PRE_DONE pre.log; do sleep 5; done
grep -q 'PREPARE_RC=0' pre.log || { echo PREPARE_FAILED; echo COMBINED_DONE; exit 1; }
set -x
export AMDSERVE_WEIGHTS=/tmp/qwen3.8-27b-fp8
sha256sum -c objects.sha256 || { echo OBJECT_DIGEST_MISMATCH; echo COMBINED_DONE; exit 69; }
E=CUDApp/evidence/amd-qwen38-native-20261113
mkdir -p plan runs
python3 $E/oracle/emit-plan.py $E/oracle/driver-real2.py "$AMDSERVE_WEIGHTS" plan/real2.plan --capacity=131072 --fixture-json=$E/oracle/lighthouse256.json; echo "EMIT_RC=$?"
[ "$(sha256sum < plan/real2.plan | cut -d' ' -f1)" = 2a292dadc236f224005ed4e2d8ac2ba8b33dc71d79dabcc8497d177561f9c012 ] || { echo PLAN_DIGEST_MISMATCH; echo COMBINED_DONE; exit 1; }
python3 $E/oracle/emit-plan.py $E/oracle/driver-real2.py "$AMDSERVE_WEIGHTS" plan/cap286.plan --capacity=286 --fixture-json=$E/oracle/lighthouse256.json; echo "EMIT286_RC=$?"
python3 $E/oracle/emit-plan.py $E/oracle/driver-real2.py "$AMDSERVE_WEIGHTS" plan/real2-controls.plan --controls --capacity=131072 --fixture-json=$E/oracle/lighthouse256.json; echo "EMITC_RC=$?"
mkdir -p bin-old bin-new && cp launch-x86 bin-old/omega-launch && cp launch-new bin-new/omega-launch && chmod 755 bin-*/omega-launch
mkset() { # NAME OBJECT...   a directory with the plans (symlinks), their .cmd files and its own code/ copy
  d=sets/$1; shift; mkdir -p $d && cp -r plan/code $d/code
  for p in real2 cap286 real2-controls; do ln -sf ../../plan/$p.plan $d/$p.plan; cp plan/$p.plan.cmd $d/; done
  for o in "$@"; do cp "$o" $d/code/$(echo "$o" | sed 's/-v2[a-z0-9]*\.hsaco$/.hsaco/'); done
  sha256sum $d/code/*.hsaco > $d/code.sha256
}
mkset v1
mkset k rmsnorm5120-v2.hsaco gdnstate_step-v2.hsaco
mkset k1 rmsnorm5120-v2.hsaco gdnstate_step-v2.hsaco fp8gemv_b128f-v2b1.hsaco
mkset k2 rmsnorm5120-v2.hsaco gdnstate_step-v2.hsaco fp8gemv_b128f-v2b2.hsaco
run() { # LABEL BIN MODE SET PLANBASE
  sed "s/^omega-launch plan /omega-launch $3 /" sets/$4/$5.plan.cmd > sets/$4/cmd-$1.sh
  echo "RUN $1 start $(date +%s.%N)"
  (cd sets/$4 && PATH="$PWD/../../$2:$PATH" timeout -k 5 150 sh cmd-$1.sh > ../../runs/$1.txt 2> ../../runs/$1.err); echo "RUN $1 rc=$? end $(date +%s.%N)"
}
for r in 1 2 3; do
  run A-$r bin-old plan v1 real2
  run B-$r bin-new plan3 v1 real2
  run C-$r bin-new plan3 k real2
  run D1-$r bin-new plan3 k1 real2
  run D2-$r bin-new plan3 k2 real2
  run E1-$r bin-new plan3 k1 cap286
  run E2-$r bin-new plan3 k2 cap286
done
run control bin-new plan3 k2 real2-controls
sha256sum launch-x86 launch-new plan/*.plan sets/*/code.sha256 card-combined.sh
echo COMBINED_DONE
