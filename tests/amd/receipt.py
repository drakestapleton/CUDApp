#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Host-test receipt admission. Historical evidence is never current-head evidence.

Usage: receipt.py write|check ROOT OUTPUT_DIRECTORY
All tracked backend and gate inputs are included, not a hand-maintained file list.
The compiler is external: its independently enforced pin stays in the gate log,
not an unresolvable absolute pathname in inputs.sha256.
"""
import hashlib
import pathlib
import re
import subprocess
import sys


def git(root, *args):
    return subprocess.check_output(['git', '-C', str(root), *args])


def inputs(root):
    paths = git(root, 'ls-tree', '-r', '--name-only', 'HEAD', '--',
                'backend', 'tests/amd', 'tests/amd-kfd', 'tests/amd-exec',
                'tests/multitarget', 'Makefile').decode().splitlines()
    assert paths, 'empty committed input set'
    result = {}
    for path in paths:
        data = git(root, 'show', 'HEAD:' + path)
        digest = hashlib.sha256(data).hexdigest()
        actual = root / path
        assert not actual.is_symlink(), f'symlink input: {path}'
        assert actual.read_bytes() == data, f'dirty input: {path}'
        result[path] = digest
    # Untracked source can affect glob-based builds and Python imports too.
    extra = git(root, 'ls-files', '--others', '--exclude-standard', '--',
                'backend', 'tests/amd', 'tests/amd-kfd', 'tests/amd-exec',
                'tests/multitarget').decode().splitlines()
    assert not extra, f'untracked inputs: {extra}'
    return result


def check(root, out, expected=None):
    head = git(root, 'rev-parse', 'HEAD').decode().strip()
    assert (out / 'source-head').read_text() == head + '\n', 'receipt is not this HEAD'
    expected = inputs(root) if expected is None else expected
    rows = {}
    for line in (out / 'inputs.sha256').read_text().splitlines():
        match = re.fullmatch(r'([0-9a-f]{64})  ([^\s]+)', line)
        assert match, f'malformed receipt line: {line!r}'
        digest, path = match.groups()
        assert not path.startswith('/') and '$' not in path and '\\' not in path, f'non-relative path: {path}'
        assert all(p not in ('', '.', '..') for p in path.split('/')), f'path alias: {path}'
        assert path not in rows, f'duplicate input: {path}'
        rows[path] = digest
    assert rows == expected, 'receipt differs from complete committed HEAD input bytes'
    for path, digest in rows.items():
        print(f'SOURCE sha256={digest} path={path}')
    print(f'RECEIPT PASS head={head} inputs={len(rows)} skipped=0')


def main():
    mode, root, out = sys.argv[1:]
    root, out = pathlib.Path(root).resolve(), pathlib.Path(out)
    assert mode in ('write', 'check'), mode
    expected = inputs(root)
    if mode == 'write':
        # Never overwrite a receipt: an existing output must first prove this HEAD.
        if (out / 'inputs.sha256').exists() or (out / 'source-head').exists():
            check(root, out, expected)
            return
        out.mkdir(parents=True, exist_ok=True)
        (out / 'source-head').write_text(git(root, 'rev-parse', 'HEAD').decode())
        (out / 'inputs.sha256').write_text(''.join(f'{digest}  {path}\n' for path, digest in expected.items()))
    check(root, out, expected)


if __name__ == '__main__':
    try:
        main()
    except (AssertionError, OSError, ValueError, subprocess.SubprocessError) as error:
        print(f'RECEIPT FAIL {error}', file=sys.stderr)
        sys.exit(1)
