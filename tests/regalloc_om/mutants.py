#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Lower each mutated .om with the pinned portable bridge, then relink only that unit's generated C.

Run inside the same device-free namespace as run.sh, after run.sh (it reuses oracle.o and successor.o).
Compilation refusal is not credited as a killed semantic mutant. The unmodified executables must pass first.
"""
import hashlib
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'build/regalloc-om'
ENV = dict(os.environ)
OMC = Path(ENV.get('OMEGA_OMC', str(Path.home() / 'work/cudastack/omega-mv/seed/out/omc')))
BRIDGE = Path(ENV.get('OMEGA_BRIDGE', str(Path.home() / 'work/cudastack/omega-bridge')))
CC = ENV.get('CC', 'cc')
UNITS = ['regalloc', 'regalloc_ir', 'regalloc_live', 'cg_matmul', 'cg_matmul_params', 'cg_matmul_i32',
         'cg_matmul_tensor', 'cg_matmul_loop', 'cg_matmul_frag', 'cg_matmul_block', 'cg_vecadd']
FIXED = ['src/cudapp_om_bw_enc_sm121.c', 'src/cudapp_freest_f16.c']
FLAGS = ['-std=gnu11', '-O2', '-Wall', '-Wextra', '-Werror', '-ffunction-sections', '-fdata-sections', '-Isrc']
LINK = {
    'liveness': ['tests/regalloc_om/liveness.c', str(OUT / 'successor.o'), 'src/omega_gpu_om_ir.c'],
    'differential': ['tests/regalloc_om/differential.c', str(OUT / 'oracle.o'), str(OUT / 'successor.o'),
                     'src/omega_gpu_om_ir.c', 'src/omega_blackwell_matmul.c', 'src/sha256.c'],
}
CASES = [
    ('pair-alignment', 'regalloc.om', 'step = 2\n', 'step = 1\n', 'random'),
    ('reserved-gpr', 'regalloc.om', 'start = long 2\n', 'start = long 0\n', 'random'),
    ('equal-endpoint-release', 'regalloc.om', 'if lt ra_get m add b 2 first\n', 'if le ra_get m add b 2 first\n', 'random'),
    ('sort-order', 'regalloc.om', 'ret gt ra_get m ai ra_get m bi\n', 'ret lt ra_get m ai ra_get m bi\n', 'random'),
    ('reserved-ugpr', 'regalloc.om', 'cur = long 4\n', 'cur = long 0\n', 'random'),
    ('quad-alignment', 'regalloc.om', 'start = 4\n', 'start = 2\n', 'random'),
    ('shortened-override-check', 'regalloc_live.om', 'if or lt a ulong b lt a ulong c\n', 'if gt ulong 0 ulong 1\n', 'live'),
    ('v159-boundary', 'regalloc_live.om', 'if and eq a 1 gt b 160\n', 'if and eq a 1 ge b 160\n', 'live'),
    ('override-ignored', 'cg_matmul_loop.om', 'put m ulong 7 ulong 1\n', 'put m ulong 7 ulong 0\n', 'builders'),
    ('override-shortened', 'cg_matmul_loop.om', 'put m add ulong 352 r x\n', 'put m add ulong 352 r sub x ulong 1\n', 'builders'),
]


def run(command, path, env=ENV):
    with path.open('w') as log:
        return subprocess.run(command, cwd=ROOT, env=env, stdout=log, stderr=subprocess.STDOUT, timeout=600).returncode


def pinned():
    pins = dict(part.split('=', 1) for part in (ROOT / 'src/cudapp_om_regalloc.ident').read_text().split() if '=' in part)
    if hashlib.sha256(OMC.read_bytes()).hexdigest() != pins['omc']:
        raise RuntimeError('NOT_RUN: OMEGA_OMC is not the pinned compiler')
    if hashlib.sha256((BRIDGE / 'tools/bridge-seeded').read_bytes()).hexdigest() != pins['frontend']:
        raise RuntimeError('NOT_RUN: bridge-seeded is not the pinned frontend')
    if hashlib.sha256((BRIDGE / 'src/toc.om').read_bytes()).hexdigest() != pins['converter']:
        raise RuntimeError('NOT_RUN: toc.om is not the pinned converter')


def main():
    pinned()
    summary = []
    for name, unit, before, after, mode in CASES:
        source = (ROOT / 'om/codegen' / unit).read_text()
        if source.count(before) != 1:
            raise RuntimeError(f'{name}: mutation anchor is not unique')
        work = OUT / 'mutants' / name
        work.mkdir(parents=True, exist_ok=True)
        mutated = work / unit
        mutated.write_text(source.replace(before, after))
        stem = unit[:-3]
        generated = work / f'cudapp_om_{stem}.c'
        with generated.open('w') as c, (work / 'compile.log').open('w') as log:
            rc = subprocess.run(['sh', str(BRIDGE / 'tools/bridge-seeded'), 'c', str(mutated)], cwd=ROOT,
                                env=dict(ENV, OMEGA_SEEDED_OMC=str(OMC)), stdout=c, stderr=log).returncode
        if rc:
            raise RuntimeError(f'{name}: NOT_KILLED compiler rc={rc}, see {work}/compile.log')
        target = 'liveness' if mode == 'live' else 'differential'
        units = [str(generated) if u == stem else f'src/cudapp_om_{u}.c' for u in UNITS]
        binary = work / target
        rc = run([CC, *FLAGS, *LINK[target], *units, *FIXED, '-Wl,--gc-sections', '-o', str(binary)], work / 'link.log')
        if rc:
            raise RuntimeError(f'{name}: NOT_KILLED link rc={rc}, see {work}/link.log')
        command = [str(binary)]
        if mode == 'random':
            command.append('random-only')
        rc = run(command, work / 'exercise.log')
        if rc != 1:
            raise RuntimeError(f'{name}: NOT_KILLED expected differential rc1, got {rc}; see {work}/exercise.log')
        final = (work / 'exercise.log').read_text().splitlines()[-1]
        summary.append(f'{name}\tcompile=0\texercise={rc}\t{final}')
        print('KILLED', summary[-1], flush=True)
    (OUT / 'mutants.tsv').write_text('\n'.join(summary) + '\n')
    print(f'PASS recompiled Omega semantic mutants={len(summary)}')


if __name__ == '__main__':
    main()
