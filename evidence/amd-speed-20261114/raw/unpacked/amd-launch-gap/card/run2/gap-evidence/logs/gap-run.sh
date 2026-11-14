#!/bin/sh
# Launch-gap card run 2: published launcher (plan), new launcher (plan3) and HIP graph replay of the
# same 18 kernels, interleaved in one session; then plan2 for attribution and the altered-weights controls.
G=/workspace/gap2
L=$G/logs
mkdir -p $L
exec > $G/run.log 2>&1
set -x
if test -d /tmp/qwen3.8-27b-fp8 && test -f /workspace/gap/pre.log && grep -q 'PREPARE_RC=0' /workspace/gap/pre.log; then echo REUSE_WEIGHTS; fi
while ! grep -q PRE_DONE $G/pre.log; do sleep 5; done
grep -q 'PREPARE_RC=0' $G/pre.log || { echo PREPARE_FAILED; exit 1; }
while ! test -f $G/bin-upload.json; do sleep 5; done
sha256sum $G/bin-old/omega-launch $G/bin-new/omega-launch
echo "08a504911ec6e87a9746632a14d22245155597e3a8ed384c93c0e5260bb691e1  $G/bin-old/omega-launch" | sha256sum -c || exit 1
echo "de67f27eb4c12aa8059b1dfa24ff419095bde6ca4412c3ba7a1b82230405a738  $G/bin-new/omega-launch" | sha256sum -c || exit 1
{ date -u +%FT%TZ; uname -a; cat /opt/rocm/.info/version; rocminfo 2>/dev/null | grep -E 'Marketing Name|Name: +gfx' | head -6;
  python3 -c 'import torch;print("torch",torch.__version__,torch.version.hip)'; nproc; grep -m1 'model name' /proc/cpuinfo; } > $L/card.txt 2>&1
cd $G/CUDApp/evidence/amd-qwen38-native-20261113
export AMDSERVE_WEIGHTS=/tmp/qwen3.8-27b-fp8
mkdir -p out
python3 oracle/emit-plan.py oracle/driver-real2.py "$AMDSERVE_WEIGHTS" out/real2.plan --capacity=131072 --fixture-json=oracle/lighthouse256.json; echo "EMIT_RC=$?"
python3 oracle/check-plan.py out/real2.plan "$AMDSERVE_WEIGHTS"; echo "CHECK_RC=$?"
python3 oracle/emit-plan.py oracle/driver-real2.py "$AMDSERVE_WEIGHTS" out/real2-controls.plan --controls --capacity=131072 --fixture-json=oracle/lighthouse256.json; echo "EMITC_RC=$?"
python3 oracle/check-plan.py out/real2-controls.plan "$AMDSERVE_WEIGHTS"; echo "CHECKC_RC=$?"
sha256sum out/real2.plan out/real2-controls.plan out/code/*.hsaco > $L/plan-and-code.sha256
cat $L/plan-and-code.sha256
cd out
for mode in plan plan2 plan3; do
  sed "s/^omega-launch plan /omega-launch $mode /" real2.plan.cmd > cmd-$mode.sh
  sed "s/^omega-launch plan /omega-launch $mode /" real2-controls.plan.cmd > cmdc-$mode.sh
done
cp real2.plan.cmd real2-controls.plan.cmd cmd-*.sh cmdc-*.sh $L/
omega() { # LABEL BINDIR CMDFILE
  echo "$1 start $(date -u +%FT%T.%NZ)"
  PATH="$2:$PATH" sh "$3" > $L/omega-$1.txt 2> $L/omega-$1.err; echo "$1 rc=$?"
  echo "$1 end $(date -u +%FT%T.%NZ)"; grep -c OMEGA_STEP $L/omega-$1.txt
}
hip() { # LABEL
  echo "$1 start $(date -u +%FT%T.%NZ)"
  (cd .. && python3 $G/gap-hip.py oracle/driver-real2.py "$AMDSERVE_WEIGHTS" oracle/lighthouse256.json $L/hip-$1.json 131072 > $L/hip-$1.txt 2>&1); echo "$1 rc=$?"
  echo "$1 end $(date -u +%FT%T.%NZ)"; tail -1 $L/hip-$1.txt | cut -c1-400
}
for r in warmup run1 run2 run3 run4 run5; do
  omega old-plan-$r $G/bin-old cmd-plan.sh
  omega new-plan3-$r $G/bin-new cmd-plan3.sh
  hip graph-$r
  echo "ROUND_DONE $r"
done
omega control-new-plan3 $G/bin-new cmdc-plan3.sh
echo MAIN_DONE
for r in run1 run2; do
  omega new-plan2-$r $G/bin-new cmd-plan2.sh
done
echo ATTRIBUTION_DONE
cd $G
python3 gap-verdict.py $L published-ids.json > $L/verdict.txt 2>&1; echo "VERDICT_RC=$?"
cat $L/verdict.txt
cp pre.log run.log gap-*.sh gap-*.py published-ids.json $L/ 2>/dev/null
cd $L && sha256sum $(ls | grep -v SHA256SUMS) > SHA256SUMS
cd $G && tar czf gap-evidence.tar.gz logs && sha256sum gap-evidence.tar.gz > gap-evidence.tar.gz.sha256
cat gap-evidence.tar.gz.sha256
echo RUN_DONE
