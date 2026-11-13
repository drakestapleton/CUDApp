#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Reject stdlib-named sibling modules before instrument imports."""
import pathlib
import subprocess
import sys
import tempfile


def check(directory):
    found = sorted(p.stem for p in pathlib.Path(directory).glob('*.py')
                   if p.stem in sys.stdlib_module_names)
    if found:
        print(f'STDLIB-SHADOW FAIL directory={directory} names={found}')
        return 1
    print(f'STDLIB-SHADOW PASS directory={directory}')
    return 0


if __name__ == '__main__':
    if sys.argv[1] == 'test':
        with tempfile.TemporaryDirectory(prefix='stdlib-shadow.') as tmp:
            path = pathlib.Path(tmp) / 'token.py'
            path.write_text('raise RuntimeError("shadow must never be imported")\n')
            cmd = [sys.executable, '-I', __file__, tmp]
            p = subprocess.run(cmd, capture_output=True, text=True)
            print(f'command={cmd!r} exit={p.returncode}\n{p.stdout}{p.stderr}')
            assert p.returncode == 1 and "names=['token']" in p.stdout, 'INVALID or missed shadow control'
            print('CONTROL restored-token.py CAUGHT exit=1 before import')
        sys.exit(check(sys.argv[2]))
    sys.exit(check(sys.argv[1]))
