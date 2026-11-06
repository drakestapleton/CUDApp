#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Host-only typed-view differential, mutation gate, coverage and loop benchmark.
Run through the mandatory device-hiding bwrap command in the evidence README.
"""
import argparse
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
p = argparse.ArgumentParser()
p.add_argument('--mutants', action='store_true')
p.add_argument('--coverage', action='store_true')
p.add_argument('--bench', action='store_true')
p.add_argument('--api', action='store_true')
p.add_argument('--out')
a = p.parse_args()
tmp = None if a.out else tempfile.TemporaryDirectory(prefix='attn-views-')
out = Path(a.out if a.out else tmp.name).resolve()
out.mkdir(parents=True, exist_ok=True)
os.chdir(ROOT)
commands = []
def run(cmd, *, expect=0, stdout=None):
    r = subprocess.run([str(x) for x in cmd], text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    commands.append({'command': [str(x) for x in cmd], 'rc': r.returncode, 'output': r.stdout})
    (out / 'commands.json').write_text(json.dumps(commands, indent=2)+'\n')
    if stdout is not None:
        Path(stdout).write_text(r.stdout)
    elif r.stdout:
        print(r.stdout, end='', flush=True)
    if expect is not None and r.returncode != expect:
        raise SystemExit('FAIL command rc '+str(r.returncode)+': '+str(cmd))
    return r
cc = os.environ.get('CC', 'cc')
flags = ['-std=gnu11', '-O2', '-ffp-contract=off', '-Isrc', '-Itests']
units = dict(re.findall(r'^UNITS="\$UNITS (attn_\w+)=([^"\n]+)"$', (ROOT/'om/units.sh').read_text(), re.M))
objects = {}
for name in units:
    obj = out/(name+'.o')
    providers = sorted((ROOT/'src').glob('cudapp_*_'+name+'.c'))
    if len(providers) != 1:
        raise SystemExit('FAIL expected exactly one checked provider for '+name+': '+str(providers))
    run([cc, *flags, '-c', providers[0], '-o', obj])
    objects[name] = obj
for name in ['api_mm_plan','api_at_plan','api_ew_plan','api_bf_plan','api_da_plan','api_rt_plan','core_hostmath']:
    obj = out/(name+'.o')
    run([cc, *flags, '-c', 'src/cudapp_om_'+name+'.c', '-o', obj])
    objects[name] = obj
hostmath = out/'hostmath.o'
run([cc, *flags, '-c', 'src/omega_gpu_hostmath.c', '-o', hostmath])
omega = out/'cases-omega.o'
oracle = out/'cases-oracle.o'
run([cc, *flags, '-DAV_ENTRY=attn_omega', '-c', 'tests/attn_views_cases.c', '-o', omega])
oracle_flags = ['-O0', '--coverage'] if a.coverage else []
run([cc, *flags, *oracle_flags, '-DCUDAPP_MIGRATION_PENDING_ATTN_VIEWS=1', '-DAV_ENTRY=attn_oracle', '-c', 'tests/attn_views_cases.c', '-o', oracle])
main = out/'diff.o'
run([cc, *flags, '-c', 'tests/attn_views_diff.c', '-o', main])
def link(dest, replacements=None):
    objs = dict(objects)
    objs.update(replacements or {})
    run([cc, *flags, *(['--coverage'] if a.coverage else []), main, omega, oracle, hostmath, *objs.values(), '-lm', '-o', dest])
link(out/'diff')
run([out/'diff'])
if a.mutants:
    bridge = Path(os.environ['OMEGA_BRIDGE']).resolve()
    env = os.environ.copy()
    env['OMEGA_SEEDED_OMC'] = os.environ['OMEGA_OMC']
    mutants = [
        ('attn_args', 'shr v 32', 'shr v 31'),
        ('attn_lay', 'shr v 32', 'shr v 31'),
        ('attn_buf', 'get s i', 'get s 0'),
        ('attn_bfexp', 'shl uint get s i 16', 'shl uint get s i 15'),
        ('attn_rows', 'v = and add add base i 1', 'v = and add add base i 2'),
        ('attn_gtab', 'put tab add mul i 4 3 uint 0', 'put tab add mul i 4 3 uint 1'),
        ('attn_comp', 'if lt v 2147483648', 'if ge v 2147483648'),
        ('attn_math', 'put dest 1 bitcast ulong pc', 'put dest 1 bitcast ulong neg pc'),
        ('attn_tables', 'ret mul a get w 16', 'ret a'),
    ]
    killed = 0
    for num, (name, site, replacement) in enumerate(mutants):
        source = '\n'.join((ROOT/x).read_text() for x in units[name].split(','))+'\n'
        if source.count(site) != 1:
            raise SystemExit('FAIL mutant site '+name+' count '+str(source.count(site)))
        om = out/('mutant-'+str(num)+'.om')
        om.write_text(source.replace(site,replacement))
        c = out/('mutant-'+str(num)+'.c')
        providers = sorted((ROOT/'src').glob('cudapp_*_'+name+'.c'))
        if len(providers) != 1:
            raise SystemExit('FAIL expected exactly one checked provider for '+name)
        freestanding = providers[0].name.startswith('cudapp_freest_')
        cmd = ['sh', bridge/('tools/bridge-freest' if freestanding else 'tools/bridge-seeded')]
        if not freestanding and name not in ['attn_args','attn_lay']:
            cmd.append('--runtime-views')
        cmd += ['c', om]
        r = subprocess.run([str(x) for x in cmd], env=env, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        commands.append({'command':[str(x) for x in cmd], 'rc':r.returncode, 'output':r.stderr})
        if r.returncode:
            raise SystemExit('FAIL mutant did not generate '+name+': '+r.stderr)
        c.write_text(r.stdout)
        obj = out/('mutant-'+str(num)+'.o')
        run([cc, *flags, '-c', c, '-o', obj])
        exe = out/('mutant-'+str(num))
        link(exe, {name:obj})
        r = run([exe], expect=None)
        if r.returncode == 0:
            raise SystemExit('FAIL surviving mutant '+name)
        killed += 1
        print('KILLED '+name+' rc='+str(r.returncode), flush=True)
    print('PASS attention host typed views mutants killed='+str(killed), flush=True)
if a.coverage:
    run(['gcov', '-b', '-c', '-o', out, oracle], stdout=out/'gcov.txt')
    # gcov writes source listings in cwd; retain only this lane's fresh files.
    for path in ROOT.glob('*.gcov'):
        if path.name in ['attn_views_cases.c.gcov','omega_attn_views.h.gcov','omega_attn_tables.h.gcov','omega_api_plan.h.gcov']:
            path.replace(out/path.name)
    print('PASS attention C oracle coverage captured in '+str(out/'gcov.txt'), flush=True)
if a.bench:
    benchobjs = []
    for label, extra in [('omega', []), ('oracle', ['-DCUDAPP_MIGRATION_PENDING_ATTN_VIEWS=1'])]:
        obj = out/('bench-'+label+'.o')
        run([cc, *flags, *extra, '-DAV_BENCH=attn_bench_'+label, '-c', 'tests/attn_views_bench.c', '-o', obj])
        benchobjs.append(obj)
    exe = out/'bench'
    run([cc, *flags, 'tests/attn_views_bench_main.c', *benchobjs, hostmath, *objects.values(), '-lm', '-o', exe])
    run([exe], stdout=out/'bench.tsv')
    print('PASS attention loop benchmark raw samples '+str(out/'bench.tsv'), flush=True)
if a.api:
    for target in ['test-gpu-attention','test-gpu-decode-attn','test-gpu-decode-attn256','test-gpu-router','test-gpu-bf16-gemv']:
        run(['make', '-j3', target])
    run(['make', '-j3', 'build/gpu_prefill_attn_test', 'build/gpu_prefill_attn256_test'])
    for exe in ['build/gpu_prefill_attn_test','build/gpu_prefill_attn256_test']:
        run([exe, '--host-only'])
        run([exe, '--sim'])
    print('PASS seven attention-family API host/sim gates', flush=True)
print('PASS attention typed-view host gate', flush=True)
