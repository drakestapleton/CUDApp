#!/bin/sh
# Launch-gap card run 4: buffer-placement hypotheses. Strictly interleaved cycles (1 warmup + 9) of
# plan3, plan3a (2 MiB buffer placement), plan3v (token cell in VRAM) and HIP graph; then the plan3v
# altered-weights control.
G=/workspace/gap4
L=$G/logs
mkdir -p $L
exec > $G/run.log 2>&1
set -x
while ! test -f $G/bin-upload.json; do sleep 5; done
echo "657db853484df0f6ad1a2ac861522f16cce5542283201b53d60b4924cb16c3be  $G/bin-new/omega-launch" | sha256sum -c || exit 1
while ! grep -q PRE_DONE $G/pre.log; do sleep 5; done
grep -q 'PREPARE_RC=0' $G/pre.log || { echo PREPARE_FAILED; exit 1; }
{ date -u +%FT%TZ; uname -a; rocminfo 2>/dev/null | grep -E 'Marketing Name|Name: +gfx' | head -4; } > $L/card.txt 2>&1
cd $G/CUDApp/evidence/amd-qwen38-native-20261113
export AMDSERVE_WEIGHTS=/tmp/qwen3.8-27b-fp8
mkdir -p out
python3 oracle/emit-plan.py oracle/driver-real2.py "$AMDSERVE_WEIGHTS" out/real2.plan --capacity=131072 --fixture-json=oracle/lighthouse256.json; echo "EMIT_RC=$?"
python3 oracle/check-plan.py out/real2.plan "$AMDSERVE_WEIGHTS"; echo "CHECK_RC=$?"
python3 oracle/emit-plan.py oracle/driver-real2.py "$AMDSERVE_WEIGHTS" out/real2-controls.plan --controls --capacity=131072 --fixture-json=oracle/lighthouse256.json; echo "EMITC_RC=$?"
python3 $G/gap-tokvram.py out/real2.plan out/real2v.plan; python3 $G/gap-tokvram.py out/real2-controls.plan out/real2v-controls.plan
sha256sum out/*.plan out/code/*.hsaco > $L/plan-and-code.sha256
cd out
for mode in plan3 plan3a; do sed "s/^omega-launch plan /omega-launch $mode /" real2.plan.cmd > cmd-$mode.sh; done
sed "s/^omega-launch plan real2.plan /omega-launch plan3v real2v.plan /" real2.plan.cmd > cmd-plan3v.sh
sed "s/^omega-launch plan real2-controls.plan /omega-launch plan3v real2v-controls.plan /" real2-controls.plan.cmd > cmdc-plan3v.sh
grep -c "plan3v real2v" cmd-plan3v.sh cmdc-plan3v.sh
cp real2.plan.cmd cmd-*.sh cmdc-*.sh $L/
omega() { echo "$1 start $(date -u +%FT%T.%NZ)"; PATH="$G/bin-new:$PATH" sh "$2" > $L/omega-$1.txt 2> $L/omega-$1.err; echo "$1 rc=$?"; }
hip() { echo "$1 start $(date -u +%FT%T.%NZ)"; (cd .. && python3 $G/gap-hip.py oracle/driver-real2.py "$AMDSERVE_WEIGHTS" oracle/lighthouse256.json $L/hip-$1.json 131072 > $L/hip-$1.txt 2>&1); echo "$1 rc=$?"; }
for c in warmup c1 c2 c3 c4 c5 c6 c7 c8 c9; do
  omega new-plan3-$c cmd-plan3.sh
  omega new-plan3a-$c cmd-plan3a.sh
  omega new-plan3v-$c cmd-plan3v.sh
  hip graph-$c
  echo "ROUND_DONE $c"
done
omega control-new-plan3v cmdc-plan3v.sh
echo MAIN_DONE
cd $G
python3 gap-verdict.py $L published-ids.json > $L/verdict.txt 2>&1; echo "VERDICT_RC=$?"
cp pre.log run.log gap-*.sh gap-*.py published-ids.json $L/ 2>/dev/null
cd $L && sha256sum $(ls | grep -v SHA256SUMS) > SHA256SUMS
cd $G && tar czf gap-evidence.tar.gz logs && sha256sum gap-evidence.tar.gz > gap-evidence.tar.gz.sha256
echo RUN_DONE
