#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Run only inside the mandatory device-free bwrap boundary."""
import hashlib
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'build/no_posix/check'
OUT.mkdir(parents=True, exist_ok=True)
BRIDGE = Path(os.environ['OMEGA_BRIDGE']).resolve()
ENV = dict(os.environ, OMEGA_NATIVE_OMC2=os.environ['OMEGA_OMC'])
LOG = OUT / 'commands.log'
LOG.write_text('')

def run(args, expected=0):
    p = subprocess.run([str(x) for x in args], env=ENV, cwd=OUT, capture_output=True)
    with LOG.open('ab') as f:
        f.write((' '.join(map(str, args)) + '\nrc=' + str(p.returncode) + '\n').encode())
        for raw in (p.stdout, p.stderr):
            if '--dump' in args:
                f.write(('dump sha256=' + hashlib.sha256(raw).hexdigest() + '\n').encode())
                continue
            printable = all(32 <= b <= 126 or b in (9, 10, 13) for b in raw)
            f.write((raw if printable else b'HEX:' + raw.hex().encode()) + b'\n')
    assert p.returncode == expected, (args, p.returncode, p.stdout, p.stderr)
    return p.stdout

sources = sorted((ROOT / 'build/no_posix/sources').glob('*.om'))
assert {p.stem for p in sources} == {'bw_enc_sm121', 'cubin_elf', 'no_bf16', 'admit', 'words', 'park', 'book', 'streams', 'no_avbf'}
native = ROOT / 'build/no_posix/native'
host = OUT / 'host'
run([BRIDGE / 'tools/native', host, *sources])
flags = ['gcc', '-std=gnu11', '-O2', '-Wall', '-Wextra', '-Werror', '-ffreestanding', '-fno-builtin', '-fno-stack-protector', '-ffp-contract=off']
caller = ROOT / 'tests/no_posix_native.c'

def bare(lib, path, red=False):
    run([*flags, '-DNO_POSIX_BARE', *(['-DNO_POSIX_RED'] if red else []), '-I', lib,
         caller, lib / 'omega_native.o', lib / 'omega_api_defs.o', '-nostdlib', '-static', '-no-pie', '-Wl,-e,_start', '-o', path])

binary = OUT / 'bare'
bare(native, binary)
assert not run(['nm', '-u', binary]).strip()
assert b'INTERP' not in run(['readelf', '-lW', binary])
assert b'.tdata' not in run(['readelf', '-SW', binary])
assert b'.tbss' not in run(['readelf', '-SW', binary])
data = run([binary])
(OUT / 'bare.bin').write_bytes(data)
hosted = OUT / 'hosted'
run([*flags, '-I', host, caller, '-L', host, '-Wl,-rpath,' + str(host), '-lomega_native', '-o', hosted])
reference = run([hosted])
(OUT / 'hosted.bin').write_bytes(reference)
assert data == reference
wrong = OUT / 'red'
bare(native, wrong, True)
assert not run([wrong], expected=90)
print('PASS bare ELF: no imports/interpreter/TLS; hosted bytes identical; red exits 90')
mutants = [
    ('nop-word', 'bw_enc_sm121.om', 'const NOPW ulong 31000', 'const NOPW ulong 31001', 2),
    ('nop-control', 'bw_enc_sm121.om', 'const NOPC ulong 1032192', 'const NOPC ulong 1032193', 2),
    ('pad-floor', 'bw_enc_sm121.om', 'cnt = 32', 'cnt = 31', 1),
    ('byte-size', 'bw_enc_sm121.om', 'ret mul cnt 16', 'ret mul cnt 15', 1),
    ('nan-quiet', 'no_bf16.om', 'ret ulong or shr u 16 64', 'ret ulong shr u 16', 7),
    ('rne-tie', 'no_bf16.om', 'and shr u 16 1', 'and shr u 16 0', 7),
]
for name, unit, before, after, rc in mutants:
    work = OUT / name
    work.mkdir(exist_ok=True)
    copies = []
    for source in sources:
        content = source.read_text()
        if source.name == unit:
            assert content.count(before) == 1
            content = content.replace(before, after)
        dest = work / source.name
        dest.write_text(content)
        copies.append(dest)
    lib = work / 'native'
    run([BRIDGE / 'tools/native', '--freestanding', lib, *copies])
    executable = work / 'bare'
    bare(lib, executable)
    run([executable], expected=rc)
    print('PASS killed source mutant', name, 'rc', rc)
print('PASS NO_POSIX native differential, red control and six killed mutants')

# Exercise the existing complete C-vs-Omega packer sweep through the NO_POSIX
# production shell, including exact refusal messages and complete image bytes.
for mode, extra, expected in (
    ('cubin', [], 0),
    ('cubin-red', ['-DCUDAPP_CUBIN_DIFF_SELFTEST'], -6),
):
    executable = OUT / mode
    run([*flags, '-O0', '--coverage', '-DOMEGA_GPU_NO_POSIX', '-DCUDAPP_MIGRATION_PENDING_CUBIN_DIFF',
         *extra, '-I', ROOT / 'src', '-I', native,
         ROOT / 'tests/no_posix_cubin.c', ROOT / 'src/omega_cubin.c',
         ROOT / 'src/cudapp_freest_core_cubinmsg.c', ROOT / 'src/cudapp_freest_core_cubadmit.c',
         native / 'omega_native.o', native / 'omega_api_defs.o', '-o', executable])
    run([executable], expected=expected)
    if mode == 'cubin':
        run(['gcov', '-b', '-f', '-o', OUT / 'cubin-omega_cubin.gcno',
             ROOT / 'src/omega_cubin.c'])
print('PASS NO_POSIX cubin production shell: C differential and injected-byte red control')

oracle = OUT / 'codegen-oracle'
run([*flags, '-O0', '--coverage', '-DOMEGA_GPU_NO_POSIX',
     '-DCUDAPP_MIGRATION_PENDING_C_NO_POSIX', '-I', ROOT / 'src', '-I', native,
     ROOT / 'tests/test_omega_codegen_freestanding.c',
     ROOT / 'src/omega_blackwell_codegen.c', ROOT / 'src/omega_blackwell_matmul.c',
     ROOT / 'src/sha256.c', ROOT / 'src/cudapp_freest_f16.c',
     ROOT / 'src/cudapp_freest_bw_scalar.c', ROOT / 'src/cudapp_freest_cg_fields.c',
     native / 'omega_native.o', native / 'omega_api_defs.o', '-o', oracle])
# NO_POSIX's --no-sweep runs buffer/golden fixtures but not a second exhaustive
# bf16 sweep. The normal test-codegen-freestanding target retains that sweep.
run([oracle, '--no-sweep'])
run([ROOT / 'build/test_codegen_freestanding', '--no-sweep'])
observed = run([oracle, '--dump'])
assert observed == run([ROOT / 'build/test_codegen_freestanding', '--dump'])
run(['gcov', '-b', '-f', '-o', OUT / 'codegen-oracle-omega_blackwell_codegen.gcno',
     ROOT / 'src/omega_blackwell_codegen.c'])
print('PASS C_NO_POSIX encoder oracle dump equals native dump; gcov recorded')
