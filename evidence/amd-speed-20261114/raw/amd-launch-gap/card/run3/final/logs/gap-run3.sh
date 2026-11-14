#!/bin/sh
# Launch-gap card run 3 (residual diagnosis). Part A, no weights: launch micro-benchmark, N=1 and N=1126
# step_advance dispatches per step, Omega plan3 / plan3c / plan3t vs HIP graph, strictly interleaved
# cycles. Part B, weights: strictly interleaved cycles of plan3, plan3c, HIP graph (1 warmup + 9),
# then the published launcher (plan) x3, plan3t x1 and the plan3 altered-weights control.
G=/workspace/gap3
L=$G/logs
mkdir -p $L $G/micro
exec > $G/run.log 2>&1
set -x
while ! test -f $G/bin-upload.json; do sleep 5; done
echo "08a504911ec6e87a9746632a14d22245155597e3a8ed384c93c0e5260bb691e1  $G/bin-old/omega-launch" | sha256sum -c || exit 1
echo "a6aee5ef6f144ada59941197c72e3ca8c84fab1f448b99484bf727f67982afc7  $G/bin-new/omega-launch" | sha256sum -c || exit 1
{ date -u +%FT%TZ; uname -a; cat /opt/rocm/.info/version; rocminfo 2>/dev/null | grep -E 'Marketing Name|Name: +gfx' | head -6; nproc; grep -m1 'model name' /proc/cpuinfo; } > $L/card.txt 2>&1
DRV=$G/CUDApp/evidence/amd-qwen38-native-20261113/oracle/driver-real2.py
while ! test -f $DRV; do sleep 5; done
python3 $G/gap-microplan.py $DRV 1126 100 $G/micro/m1126.plan $G/micro; python3 $G/gap-microplan.py $DRV 1 100 $G/micro/m1.plan $G/micro
sha256sum $G/micro/* > $L/micro.sha256
micro() { # LABEL MODE PLAN
  echo "$1 start $(date -u +%FT%T.%NZ)"
  $G/bin-new/omega-launch $2 $3 $G/micro/step_advance.hsaco > $L/micro-$1.txt 2> $L/micro-$1.err; echo "$1 rc=$?"
}
hipmicro() { # LABEL N
  echo "$1 start $(date -u +%FT%T.%NZ)"
  python3 $G/gap-hipmicro.py $DRV $2 100 $L/hipmicro-$1.json > $L/hipmicro-$1.txt 2>&1; echo "$1 rc=$?"
}
for c in 0 1 2 3 4 5 6 7 8 9; do
  micro plan3-n1-c$c plan3 $G/micro/m1.plan
  micro plan3c-n1-c$c plan3c $G/micro/m1.plan
  hipmicro hip-n1-c$c 1
  micro plan3-n1126-c$c plan3 $G/micro/m1126.plan
  micro plan3c-n1126-c$c plan3c $G/micro/m1126.plan
  hipmicro hip-n1126-c$c 1126
  echo "MICRO_CYCLE_DONE $c"
done
micro plan3t-n1 plan3t $G/micro/m1.plan
micro plan3t-n1126 plan3t $G/micro/m1126.plan
echo MICRO_DONE
while ! grep -q PRE_DONE $G/pre.log; do sleep 5; done
grep -q 'PREPARE_RC=0' $G/pre.log || { echo PREPARE_FAILED; exit 1; }
cd $G/CUDApp/evidence/amd-qwen38-native-20261113
export AMDSERVE_WEIGHTS=/tmp/qwen3.8-27b-fp8
mkdir -p out
python3 oracle/emit-plan.py oracle/driver-real2.py "$AMDSERVE_WEIGHTS" out/real2.plan --capacity=131072 --fixture-json=oracle/lighthouse256.json; echo "EMIT_RC=$?"
python3 oracle/check-plan.py out/real2.plan "$AMDSERVE_WEIGHTS"; echo "CHECK_RC=$?"
python3 oracle/emit-plan.py oracle/driver-real2.py "$AMDSERVE_WEIGHTS" out/real2-controls.plan --controls --capacity=131072 --fixture-json=oracle/lighthouse256.json; echo "EMITC_RC=$?"
sha256sum out/real2.plan out/real2-controls.plan out/code/*.hsaco > $L/plan-and-code.sha256
cd out
for mode in plan plan3 plan3c plan3t; do
  sed "s/^omega-launch plan /omega-launch $mode /" real2.plan.cmd > cmd-$mode.sh
  sed "s/^omega-launch plan /omega-launch $mode /" real2-controls.plan.cmd > cmdc-$mode.sh
done
cp real2.plan.cmd cmd-*.sh cmdc-*.sh $L/
omega() { # LABEL BINDIR CMDFILE
  echo "$1 start $(date -u +%FT%T.%NZ)"
  PATH="$2:$PATH" sh "$3" > $L/omega-$1.txt 2> $L/omega-$1.err; echo "$1 rc=$?"
}
hip() { # LABEL
  echo "$1 start $(date -u +%FT%T.%NZ)"
  (cd .. && python3 $G/gap-hip.py oracle/driver-real2.py "$AMDSERVE_WEIGHTS" oracle/lighthouse256.json $L/hip-$1.json 131072 > $L/hip-$1.txt 2>&1); echo "$1 rc=$?"
}
for c in warmup c1 c2 c3 c4 c5 c6 c7 c8 c9; do
  omega new-plan3-$c $G/bin-new cmd-plan3.sh
  omega new-plan3c-$c $G/bin-new cmd-plan3c.sh
  hip graph-$c
  echo "ROUND_DONE $c"
done
omega new-plan3t-c1 $G/bin-new cmd-plan3t.sh
for c in c1 c2 c3; do omega old-plan-$c $G/bin-old cmd-plan.sh; done
omega control-new-plan3 $G/bin-new cmdc-plan3.sh
echo MAIN_DONE
cd $G
python3 gap-verdict.py $L published-ids.json > $L/verdict.txt 2>&1; echo "VERDICT_RC=$?"
cp pre.log run.log gap-*.sh gap-*.py published-ids.json $L/ 2>/dev/null
cd $L && sha256sum $(ls | grep -v SHA256SUMS) > SHA256SUMS
cd $G && tar czf gap-evidence.tar.gz logs && sha256sum gap-evidence.tar.gz > gap-evidence.tar.gz.sha256
echo RUN_DONE
