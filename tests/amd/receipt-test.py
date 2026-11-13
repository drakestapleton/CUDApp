#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Independent temporary-repository controls for current-head receipt admission."""
import pathlib
import subprocess
import sys
import tempfile

checker = pathlib.Path(__file__).with_name('receipt.py')
with tempfile.TemporaryDirectory() as tmp:
    root = pathlib.Path(tmp) / 'repo'
    root.mkdir()
    def git(*args):
        subprocess.run(['git', '-C', str(root), '-c', 'user.name=Receipt Test',
                        '-c', 'user.email=receipt@example.invalid', *args], check=True, capture_output=True)
    git('init', '-q')
    (root / 'backend').mkdir()
    source = root / 'backend/input.om'
    source.write_text('first\n')
    git('add', '.')
    git('commit', '-qm', 'parent')
    out = pathlib.Path(tmp) / 'receipt'
    def run(mode, expected, label):
        p = subprocess.run([sys.executable, '-I', str(checker), mode, str(root), str(out)], capture_output=True, text=True)
        print(f'{label}: exit={p.returncode}\n{p.stdout}{p.stderr}', flush=True)
        assert p.returncode == expected, label
    run('write', 0, 'clean parent')
    parent = (out / 'inputs.sha256').read_text()
    source.write_text('child\n')
    run('check', 1, 'dirty input')
    git('add', '.')
    git('commit', '-qm', 'child')
    run('check', 1, 'copied parent receipt')
    (out / 'source-head').write_text(subprocess.check_output(['git', '-C', str(root), 'rev-parse', 'HEAD']).decode())
    run('check', 1, 'parent bytes relabelled current head')
    (out / 'inputs.sha256').unlink()
    (out / 'source-head').unlink()
    run('write', 0, 'clean child')
    good = (out / 'inputs.sha256').read_text()
    for label, bad in [('HOME path', good.replace('backend/', '$HOME/backend/')),
                       ('absolute path', good.replace('backend/', '/backend/')),
                       ('missing input', ''), ('duplicate input', good + good)]:
        (out / 'inputs.sha256').write_text(bad)
        run('check', 1, label)
    (out / 'inputs.sha256').write_text(good)
    p = subprocess.run(['sha256sum', '-c', str(out / 'inputs.sha256')], cwd=root, capture_output=True, text=True)
    print(f'sha256sum -c: exit={p.returncode}\n{p.stdout}{p.stderr}')
    assert p.returncode == 0
print('RECEIPT CONTROLS PASS')
