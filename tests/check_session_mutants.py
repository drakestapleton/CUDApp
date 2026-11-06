#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Mutate hosted default-path Omega units and platform glue, never the C oracle.

A missing/ambiguous site, converter failure, compile failure, signal or timeout
is a harness error, NOT a killed mutant. Each kill requires rc=1 and FAIL output
from an assertion in the same tests that pass unmodified.
"""
import os
from pathlib import Path
import shlex
import subprocess
import sys

root = Path(__file__).resolve().parents[1]
work = root / 'build/gpu_session_mut'
work.mkdir(parents=True, exist_ok=True)
cc = shlex.split(os.environ.get('CC', 'cc'))
flags = ['-std=gnu11', '-O2', '-Wall', '-Wextra', '-Werror', '-D_GNU_SOURCE', '-Isrc']
objects = [p for p in shlex.split(os.environ['SESSION_FS_OBJECTS']) if not p.endswith('/omega_gpu_session.o')]
units = {'session_admit': 'om/session/admit.om', 'session_words': 'om/session/words.om',
         'session_park': 'om/session/park.om', 'cmdlist_book': 'om/cmdlist/book.om'}
# name, source, exact replacements (old, new, expected occurrence count)
mutants = [
    ('free-while-latched', 'session_admit', [('if ne and blocked WMASK 0\nret 0', 'if ne and blocked WMASK 0\nret 2', 1)]),
    ('open-while-latched', 'session_admit', [('fn opendec blocked ulong epoch ulong plat ulong opened ulong to ulong\nif ne and blocked WMASK 0\nret 1', 'fn opendec blocked ulong epoch ulong plat ulong opened ulong to ulong\nif ne and blocked WMASK 0\nret 0', 1)]),
    ('recover-without-latch', 'session_admit', [('if eq and latched WMASK 0\nret 2', 'if eq and latched WMASK 0\nret 0', 1)]),
    ('marker-payload', 'session_words', [('const MARKER1 ulong 1145324612', 'const MARKER1 ulong 0', 1)]),
    ('parked-generation', 'session_park', [('if eq get gen i wgen', 'if ne get gen i wgen', 1)]),
    ('stale-epoch', 'cmdlist_book', [('if eq get a 2 get a 3', 'if ne get a 2 get a 3', 1)]),
    ('latch-generation', 'cmdlist_book', [('and add get a 1 1 WMASK', 'get a 1', 1)]),
    ('ignore-cancel', 'shell', [('if (LOAD(g_cancel)) return WAIT_CANCELLED;', ';', 3)]),
    ('no-latch-on-timeout', 'shell', [('latch(w == WAIT_TIMED_OUT', 'omega_gpu_session_set_error(w == WAIT_TIMED_OUT', 1)]),
    ('stale-not-counted', 'shell', [('else { BUMP(g_stats.stale_refusals, 1u); fail_plat("dma_free", rc); }', 'else {}', 1)]),
    ('marker-not-polled', 'shell', [('RIGHTS_MARKER, PAGE', 'RIGHTS_DATA, PAGE', 1)]),
    ('close-while-latched', 'shell', [('if (g_open && !LOAD(g_blocked) && p) {', 'if (g_open && p) {', 1), ('if (!LOAD(g_blocked)) { /* a hook', 'if (1) { /* a hook', 1)]),
    ('recover-keeps-latch', 'shell', [('STORE(g_blocked, 0u);', ';', 1)]),
    ('recover-without-close', 'shell', [('const int prc = p->device_close(p->ctx, g_s.dev);', 'const int prc = OMEGA_GPU_PLAT_OK;', 1)]),
    ('recover-ignores-teardown-fault', 'shell', [('if (prc != OMEGA_GPU_PLAT_OK) {', 'if (0) {', 1)]),
    ('recover-ignores-module-latch', 'shell', [('const int latched = LOAD(g_blocked) || LOAD(g_module_latched);', 'const int latched = LOAD(g_blocked);', 1)]),
]

def exercise(directory, unit=None, changed=None):
    bindings = {name: str(root / ('src/cudapp_om_' + name + '.c')) for name in units}
    shell = 'src/omega_gpu_session.c'
    with (directory / 'build.log').open('w') as log:
        if unit == 'shell':
            shell = str(changed)
        elif unit is not None:
            env = dict(os.environ, OMEGA_UNITS=unit + '=' + str(changed), OMEGA_UNITS_DEST=str(directory))
            subprocess.run(['sh', 'om/units.sh'], cwd=root, env=env, stdout=log, stderr=log, check=True)
            bindings[unit] = str(directory / ('cudapp_om_' + unit + '.c'))
        subprocess.run(cc + flags + ['-c', shell, '-o', str(directory/'session.o')], cwd=root, stdout=log, stderr=log, check=True)
        subprocess.run(cc + flags + ['-DOMEGA_GPU_NO_POSIX', 'tests/test_omega_gpu_session_platform.c',
                       'tests/session_no_default.c', 'src/omega_ampere_qmd.c', 'src/cudapp_om_amp_runtime.c',
                       'src/cudapp_om_streams.c', str(directory/'session.o')] + objects +
                       [bindings[n] for n in ('session_admit', 'session_words', 'session_park')] +
                       ['-lpthread', '-o', str(directory/'platform')], cwd=root, stdout=log, stderr=log, check=True)
        subprocess.run(cc + flags + ['tests/session_policy_test.c', bindings['session_admit'], bindings['cmdlist_book'],
                       '-o', str(directory/'policy')], cwd=root, stdout=log, stderr=log, check=True)
    results = []
    for test in ('platform', 'policy'):
        run = subprocess.run([str(directory/test)], cwd=root, stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT, text=True, timeout=60)
        (directory/(test+'.log')).write_text(run.stdout)
        if run.returncode not in (0, 1) or (run.returncode == 1 and 'FAIL' not in run.stdout):
            sys.exit('FAIL harness: ' + str(directory) + '/' + test + '\n' + run.stdout)
        results.append(run.returncode)
    return results

baseline = work/'baseline'
baseline.mkdir(exist_ok=True)
if any(exercise(baseline)):
    sys.exit('FAIL unmodified hosted default path; see ' + str(baseline))
print('PASS unmodified hosted Omega session and policy boundaries', flush=True)
# Unit mutants regenerate .om through om/units.sh, which executes the pinned
# AArch64 omc. make sets SESSION_UNIT_MUTANTS=1 only where that compiler runs
# (CUDAPP_OMC_HOST); elsewhere those mutants are reported NOT_RUN, never killed.
run_units = os.environ['SESSION_UNIT_MUTANTS'] == '1'
killed = not_run = 0
for name, unit, replacements in mutants:
    directory = work/name
    directory.mkdir(exist_ok=True)
    source = root / ('src/omega_gpu_session.c' if unit == 'shell' else units[unit])
    text = source.read_text()
    for old, new, count in replacements:
        if text.count(old) != count:
            sys.exit('FAIL harness: missing/ambiguous mutant site ' + name)
        text = text.replace(old, new)
    if unit != 'shell' and not run_units:
        print('NOT_RUN Omega-unit mutant: ' + name + ' [' + unit + '] (requires the pinned AArch64 omc)', flush=True)
        not_run += 1
        continue
    changed = directory/source.name
    changed.write_text(text)
    if not any(exercise(directory, unit, changed)):
        sys.exit('FAIL mutant SURVIVED: ' + name)
    print('PASS behavioral mutant killed: ' + name + ' [' + unit + ']', flush=True)
    killed += 1
print('test-gpu-session-mutants: %d of %d mutants killed; %d Omega-unit mutants NOT_RUN' % (killed, killed, not_run))
