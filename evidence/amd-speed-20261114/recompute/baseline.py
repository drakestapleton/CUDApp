#!/usr/bin/env python3
# Baseline: the published run (10:12 UTC) and the same-session repeat (18:50 UTC session), from raw stdout captures.
import json, statistics
from common import ROOT, Book, omega_run, first_difference

b = Book('baseline')
PUB = 'amd-launch-gap/src/CUDApp-base/evidence/amd-qwen38-native-refusal-20261113'   # CUDApp dc536882 clone
REP = 'card-evidence'

pub = [omega_run(ROOT / f'{PUB}/native/sample{i}.stdout') for i in range(1, 6)]
pub_ids = pub[0]['ids']
b.check('published 5 runs x 285 steps', all(r['steps'] == 285 for r in pub))
b.check('published ids equal across runs', all(r['ids'] == pub_ids for r in pub))
b.check('published 256 ids', len(pub_ids) == 256)
meds = [round(r['decode_ms_median'], 3) for r in pub]
b.add('published.run_medians', meds, 'ms/token', 'MEASURED', [f'{PUB}/native/sample{i}.stdout' for i in range(1, 6)],
      'median of the 255 OMEGA_STEP us values after the 30 prompt steps, per run')
pm = b.add('published.decode_ms', round(statistics.median(meds), 3), 'ms/token', 'MEASURED',
           [f'{PUB}/native/sample{i}.stdout' for i in range(1, 6)], 'median of the five run medians')
calc = json.loads((ROOT / f'{PUB}/calculations.json').read_text())
b.check('published equals filed calculations.json', pm == calc['median_decode_ms'], f'{pm} vs {calc["median_decode_ms"]}')
b.add('published.prefill_ms', round(statistics.median(r['prefill_ms'] for r in pub), 3), 'ms', 'MEASURED',
      [f'{PUB}/native/sample{i}.stdout' for i in range(1, 6)], 'median over runs of the sum of the 30 prompt step times')

rep = [omega_run(ROOT / f'{REP}/uxl-native-run{i}.txt') for i in range(1, 6)]
b.check('repeat ids equal published', all(r['ids'] == pub_ids for r in rep))
rmeds = [round(r['decode_ms_median'], 3) for r in rep]
b.add('repeat.run_medians', rmeds, 'ms/token', 'MEASURED', [f'{REP}/uxl-native-run{i}.txt' for i in range(1, 6)],
      'median of 255 decode steps per run')
rm = b.add('repeat.decode_ms', round(statistics.median(rmeds), 3), 'ms/token', 'MEASURED',
           [f'{REP}/uxl-native-run{i}.txt' for i in range(1, 6)], 'median of the five run medians')
b.add('repeat.prefill_ms', round(statistics.median(r['prefill_ms'] for r in rep), 3), 'ms', 'MEASURED',
      [f'{REP}/uxl-native-run{i}.txt' for i in range(1, 6)], 'median over runs of the 30 prompt step times')

ctl = omega_run(ROOT / f'{REP}/uxl-native-control.txt')
b.add('repeat.control_first_difference', first_difference(ctl['ids'], pub_ids), 'index', 'MEASURED',
      f'{REP}/uxl-native-control.txt', 'first index where the altered-weights plan differs from the published ids')

g = json.loads((ROOT / f'{REP}/uxl-graph-fresh.json').read_text())
b.check('HIP graph ids equal published', g['ids'] == pub_ids)
hip = b.add('repeat.hip_graph_ms', round(g['decode_ms_median'], 3), 'ms/token', 'MEASURED', f'{REP}/uxl-graph-fresh.json',
            'decode_ms_median of one HIP graph replay process (255 steps)')
r = json.loads((ROOT / f'{REP}/uxl-reference-fresh.json').read_text())
b.add('repeat.pytorch_fp8_ms', round(r['decode_ms_median'], 3), 'ms/token', 'MEASURED', f'{REP}/uxl-reference-fresh.json',
      'decode_ms_median, PyTorch plus transformers on ROCm, FP8 checkpoint')
b.add('repeat.pytorch_fp8_first_difference_vs_spark', r['first_divergence'], 'index', 'MEASURED', f'{REP}/uxl-reference-fresh.json',
      'first_divergence field')
b.add('repeat.omega_vs_pytorch_first_difference', first_difference(rep[0]['ids'], r['ids']), 'index', 'MEASURED',
      [f'{REP}/uxl-native-run1.txt', f'{REP}/uxl-reference-fresh.json'], 'first generated index where Omega and PyTorch (same card, session and checkpoint) differ')
s = json.loads((ROOT / f'{REP}/uxl-stock.json').read_text())
smeds = [x['decode_ms_median'] for x in s['rows'] if x['label'].startswith('run')]
b.add('repeat.stock_bf16_ms', round(statistics.median(smeds), 3), 'ms/token', 'MEASURED', f'{REP}/uxl-stock.json',
      'median of the five timed rows (warmup row left out); BF16, 70 prompt tokens, not like for like')
b.add('gap.ms', round(rm - hip, 3), 'ms/token', 'DERIVED', [f'{REP}/uxl-native-run1.txt', f'{REP}/uxl-graph-fresh.json'],
      'repeat.decode_ms minus repeat.hip_graph_ms (same session)')
b.close()
