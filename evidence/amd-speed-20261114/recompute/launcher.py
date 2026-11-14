#!/usr/bin/env python3
# Launcher: AmdLaunchGap card run 1 (session 52ef268c), raw logs in amd-launch-gap/card/final/logs.
# Run 2 (plan3, session e8cddcb1) is added here when its tarball lands in amd-launch-gap/card/run2/.
import json, statistics
from pathlib import Path
from common import ROOT, Book, omega_run, first_difference, sha256

b = Book('launcher')
L = 'amd-launch-gap/card/final/logs'
pub = json.loads((ROOT / 'amd-launch-gap/card/bundle/published-ids.json').read_text())['ids']
b.check('published-ids.json equals the published sample1 ids', pub == omega_run(
    ROOT / 'amd-launch-gap/src/CUDApp-base/evidence/amd-qwen38-native-refusal-20261113/native/sample1.stdout')['ids'])
tar = ROOT / 'amd-launch-gap/card/gap-evidence.tar.gz'
want = (ROOT / 'amd-launch-gap/card/gap-evidence.tar.gz.sha256').read_text().split()[0]
b.check('run1 tarball sha256 equals the card-side value', sha256(tar) == want)


def arm(prefix, runs, kind='omega', L=L):
    meds, files = [], []
    for r in runs:
        if kind == 'omega':
            f = f'{L}/{prefix}-{r}.txt'
            x = omega_run(ROOT / f)
            b.check(f'{prefix}-{r} ids', x['ids'] == pub, f'first difference {first_difference(x["ids"], pub)}')
            meds.append(x['decode_ms_median'])
        else:
            f = f'{L}/{prefix}-{r}.json'
            x = json.loads((ROOT / f).read_text())
            b.check(f'{prefix}-{r} ids', x['ids'] == pub)
            meds.append(x['decode_ms_median'])
        files.append(f)
    return statistics.median(meds), files, len(meds)


five = ['run1', 'run2', 'run3', 'run4', 'run5']
two = ['run1', 'run2']
arms = {}
for key, prefix, runs, kind in [('plan', 'omega-old-plan', five, 'omega'), ('plan2', 'omega-new-plan2', five, 'omega'),
                                ('hip', 'hip-graph', five, 'hip'), ('planscope', 'omega-new-planscope', two, 'omega'),
                                ('planstage', 'omega-new-planstage', two, 'omega'), ('plan_newbin', 'omega-new-plan', two, 'omega')]:
    m, files, n = arm(prefix, runs, kind)
    arms[key] = m
    b.add(f'run1.{key}_ms', round(m, 4), 'ms/token', 'MEASURED', files, f'median of {n} per-run decode medians')

for ctl in ('omega-control-old-plan', 'omega-control-new-plan2'):
    x = omega_run(ROOT / f'{L}/{ctl}.txt')
    b.add(f'run1.{ctl}_first_difference', first_difference(x['ids'], pub), 'index', 'MEASURED', f'{L}/{ctl}.txt',
          'altered-weights plan: first index differing from the published ids')

# every non-control run in the session, warmups included, must print the published ids
allruns = sorted(p for p in (ROOT / L).glob('omega-*.txt') if 'control' not in p.name)
hips = sorted((ROOT / L).glob('hip-graph-*.json'))
ok = sum(omega_run(p)['ids'] == pub for p in allruns) + sum(json.loads(p.read_text())['ids'] == pub for p in hips)
b.check('all non-control runs print the published ids', ok == len(allruns) + len(hips), f'{ok} of {len(allruns) + len(hips)}')
b.add('run1.runs_ids_equal', ok, 'runs', 'COUNT', L, 'non-control runs (warmups included) whose 256 ids equal the published ids')

src = [f'{L}/omega-old-plan-run1.txt', f'{L}/omega-new-plan2-run1.txt', f'{L}/hip-graph-run1.json']
b.add('run1.gain_scope_ms', round(arms['plan'] - arms['planscope'], 4), 'ms/token', 'DERIVED', src, 'plan minus planscope')
b.add('run1.gain_stage_ms', round(arms['plan'] - arms['planstage'], 4), 'ms/token', 'DERIVED', src, 'plan minus planstage')
b.add('run1.gain_plan2_ms', round(arms['plan'] - arms['plan2'], 4), 'ms/token', 'DERIVED', src, 'plan minus plan2')
b.add('run1.residual_vs_hip_ms', round(arms['plan2'] - arms['hip'], 4), 'ms/token', 'DERIVED', src, 'plan2 minus HIP graph')
b.add('run1.plan2_tokens_s', round(1000 / arms['plan2'], 2), 'tokens/s', 'DERIVED', src, '1000 / plan2 ms')

# Run 2 (session e8cddcb1): old plan, plan3, HIP interleaved 1+5; plan2 x2; plan3 control.
L2 = 'amd-launch-gap/card/run2/final/logs'
t2 = ROOT / 'amd-launch-gap/card/run2/gap-evidence.tar.gz'
b.check('run2 tarball sha256 equals the card-side value',
        sha256(t2) == (ROOT / 'amd-launch-gap/card/run2/gap-evidence.tar.gz.sha256').read_text().split()[0])
a2 = {}
for key, prefix, runs, kind in [('plan', 'omega-old-plan', five, 'omega'), ('plan3', 'omega-new-plan3', five, 'omega'),
                                ('hip', 'hip-graph', five, 'hip'), ('plan2', 'omega-new-plan2', two, 'omega')]:
    m, files, n = arm(prefix, runs, kind, L2)
    a2[key] = m
    b.add(f'run2.{key}_ms', round(m, 4), 'ms/token', 'MEASURED', files, f'median of {n} per-run decode medians')
for p in sorted((ROOT / L2).glob('omega-control-*.txt')):
    x = omega_run(p)
    b.add(f'run2.{p.stem}_first_difference', first_difference(x['ids'], pub), 'index', 'MEASURED', f'{L2}/{p.name}',
          'altered-weights plan: first index differing from the published ids')
r2 = sorted(p for p in (ROOT / L2).glob('omega-*.txt') if 'control' not in p.name)
h2 = sorted((ROOT / L2).glob('hip-graph-*.json'))
ok2 = sum(omega_run(p)['ids'] == pub for p in r2) + sum(json.loads(p.read_text())['ids'] == pub for p in h2)
b.check('run2 all non-control runs print the published ids', ok2 == len(r2) + len(h2), f'{ok2} of {len(r2) + len(h2)}')
b.add('run2.runs_ids_equal', ok2, 'runs', 'COUNT', L2, 'non-control runs (warmups included) whose 256 ids equal the published ids')
s2 = [f'{L2}/omega-old-plan-run1.txt', f'{L2}/omega-new-plan3-run1.txt', f'{L2}/hip-graph-run1.json']
b.add('run2.gain_plan3_ms', round(a2['plan'] - a2['plan3'], 4), 'ms/token', 'DERIVED', s2, 'plan minus plan3')
b.add('run2.residual_plan3_vs_hip_ms', round(a2['plan3'] - a2['hip'], 4), 'ms/token', 'DERIVED', s2, 'plan3 minus HIP graph')
b.add('run2.residual_plan2_vs_hip_ms', round(a2['plan2'] - a2['hip'], 4), 'ms/token', 'DERIVED', s2, 'plan2 minus HIP graph')
b.add('run2.plan2_minus_plan3_ms', round(a2['plan2'] - a2['plan3'], 4), 'ms/token', 'DERIVED', s2, 'plan2 minus plan3 (2 against 5 runs)')
b.add('run2.plan3_tokens_s', round(1000 / a2['plan3'], 2), 'tokens/s', 'DERIVED', s2, '1000 / plan3 ms')

# Drift (Main's finding, MAIN-VERIFY.md): the unchanged published launcher across and within sessions.
def per_run(prefix, runs, L, kind='omega'):
    out = []
    for r in runs:
        if kind == 'omega':
            out.append(round(omega_run(ROOT / f'{L}/{prefix}-{r}.txt')['decode_ms_median'], 3))
        else:
            out.append(round(json.loads((ROOT / f'{L}/{prefix}-{r}.json').read_text())['decode_ms_median'], 3))
    return out
old2 = per_run('omega-old-plan', five, L2)
b.add('run2.plan_run_medians', old2, 'ms/token', 'MEASURED', [f'{L2}/omega-old-plan-{r}.txt' for r in five],
      'per-run decode medians of the published launcher, in run order')
b.add('run2.plan_monotonic_rise', all(x < y for x, y in zip(old2, old2[1:])), '', 'RECORD', f'{L2}/omega-old-plan-run1.txt',
      'True if the five per-run medians rise strictly in run order')
b.add('run2.plan_warmup_ms', round(omega_run(ROOT / f'{L2}/omega-old-plan-warmup.txt')['decode_ms_median'], 3), 'ms/token',
      'MEASURED', f'{L2}/omega-old-plan-warmup.txt', 'warmup run decode median (not counted in the arm)')
b.add('run2.plan_within_spread_ms', round(max(old2) - min(old2), 3), 'ms/token', 'DERIVED', f'{L2}/omega-old-plan-run1.txt',
      'last minus first of the five run medians')
b.add('run2.plan3_run_medians', per_run('omega-new-plan3', five, L2), 'ms/token', 'MEASURED',
      [f'{L2}/omega-new-plan3-{r}.txt' for r in five], 'per-run decode medians, in run order')
b.add('run2.hip_run_medians', per_run('hip-graph', five, L2, 'hip'), 'ms/token', 'MEASURED',
      [f'{L2}/hip-graph-{r}.json' for r in five], 'per-run decode medians, in run order')
b.add('drift.plan_run2_minus_run1_ms', round(a2['plan'] - arms['plan'], 4), 'ms/token', 'DERIVED',
      [f'{L}/omega-old-plan-run1.txt', f'{L2}/omega-old-plan-run1.txt'], 'same published launcher, run 2 session minus run 1 session')
b.add('run2.state', 'LANDED', '', 'RECORD', 'amd-launch-gap/card/run2/gap-evidence.tar.gz', 'plan3 run evidence present')

# Hang check (HANG-20261114.md): every launcher plan process in the two launcher sessions and the kernel session
# completed (prints its OMEGA_PLAN ids line) and none raised the 10 s slow-wait alarm.
AKR = 'amd-kernel-speed/card/261114001549-e787e65d/out/runs'
procs = sorted((ROOT / L).glob('omega-*.txt')) + sorted((ROOT / L2).glob('omega-*.txt')) + sorted((ROOT / AKR).glob('*.txt'))
complete = sum('\nOMEGA_PLAN ids=' in ('\n' + p.read_text(errors='replace')) for p in procs)
alarms = sum('OMEGA alarm' in p.read_text(errors='replace') for p in procs)
b.add('hang.plan_processes', len(procs), 'processes', 'COUNT', [L, L2, AKR], 'launcher plan processes in sessions 52ef268c, e8cddcb1, e787e65d')
b.add('hang.plan_processes_complete', complete, 'processes', 'COUNT', [L, L2, AKR], 'of those, printing the OMEGA_PLAN ids line')
b.add('hang.plan_processes_alarmed', alarms, 'processes', 'COUNT', [L, L2, AKR], 'of those, with the 10 s slow-wait alarm')
b.check('every plan process completed without an alarm', complete == len(procs) and alarms == 0)
import re
def gate_pass(path):
    """The AMD-KFD-LAUNCH CPU PASS line of a check.py gate log, parsed: checks, failures, device and launcher sha256.
    The log names the launcher binaries it built, not a commit."""
    lines = re.findall(r'^AMD-KFD-LAUNCH CPU PASS (\{.*\})$', (ROOT / path).read_text(), re.M)
    return json.loads(lines[-1]) if len(lines) == 1 else {}
# The gate log of branch head d957526e (the fix plus the alarm diagnostics; x86 launcher a6aee5ef, which the run 3
# card logs hash-check). The log names the launcher binaries, not a commit.
G14 = 'amd-launch-gap/build-r14-spark-gate.log'
g14 = gate_pass(G14)
b.check('r14 gate log: one PASS line, launcher a6aee5ef, 0 failures, device NOT_RUN',
        g14.get('binaries', {}).get('x86', '').startswith('a6aee5ef6f144ada') and g14.get('failures') == 0 and g14.get('device') == 'NOT_RUN')
b.add('hang.fix_gate_checks', int(g14.get('checks', -1)), 'checks', 'COUNT', G14,
      '"checks" in the AMD-KFD-LAUNCH CPU PASS line; the line names x86 launcher a6aee5ef and no commit')

# ---- Run 3 (session 7f66e322): micro-benchmark, then 9 strictly interleaved cycles plan3, plan3c, HIP; then plan3t,
# three runs of the published launcher, and the plan3 control. New launcher a6aee5ef (branch head d957526e).
R3 = 'amd-launch-gap/card/run3'
L3 = f'{R3}/final/logs'
for t in ('gap-evidence.tar.gz', 'gap-micro_done.tar.gz'):
    b.check(f'run3 {t} sha256 equals the card-side value', sha256(ROOT / f'{R3}/{t}') == (ROOT / f'{R3}/{t}.sha256').read_text().split()[0])
bad3 = 0
for line in (ROOT / f'{L3}/SHA256SUMS').read_text().splitlines():
    h, name = line.split(None, 1)
    p = ROOT / L3 / name.strip().lstrip('*')
    bad3 += (not p.is_file()) or sha256(p) != h
b.check('run3 logs SHA256SUMS all match', bad3 == 0, f'{bad3} mismatches')
rl = (ROOT / f'{L3}/run.log').read_text()
b.check('run3 launcher binaries: published 08a50491 and new a6aee5ef verified on the card',
        '08a504911ec6e87a9746632a14d22245155597e3a8ed384c93c0e5260bb691e1  /workspace/gap3/bin-old/omega-launch' in rl
        and 'a6aee5ef6f144ada59941197c72e3ca8c84fab1f448b99484bf727f67982afc7  /workspace/gap3/bin-new/omega-launch' in rl
        and rl.count('omega-launch: OK') >= 2)
# strict interleaving, read from the start stamps in run.log
starts = [m.group(1) for m in re.finditer(r'^(\S+) start 2026-11-14T', rl, re.M)]
model_order = [s for s in starts if re.fullmatch(r'(new-plan3|new-plan3c|graph)-c\d', s)]
b.check('run3 model cycles strictly interleaved plan3, plan3c, HIP for c1..c9',
        model_order == [f'{a}-c{c}' for c in range(1, 10) for a in ('new-plan3', 'new-plan3c', 'graph')], str(model_order[:6]))
b.check('run3 published-launcher runs came after the nine cycles', starts.index('old-plan-c1') > starts.index('graph-c9'))
lane3 = json.loads((ROOT / f'{R3}/final/analysis.json').read_text())
cyc = range(1, 10)


def om3(name):
    x = omega_run(ROOT / f'{L3}/{name}.txt')
    b.check(f'run3 {name} ids', x['ids'] == pub and x['steps'] == 285)
    return x['decode_ms_median']


def hip3(c, key='decode_ms_median'):
    d = json.loads((ROOT / f'{L3}/hip-graph-c{c}.json').read_text())
    b.check(f'run3 hip-graph-c{c} ids', d['ids'] == pub)
    return d[key]


p3 = [om3(f'omega-new-plan3-c{c}') for c in cyc]
p3c = [om3(f'omega-new-plan3c-c{c}') for c in cyc]
h3 = [hip3(c) for c in cyc]
hw3 = [hip3(c, 'forward_wall_ms_median') for c in cyc]
old3 = [om3(f'omega-old-plan-c{c}') for c in (1, 2, 3)]
for key, mine in (('plan3', p3), ('plan3c', p3c), ('hip', h3), ('hip-forward-wall', hw3), ('old-plan', old3)):
    b.check(f'run3 {key} equals lane analysis.json', all(abs(a - c) < 1e-9 for a, c in zip(mine, lane3['model'][key])) and len(mine) == len(lane3['model'][key]))
f3 = {'plan3': [f'{L3}/omega-new-plan3-c{c}.txt' for c in cyc], 'plan3c': [f'{L3}/omega-new-plan3c-c{c}.txt' for c in cyc],
      'hip': [f'{L3}/hip-graph-c{c}.json' for c in cyc], 'old': [f'{L3}/omega-old-plan-c{c}.txt' for c in (1, 2, 3)]}
b.add('run3.plan3_cycles', [round(x, 3) for x in p3], 'ms/token', 'MEASURED', f3['plan3'], 'decode median per cycle c1..c9')
b.add('run3.plan3c_cycles', [round(x, 3) for x in p3c], 'ms/token', 'MEASURED', f3['plan3c'], 'decode median per cycle c1..c9')
b.add('run3.hip_cycles', [round(x, 3) for x in h3], 'ms/token', 'MEASURED', f3['hip'], 'HIP decode_ms median per cycle c1..c9')
b.add('run3.hip_forward_wall_cycles', [round(x, 3) for x in hw3], 'ms/token', 'MEASURED', f3['hip'],
      'HIP forward_wall_ms median per cycle: decode plus the logits copy, token readback and Python call')
b.add('run3.plan_runs', [round(x, 3) for x in old3], 'ms/token', 'MEASURED', f3['old'], 'published launcher, three runs after the cycles')
for key, v, fs in (('plan3', p3, f3['plan3']), ('plan3c', p3c, f3['plan3c']), ('hip', h3, f3['hip']), ('plan', old3, f3['old'])):
    b.add(f'run3.{key}_ms', round(statistics.median(v), 3), 'ms/token', 'MEASURED', fs, f'median of {len(v)} run medians')
    b.add(f'run3.{key}_min_ms', round(min(v), 3), 'ms/token', 'MEASURED', fs, 'smallest run median')
    b.add(f'run3.{key}_max_ms', round(max(v), 3), 'ms/token', 'MEASURED', fs, 'largest run median')
b.add('run3.hip_forward_wall_min_ms', round(min(hw3), 3), 'ms/token', 'MEASURED', f3['hip'], 'smallest per-cycle forward wall median')
b.add('run3.hip_forward_wall_max_ms', round(max(hw3), 3), 'ms/token', 'MEASURED', f3['hip'], 'largest per-cycle forward wall median')


def paired(name, a, c, how, src):
    d = [x - y for x, y in zip(a, c)]
    b.add(f'run3.{name}_cycles', [round(x, 3) for x in d], 'ms/token', 'DERIVED', src, how + ', per cycle c1..c9')
    b.add(f'run3.{name}_median', round(statistics.median(d), 3), 'ms/token', 'DERIVED', src, how + ': median of 9 paired cycles')
    b.add(f'run3.{name}_min', round(min(d), 3), 'ms/token', 'DERIVED', src, how + ': smallest')
    b.add(f'run3.{name}_max', round(max(d), 3), 'ms/token', 'DERIVED', src, how + ': largest')
    b.add(f'run3.{name}_cycles_omega_behind', sum(x > 0 for x in d), 'cycles', 'COUNT', src, how + ': cycles with a positive difference')
    return d


s3 = f3['plan3'] + f3['hip']
raw_d = paired('plan3_minus_hip', p3, h3, 'plan3 minus HIP decode_ms (raw clocks)', s3)
paired('plan3c_minus_hip', p3c, h3, 'plan3c minus HIP decode_ms (raw clocks)', f3['plan3c'] + f3['hip'])
paired('plan3c_minus_plan3', p3c, p3, 'plan3c minus plan3', f3['plan3c'] + f3['plan3'])
paired('plan3_minus_hip_forward_wall', p3, hw3, 'plan3 minus HIP forward_wall_ms (not like for like: HIP side includes a logits copy)', s3)
wall_extra = [w - d for w, d in zip(hw3, h3)]
b.add('run3.hip_wall_minus_decode_median', round(statistics.median(wall_extra), 3), 'ms/token', 'DERIVED', f3['hip'], 'forward wall minus decode, median of 9 cycles')
b.add('run3.hip_wall_minus_decode_min', round(min(wall_extra), 3), 'ms/token', 'DERIVED', f3['hip'], 'smallest')
b.add('run3.hip_wall_minus_decode_max', round(max(wall_extra), 3), 'ms/token', 'DERIVED', f3['hip'], 'largest')
b.check('run3 raw paired median equals lane analysis.json', abs(statistics.median(raw_d) - lane3['model']['paired-plan3-minus-hip-ms']['median']) < 1e-9)

# micro-benchmark: N dispatches of step_advance per step, 100 steps per process, cycles c1..c9 (c0 exists and is left
# out, as in the lane's analysis); Omega per-step = median of steps 1.., HIP per-step = the file's median_step_us
def micro_omega(arm, N, c):
    us = [int(u) for u in re.findall(r'^OMEGA_STEP \d+ tok=-?\d+ us=(\d+)$', (ROOT / f'{L3}/micro-{arm}-n{N}-c{c}.txt').read_text(), re.M)]
    b.check(f'micro {arm} n{N} c{c} has 100 steps', len(us) == 100)
    return statistics.median(us[1:])


def micro_hip(N, c, key='median_step_us'):
    return json.loads((ROOT / f'{L3}/hipmicro-hip-n{N}-c{c}.json').read_text())[key]


mo = {(arm, N): [micro_omega(arm, N, c) for c in cyc] for arm in ('plan3', 'plan3c') for N in (1, 1126)}
mh = {N: [micro_hip(N, c) for c in cyc] for N in (1, 1126)}
for arm in ('plan3', 'plan3c'):
    for N in (1, 1126):
        b.check(f'micro {arm} n{N} equals lane analysis.json', mo[(arm, N)] == lane3['micro'][f'{arm}-n{N}'])
for N in (1, 1126):
    b.check(f'micro hip n{N} equals lane analysis.json', all(abs(a - c) < 1e-9 for a, c in zip(mh[N], lane3['micro'][f'hip-n{N}'])))
ms = [f'{L3}/micro-plan3-n1126-c1.txt', f'{L3}/hipmicro-hip-n1126-c1.json']
for N in (1, 1126):
    b.add(f'run3.micro.plan3_n{N}_us', round(statistics.median(mo[('plan3', N)]), 1), 'us/step', 'MEASURED', ms, f'N = {N}: median over 9 cycles of the per-process median step')
    b.add(f'run3.micro.plan3c_n{N}_us', round(statistics.median(mo[('plan3c', N)]), 1), 'us/step', 'MEASURED', ms, f'N = {N}: same, cached ring')
    b.add(f'run3.micro.hip_n{N}_us', round(statistics.median(mh[N]), 1), 'us/step', 'MEASURED', ms, f'N = {N}: median over 9 cycles of median_step_us')
    d = [a - c for a, c in zip(mo[('plan3', N)], mh[N])]
    b.add(f'run3.micro.plan3_minus_hip_n{N}_median_us', round(statistics.median(d), 1), 'us/step', 'DERIVED', ms, f'N = {N}: paired Omega minus HIP, median')
    b.add(f'run3.micro.plan3_minus_hip_n{N}_min_us', round(min(d), 1), 'us/step', 'DERIVED', ms, 'smallest')
    b.add(f'run3.micro.plan3_minus_hip_n{N}_max_us', round(max(d), 1), 'us/step', 'DERIVED', ms, 'largest')
pd = {k: [(a - c) / 1125 for a, c in zip(hi, lo)] for k, hi, lo in (('plan3', mo[('plan3', 1126)], mo[('plan3', 1)]),
                                                                   ('plan3c', mo[('plan3c', 1126)], mo[('plan3c', 1)]), ('hip', mh[1126], mh[1]))}
for k in ('plan3', 'plan3c', 'hip'):
    b.add(f'run3.micro.{k}_per_dispatch_us', round(statistics.median(pd[k]), 3), 'us', 'DERIVED', ms, '(N=1126 step minus N=1 step) / 1125, median over 9 cycles')
dd = [a - c for a, c in zip(pd['plan3'], pd['hip'])]
b.add('run3.micro.per_dispatch_plan3_minus_hip_median_us', round(statistics.median(dd), 4), 'us', 'DERIVED', ms, 'paired per-dispatch difference, median')
b.add('run3.micro.per_dispatch_plan3_minus_hip_min_us', round(min(dd), 4), 'us', 'DERIVED', ms, 'smallest')
b.add('run3.micro.per_dispatch_plan3_minus_hip_max_us', round(max(dd), 4), 'us', 'DERIVED', ms, 'largest')
b.add('run3.micro.launch_per_token_plan3_ms', round(statistics.median(mo[('plan3', 1126)]) / 1e3, 3), 'ms/token', 'DERIVED', ms,
      '1,126 dispatches of a trivial kernel per step, Omega plan3')
b.add('run3.micro.launch_per_token_hip_ms', round(statistics.median(mh[1126]) / 1e3, 3), 'ms/token', 'DERIVED', ms, 'the same under HIP graph replay')
rb = [micro_hip(N, c, 'median_token_readback_us') for N in (1, 1126) for c in cyc]
b.add('run3.micro.hip_token_readback_min_us', round(min(rb), 1), 'us', 'MEASURED', ms, 'HIP 4-byte token readback, smallest per-process median (18 processes)')
b.add('run3.micro.hip_token_readback_max_us', round(max(rb), 1), 'us', 'MEASURED', ms, 'largest')
hip_rb_ms = statistics.median(rb) / 1e3
b.add('run3.micro.hip_token_readback_median_ms', round(hip_rb_ms, 4), 'ms', 'MEASURED', ms, 'median of the 18 per-process medians')

# plan3t phases in the model run: token read after the completion signal (ns from step start)
ph = [tuple(map(int, r)) for r in re.findall(r'^OMEGA_PHASE (\d+) d1=(\d+) dn=(\d+) sig=(\d+) tok=(\d+)$',
                                              (ROOT / f'{L3}/omega-new-plan3t-c1.txt').read_text(), re.M)]
dec = [r for r in ph if r[0] >= 30]
om_read_ms = statistics.median(r[4] - r[3] for r in dec) / 1e6
b.add('run3.omega_token_read_ms', round(om_read_ms, 5), 'ms', 'MEASURED', f'{L3}/omega-new-plan3t-c1.txt',
      'plan3t: token read minus completion signal, median over decode steps')
b.add('run3.last_doorbell_ms', round(statistics.median(r[2] for r in dec) / 1e6, 3), 'ms', 'MEASURED', f'{L3}/omega-new-plan3t-c1.txt',
      'plan3t: last doorbell after step start, median over decode steps')
# like for like: "next token id in host memory" = Omega step + its token read, against HIP decode_ms + HIP token readback.
# The HIP readback is taken from the micro-benchmark (it is not timed inside the model run).
lfl = paired('like_for_like', [x + om_read_ms for x in p3], [y + hip_rb_ms for y in h3],
             '(plan3 + Omega token read) minus (HIP decode_ms + HIP token readback from the micro-benchmark)', s3)

b.add('run3.like_for_like_pct_of_hip', round(100 * statistics.median(lfl) / statistics.median(h3), 1), '%', 'DERIVED', s3,
      'like-for-like paired median as percent of the HIP decode median')

# drift inside the session and the non-interleaved published-launcher comparison
b.add('run3.plan3_first_to_max_ms', round(max(p3) - min(p3), 3), 'ms/token', 'DERIVED', f3['plan3'], 'largest minus smallest plan3 cycle')
b.add('run3.hip_first_to_max_ms', round(max(h3) - min(h3), 3), 'ms/token', 'DERIVED', f3['hip'], 'largest minus smallest HIP cycle')
b.add('run3.plan_minus_plan3_ms', round(statistics.median(old3) - statistics.median(p3), 3), 'ms/token', 'DERIVED', f3['old'] + f3['plan3'],
      'published launcher (3 runs after the cycles) minus plan3 (9 cycles); not interleaved')

# ids, control, alarms, deadline
r3 = sorted(p for p in (ROOT / L3).glob('omega-*.txt') if 'control' not in p.name)
g3 = sorted((ROOT / L3).glob('hip-graph-*.json'))
ok3 = sum(omega_run(p)['ids'] == pub for p in r3) + sum(json.loads(p.read_text())['ids'] == pub for p in g3)
b.check('run3 all non-control model runs print the published ids', ok3 == len(r3) + len(g3), f'{ok3} of {len(r3) + len(g3)}')
b.add('run3.runs_ids_equal', ok3, 'runs', 'COUNT', L3, 'non-control model runs (warmups and plan3t included) whose 256 ids equal the published ids')
c3 = omega_run(ROOT / f'{L3}/omega-control-new-plan3.txt')
b.add('run3.control_first_difference', first_difference(c3['ids'], pub), 'index', 'MEASURED', f'{L3}/omega-control-new-plan3.txt',
      'altered-weights plan under plan3: first index differing from the published ids')
lp = sorted((ROOT / L3).glob('omega-*.txt')) + sorted((ROOT / L3).glob('micro-*.txt'))
al3 = sum('OMEGA alarm' in p.read_text(errors='replace') for p in lp)
fin3 = sum('OMEGA_PLAN complete' in p.read_text(errors='replace') for p in lp)
b.add('run3.launcher_processes', len(lp), 'processes', 'COUNT', L3, 'launcher processes in run 3: model runs, control and micro-benchmark')
b.add('run3.launcher_processes_complete', fin3, 'processes', 'COUNT', L3, 'of those, printing OMEGA_PLAN complete')
b.add('run3.launcher_processes_alarmed', al3, 'processes', 'COUNT', L3, 'of those, with the 10 s slow-wait alarm')
b.check('run3 no launcher process raised the alarm and all completed', al3 == 0 and fin3 == len(lp), f'{fin3} of {len(lp)} complete, {al3} alarms')
b.add('run3.launches_of_new_launcher', sum(1 for p in lp if 'old-plan' not in p.name), 'processes', 'COUNT', L3,
      'of those, run with the new launcher a6aee5ef')
g16 = (ROOT / 'amd-launch-gap/build-r16-spark-gate-94af5a38.log').read_text()
c16 = re.search(r'"checks": (\d+)', g16); f16 = re.search(r'"failures": (\d+)', g16)
b.check('94af5a38 gate log: PASS line, launcher 8b6005e8', 'AMD-KFD-LAUNCH CPU PASS' in g16 and '8b6005e8538669db' in g16 and f16 and f16.group(1) == '0')
b.add('hang.node_fix_gate_checks', int(c16.group(1)), 'checks', 'COUNT', 'amd-launch-gap/build-r16-spark-gate-94af5a38.log', 'checks in the PASS line at 94af5a38')
# Run 4 (plan3a, plan3v): not run on a card. The prepared card script names both modes and pins the launcher by sha256;
# the gate log is the one whose PASS line names that launcher. The log does not name the two modes.
G15 = 'amd-launch-gap/build-r15-spark-gate.log'
g15 = gate_pass(G15)
b.check('r15 gate log: one PASS line, launcher 657db853, 0 failures, device NOT_RUN',
        g15.get('binaries', {}).get('x86', '').startswith('657db853484df0f6') and g15.get('failures') == 0 and g15.get('device') == 'NOT_RUN')
b.add('run4.gate_checks', int(g15.get('checks', -1)), 'checks', 'COUNT', G15,
      '"checks" in the AMD-KFD-LAUNCH CPU PASS line; the line names x86 launcher 657db853; the log does not name plan3a or plan3v')
S4 = 'amd-launch-gap/card/bundle4/gap-run4.sh'
s4 = (ROOT / S4).read_text()
b.check('run 4 card script pins launcher 657db853 and runs plan3a and plan3v',
        bool(re.search(r'^echo "' + g15.get('binaries', {}).get('x86', 'missing') + r'  \$G/bin-new/omega-launch" \| sha256sum -c', s4, re.M))
        and 'omega new-plan3a-$c cmd-plan3a.sh' in s4 and 'omega new-plan3v-$c cmd-plan3v.sh' in s4)
b.add('run4.card_script_modes', sorted(set(re.findall(r'omega new-(plan3[av])-', s4))), '', 'RECORD', S4,
      'launcher modes the prepared run 4 card script runs')
b.close()
