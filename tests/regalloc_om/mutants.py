#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Recompile each changed .om, then interpose only its generated native ABI.

Run inside the same device-free namespace as run.sh. Compilation refusal is not
credited as a killed semantic mutant. The unmodified executables must pass first.
"""
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'build/regalloc-om'
ENV = dict(os.environ)
ENV.setdefault('OMEGA_NATIVE_OMC', str(Path.home() / 'work/cudastack/lanes/bh-tools/omc-native'))
ENV.setdefault('OMEGA_NATIVE_OMC2', str(Path.home() / 'work/cudastack/omega-mv/seed/out/omc'))
BRIDGE = Path(ENV.get('OMEGA_BRIDGE', str(Path.home() / 'work/cudastack/omega-bridge')))
UNITS = ['regalloc.om', 'regalloc_ir.om', 'regalloc_live.om', 'cg_matmul.om', 'cg_matmul_params.om',
         'cg_matmul_i32.om', 'cg_matmul_tensor.om', 'cg_matmul_loop.om', 'cg_matmul_frag.om', 'cg_matmul_block.om']
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


def main():
    summary = []
    for name, unit, before, after, mode in CASES:
        source = (ROOT / 'om/codegen' / unit).read_text()
        if source.count(before) != 1:
            raise RuntimeError(f'{name}: mutation anchor is not unique')
        work = OUT / 'mutants' / name
        work.mkdir(parents=True, exist_ok=True)
        mutated = work / unit
        mutated.write_text(source.replace(before, after))
        native = work / 'native'
        # The generator uses a fixed SONAME, so preload a complete aggregate with exactly one changed unit.
        units = [str(mutated if name == unit else ROOT / 'om/codegen' / name) for name in UNITS]
        rc = run([str(BRIDGE / 'tools/native'), str(native), *units], work / 'compile.log')
        if rc:
            raise RuntimeError(f'{name}: NOT_KILLED compiler rc={rc}, see {work}/compile.log')
        env = dict(ENV, LD_PRELOAD=str(native / 'libomega_native.so'))
        command = [str(OUT / ('liveness' if mode == 'live' else 'differential'))]
        if mode == 'random':
            command.append('random-only')
        rc = run(command, work / 'exercise.log', env)
        if rc != 1:
            raise RuntimeError(f'{name}: NOT_KILLED expected differential rc1, got {rc}; see {work}/exercise.log')
        final = (work / 'exercise.log').read_text().splitlines()[-1]
        summary.append(f'{name}\tcompile=0\texercise={rc}\t{final}')
        print('KILLED', summary[-1], flush=True)
    (OUT / 'mutants.tsv').write_text('\n'.join(summary) + '\n')
    print(f'PASS recompiled Omega semantic mutants={len(summary)}')


if __name__ == '__main__':
    main()
