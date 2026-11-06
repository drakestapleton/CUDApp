#!/usr/bin/env python3
"""Recompile semantic Omega mutants; refusal never counts as a kill."""
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
BRIDGE = Path(os.environ['OMEGA_BRIDGE'])
OMC = os.environ['OMEGA_OMC']
ENV = dict(os.environ, OMEGA_SEEDED_OMC=OMC)
SCALAR = ROOT / 'om/blackwell/scalar.om'
IDENT = ROOT / 'om/blackwell/identity.om'
CPUI = ROOT / 'om/blackwell/cpui.om'
CPUH = ROOT / 'om/blackwell/cpuf16.om'
CPUB = ROOT / 'om/blackwell/cpubf16.om'
SHAPE = ROOT / 'om/blackwell/shape.om'
NAME = ROOT / 'om/blackwell/name.om'
CASES = [
    ('half-exponent', SCALAR, 'shl add exp 112 23', 'shl add exp 111 23'),
    ('half-subnormal', SCALAR, 'shl sub BEXP 10 23', 'shl sub BEXP 9 23'),
    ('bf16-expand', SCALAR, 'ret shl and x 65535 16', 'ret shl and x 65535 15'),
    ('bf16-round', SCALAR, 'add add x 32767', 'add add x 32766'),
    ('alloc-round', SCALAR, 'b = and add add x y 4095', 'b = and add add x y 4094'),
    ('admit-cbank', SCALAR, 'gt v 128', 'gt v 127'),
    ('identity-byte', IDENT, 'put msg i get data i', 'put msg i xor get data i uchar 1'),
    ('integer-product', CPUI, 'mul ulong a ulong b', 'add ulong a ulong b'),
    ('half-dot-zero', CPUH, 'sum = bitcast f32 uint 0', 'sum = bitcast f32 uint 1065353216'),
    ('bf16-dot-stride', CPUB, 'bi = and add bi nn 4294967295', 'bi = and add bi 1 4294967295'),
    ('shape-root-equality', SHAPE, 'if le mid div x mid', 'if lt mid div x mid'),
    ('name-terminator', NAME, 'if eq get data i 0', 'if ne get data i 0'),
]


def run(args, **kw):
    p = subprocess.run([str(a) for a in args], cwd=ROOT, env=ENV, capture_output=True, **kw)
    if p.returncode:
        raise RuntimeError(f'{args}: rc={p.returncode}\n{p.stdout!r}\n{p.stderr!r}')
    return p.stdout


with tempfile.TemporaryDirectory(prefix='bw-mutants-') as td:
    t = Path(td)
    for name, source, old, new in CASES:
        text = source.read_text()
        assert text.count(old) == 1, (name, text.count(old))
        text = text.replace(old, new)
        if source == IDENT:
            text = (ROOT / 'om/cache/digest.om').read_text() + '\n' + text
        elif source in (CPUI, CPUH, CPUB, SHAPE):
            parts = [ROOT / 'om/blackwell/views.om']
            if source == CPUH:
                parts.append(SCALAR)
            if source == SHAPE:
                text += '\n' + (ROOT / 'om/blackwell/shape_probe.om').read_text()
            else:
                parts.append(SHAPE)
            text = '\n'.join(p.read_text() for p in parts) + '\n' + text
        unit = t / f'{name}.om'
        unit.write_text(text)
        generated = t / f'{name}.c'
        frontend = 'bridge-freest' if source == SCALAR else 'bridge-seeded'
        profile = ['--runtime-views'] if source in (CPUI, CPUH, CPUB, SHAPE, NAME) else []
        generated.write_bytes(run([BRIDGE / 'tools' / frontend, *profile, 'c', unit]))
        exe = t / name
        run(['cc', '-std=gnu11', '-O0', '-g', '-Wall', '-Wextra', '-Werror',
             '-ffp-contract=off', '-Isrc', '-DCUDAPP_MIGRATION_PENDING_BW_MATMUL=1',
             'tests/om_bw_diff_test.c', 'tests/om_bw_matmul_oracle.c',
             'tests/om_bw_realize_oracle.c', generated, 'build/libomega_gpu.a',
             '-lpthread', '-lm', '-o', exe])
        p = subprocess.run([exe], cwd=ROOT, env=ENV, capture_output=True)
        assert p.returncode == 1 and b'FAIL line' in p.stderr, (name, p.returncode, p.stdout, p.stderr)
        print(f'KILLED {name}: compiled=0 differential=1', flush=True)
    print(f'PASS bw semantic mutants killed={len(CASES)}')
