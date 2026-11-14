#!/bin/bash
# spec-decode S4 rehearsal on the Spark CPU (never on the card). Usage: dry_s4.sh BUNDLE_DIR OUT_DIR
# 1. runs the cell's own check-and-unpack code with /workspace redirected to OUT_DIR/ws;
# 2. runs s4.sh with DRY=1 (real launchers under qemu must stop at exit 77 for every plan; fake_launch.py for the
#    timed sequence; analysis and packaging as on the card); expects ANALYZE_RC=0 and every DRY-PLAN OK;
# 3. controls: a corrupted bundle is refused by the cell's sha256 assert; FAKE_MUTANT=1 (one wrong id in every
#    OMPLAN02 run) makes the analysis fail.
set -u
B=$(cd "$1" && pwd); O=$2; rm -rf "$O"; mkdir -p "$O/ws"; O=$(cd "$O" && pwd)
S=$HOME/work/hive/uxl-brief-20261113/SpecDecode
export PATH=$HOME/work/hive/t_760d3a2e/venv-vllm/bin:$PATH
cell() { # WS: run the cell up to its background start, /workspace -> WS
  python3 - "$1" "$B/cell-s4.py" <<'PY'
import sys; ws, cell = sys.argv[1], open(sys.argv[2]).read()
exec(compile(cell.split("subprocess.Popen")[0].replace("/workspace", ws), "cell", "exec"), {}); print("CELL UNPACK OK")
PY
}
cp "$B/s4-bundle.tar.xz" "$O/ws/"; cell "$O/ws" || { echo "DRY-S4 FAIL cell"; exit 1; }
export DRY=1 CUDAPP=$S/CUDApp AMDSERVE_WEIGHTS=$HOME/workspace/l182-final/aien-demo/weights/qwen3.8-27b-fp8 S4_DIR=$O/ws/s4
sh "$O/ws/s4/s4.sh"; grep -h "DRY-PLAN\|EMIT \|PLAN \|ANALYZE_RC\|S4-DONE\|MISMATCH\|FAILED" "$O/ws/s4/s4.log" | cut -c1-160
tail -n 3 "$O/ws/s4/analyze.log"
ok=1; grep -q "ANALYZE_RC=0" "$O/ws/s4/s4.log" || ok=0; grep -q "DRY-PLAN FAIL" "$O/ws/s4/s4.log" && ok=0
# control: corrupted bundle
mkdir -p "$O/bad"; python3 -c "import sys;b=bytearray(open(sys.argv[1],'rb').read());b[1000]^=1;open(sys.argv[2],'wb').write(b)" "$B/s4-bundle.tar.xz" "$O/bad/s4-bundle.tar.xz"
if cell "$O/bad" 2>"$O/bad.err"; then echo "DRY-S4 CONTROL FAIL: corrupted bundle accepted"; ok=0
else grep -q "bundle sha256 mismatch" "$O/bad.err" && echo "DRY-S4 CONTROL corrupted bundle refused" || { echo "DRY-S4 CONTROL FAIL: other error"; ok=0; }; fi
# control: one wrong id in OMPLAN02 runs must fail the analysis
( cd "$O/ws/s4" && rm -rf runs && mkdir runs && FAKE_MUTANT=1 python3 fake_launch.py plans/lh-r2h/plan.plan > runs/r2h-1.txt &&
  python3 fake_launch.py plans/lh-plain/plan.plan > runs/plain-1.txt &&
  if python3 analyze_s4.py runs published-ids.json golden24.json mut.json > mut.log; then echo "DRY-S4 CONTROL FAIL: wrong id passed"; exit 1
  else echo "DRY-S4 CONTROL wrong id caught"; fi ) || ok=0
[ $ok = 1 ] && echo "DRY-S4 OK" || echo "DRY-S4 FAIL"
