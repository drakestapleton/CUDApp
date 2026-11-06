#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Run only inside the lane's device-hidden bwrap. No device access is needed."""
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'build/no_posix_bf16'
OUT.mkdir(parents=True, exist_ok=True)
BRIDGE = Path(os.environ['OMEGA_BRIDGE'])
ENV = dict(os.environ, OMEGA_NATIVE_OMC2=os.environ['OMEGA_OMC'])
subprocess.run(['sh', ROOT / 'om/no-posix.sh', '--tools', OUT], check=True)
ENV.update(OMEGA_NATIVE_OMC=str(OUT / 'tools/omc-gen'),
           OMEGA_NATIVE_X86_OMC=str(OUT / 'tools/omc-x86'))
NATIVE = ROOT / 'build/no_posix/native'
SOURCE = ROOT / 'build/no_posix/sources/no_avbf.om'
CALLER = ROOT / 'tests/no_posix_bf16.c'
FLAGS = ['gcc', '-std=gnu11', '-O2', '-Wall', '-Wextra', '-Werror', '-Wno-unused-function', '-ffp-contract=off', '-I' + str(ROOT / 'src')]


def run(args, expected=0):
    p = subprocess.run([str(a) for a in args], cwd=ROOT, env=ENV, capture_output=True)
    if p.returncode != expected:
        raise AssertionError((args, p.returncode, p.stdout[-2000:], p.stderr[-4000:]))
    return p.stdout


def dump(name, extra, include, objects):
    binary = OUT / name
    run([*FLAGS, *extra, '-I' + str(include), CALLER, *objects, '-lm', '-o', binary])
    data = run([binary])
    (OUT / (name + '.bin')).write_bytes(data)
    return data


hosted = dump('canonical-hosted', [], NATIVE, [ROOT / 'src/cudapp_om_av_bf16.c'])
fs = dump('freestanding', ['-DOMEGA_GPU_NO_POSIX'], NATIVE,
          [NATIVE / 'omega_native.o', NATIVE / 'omega_api_defs.o'])
assert fs == hosted, 'canonical hosted view versus freestanding scalar differs'
red = dump('c-oracle', ['-DOMEGA_GPU_NO_POSIX', '-DCUDAPP_MIGRATION_PENDING_C_NO_POSIX_SESSION=1'], NATIVE, [])
assert red == hosted, 'explicit C oracle differs from canonical hosted view'
# Native-hosted and native-freestanding bindings must also implement the same scalar.
scalarhost = OUT / 'scalar-hosted'
run(['sh', BRIDGE / 'tools/native', scalarhost, SOURCE])
assert dump('native-hosted', ['-DOMEGA_GPU_NO_POSIX'], scalarhost,
            [scalarhost / 'libomega_native.so', '-Wl,-rpath,' + str(scalarhost)]) == hosted
print('PASS canonical hosted view, native hosted scalar and freestanding scalar differential; C flag ON control')

# The production matmul object is inspected at O0 so C computations cannot hide
# behind inlining. Both nm and relocation/disassembly must name the selected arm.
for enabled in (0, 1):
    obj = OUT / ('matmul-' + str(enabled) + '.o')
    run([*FLAGS, '-O0', '-ffreestanding', '-fno-builtin', '-fno-stack-protector',
         '-DOMEGA_GPU_NO_POSIX', '-DOMEGA_NUMERIC_CPU_ONLY',
         '-DCUDAPP_MIGRATION_PENDING_C_NO_POSIX_SESSION=' + str(enabled),
         '-I' + str(NATIVE), '-c', ROOT / 'src/omega_gpu_matmul_api.c', '-o', obj])
    nm = run(['nm', obj]).decode()
    dis = run(['objdump', '-dr', obj]).decode()
    (OUT / (obj.stem + '.nm')).write_text(nm)
    (OUT / (obj.stem + '.objdump')).write_text(dis)
    assert 'omega_av_bf16' not in nm
    if enabled:
        assert 'av_bf16_c' in nm and '<av_bf16_c>' in dis
        assert 'omega2_no_avbf' not in nm
    else:
        assert 'av_bf16_c' not in nm and 'av_bf16_c' not in dis
        assert 'omega2_no_avbf' in nm and 'omega2_no_avbf' in dis
print('PASS production NO_POSIX matmul nm/objdump OFF Omega / ON C red control')

# Strong reference: removing the native objects must fail at the selected export.
p = subprocess.run([*FLAGS, '-DOMEGA_GPU_NO_POSIX', '-I' + str(NATIVE), str(CALLER), '-o', str(OUT / 'missing')], cwd=ROOT, env=ENV, capture_output=True)
assert p.returncode != 0 and b'omega2_no_avbf' in p.stderr
(OUT / 'missing-link.txt').write_bytes(p.stderr)
# The combined freestanding unit object must not import libc, TLS or an oracle.
combined = OUT / 'combined.o'
run(['ld', '-r', NATIVE / 'omega_native.o', NATIVE / 'omega_api_defs.o', '-o', combined])
assert run(['nm', '-u', combined]) == b''
print('PASS strong missing-unit link refusal; combined native objects have zero undefined imports')

text = SOURCE.read_text()
mutants = [
    ('round-bias', 'ret shr add add u 32767 and shr u 16 1 16', 'ret shr add add u 32766 and shr u 16 1 16'),
    ('tie-parity', 'ret shr add add u 32767 and shr u 16 1 16', 'ret shr add add u 32767 and shr u 16 0 16'),
    ('quiet-bit', 'ret or shr u 16 64', 'ret shr u 16'),
    ('nan-payload', 'ret or shr u 16 64', 'ret 32704'),
    ('canonical-nan', 'ret 32704', 'ret 0'),
    ('mode-select', 'if eq md 0', 'if ne md 0'),
]
for name, old, new in mutants:
    assert old in text
    directory = OUT / name
    directory.mkdir(exist_ok=True)
    source = directory / 'mutant.om'
    source.write_text(text.replace(old, new, 1))
    unit = directory / 'native'
    run(['sh', BRIDGE / 'tools/native', '--freestanding', unit, source])
    observed = dump(name + '-bin', ['-DOMEGA_GPU_NO_POSIX'], unit,
                    [unit / 'omega_native.o', unit / 'omega_api_defs.o'])
    assert observed != hosted, 'SURVIVED ' + name
    print('KILLED ' + name + ': compiled native behavior differs from canonical hosted')
print('PASS no-posix BF16: canonical/native hosted/freestanding differential; symbol OFF/ON red; 6 killed mutants')
