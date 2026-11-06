#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Host-only acceptance. Run under the repository's bwrap device isolation."""
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'build/no_posix/session-check'
OUT.mkdir(parents=True, exist_ok=True)
BRIDGE = Path(os.environ['OMEGA_BRIDGE'])
ENV = dict(os.environ, OMEGA_NATIVE_OMC2=os.environ['OMEGA_OMC'])
subprocess.run(['sh', ROOT / 'om/no-posix.sh', '--tools', OUT], check=True)
ENV.update(OMEGA_NATIVE_OMC=str(OUT / 'tools/omc-gen'),
           OMEGA_NATIVE_X86_OMC=str(OUT / 'tools/omc-x86'))
LOG = OUT / 'commands.log'
LOG.write_text('')

def run(args, rc=0, binary=None):
    args = [str(a) for a in args]
    p = subprocess.run(args, cwd=ROOT, env=ENV, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    with LOG.open('a') as f:
        f.write(' '.join(args) + '\nrc=' + str(p.returncode) + '\n')
        f.write(p.stderr.decode(errors='replace'))
        if binary is None:
            f.write(p.stdout.decode(errors='replace'))
    if binary is not None:
        binary.write_bytes(p.stdout)
    assert p.returncode == rc, (args, p.returncode, p.stderr.decode(errors='replace'))
    return p.stdout

names = ['admit', 'words', 'park', 'book', 'streams']
sources = [ROOT / 'build/no_posix/sources' / (n + '.om') for n in names]
fs = ROOT / 'build/no_posix/native'
host = OUT / 'host'
run([BRIDGE / 'tools/native', host, *sources])

def executable(native, name, red=False):
    exe = OUT / (name + '.exe')
    link = [native / 'omega_native.o'] if (native / 'omega_native.o').exists() else ['-L' + str(native), '-Wl,-rpath,' + str(native), '-lomega_native']
    args = ['gcc', '-std=gnu11', '-O2', '-I' + str(native), 'tests/no_posix_session.c', *link, '-o', exe]
    if red:
        args.insert(1, '-DNO_POSIX_SESSION_RED')
    run(args)
    return exe

host_exe = executable(host, 'hosted')
fs_exe = executable(fs, 'freestanding')
expected = run([host_exe], binary=OUT / 'hosted.bin')
assert run([fs_exe], binary=OUT / 'freestanding.bin') == expected
red_exe = executable(fs, 'red', True)
run([red_exe], rc=90, binary=OUT / 'red.bin')
combined = OUT / 'native-set.o'
run(['gcc', '-r', '-nostdlib', fs / 'omega_native.o', fs / 'omega_api_defs.o', '-o', combined])
assert not run(['nm', '-u', combined]).strip()
run(['gcc', '-std=gnu11', '-O2', '-I' + str(fs), 'tests/no_posix_session.c', '-o', OUT / 'missing-native'], rc=1)

# Inspect actual consumers, including explicit flag=0 (not merely undefined).
for flag in (0, 1):
    for unit, omega, oracle in [('session', ['omega2_decide', 'omega2_emit', 'omega2_scan', 'omega2_streams_words'], ['ses_c', 'bw_mthd', 'xq_admit_c']),
                                ('cmdlist', ['omega2_book'], ['clbook_c'])]:
        obj = OUT / (unit + '-' + str(flag) + '.o')
        run(['gcc', '-std=gnu11', '-O0', '-ffreestanding', '-DOMEGA_GPU_NO_POSIX',
             '-DCUDAPP_MIGRATION_PENDING_C_NO_POSIX_SESSION=' + str(flag), '-Isrc', '-I' + str(fs),
             '-c', 'src/omega_gpu_' + unit + '.c', '-o', obj])
        nm = run(['nm', '-a', obj]).decode()
        dis = run(['objdump', '-dr', obj]).decode()
        (OUT / (obj.stem + '.nm')).write_text(nm)
        (OUT / (obj.stem + '.objdump')).write_text(dis)
        for symbol in omega:
            assert (symbol in nm) == (flag == 0), (unit, flag, symbol)
            assert (symbol in dis) == (flag == 0), (unit, flag, symbol, 'relocation')
        for symbol in oracle:
            assert (symbol in nm) == (flag == 1), (unit, flag, symbol)

# Each executable must build. A compiler failure never counts as a kill.
mutants = [
    ('gpr-floor', 'admit', 'ret 64\n', 'ret 65\n'),
    ('arg-offset', 'admit', 'ret 896\n', 'ret 895\n'),
    ('packet-tag', 'words', 'const MTHDTAG ulong 536870912', 'const MTHDTAG ulong 536870913'),
    ('park-found', 'park', 'ret 1\n', 'ret 0\n'),
    ('book-successor', 'book', 'add get a 1 1', 'add get a 1 2'),
    ('queue-bound', 'streams', 'if or eq i 0 gt i 3', 'if or eq i 0 gt i 2'),
]
for name, unit, old, new in mutants:
    directory = OUT / name
    directory.mkdir(exist_ok=True)
    altered = []
    for src in sources:
        dest = directory / src.name
        text = src.read_text()
        if src.stem == unit:
            assert text.count(old) == 1, (name, text.count(old))
            text = text.replace(old, new)
        dest.write_text(text)
        altered.append(dest)
    native = directory / 'native'
    run([BRIDGE / 'tools/native', '--freestanding', native, *altered])
    exe = executable(native, name)
    p = subprocess.run([str(exe)], cwd=ROOT, env=ENV, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    (directory / 'observed.bin').write_bytes(p.stdout)
    assert p.returncode != 0 or p.stdout != expected, ('SURVIVED', name)
    with LOG.open('a') as f:
        f.write('KILLED ' + name + ' rc=' + str(p.returncode) + '\n')
    print('KILLED', name)
print('PASS no-posix session: 5 exports hosted/freestanding differential; ABI refusal; symbol OFF/ON red; 6 killed mutants')
