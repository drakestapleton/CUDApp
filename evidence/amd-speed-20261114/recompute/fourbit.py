#!/usr/bin/env python3
# 4-bit: FourBit's partial card log (session f591aa44, killed), the rerun (session c715b669) and the container manifest.
# Partial log: each KERNEL line was cut at 900 characters by the reader; only complete {...} objects are parsed.
import csv, json, re, statistics
from common import ROOT, OUT, Book, omega_run, sha256

b = Book('fourbit')
LOG = 'four-bit/evidence/card-partial-20261114/card_q4.log.partial'
PEAK = 5.3e12
OBJ = re.compile(r'"(q4|q4r2)": (\{[^{}]*\})')
lines = [l for l in (ROOT / LOG).read_text().splitlines() if ' KERNEL ' in l]
b.add('card.shapes_recovered', len(lines), 'shapes', 'COUNT', LOG, 'KERNEL lines in the partial log')
fr = []
for l in lines:
    shape = l.split(' KERNEL ', 1)[1].split(' {', 1)[0]
    key = re.sub(r'[^A-Za-z0-9]+', '_', shape).strip('_')
    for kern, body in OBJ.findall(l):
        o = json.loads(body)
        # recompute time per dispatch from the raw nanosecond samples: (median r50 - median r1) / (repeat - 1)
        per = (statistics.median(o['r50_ns']) - statistics.median(o['r1_ns'])) / (o['repeat'] - 1) / 1e6
        b.check(f'{shape} {kern} per_dispatch_ms', abs(per - o['per_dispatch_ms']) < 1e-12, f'{per} vs {o["per_dispatch_ms"]}')
        tbps = o['weight_bytes_per_dispatch'] / (per / 1e3) / 1e12
        b.check(f'{shape} {kern} GBps', abs(tbps * 1e3 - o['weight_GBps']) < 1e-6)
        b.add(f'card.{key}.{kern}_TBps', round(tbps, 2), 'TB/s', 'MEASURED', LOG,
              'weight bytes per dispatch / ((median r50 - median r1) / 49), host clock')
        b.add(f'card.{key}.{kern}_pct_peak', round(100 * tbps * 1e12 / PEAK, 1), '%', 'DERIVED', LOG, 'percent of 5.3 TB/s')
        b.add(f'card.{key}.{kern}_bits', f"{o['output_bits_equal_oracle']}/{o['rows']}", 'outputs', 'MEASURED', LOG,
              'output words bit-equal to the oracle / outputs')
        b.check(f'{shape} {kern} bit-exact', o['output_bits_equal_oracle'] == o['rows'] and o['verdict'] == 'PASS')
        fr.append((kern, tbps))
for kern in ('q4', 'q4r2'):
    v = [t for k, t in fr if k == kern]
    b.add(f'card.{kern}_TBps_min', round(min(v), 2), 'TB/s', 'MEASURED', LOG, 'minimum over recovered shapes')
    b.add(f'card.{kern}_TBps_max', round(max(v), 2), 'TB/s', 'MEASURED', LOG, 'maximum over recovered shapes')

# ---- Rerun, session c715b669: all 10 shapes, FP8/BF16 baseline on the same tensors, raw samples in the state file ----
RR = 'four-bit/evidence/card-20261114'
ST = f'{RR}/card_q4.state.json'
TAR = f'{RR}/q4card.tar.gz'
card_hash = re.search(r'FINAL tarball sha256=([0-9a-f]{64})', (ROOT / f'{RR}/card_q4.log').read_text())
b.check('rerun tarball sha256 equals the value the card printed', bool(card_hash) and sha256(ROOT / TAR) == card_hash.group(1))
st = json.loads((ROOT / ST).read_text())['results']
b.check('rerun session named in the state file', 'c715b669' in st['env']['uname']['node'])
conv = st['convert']
b.add('rerun.container_shards_equal_spark', sum(1 for v in conv.values() if v['equal'] and v['card'] == v['spark']), 'shards', 'COUNT', ST,
      f'on-card conversions whose sha256 equals the Spark container (of {len(conv)} checked)')
b.check('rerun on-card conversion equals the Spark container', all(v['equal'] and v['card'] == v['spark'] for v in conv.values()))


def timing(o):
    per = (statistics.median(o['r50_ns']) - statistics.median(o['r1_ns'])) / (o['repeat'] - 1) / 1e6   # ms per dispatch
    return per, o['weight_bytes_per_dispatch'] / (per / 1e3) / 1e12                                     # TB/s


K = st['kernels']
b.add('rerun.shapes', len(K), 'shapes', 'COUNT', ST, 'shapes in the rerun state file')
rates = {'q4': [], 'q4r2': [], 'fp8': [], 'bf16': []}
per_tensor = {}                       # (shape, kernel) -> ms per tensor
bit_ok = {'q4': 0, 'q4r2': 0}
missing = []
for shape, rec in K.items():
    key = re.sub(r'[^A-Za-z0-9]+', '_', shape).strip('_')
    for kern in ('q4', 'q4r2', 'baseline'):
        o = rec[kern]
        name = kern if kern != 'baseline' else rec['baseline_kind']
        if 'error' in o:
            missing.append(f'{shape}: {name} ({o["error"]})')
            b.add(f'rerun.{key}.{name}_TBps', 'NOT_RUN', 'TB/s', 'RECORD', ST, f'launch hung and was killed: {o["error"]}')
            continue
        per, tbps = timing(o)
        b.check(f'rerun {shape} {name} per_dispatch_ms', abs(per - o['per_dispatch_ms']) < 1e-12)
        b.check(f'rerun {shape} {name} GBps', abs(tbps * 1e3 - o['weight_GBps']) < 1e-6)
        per_tensor[(shape, name)] = per / rec['tile_copies']
        b.check(f'rerun {shape} {name} per_tensor_ms', abs(per / rec['tile_copies'] - o['per_tensor_ms']) < 1e-12)
        rates[name].append(tbps)
        b.add(f'rerun.{key}.{name}_TBps', round(tbps, 2), 'TB/s', 'MEASURED', ST,
              'weight and scale bytes per dispatch / ((median of 3 at repeat 50 - median of 3 at repeat 1) / 49), host clock')
        if kern != 'baseline':
            ok = o['output_bits_equal_oracle'] == o['rows'] and o['verdict'] == 'PASS' and o['repeat_output_equal']
            b.check(f'rerun {shape} {name} bit-exact', ok)
            bit_ok[kern] += ok
            b.add(f'rerun.{key}.{name}_bits', f"{o['output_bits_equal_oracle']}/{o['rows']}", 'outputs', 'MEASURED', ST,
                  'output words bit-equal to the oracle / outputs')
part = {}
for l in lines:
    shp = l.split(' KERNEL ', 1)[1].split(' {', 1)[0]
    for kern, body in OBJ.findall(l):
        o = json.loads(body)
        part[(shp, kern)] = o['weight_bytes_per_dispatch'] / ((statistics.median(o['r50_ns']) - statistics.median(o['r1_ns'])) / (o['repeat'] - 1) / 1e9) / 1e12
diffs = [100 * (1 - timing(K[s][k])[1] / v) for (s, k), v in part.items() if s in K and 'error' not in K[s][k]]
b.add('both_sessions.shared_timings', len(diffs), 'timings', 'COUNT', [ST, LOG], 'kernel and shape pairs timed in both the killed session and the rerun')
b.add('both_sessions.rerun_slower_pct_min', round(min(diffs), 1), '%', 'DERIVED', [ST, LOG], 'rerun rate below the killed session rate, smallest difference')
b.add('both_sessions.rerun_slower_pct_max', round(max(diffs), 1), '%', 'DERIVED', [ST, LOG], 'largest difference')
b.add('both_sessions.rerun_slower_pct_median', round(statistics.median(diffs), 1), '%', 'DERIVED', [ST, LOG], 'median difference')
b.add('rerun.q4_bitexact_shapes', bit_ok['q4'], 'shapes', 'COUNT', ST, 'shapes where q4gemv_d8 ran and every output word equals the oracle')
b.add('rerun.q4r2_bitexact_shapes', bit_ok['q4r2'], 'shapes', 'COUNT', ST, 'shapes where q4gemv_d8r2 ran and every output word equals the oracle')
b.add('rerun.bitexact_results', bit_ok['q4'] + bit_ok['q4r2'], 'results', 'COUNT', ST, 'kernel x shape results that ran, all bit-exact')
b.add('rerun.timings_missing', len(missing), 'timings', 'COUNT', ST, '; '.join(missing))
# across both sessions: every shape has a bit-exact result for each kernel
partial_q4 = {l.split(' KERNEL ', 1)[1].split(' {', 1)[0] for l in lines if '"q4": {' in l}
both = sum(1 for s, r in K.items() if ('error' not in r['q4'] or s in partial_q4) and 'error' not in r['q4r2'])
b.add('both_sessions.shapes_bitexact_both_kernels', both, 'shapes', 'COUNT', [ST, LOG],
      'shapes with a bit-exact result for both kernels, counting the killed session for a shape whose rerun launch hung')
for name, label in [('q4', 'q4'), ('q4r2', 'q4r2'), ('fp8', 'fp8')]:
    v = rates[name]
    b.add(f'rerun.{label}_TBps_min', round(min(v), 2), 'TB/s', 'MEASURED', ST, f'minimum over the {len(v)} shapes with a timing')
    b.add(f'rerun.{label}_TBps_max', round(max(v), 2), 'TB/s', 'MEASURED', ST, f'maximum over the {len(v)} shapes with a timing')
    b.add(f'rerun.{label}_pct_peak_min', round(100 * min(v) * 1e12 / PEAK, 1), '%', 'DERIVED', ST, 'minimum as percent of 5.3 TB/s')
    b.add(f'rerun.{label}_pct_peak_max', round(100 * max(v) * 1e12 / PEAK, 1), '%', 'DERIVED', ST, 'maximum as percent of 5.3 TB/s')
    b.add(f'rerun.{label}_timed_shapes', len(v), 'shapes', 'COUNT', ST, 'shapes with a timing for this kernel')
b.add('rerun.bf16_head_TBps', round(rates['bf16'][0], 2), 'TB/s', 'MEASURED', ST, 'BF16 output head with the bf16gemv2 kernel, same method')

# Per-token sums of isolated per-tensor times (MODELED from MEASURED): counts per token from the plan's class table.
cls = list(csv.DictReader(open(ROOT / 'amd-kernel-speed/model/classes.tsv'), delimiter='\t'))


def count(shape):
    stem = shape.replace(' seg4', '')
    n = [int(r['dispatches_per_step']) for r in cls if r['class'] == stem]
    b.check(f'count for {shape}', len(n) == 1)
    return n[0]


def t(shape, kern):
    # the lane's two fills, stated in the report: FP8 gate_proj uses up_proj's time (its launch hung);
    # v_proj was not measured and uses k_proj's time (same shape)
    if (shape, kern) not in per_tensor and shape.startswith('mlp gate_proj'):
        shape = 'mlp up_proj 17408x5120'
    return per_tensor[(shape, kern)]


lin = [s for s in K if not s.startswith('lm_head')]
vcount = [int(r['dispatches_per_step']) for r in cls if r['class'] == 'attn v_proj 1024x5120'][0]
ksh = 'attn k_proj 1024x5120'


def total(kern):
    return sum(count(s) * t(s, kern) for s in lin) + vcount * t(ksh, kern)


head = 'lm_head 248320x5120'
src_sum = [ST, 'amd-kernel-speed/model/classes.tsv']
fp8_lin, q4_lin = total('fp8'), total('q4r2')
how = ('sum over linear tensors of dispatches per token x isolated per-tensor time; excludes launch gaps and every non-matrix kernel; '
       'FP8 gate_proj filled with up_proj, v_proj with k_proj')
b.add('rerun.sum_fp8_linears_ms', round(fp8_lin, 2), 'ms/token', 'MODELED', src_sum, how)
b.add('rerun.sum_q4r2_linears_ms', round(q4_lin, 2), 'ms/token', 'MODELED', src_sum, how)
s_fp8 = b.add('rerun.sum_fp8_bf16head_ms', round(fp8_lin + per_tensor[(head, 'bf16')], 2), 'ms/token', 'MODELED', src_sum, how + '; plus the BF16 head')
b.add('rerun.sum_q4_bf16head_ms', round(q4_lin + per_tensor[(head, 'bf16')], 2), 'ms/token', 'MODELED', src_sum, how + '; plus the BF16 head')
b.add('rerun.sum_q4_q4head_ms', round(q4_lin + per_tensor[(head, 'q4r2')], 2), 'ms/token', 'MODELED', src_sum, how + '; plus the 4-bit head')
b.add('rerun.sum_fp8_over_q4_q4head', round((fp8_lin + per_tensor[(head, 'bf16')]) / (q4_lin + per_tensor[(head, 'q4r2')]), 2), 'x', 'DERIVED',
      src_sum, 'FP8 with BF16 head sum / 4-bit with 4-bit head sum')
# The 4-bit sum against the 4-bit floor: candidate A's bytes per token (four-bit/FORMAT.md table row A) at 5.3 TB/s.
FMT = 'four-bit/FORMAT.md'
ma_ = re.search(r'^\| A: Q4 everything, `lm_head` BF16 \| ([\d,]+) \| 0\.577 \| 2\.93 ms \|$', (ROOT / FMT).read_text(), re.M)
b.check('FORMAT.md candidate A row present', bool(ma_))
q4_floor = int(ma_.group(1).replace(',', '')) / PEAK * 1e3
b.check('candidate A floor rounds to the quoted 2.93 ms', round(q4_floor, 2) == 2.93)
b.add('rerun.sum_q4_bf16head_over_q4_floor', round((q4_lin + per_tensor[(head, 'bf16')]) / q4_floor, 2), 'x', 'DERIVED', src_sum + [FMT],
      '4-bit linears with BF16 head sum / (candidate A bytes per token, which also keep the head in BF16, / 5.3 TB/s)')

# The measured whole step against the sum of its matrix kernels (FP8 linears and BF16 head, isolated).
AK = 'amd-kernel-speed/card/261114001549-e787e65d/out/runs'
for name in ('full-a', 'v1all'):
    ms = omega_run(ROOT / f'{AK}/{name}.txt')['decode_ms_median']
    b.add(f'rerun.step_{name}_over_fp8_sum', round(ms / (fp8_lin + per_tensor[(head, "bf16")]), 2), 'x', 'DERIVED', [f'{AK}/{name}.txt', ST],
          f'measured whole decode step ({name}, kernel session) / isolated FP8 matrix-kernel sum; two different sessions and methods')
kj = OUT / 'kernels.json'
b.check('kernels.json present (run kernels.py first)', kj.exists())
inplan = sum(r['value'] for r in json.loads(kj.read_text())
             if r['id'].endswith('.ms_per_step') and ('fp8gemv_b128f' in r['id'] or 'bf16gemv2_248320x5120' in r['id']))
b.add('rerun.inplan_matrix_sum_ms', round(inplan, 2), 'ms/token', 'DERIVED', [AK, 'amd-kernel-speed/model/classes.tsv'],
      'sum of the in-plan (inserted-copy) ms per step of the FP8 matrix-vector classes and the BF16 head, kernel session (from kernels.py)')

# Launcher hangs in the rerun: every launch's stdout in the card tarball (unpacked by the lane as tar/out).
OUTD = ROOT / f'{RR}/tar/out'
outs = sorted(OUTD.glob('*.stdout'))
done_ = [p for p in outs if 'OMEGA step cleanup ok errno=0 phase=after' in p.read_text(errors='replace')]
alarm = [p for p in outs if 'OMEGA alarm slow_dispatch_ns=' in p.read_text(errors='replace')]
b.add('hang.launches', len(outs), 'launches', 'COUNT', f'{RR}/tar/out', 'launcher stdout captures in the rerun tarball')
b.add('hang.launches_completed', len(done_), 'launches', 'COUNT', f'{RR}/tar/out', 'captures that end with the cleanup line')
b.add('hang.launches_alarmed', len(alarm), 'launches', 'COUNT', f'{RR}/tar/out', 'captures with the 10 s slow-wait alarm')
b.check('the alarmed launches are exactly the ones that did not complete', set(alarm) == set(outs) - set(done_))
hung = {p.name: p.read_text(errors='replace').strip().splitlines()[-3:] for p in alarm}
b.check('hang 1 is q4gemv_d8 out_proj at repeat 1, stopped after signal_wait before',
        'out_proj-layers-0.q4.r1.0.stdout' in hung and any('signal_wait ok errno=0 phase=before' in l for l in hung['out_proj-layers-0.q4.r1.0.stdout']))
b.check('hang 2 is the FP8 gate_proj third launch at repeat 50, stopped before dispatch after',
        'gate_proj-layers-0.f8.r50.2.stdout' in hung and any('dispatch ok errno=0 phase=before detail=50' in l for l in hung['gate_proj-layers-0.f8.r50.2.stdout']))
gp = [p for p in done_ if p.name.startswith('gate_proj-layers-0.f8.')]
b.add('hang.gate_fp8_completed_same_shape', len(gp), 'launches', 'COUNT', f'{RR}/tar/out',
      'completed launches of the same FP8 kernel and shape before hang 2 (3 at repeat 1, 2 at repeat 50)')

man = json.loads((ROOT / 'four-bit/evidence/container-MANIFEST.json').read_text())
sh = man['shards']
size = sum(v['bytes'] for v in sh.values())
src = sum(v['source_bytes'] for v in sh.values())
b.add('container.bytes', size, 'B', 'COUNT', 'four-bit/evidence/container-MANIFEST.json', 'sum of shard bytes')
b.add('container.source_bytes', src, 'B', 'COUNT', 'four-bit/evidence/container-MANIFEST.json', 'sum of source shard bytes')
b.add('container.ratio', round(size / src, 3), 'x', 'DERIVED', 'four-bit/evidence/container-MANIFEST.json', 'container / source bytes')
b.add('container.shards', len(sh), 'files', 'COUNT', 'four-bit/evidence/container-MANIFEST.json', 'shard count')
# Four-row kernel q4gemv_d8r4: CPU gate only, its card timing not run
G4 = 'four-bit/evidence/gate-r4.log'
gr4 = (ROOT / G4).read_text()
m4 = re.search(r'^Q4GEMV GATE PASS cases=(\d+) mutants=(\d+) caught=(\d+)$', gr4, re.M)
b.check('r4 gate PASS line present, every mutant caught', bool(m4) and m4.group(2) == m4.group(3))
per_k = {}
for k in re.findall(r'^PASS (q4gemv_d8\w*) (?:case|real rows) ', gr4, re.M):
    per_k[k] = per_k.get(k, 0) + 1
b.check('r4 gate: three kernels, the same case count each, summing to the PASS line',
        sorted(per_k) == ['q4gemv_d8', 'q4gemv_d8r2', 'q4gemv_d8r4'] and len(set(per_k.values())) == 1 and sum(per_k.values()) == int(m4.group(1)))
mut = re.findall(r'^CONTROL kernel mutant (\S+) CAUGHT ', gr4, re.M)
b.add('r4_gate.cases', int(m4.group(1)), 'cases', 'COUNT', G4, 'Q4GEMV GATE PASS line, three kernels including q4gemv_d8r4')
b.add('r4_gate.cases_d8r4', per_k['q4gemv_d8r4'], 'cases', 'COUNT', G4, 'PASS q4gemv_d8r4 case and real-rows lines')
b.add('r4_gate.mutants', int(m4.group(2)), 'mutants', 'COUNT', G4, 'same line')
b.add('r4_gate.mutants_caught', int(m4.group(3)), 'mutants', 'COUNT', G4, 'same line')
b.add('r4_gate.mutants_r4_specific', sum(m.startswith('r4-') for m in mut), 'mutants', 'COUNT', G4, 'CONTROL kernel mutant r4-... CAUGHT lines')
b.add('quality.state', 'PENDING', '', 'RECORD', [], 'quality arms not finished when this report was written (four-bit/RESULT.md section 4, lane notes)')
b.close()
