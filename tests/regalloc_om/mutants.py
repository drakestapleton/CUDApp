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
         'cg_matmul_tensor', 'cg_matmul_loop', 'cg_matmul_frag', 'cg_matmul_block', 'cg_vecadd', 'cg_bind',
         'bw_hazard', 'bw_hazard_dec', 'cg_life', 'cg_replay']
FIXED = ['src/cudapp_om_bw_enc_sm121.c', 'src/cudapp_freest_f16.c',
         'src/cudapp_om_core_sha.c', 'src/cudapp_freest_leftovers.c', 'src/cudapp_freest_pathcopy.c',
         'src/cudapp_om_bw_identity.c', 'src/cudapp_freest_bw_scalar.c', 'src/cudapp_om_bw_cpui.c',
         'src/cudapp_om_bw_cpuf16.c', 'src/cudapp_om_bw_cpubf16.c', 'src/cudapp_om_bw_shape.c',
         'src/cudapp_om_bw_name.c', 'src/cudapp_om_cg_pack_len.c', 'src/cudapp_om_cg_pack_h.c',
         'src/cudapp_om_cg_em.c', 'src/cudapp_om_cg_reconv.c', 'src/cudapp_om_cg_sentin.c',
         'src/cudapp_freest_cg_fields.c', 'src/cudapp_om_cg_invar.c', 'src/cudapp_om_cg_budget.c']
FLAGS = ['-std=gnu11', '-O2', '-Wall', '-Wextra', '-Werror', '-ffunction-sections', '-fdata-sections', '-Isrc']
LINK = {
    'liveness': ['tests/regalloc_om/liveness.c', str(OUT / 'successor.o'), 'src/omega_gpu_om_ir.c'],
    'differential': ['tests/regalloc_om/differential.c', str(OUT / 'oracle.o'), str(OUT / 'successor.o'),
                     'src/omega_gpu_om_ir.c', 'src/omega_blackwell_matmul.c', 'src/sha256.c'],
    'hazard': ['tests/regalloc_om/hazard_diff.c', str(OUT / 'hz_oracle.o'), 'src/omega_bw_hazard.c'],
    'bind': ['tests/regalloc_om/bind_diff.c', str(OUT / 'bind_oracle_matmul.o'), str(OUT / 'bind_oracle_realize.o'),
             str(OUT / 'oracle.o'), str(OUT / 'successor.o'), 'src/omega_blackwell_matmul.c',
             'src/omega_blackwell_realize.c', 'src/omega_blackwell_encoder.c', 'src/omega_gpu_om_ir.c', 'src/sha256.c'],
}
# om/units.sh concatenates these parts after the mutated file into a second generated unit.
TAILS = {'bw_hazard.om': [('bw_hazard_dec', 'bw_hazard_dec.om')]}
CASES = [
    ('pair-alignment', 'regalloc.om', 'step = 2\n', 'step = 1\n', 'random'),
    ('reserved-gpr', 'regalloc.om', 'start = long 2\n', 'start = long 0\n', 'random'),
    ('equal-endpoint-release', 'regalloc.om', 'if lt ra_get m add b 2 first\n', 'if le ra_get m add b 2 first\n', 'random'),
    ('sort-order', 'regalloc.om', 'ret gt ra_get m ai ra_get m bi\n', 'ret lt ra_get m ai ra_get m bi\n', 'random'),
    ('reserved-ugpr', 'regalloc.om', 'cur = long 4\n', 'cur = long 0\n', 'random'),
    ('quad-alignment', 'regalloc.om', 'start = 4\n', 'start = 2\n', 'random'),
    ('shortened-override-check', 'cg_life.om', 'if or lt h ulong fd or lt h ulong lu ge h n\n', 'if gt ulong 0 ulong 1\n', 'live'),
    ('v159-boundary', 'cg_replay.om', 'if and eq get meta 7 1 gt get meta 1 160\n', 'if and eq get meta 7 1 ge get meta 1 160\n', 'live'),
    ('override-ignored', 'cg_matmul_loop.om', 'put m ulong 7 ulong 1\n', 'put m ulong 7 ulong 0\n', 'builders'),
    ('override-shortened', 'cg_matmul_loop.om', 'put m add ulong 352 r x\n', 'put m add ulong 352 r sub x ulong 1\n', 'builders'),
    ('spec-m-limit', 'cg_bind.om', 'gt m 8192 ', 'gt m 8193 ', 'bind'),
    ('spec-precision-byte', 'cg_bind.om', 'cb_le32 w 12 and p 255\n', 'cb_le32 w 12 and p 65535\n', 'bind'),
    ('dispatch-bf16', 'cg_bind.om', 'if or eq p 1 eq p 2\n', 'if eq p 1\n', 'bind'),
    ('vector-ceil-div', 'cg_bind.om', 'g = div add c uint 63 uint 64\n', 'g = div add c uint 62 uint 64\n', 'bind'),
    ('pad-minimum', 'cg_bind.om', 'if lt p 32\n', 'if lt p 16\n', 'bind'),
    ('fit-boundary', 'cg_bind.om', 'if lt b need\n', 'if le b need\n', 'bind'),
    ('hazard-size-6', 'bw_hazard.om', 'ret long 4\n', 'ret long 8\n', 'hazard'),
    ('hazard-size-class', 'bw_hazard.om', 'if le s 4\n', 'if lt s 4\n', 'hazard'),
    ('hazard-op-2433', 'bw_hazard.om', 'if eq op 2433\n', 'if eq op 2434\n', 'hazard'),
    ('arch-le', 'cg_bind.om', 'cb_le32 w 96 and a 4294967295\n', 'cb_le32 w 96 and a 255\n', 'bind'),
]

# Listed explicitly, never credited as killed. The exact source formula first
# shifts the extension left six and masks to uint32, erasing their only differing bit.
EQUIVALENTS = [
    ('hazard-branch-sign', 'bw_hazard.om', 's = or s 3221225472\n', 's = or s 1073741824\n',
     'tests/regalloc_om/hazard-branch-sign-proof.txt'),
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
        regenerated = {stem: generated}
        for tail_stem, tail in TAILS.get(unit, []):
            tail_om = work / f'{tail_stem}.om'
            tail_om.write_text(mutated.read_text() + (ROOT / 'om/codegen' / tail).read_text())
            regenerated[tail_stem] = work / f'cudapp_om_{tail_stem}.c'
            with regenerated[tail_stem].open('w') as c, (work / 'compile.log').open('a') as log:
                rc = subprocess.run(['sh', str(BRIDGE / 'tools/bridge-seeded'), 'c', str(tail_om)], cwd=ROOT,
                                    env=dict(ENV, OMEGA_SEEDED_OMC=str(OMC)), stdout=c, stderr=log).returncode
            if rc:
                raise RuntimeError(f'{name}: NOT_KILLED compiler rc={rc}, see {work}/compile.log')
        target = {'live': 'liveness', 'hazard': 'hazard', 'bind': 'bind'}.get(mode, 'differential')
        units = [str(regenerated[u]) if u in regenerated else f'src/cudapp_om_{u}.c' for u in UNITS]
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
    for name, unit, before, after, proof in EQUIVALENTS:
        if (ROOT / 'om/codegen' / unit).read_text().count(before) != 1:
            raise RuntimeError(f'{name}: equivalent anchor is not unique')
        if not (ROOT / proof).is_file():
            raise RuntimeError(f'{name}: missing proof')
        print(f'EQUIVALENT {name} not counted as killed; proof={proof}', flush=True)
    print(f'PASS recompiled Omega semantic mutants={len(summary)}')


if __name__ == '__main__':
    main()
