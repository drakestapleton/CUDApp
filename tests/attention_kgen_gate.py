#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Serialize the attention migration host gates and capture exact receipts.
Invoke only inside bwrap with private /dev and CPU affinity 10-14. Production
code is seeded Omega; this Python program is a read-only verification oracle.
"""
import os
from pathlib import Path
import shlex
import subprocess
import sys

root = Path(__file__).resolve().parents[1]
out = root / 'evidence/api-kernel-gen-attn/2026-11-06'
out.mkdir(parents=True, exist_ok=True)
if Path('/dev/nvidiactl').exists() or Path('/dev/nvidia0').exists():
    raise SystemExit('FAIL attention gates require bwrap private /dev')
commands = [
    ('differential', ['make', '-j3', '-f', 'Makefile', '-f', 'tests/attention_kgen.mk', 'test-api-kgen-attention']),
    ('mutants', ['make', '-j3', '-f', 'Makefile', '-f', 'tests/attention_kgen.mk', 'test-api-kgen-attention-mutants']),
    ('attention', ['make', '-j3', 'test-gpu-attention']),
    ('decode', ['make', '-j3', 'test-gpu-decode-attn']),
    ('maxpos', ['make', '-j3', 'test-gpu-decode-attn-maxpos']),
    ('decode256', ['make', '-j3', 'test-gpu-decode-attn256']),
    ('prefill', ['make', '-j3', 'test-gpu-prefill-host']),
    ('units', ['sh', 'om/units.sh', '--check']),
    ('crumb-compile', ['crumb', 'compile', '.']),
    ('crumb-verify', ['crumb', 'verify', '.']),
]
selected = set(sys.argv[1:])
if selected - {name for name, _ in commands}:
    raise SystemExit('unknown attention gate selection')
if not selected or 'differential' in selected:
    (root / 'build/api-kgen/attention_diff.gcda').unlink(missing_ok=True)
receipt = out / 'rc.tsv'
rows = {}
if selected and receipt.exists():
    rows = {line.split('\t', 1)[0]: line for line in receipt.read_text().splitlines()[1:]}
for name, command in commands:
    if selected and name not in selected:
        continue
    print('RUN ' + name + ': ' + shlex.join(command), flush=True)
    result = subprocess.run(command, cwd=root, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    text = result.stdout.replace(str(Path.home()), '~')
    (out / (name + '.log')).write_text(text)
    final = next((line for line in reversed(text.splitlines()) if line.strip()), '(no output)')
    row = f'{name}\t{result.returncode}\t{shlex.join(command)}\t{final}'
    rows[name] = row
    print(row, flush=True)
    if result.returncode:
        receipt.write_text('name\trc\tcommand\tfinal line\n' + '\n'.join(rows[key] for key, _ in commands if key in rows) + '\n')
        raise SystemExit(result.returncode)
receipt.write_text('name\trc\tcommand\tfinal line\n' + '\n'.join(rows[key] for key, _ in commands if key in rows) + '\n')
print('PASS attention migration host acceptance; chip NOT_RUN', flush=True)
