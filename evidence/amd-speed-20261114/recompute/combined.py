#!/usr/bin/env python3
# Combined run: AmdKernelSpeed card session 6e585e21. Seven arms, three interleaved 256-token runs each, plus an
# altered-weights control. Everything is recomputed from out/runs/*.txt and cross-checked against the lane's analysis.json.
import csv, json, statistics
from common import ROOT, Book, omega_run, first_difference, sha256

b = Book('combined')
D = 'amd-kernel-speed/combined/card-2026-11-14T0206Z'
R = f'{D}/out/runs'
PEAK = 5.3e12
pub = json.loads((ROOT / 'amd-launch-gap/card/bundle/published-ids.json').read_text())['ids']
b.check('published-ids.json equals the published sample1 ids', pub == omega_run(
    ROOT / 'amd-launch-gap/src/CUDApp-base/evidence/amd-qwen38-native-refusal-20261113/native/sample1.stdout')['ids'])
tar = f'{D}/akc-results.tar.gz'
side = (ROOT / f'{D}/SHA256SUMS').read_text()
b.check('tarball sha256 equals the value recorded beside it', sha256(ROOT / tar) in side)
card = [json.loads(l)['shaCard'] for l in (ROOT / f'{D}/driver.jsonl').read_text().splitlines() if '"shaCard"' in l]
b.check('tarball sha256 equals the value the card computed (driver.jsonl shaCard)', len(card) == 1 and sha256(ROOT / tar) == card[0])
# every file listed in the card's own SHA256SUMS matches the unpacked copy
bad = 0
for line in (ROOT / f'{D}/out/SHA256SUMS').read_text().splitlines():
    h, name = line.split(None, 1)
    p = ROOT / f'{D}/out' / name.lstrip('*').strip()
    bad += (not p.is_file()) or sha256(p) != h
b.check('out/SHA256SUMS all match', bad == 0, f'{bad} mismatches')

rows = list(csv.DictReader(open(ROOT / 'amd-kernel-speed/model/classes.tsv'), delimiter='\t'))
total = sum(int(r['weight_bytes_per_step']) for r in rows)
floor = total / PEAK * 1e3
lane = json.loads((ROOT / f'{D}/analysis.json').read_text())

ARMS = [('A', 'published launcher, full plan, v1 kernels'),
        ('B', 'plan3 launcher, full plan, v1 kernels'),
        ('C', 'B with rmsnorm5120 v2 and gdnstate_step v2'),
        ('D1', 'C with fp8gemv_b128f v2b1'),
        ('D2', 'C with fp8gemv_b128f v2b2'),
        ('E1', 'D1 on the 286-position plan'),
        ('E2', 'D2 on the 286-position plan')]
med, spread, n_ok, n_runs, alarms = {}, {}, 0, 0, 0
for arm, what in ARMS:
    runs, files = [], []
    for i in (1, 2, 3):
        f = f'{R}/{arm}-{i}.txt'
        x = omega_run(ROOT / f)
        text = (ROOT / f).read_text(errors='replace')
        alarms += 'OMEGA alarm' in text
        n_runs += 1
        ok = x['ids'] == pub and x['steps'] == 285
        n_ok += ok
        b.check(f'{arm}-{i} prints the 256 published ids over 285 steps', ok, f'first difference {first_difference(x["ids"], pub)}')
        us = round(x['decode_ms_median'] * 1000)
        b.check(f'{arm}-{i} equals lane analysis.json', us == lane['runs'][f'{arm}-{i}']['decode_us'], f'{us} vs lane')
        runs.append(x['decode_ms_median'])
        files.append(f)
    m = statistics.median(runs)
    med[arm], spread[arm] = m, max(runs) - min(runs)
    b.check(f'arm {arm} median equals lane analysis.json', round(m * 1000) == lane['arms'][arm]['median_us'])
    b.add(f'{arm}.run_medians', [round(r, 3) for r in runs], 'ms/token', 'MEASURED', files, f'{what}: median of 255 decode steps, runs in order')
    b.add(f'{arm}.decode_ms', round(m, 3), 'ms/token', 'MEASURED', files, 'median of the three run medians')
    b.add(f'{arm}.spread_ms', round(spread[arm], 3), 'ms/token', 'DERIVED', files, 'largest minus smallest of the three run medians')
    b.add(f'{arm}.tokens_s', round(1000 / m, 1), 'tokens/s', 'DERIVED', files, '1000 / decode ms')
    b.add(f'{arm}.TBps', round(total / (m / 1e3) / 1e12, 3), 'TB/s', 'DERIVED', files + ['amd-kernel-speed/model/classes.tsv'],
          'modelled bytes per step / measured time')
    b.add(f'{arm}.pct_peak', round(100 * total / (m / 1e3) / PEAK, 1), '%', 'DERIVED', files + ['amd-kernel-speed/model/classes.tsv'],
          'percent of 5.3 TB/s')
b.add('runs_timed', n_runs, 'runs', 'COUNT', R, 'timed runs, seven arms by three')
b.add('runs_ids_equal', n_ok, 'runs', 'COUNT', R, 'timed runs whose 256 ids equal the published ids')
allp = sorted((ROOT / R).glob('*.txt'))
alarms = sum('OMEGA alarm' in p.read_text(errors='replace') for p in allp)
b.add('launches', len(allp), 'processes', 'COUNT', R, 'launcher plan processes: 21 timed runs and the control')
b.add('launches_alarmed', alarms, 'processes', 'COUNT', R, 'of those, with the 10 s slow-wait alarm')
b.check('no launch raised the slow-wait alarm', alarms == 0)

ctl = omega_run(ROOT / f'{R}/control.txt')
cd = first_difference(ctl['ids'], pub)
b.check('altered-weights control differs from the published ids', cd is not None)
b.add('control_first_difference', cd, 'index', 'MEASURED', f'{R}/control.txt', 'first index where the altered-weights plan differs from the published ids')

src = [f'{R}/A-1.txt', f'{R}/E2-1.txt']
for name, a, c, how in [('gain_launcher_ms', 'A', 'B', 'A minus B: plan3 launcher against the published launcher, same kernels'),
                        ('gain_v2_kernels_ms', 'B', 'C', 'B minus C: rmsnorm5120 v2 and gdnstate_step v2'),
                        ('gain_fp8_v2b1_ms', 'C', 'D1', 'C minus D1'), ('gain_fp8_v2b2_ms', 'C', 'D2', 'C minus D2'),
                        ('cap286_on_v2b1_ms', 'D1', 'E1', 'D1 minus E1: effect of the 286-position plan'),
                        ('cap286_on_v2b2_ms', 'D2', 'E2', 'D2 minus E2: effect of the 286-position plan'),
                        ('v2b2_minus_v2b1_full_ms', 'D1', 'D2', 'D1 minus D2'), ('total_gain_ms', 'A', 'E2', 'A minus E2')]:
    b.add(name, round(med[a] - med[c], 3), 'ms/token', 'DERIVED', [f'{R}/{a}-1.txt', f'{R}/{c}-1.txt'], how)
b.add('speedup_A_over_E2', round(med['A'] / med['E2'], 3), 'x', 'DERIVED', src, 'A / E2, same session')
b.add('speedup_A_over_D2', round(med['A'] / med['D2'], 3), 'x', 'DERIVED', src, 'A / D2 (full 131,072-position plan), same session')
b.add('launcher_gain_pct', round(100 * (med['A'] - med['B']) / med['A'], 1), '%', 'DERIVED', src, '100 * (A - B) / A')
b.add('E2_over_floor', round(med['E2'] / floor, 2), 'x', 'DERIVED', src + ['amd-kernel-speed/model/classes.tsv'], 'E2 / modelled floor')
b.add('D2_over_floor', round(med['D2'] / floor, 2), 'x', 'DERIVED', src + ['amd-kernel-speed/model/classes.tsv'], 'D2 / modelled floor')
b.add('max_spread_ms', round(max(spread.values()), 3), 'ms/token', 'DERIVED', R, 'largest per-arm spread of run medians')

# CPU gates of the two FP8 candidates (taken over from GpuBenchSync), filed by AmdKernelSpeed.
import re
T = 'amd-kernel-speed/takeover-gpubenchsync'
objs = dict(reversed(l.split()) for l in (ROOT / f'{D}/out/objects.sha256').read_text().splitlines() if l.strip())
for tag, obj in (('b1', 'fp8gemv_b128f-v2b1.hsaco'), ('b2', 'fp8gemv_b128f-v2b2.hsaco')):
    g = (ROOT / f'{T}/gate-{tag}.log').read_text()
    m = re.search(r'^FP8GEMV-B128F-V2-GATE PASS cases=(\d+) mutants=(\d+) fail=0$', g, re.M)
    b.check(f'gate-{tag}.log PASS line', bool(m))
    b.check(f'gate-{tag}.log names the object the card ran ({obj})', objs[obj] in g)
    caught = len(re.findall(r'^CONTROL .* CAUGHT', g, re.M))
    b.check(f'gate-{tag}.log every mutant caught', m and caught == int(m.group(2)))
    b.add(f'gate_{tag}.cases', int(m.group(1)), 'cases', 'COUNT', f'{T}/gate-{tag}.log', 'FP8GEMV-B128F-V2-GATE PASS line')
    b.add(f'gate_{tag}.mutants_caught', caught, 'mutants', 'COUNT', f'{T}/gate-{tag}.log', 'CONTROL ... CAUGHT lines')
    a = (ROOT / f'{T}/acc-{tag}.log').read_text()
    ma = re.search(r'^PASS fp8gemv_b128f cases=(\d+) controls=(\d+) hardware=NOT_RUN', a, re.M)
    b.check(f'acc-{tag}.log PASS line', bool(ma))
    b.add(f'acc_{tag}.cases', int(ma.group(1)), 'cases', 'COUNT', f'{T}/acc-{tag}.log', 'fp8block_fast.py PASS line, real checkpoint rows')
    w = (ROOT / f'{T}/waitcheck-v2{tag}.log').read_text()
    b.check(f'waitcheck-v2{tag}.log refuses (loops), as the report states', 'WAITCHECK FAIL' in w and 'linear scan not sound' in w)
b.check(f'{T}/SHA256SUMS all match', all(sha256(ROOT / T / l.split(None, 1)[1].strip().lstrip('*')) == l.split()[0]
                                         for l in (ROOT / f'{T}/SHA256SUMS').read_text().splitlines() if l.strip()))
b.close()
