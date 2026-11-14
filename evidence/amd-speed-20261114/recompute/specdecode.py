#!/usr/bin/env python3
# Several tokens per pass: SpecDecode's acceptance counts (CPU float32 reference, not the card).
import json, re
from fractions import Fraction
from common import ROOT, Book, sha256

b = Book('specdecode')
for name in ('lighthouse', 'freerun'):
    f = f'spec-decode/acceptance/accept-mtp-{name}.json'
    p = ROOT / f
    if not p.exists():
        b.add(f'{name}.state', 'PENDING', '', 'RECORD', 'spec-decode/acceptance', f'{f} not present yet')
        continue
    d = json.loads(p.read_text())
    for variant, seqs in d['results'].items():
        vk = 'full' if variant == 'full' else 'shortlist'
        for seq, rows in seqs.items():
            sk = seq.replace('lighthouse-', '')
            best = max(rows, key=lambda r: Fraction(r['tokens_per_pass']))
            for r in rows:
                tpp = Fraction(r['tokens_per_pass'])
                b.check(f'{name} {variant} {seq} k={r["k"]} fraction', tpp == Fraction(r['tokens'], r['passes']))
                if r['k'] in (1, 2, 3):
                    b.add(f'{name}.{vk}.{sk}.k{r["k"]}', round(float(tpp), 3), 'tokens/pass', 'MEASURED', f,
                          'tokens / passes from the exact speculative loop over the recorded sequence (CPU reference)')
            b.add(f'{name}.{vk}.{sk}.max', round(float(Fraction(best['tokens_per_pass'])), 3), 'tokens/pass', 'MEASURED', f,
                  f'maximum over k = 0..8 (reached at k = {best["k"]})')
fr = ROOT / 'spec-decode/acceptance/accept-mtp-freerun.json'
if fr.exists():
    rows = json.loads(fr.read_text())['results']['full']
    varied = {k: v for k, v in rows.items() if not k.startswith('lighthouse')}
    k1 = {k: float(Fraction(next(r for r in v if r['k'] == 1)['tokens_per_pass'])) for k, v in varied.items()}
    f = 'spec-decode/acceptance/accept-mtp-freerun.json'
    b.add('varied.prompts', len(k1), 'prompts', 'COUNT', f, 'free-run prompts other than the lighthouse essay')
    b.add('varied.k1_min', round(min(k1.values()), 2), 'tokens/pass', 'MEASURED', f, f'k = 1, full head, lowest prompt ({min(k1, key=k1.get)})')
    b.add('varied.k1_max', round(max(k1.values()), 2), 'tokens/pass', 'MEASURED', f, f'k = 1, full head, highest prompt ({max(k1, key=k1.get)})')
    b.add('varied.tokens_each', sorted({next(r for r in v if r['k'] == 1)['tokens'] for v in varied.values()}), 'tokens', 'COUNT', f, 'generated tokens per varied prompt')
# extra-row cost (rho) measured on the card, session 3a2e29e5: rho = (median_R / median_1 - 1) / (R - 1),
# as in spec-decode/card-rho/spec_rho.py line 118
import statistics
RS = 'spec-decode/card-s4/results/jupyter-user-261114025237-3a2e29e5'
rr = json.loads((ROOT / f'{RS}/rho-results.json').read_text())['kernels']
log = (ROOT / f'{RS}/rho.log').read_text()
one = statistics.median(rr['fp8gemv_b128f_r1']['per_dispatch_ns'])
b.check('fp8 R=1 median equals the file', abs(one - rr['fp8gemv_b128f_r1']['median_ns']) < 1e-6)
b.add('rho.fp8_r1_us', round(one / 1e3, 3), 'us', 'MEASURED', f'{RS}/rho-results.json', 'fp8gemv_b128f 81920x5120, R=1, median per dispatch of 5 trials')
for R in (2, 3, 8):
    k = rr[f'fp8gemv_b128f_r{R}']
    med = statistics.median(k['per_dispatch_ns'])
    rho = (med / one - 1) / (R - 1)
    b.check(f'fp8 R={R} rho equals the file and the log', abs(rho - k['rho']) < 1e-12 and f'rho={rho:.4f}' in log)
    b.check(f'fp8 R={R} rows bit-identical to the one-row kernel', k['rows_bit_identical_to_one_row'] is True and k['finite'])
    b.add(f'rho.fp8_r{R}_us', round(med / 1e3, 3), 'us', 'MEASURED', f'{RS}/rho-results.json', f'R={R} rows, median per dispatch')
    b.add(f'rho.fp8_r{R}', round(rho, 4), 'fraction', 'DERIVED', f'{RS}/rho-results.json', '(median_R / median_1 - 1) / (R - 1)')
b.add('rho.bf16_r1_us', round(statistics.median(rr['bf16gemv2_r1']['per_dispatch_ns']) / 1e3, 3), 'us', 'MEASURED', f'{RS}/rho-results.json',
      'bf16gemv2 248320x5120 (output head), R=1; R>1 not reached')
# CPU gates of the speculative step (seeded Omega and the seeded gfx942 executor; hardware NOT_RUN in every file).
for name, f in (('s0', 'spec-decode/om/gate-spec-results.json'), ('s1', 'spec-decode/om/gate-loop-results.json')):
    g = json.loads((ROOT / f).read_text())
    b.check(f'{name} gate verdict PASS, positive output as expected', g['verdict'] == 'PASS' and g['positive']['ok'] and g['positive']['stdout'] == g['positive']['expected'])
    b.add(f'gate_{name}.cases', int(g['positive']['stdout'][0]), 'cases', 'COUNT', f, 'first line of the positive run: cases (S0) or runs (S1)')
    b.add(f'gate_{name}.mutants', len(g['mutants']), 'mutants', 'COUNT', f, 'mutants listed')
    b.add(f'gate_{name}.mutants_caught', sum(m['caught'] is True for m in g['mutants']), 'mutants', 'COUNT', f, 'of those, caught')
K = 'spec-decode/kernels'
for run in ('s2-run6', 's2b-conv-run3', 's2b-run4', 's2c-run2', 's2d-run2', 's3a-run1', 's3c-pos-run1', 's3c-select-run1'):
    g = json.loads((ROOT / f'{K}/{run}/report.json').read_text())
    b.check(f'{run} report verdict PASS, hardware NOT_RUN', g['verdict'] == 'PASS' and g['hardware'] == 'NOT_RUN')
S3C = 'spec-decode/s3c/run3'
g = json.loads((ROOT / f'{S3C}/report.json').read_text())
r3 = (ROOT / f'{S3C}/run3.log').read_text()
m3 = re.search(r'^SPEC-S3C PASS rows=(\d+) pass=(\d+) required_mutants=(\d+) caught=(\d+) hardware=NOT_RUN$', r3, re.M)
b.check('s3c whole-pass gate: report PASS and every required mutant caught', g['verdict'] == 'PASS' and bool(m3) and m3.group(3) == m3.group(4))
b.add('gate_s3c.required_mutants_caught', int(m3.group(4)), 'mutants', 'COUNT', f'{S3C}/run3.log', 'SPEC-S3C PASS line')
b.add('gate_s3c.rows', int(m3.group(1)), 'rows', 'COUNT', f'{S3C}/run3.log', 'SPEC-S3C PASS line: rows')
b.add('gate_s3c.rows_pass', int(m3.group(2)), 'rows', 'COUNT', f'{S3C}/run3.log', 'SPEC-S3C PASS line: rows that pass')
info = [(i, l) for i, l in enumerate(r3.splitlines(), 1) if l.startswith('S3C INFO-FAIL MUTANT ')]
b.check('s3c: the one INFO-FAIL row is accept-after-mismatch on line 105, caught_by false on both prompts',
        info == [(105, 'S3C INFO-FAIL MUTANT object spec_accept_r3 accept-after-mismatch caught {"caught_by": [false, false]}')])
b.add('gate_s3c.info_mutants_not_caught', len(info), 'mutants', 'COUNT', f'{S3C}/run3.log', 'INFO-FAIL MUTANT lines with caught_by false on both prompts')
S3A = f'{K}/s3a-run1/report.json'
aam = [m for m in json.loads((ROOT / S3A).read_text())['mutants'] if m['mutant'] == 'accept-after-mismatch']
b.check('s3a accept kernel gate lists accept-after-mismatch once', len(aam) == 1)
b.add('gate_s3a.accept_after_mismatch_failing', aam[0]['failing_cases'], 'cases', 'COUNT', S3A, 'failing cases of that mutant')
b.add('gate_s3a.accept_after_mismatch_cases', aam[0]['cases'], 'cases', 'COUNT', S3A, 'cases run against that mutant')
HL = 'amd-report/extracted/spark-logs-20261114/hostloop-gate-final-20261114T022658Z.log'
hl = (ROOT / HL).read_text()
hp = re.findall(r'^AMD-KFD-LAUNCH CPU PASS (\{.*\})$', hl, re.M)
hj = json.loads(hp[0]) if len(hp) == 1 else {}
b.check('host loop gate: one PASS line, 0 failures, head f859c88f clean', hj.get('failures') == 0 and hj.get('device') == 'NOT_RUN'
        and bool(re.search(r'^repo \S+ head f859c88fc05f7ecc9d4afcefc328e2515d3647e8 dirty=0 ', hl, re.M)))
b.add('hostloop.gate_checks', int(hj.get('checks', -1)), 'checks', 'COUNT', HL, '"checks" in the AMD-KFD-LAUNCH CPU PASS line, branch spec-decode2 head f859c88f')
DRY = 'spec-decode/card-s4/dry4.log'
dry = (ROOT / DRY).read_text().splitlines()
b.check('card bundle Spark rehearsal: SPEC-S4 PASS, both controls, DRY-S4 OK', 'SPEC-S4 PASS' in dry and 'DRY-S4 CONTROL corrupted bundle refused' in dry
        and 'DRY-S4 CONTROL wrong id caught' in dry and dry[-1] == 'DRY-S4 OK')
dp = [x for x in dry if x.startswith('DRY-PLAN ')]
b.check('card bundle Spark rehearsal: every DRY-PLAN line OK', bool(dp) and all(x.startswith('DRY-PLAN OK ') for x in dp))
b.add('s4_dry.plans_ok', len(dp), 'plan checks', 'COUNT', DRY, 'DRY-PLAN OK lines: a real launcher under qemu accepted the plan and stopped at the device (dry_s4.sh lines 4 to 5)')
b.add('s4_dry.plans_ok_spec', sum(x.endswith(' launch-spec') for x in dp), 'plan checks', 'COUNT', DRY, 'of those, with launch-spec')
b.add('s4_dry.plans_ok_x86', sum(x.endswith(' launch-x86') for x in dp), 'plan checks', 'COUNT', DRY, 'of those, with launch-x86')
pl = [x for x in dry if re.match(r'^PLAN \S+ OK [0-9a-f]{64}$', x)]
b.check('every DRY-PLAN line names a plan from a PLAN ... OK line', {x.split()[2] for x in dp} == {x.split()[1] for x in pl})
b.add('s4_dry.plans', len(pl), 'plans', 'COUNT', DRY, 'PLAN ... OK lines')
sm = {float(v) for v in re.findall(r'^RUN \S+ ids_ok complete tok/pass=1\.0 ms/token=([\d.]+) *$', '\n'.join(dry), re.M)}
b.check('rehearsal RUN lines: one stand-in figure', len(sm) == 1)
b.add('s4_dry.standin_ms', sm.pop(), 'ms/token', 'RECORD', [DRY, 'spec-decode/card-s4/fake_launch.py'],
      'printed by the stand-in fake_launch.py (us = 26000 + step), not a measurement')
b.add('card.state', 'NOT_RUN', '', 'RECORD', [], 'no speculative step has run on the card (spec-decode/RESULT.md section 6, lane notes)')
b.close()
