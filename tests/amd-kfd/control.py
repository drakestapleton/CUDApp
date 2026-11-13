#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Test instrument: acceptance comparisons and their guard-removal controls.

Both the positive gate and the negative case call matches. A mutant of this
module deletes its final comparison; each negative case must then fail, without
printing CAUGHT. Producer executions happen before this test, not in its setup.
"""
import hashlib
import pathlib
import subprocess
import sys


def matches(kind, actual, expected):
    if kind == 'table':
        def rows(path):
            return [tuple(map(int, line.split('#', 1)[0].split()))
                    for line in pathlib.Path(path).read_text().splitlines()]
        actual, expected = rows(actual), rows(expected)
    elif kind == 'digest':
        actual = hashlib.sha256(pathlib.Path(actual).read_bytes()).hexdigest()
    elif kind == 'exit':
        actual, expected = int(actual), int(expected)
    else:
        raise ValueError('unknown comparison ' + kind)
    return actual == expected


def reject(label, kind, actual, expected):
    if matches(kind, actual, expected):
        raise AssertionError('CONTROL ESCAPED ' + label)
    print('CONTROL CAUGHT ' + label)


def prove(out, label, kind, actual, expected):
    """Run the same negative case against a copy with the acceptance guard deleted."""
    source = pathlib.Path(__file__).read_text()
    guard = '    return actual == expected\n'
    assert source.count(guard) == 1, 'INVALID guard-removal setup'
    mutant = pathlib.Path(out) / 'comparison-deleted.py'
    mutant.write_text(source.replace(guard, '    return True\n'))
    cmd = [sys.executable, '-I', str(mutant), 'reject', label, kind, str(actual), str(expected)]
    p = subprocess.run(cmd, capture_output=True, timeout=10)
    log = pathlib.Path(out) / ('guard-removal-' + label + '.log')
    log.write_text('command=' + repr(cmd) + '\nexit=' + str(p.returncode) + '\n'
                   + p.stdout.decode() + p.stderr.decode())
    assert p.returncode == 1 and ('CONTROL ESCAPED ' + label).encode() in p.stderr, ('INVALID or escaped guard mutant', label, p.returncode)
    assert b'CONTROL CAUGHT' not in p.stdout, ('guard mutant printed CAUGHT', label)
    reject(label, kind, actual, expected)
    print('GUARD-REMOVAL PASS ' + label + ' mutant_exit=1 log=' + str(log))


if __name__ == '__main__':
    try:
        mode, *args = sys.argv[1:]
        if mode == 'compare':
            sys.exit(0 if matches(*args) else 1)
        elif mode == 'reject':
            reject(*args)
        elif mode == 'prove':
            prove(*args)
        else:
            raise ValueError('unknown mode ' + mode)
    except (AssertionError, ValueError, OSError, subprocess.SubprocessError) as exc:
        print(str(exc), file=sys.stderr)
        sys.exit(1)
