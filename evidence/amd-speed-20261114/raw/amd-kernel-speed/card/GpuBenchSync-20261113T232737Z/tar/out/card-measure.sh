#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Card side of the per-kernel measurement (measuring instrument; the launcher and kernels are the unmodified seeded-Omega
# objects of CUDApp dc536882). Runs on the AMD One-Click MI300X notebook, cwd /workspace/aks, after pre.sh printed PRE_DONE.
# Every launcher process is the published x86 launcher (sha256 below) on a plan derived byte-for-byte from the card's own
# emitted real2.plan, which must equal the published sha256 before anything runs.
cd /workspace/aks || exit 74
exec > measure.log 2>&1
while ! grep -q PRE_DONE pre.log; do sleep 5; done
grep -q 'PREPARE_RC=0' pre.log || { echo PREPARE_FAILED; echo MEASURE_DONE; exit 1; }
set -x
LAUNCH_SHA=08a504911ec6e87a9746632a14d22245155597e3a8ed384c93c0e5260bb691e1
PLAN_SHA=2a292dadc236f224005ed4e2d8ac2ba8b33dc71d79dabcc8497d177561f9c012
export AMDSERVE_WEIGHTS=/tmp/qwen3.8-27b-fp8
[ "$(sha256sum < launch-x86 | cut -d' ' -f1)" = "$LAUNCH_SHA" ] || { echo LAUNCHER_DIGEST_MISMATCH; echo MEASURE_DONE; exit 69; }
E=CUDApp/evidence/amd-qwen38-native-20261113
mkdir -p plan runs
python3 $E/oracle/emit-plan.py $E/oracle/driver-real2.py "$AMDSERVE_WEIGHTS" plan/real2.plan --capacity=131072 --fixture-json=$E/oracle/lighthouse256.json; echo "EMIT_RC=$?"
[ "$(sha256sum < plan/real2.plan | cut -d' ' -f1)" = "$PLAN_SHA" ] || { echo PLAN_DIGEST_MISMATCH; echo MEASURE_DONE; exit 1; }
cp launch-x86 plan/omega-launch; chmod 755 plan/omega-launch
args=$(tr ' ' '\n' < plan/real2.plan.cmd | tail -n +4 | tr '\n' ' ')
run() { # NAME PLANFILE
  echo "RUN $1 start $(date +%s.%N)"
  (cd plan && PATH="$PWD:$PATH" omega-launch plan "$2" $args > ../runs/$1.txt 2> ../runs/$1.err); echo "RUN $1 rc=$? end $(date +%s.%N)"
}
amp() { python3 amplify-plan.py write plan/real2.plan plan/v.plan "$1" "$2" > runs/$3.variant && run "$3" v.plan; rm -f plan/v.plan; }
# 1. Identity gate: the base plan, full 256 generated tokens, must print the published ids (checked in analyze.py).
run full-a real2.plan
# 2. Per-class insertion at 64 generated tokens, with the unamplified 64-token base at start, middle and end for drift.
amp base 64 base-1
CLASSES=$(python3 amplify-plan.py list plan/real2.plan | python3 -c 'import json,sys;print(" ".join(c for c in json.load(sys.stdin)["classes"] if c!="step_advance"))')
i=0
for c in empty:1 empty:17408 $CLASSES; do
  amp "$c" 64 "$(echo "$c" | tr ':' '_')"
  i=$((i+1)); [ "$i" = 14 ] && amp base 64 base-2
done
amp base 64 base-3
# 3. Control: altered-weights plan (published controls) must not print the golden ids.
python3 $E/oracle/emit-plan.py $E/oracle/driver-real2.py "$AMDSERVE_WEIGHTS" plan/real2-controls.plan --controls --capacity=131072 --fixture-json=$E/oracle/lighthouse256.json; echo "EMITC_RC=$?"
python3 amplify-plan.py write plan/real2-controls.plan plan/c.plan base 64 > runs/control.variant && run control c.plan
run full-b real2.plan
# 4. Plan-level candidate, no kernel change: attention part grid sized for the 286 positions this request uses instead of
#    131072 (2048x24 -> 5x24 workgroups per layer). Output identity is checked on all 256 ids (analyze.py, run cap286).
python3 $E/oracle/emit-plan.py $E/oracle/driver-real2.py "$AMDSERVE_WEIGHTS" plan/cap286.plan --capacity=286 --fixture-json=$E/oracle/lighthouse256.json; echo "EMIT286_RC=$?"
run cap286 cap286.plan
# 5. Kernel candidate rmsnorm5120 v2 (same operations, loads issued before one wait; CPU gate byte-identical):
#    swap the code object file only, on the full plan and on the capacity-286 plan. All 256 ids are checked.
V2=e7d75a038b5c7dd9c72540e72607805dd63d70c27cc22708b518c7e0cfbca062
[ "$(sha256sum < rmsnorm5120-v2.hsaco | cut -d' ' -f1)" = "$V2" ] || { echo V2_DIGEST_MISMATCH; echo MEASURE_DONE; exit 69; }
cp plan/code/rmsnorm5120.hsaco rmsnorm5120-v1.hsaco && cp rmsnorm5120-v2.hsaco plan/code/rmsnorm5120.hsaco
run v2rms real2.plan
run v1all cap286.plan
cp rmsnorm5120-v1.hsaco plan/code/rmsnorm5120.hsaco
sha256sum launch-x86 plan/real2.plan plan/real2-controls.plan plan/cap286.plan amplify-plan.py card-measure.sh
echo MEASURE_DONE
