# Brief section 6 Level 3, literal commands, then same-session references
cd /workspace/uxl
while ! grep -q PRE_DONE pre.log; do sleep 5; done
grep -q 'PREPARE_RC=0' pre.log || { echo PREPARE_FAILED; exit 1; }
cd CUDApp/evidence/amd-qwen38-native-20261113
export AMDSERVE_WEIGHTS=/tmp/qwen3.8-27b-fp8
set -x
date -u +%FT%TZ
mkdir out
python3 oracle/emit-plan.py oracle/driver-real2.py "$AMDSERVE_WEIGHTS" out/real2.plan --capacity=131072 --fixture-json=oracle/lighthouse256.json; echo "EMIT_RC=$?"
python3 oracle/check-plan.py out/real2.plan "$AMDSERVE_WEIGHTS"; echo "CHECK_RC=$?"
sha256sum out/real2.plan
python3 oracle/emit-plan.py oracle/driver-real2.py "$AMDSERVE_WEIGHTS" out/real2-controls.plan --controls --capacity=131072 --fixture-json=oracle/lighthouse256.json; echo "EMITC_RC=$?"
python3 oracle/check-plan.py out/real2-controls.plan "$AMDSERVE_WEIGHTS"; echo "CHECKC_RC=$?"
sha256sum out/real2-controls.plan
cut -c1-700 out/real2.plan.cmd
ls -la out out/code | head -40
cp /workspace/uxl/launch-x86 out/omega-launch
cd out
for r in warmup run1 run2 run3 run4 run5; do
  echo "$r start $(date -u +%FT%T.%NZ) $(date +%s.%N)"
  PATH="$PWD:$PATH" sh real2.plan.cmd > /workspace/uxl/uxl-native-$r.txt 2> /workspace/uxl/uxl-native-$r.err; echo "$r rc=$?"
  echo "$r end $(date -u +%FT%T.%NZ) $(date +%s.%N)"
  grep -c OMEGA_STEP /workspace/uxl/uxl-native-$r.txt; tail -c 300 /workspace/uxl/uxl-native-$r.err
done
echo "control start $(date +%s.%N)"
PATH="$PWD:$PATH" sh real2-controls.plan.cmd > /workspace/uxl/uxl-native-control.txt 2> /workspace/uxl/uxl-native-control.err; echo "control rc=$?"
echo "control end $(date +%s.%N)"
echo NATIVE_DONE
date -u +%FT%TZ
cd ..
python3 oracle/lighthouse-oracle.py oracle/driver-real2.py "$AMDSERVE_WEIGHTS" oracle/lighthouse256.json graph /workspace/uxl/uxl-graph-fresh.json --capacity=131072 > /workspace/uxl/uxl-graph-fresh.txt 2>&1; echo "GRAPH_RC=$?"
tail -3 /workspace/uxl/uxl-graph-fresh.txt | cut -c1-600
date -u +%FT%TZ
python3 oracle/lighthouse-oracle.py oracle/driver-real2.py "$AMDSERVE_WEIGHTS" oracle/lighthouse256.json reference /workspace/uxl/uxl-reference-fresh.json --capacity=131072 > /workspace/uxl/uxl-reference-fresh.txt 2>&1; echo "REFERENCE_RC=$?"
tail -3 /workspace/uxl/uxl-reference-fresh.txt | cut -c1-600
date -u +%FT%TZ
echo L3_DONE
