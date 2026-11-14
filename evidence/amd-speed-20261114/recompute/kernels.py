#!/usr/bin/env python3
# Kernels: AmdKernelSpeed card session e787e65d (one run per candidate) plus the bytes-per-step model.
# Per-class times follow the lane's method (measure/analyze.py): delta against the median of three 64-token base runs.
import csv, json, re, statistics
from common import ROOT, Book, omega_run, first_difference, sha256

b = Book('kernels')
S = 'amd-kernel-speed/card/261114001549-e787e65d'
R = f'{S}/out/runs'
PEAK = 5.3e12                      # B/s, AMD's MI300X figure (quoted, not measured)
pub = json.loads((ROOT / 'amd-launch-gap/card/bundle/published-ids.json').read_text())['ids']
b.check('aks-results tarball sha256 equals card value',
        sha256(ROOT / f'{S}/aks-results.tar.gz') == (ROOT / f'{S}/aks-results.sha256').read_text().split()[0])

rows = list(csv.DictReader(open(ROOT / 'amd-kernel-speed/model/classes.tsv'), delimiter='\t'))
total = sum(int(r['weight_bytes_per_step']) for r in rows)
b.add('model.bytes_per_step', total, 'B', 'MODELED', 'amd-kernel-speed/model/classes.tsv',
      'sum of weight_bytes_per_step (weights and block scales, BF16 lm_head in full; embedding one row)')
floor = b.add('model.floor_ms', round(total / PEAK * 1e3, 4), 'ms/token', 'MODELED', 'amd-kernel-speed/model/classes.tsv',
              'bytes_per_step / 5.3 TB/s')
b.add('model.dispatches_per_step', sum(int(r['dispatches_per_step']) for r in rows), 'dispatches', 'COUNT',
      'amd-kernel-speed/model/classes.tsv', 'sum of dispatches_per_step')

ladder = {}
for name in ['full-a', 'full-b', 'cap286', 'v2-rmsnorm5120', 'v2-gdnstate_step', 'v1all']:
    x = omega_run(ROOT / f'{R}/{name}.txt')
    b.check(f'{name} prints the 256 published ids', x['ids'] == pub, f'first difference {first_difference(x["ids"], pub)}')
    ms = x['decode_ms_median']
    ladder[name] = ms
    b.add(f'ladder.{name}_ms', round(ms, 3), 'ms/token', 'MEASURED', f'{R}/{name}.txt', 'median of 255 decode steps, one run')
    b.add(f'ladder.{name}_TBps', round(total / (ms / 1e3) / 1e12, 3), 'TB/s', 'DERIVED', [f'{R}/{name}.txt', 'amd-kernel-speed/model/classes.tsv'],
          'modelled bytes per step / measured time')
    b.add(f'ladder.{name}_pct_peak', round(100 * total / (ms / 1e3) / PEAK, 1), '%', 'DERIVED', [f'{R}/{name}.txt', 'amd-kernel-speed/model/classes.tsv'],
          'percent of 5.3 TB/s')
    b.add(f'ladder.{name}_prefill_ms', round(x['prefill_ms'], 1), 'ms', 'MEASURED', f'{R}/{name}.txt', 'sum of 30 prompt steps')
b.add('ladder.v1all_speedup_vs_full_a', round(ladder['full-a'] / ladder['v1all'], 3), 'x', 'DERIVED',
      [f'{R}/full-a.txt', f'{R}/v1all.txt'], 'full-a / v1all, same session')
b.add('ladder.baseline_spread_ms', round(ladder['full-b'] - ladder['full-a'], 3), 'ms/token', 'DERIVED',
      [f'{R}/full-a.txt', f'{R}/full-b.txt'], 'full-b minus full-a: two runs of the same unchanged plan, same session')
b.add('ladder.v1all_speedup_vs_full_b', round(ladder['full-b'] / ladder['v1all'], 3), 'x', 'DERIVED',
      [f'{R}/full-b.txt', f'{R}/v1all.txt'], 'full-b / v1all, same session')
b.add('ladder.v1all_over_floor', round(ladder['v1all'] / floor, 2), 'x', 'DERIVED', [f'{R}/v1all.txt'], 'v1all / modelled floor')
ctl = omega_run(ROOT / f'{R}/control.txt')
b.add('ladder.control_first_difference', first_difference(ctl['ids'], pub[:64]), 'index', 'MEASURED', f'{R}/control.txt',
      'altered-weights 64-token plan against the first 64 published ids')

# per-class time per dispatch (lane method), checked against the lane's analysis.json
P = 30
def steps(name):
    x = (ROOT / f'{R}/{name}.txt').read_text()
    import re
    return {int(m.group(1)): int(m.group(2)) for m in re.finditer(r'^OMEGA_STEP (\d+) tok=-?\d+ us=(\d+)$', x, re.M)}
base = [steps(f'base-{i}') for i in (1, 2, 3)]
for i in (1, 2, 3):
    b.check(f'base-{i} ids', omega_run(ROOT / f'{R}/base-{i}.txt')['ids'] == pub[:64])
dec = range(P, P + 63)
ref = {s: statistics.median(bs[s] for bs in base) for s in dec}
b.add('classes.base_decode_ms', round(statistics.median(ref.values()) / 1e3, 3), 'ms/token', 'MEASURED',
      [f'{R}/base-{i}.txt' for i in (1, 2, 3)], 'median over decode steps of the per-step median of three 64-token base runs')
lane = json.loads((ROOT / f'{S}/analysis.json').read_text())['classes']


def per_step(cls):
    k, _, shape = cls.partition(':')
    if k == 'empty':
        return None
    n = 0
    for r in rows:
        if r['kernel'] == k and (not shape or shape in r['class']):
            n += int(r['dispatches_per_step'])
    return n


table = []
for v in sorted((ROOT / R).glob('*.variant')):
    if v.stem.startswith(('base', 'control')):
        continue
    meta = json.loads(v.read_text().split('VARIANT ', 1)[1])
    st = steps(v.stem)
    b.check(f'{v.stem} ids', omega_run(ROOT / f'{R}/{v.stem}.txt')['ids'] == pub[:64])
    med = statistics.median(st[s] - ref[s] for s in dec)
    usd = med / meta['inserted']
    b.check(f'{meta["cls"]} equals lane analysis.json', abs(usd - lane[meta['cls']]['us_per_dispatch']) < 1e-9)
    n = per_step(meta['cls'])
    table.append((meta['cls'], n, usd, None if n is None else usd * n / 1e3))
table.sort(key=lambda t: -(t[3] or 0))
for cls, n, usd, ms in table:
    key = cls.replace(':', '_')
    b.add(f'classes.{key}.us_per_dispatch', round(usd, 2), 'us', 'MEASURED', f'{R}/{cls.replace(":", "_")}.txt',
          'median step delta / inserted copies, against the base runs')
    if ms is not None:
        b.add(f'classes.{key}.ms_per_step', round(ms, 3), 'ms/token', 'DERIVED', [f'{R}/{cls.replace(":", "_")}.txt', 'amd-kernel-speed/model/classes.tsv'],
              f'us per dispatch x {n} dispatches per step')
        k, _, shape = cls.partition(':')
        wb = sum(int(r['weight_bytes_per_dispatch']) for r in rows if r['kernel'] == k and shape and shape in r['class'][:]) \
            / max(1, sum(1 for r in rows if r['kernel'] == k and shape and shape in r['class']))
        if wb:
            b.add(f'classes.{key}.pct_peak', round(100 * wb / (usd / 1e6) / PEAK, 1), '%', 'DERIVED',
                  [f'{R}/{cls.replace(":", "_")}.txt', 'amd-kernel-speed/model/classes.tsv'],
                  'weight bytes per dispatch / time per dispatch, as percent of 5.3 TB/s (in place on the card, inserted copies)')
# addnorm5120 v2 (kernel step 2): CPU gate only, not run on the card
GA = 'amd-kernel-speed/gates/gate-addnorm5120.log'
ma = re.search(r'^ADDNORM5120-V2-GATE PASS cases=(\d+) mutants=(\d+) fail=0$', (ROOT / GA).read_text(), re.M)
b.check('addnorm5120 v2 gate PASS line present', bool(ma))
b.add('addnorm_gate.cases', int(ma.group(1)), 'cases', 'COUNT', GA, 'ADDNORM5120-V2-GATE PASS line')
b.add('addnorm_gate.mutants', int(ma.group(2)), 'mutants', 'COUNT', GA, 'same line')
# fp8gemv_b128f v3r2 and v3r4: CPU gates only, the lane's record copied into the folder
FR = 'amd-kernel-speed/kernels/FP8-R2-RESULT.md'
fr = (ROOT / FR).read_text()
for r in ('R2', 'R4'):
    mf = re.search(rf'^FP8GEMV-B128F-{r}-GATE PASS cases=(\d+) mutants=(\d+) controls=\d+ fail=0$', fr, re.M)
    b.check(f'fp8gemv_b128f v3{r.lower()} gate PASS line present, two builds byte-identical',
            bool(mf) and f'PASS v3{r.lower()} two builds byte-identical' in fr)
    b.add(f'fp8_v3{r.lower()}_gate.cases', int(mf.group(1)), 'cases', 'COUNT', FR, f'FP8GEMV-B128F-{r}-GATE PASS line')
    b.add(f'fp8_v3{r.lower()}_gate.mutants', int(mf.group(2)), 'mutants', 'COUNT', FR, 'same line')
# Kernel step 2 card bundle, simulated on the Spark (no card): plan fusion lines and per-run exit codes of the bundle's runs.
SN = 'amd-report/extracted/spark-logs-20261114/kernel-step2-sim-next.log'
sn = (ROOT / SN).read_text()
fu = {tuple(int(x) for x in m) for m in re.findall(r"^FUSED \{.*'dispatches_before': (\d+), 'dispatches_after': (\d+),", sn, re.M)}
fab = {tuple(int(x) for x in m) for m in re.findall(r"^FUSED_AB \{.*'dispatches_before': (\d+), 'dispatches_after': (\d+),", sn, re.M)}
b.check('step-2 sim log: one FUSED and one FUSED_AB dispatch pair, chained', len(fu) == 1 and len(fab) == 1 and next(iter(fu))[1] == next(iter(fab))[0])
b.add('step2.dispatches_before', next(iter(fu))[0], 'dispatches', 'COUNT', SN, 'FUSED line: dispatches per step before the addnorm fusion')
b.add('step2.dispatches_after_addnorm', next(iter(fu))[1], 'dispatches', 'COUNT', SN, 'FUSED line: after it')
b.add('step2.ab_merge_fewer', next(iter(fab))[0] - next(iter(fab))[1], 'dispatches', 'DERIVED', SN, 'FUSED_AB line: before minus after')
runs = re.findall(r'^RUN (\S+) rc=(\d+) end ', sn, re.M)
b.check('step-2 sim log: every run name appears once and the script finished', len({r for r, _ in runs}) == len(runs) and '\nNEXT_DONE\n' in sn)
b.add('step2.sim_runs', len(runs), 'runs', 'COUNT', SN, 'RUN ... rc= lines')
b.add('step2.sim_runs_exit0', sum(rc == '0' for _, rc in runs), 'runs', 'COUNT', SN, 'of those, rc=0')
b.add('classes.sum_ms', round(sum(t[3] for t in table if t[3]), 3), 'ms/token', 'DERIVED', R, 'sum over classes of ms per step')
b.add('classes.unattributed_ms', round(statistics.median(ref.values()) / 1e3 - sum(t[3] for t in table if t[3]), 3), 'ms/token', 'DERIVED', R,
      '64-token base step minus the sum over classes: the part this method does not attribute')
b.close()
