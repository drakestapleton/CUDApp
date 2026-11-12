#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Test only: BF16 sign/exponent/fraction -> exact Fraction -> FP32 FMA.
No production code, host floating arithmetic, or FP16 rounding implements the oracle.
"""
from fractions import Fraction
import itertools
import pathlib
import runpy
import struct
import subprocess
import sys

SG = runpy.run_path(str(pathlib.Path(__file__).with_name('sgemm.py')))
SHAPES = tuple(itertools.product((0, 1, 63, 64, 65), (0, 1, 3, 64, 65, 130)))
FINITE = (0x3f80, 0xbf80, 0x3f81, 0xbf81, 0x3f7f, 0x3eab, 0xbe4d,
          1, 0x8001, 0x007f, 0x807f, 0x0080, 0x8080, 0x7f7f, 0xff7f,
          0, 0x8000, 0x3380, 0x4c80, 0xcc80)
VALUES = FINITE + (0x7f80, 0xff80, 0x7fc1, 0x7f81, 0xffc1, 0xff81)


def decode(bits):
    # BF16 S1E8M7, bias127; same exponent/sign position as FP32 after exact widening.
    exponent, fraction = (bits >> 7) & 255, bits & 127
    sign = (bits >> 15) << 31
    if exponent == 255:
        return sign | 0x7f800000 | fraction << 16
    if not exponent and not fraction:
        return sign
    value = Fraction(fraction if not exponent else 128 + fraction, 128)
    value *= Fraction(2) ** ((exponent if exponent else 1) - 127)
    if sign: value = -value
    return SG['round32'](value)


def fixture(m, k, special=False, isolated=False):
    # A two-byte-aligned, non-dword-aligned base and odd K exercise both row-end alignments.
    wp = 66
    xp = (wp + 2*m*k + 19) & ~3
    yp = xp + 4*k + 16
    data = bytearray([0xa5] * (yp + 4*m + 64))
    struct.pack_into('<QQQII', data, 0, wp, xp, yp, m, k)
    values = VALUES if special else FINITE
    weights = [values[(row*7+col*11) % len(values)] for row in range(m) for col in range(k)]
    inputs = SG['VALUES'] if special else SG['FINITE']
    xs = [inputs[(i*3) % len(inputs)] for i in range(k)]
    if isolated:
        assert (m, k) == (len(VALUES), 1)
        weights, xs = list(VALUES), [0x3f800000]
    for i, bits in enumerate(weights): struct.pack_into('<H', data, wp+2*i, bits)
    for i, bits in enumerate(xs): struct.pack_into('<I', data, xp+4*i, bits)
    expected = bytearray(data)
    for row in range(m):
        acc = 0
        for inner in range(k):
            acc = SG['fma'](decode(weights[row*k+inner]), xs[inner], acc)
        struct.pack_into('<I', expected, yp+4*row, acc)
    return data, expected


def check(exe, text, out, m, k, special=False, isolated=False):
    memory = pathlib.Path(out)/'bf16gemv-memory.bin'
    data, expected = fixture(m, k, special, isolated)
    memory.write_bytes(data)
    result = subprocess.run([exe, str(text), str(memory), str(m+64), '1'], capture_output=True, timeout=180)
    assert result.returncode == 0 and not result.stdout and not result.stderr, result
    actual = memory.read_bytes()
    if actual != expected:
        offset = next((i for i, (a,b) in enumerate(zip(actual, expected)) if a != b), min(len(actual), len(expected)))
        raise AssertionError(f'bf16gemv bit mismatch M={m} K={k} byte={offset}')


def suite(exe, text, swapped_exe, truncated_text, out):
    for special in (False, True):
        for m, k in SHAPES:
            check(exe, text, out, m, k, special)
            print(f'PASS bf16gemv M={m} K={k} inputs={"special" if special else "finite"} ordered-FMA exact-BF16 every-bit all-guards', flush=True)
    check(exe, text, out, len(VALUES), 1, isolated=True)
    pathlib.Path(out, 'bf16-values.tsv').write_text('BF16_bits\tFP32_bits\n' + ''.join(f'{b:04x}\t{decode(b):08x}\n' for b in VALUES))
    print('PASS bf16gemv isolated BF16 signed-zeros subnormals normals extremes infinities quiet/signaling-NaNs', flush=True)
    for label, binary, code in [('big-endian-weight', swapped_exe, text), ('truncated-FP32-x', exe, truncated_text)]:
        try:
            check(binary, code, out, 1, 1)
        except AssertionError as error:
            assert str(error).startswith('bf16gemv bit mismatch'), error
            print(f'CONTROL bf16gemv-{label} CAUGHT {error}', flush=True)
        else:
            raise AssertionError(f'escaped {label}')


def swap_mutant(source, target):
    text = pathlib.Path(source).read_text()
    needle = 'ret or ulong get io 0 shl ulong get io 1 8\n'
    assert text.count(needle) == 1
    pathlib.Path(target).write_text(text.replace(needle, 'ret or ulong get io 1 shl ulong get io 0 8\n'))


def truncate_mutant(source, target):
    text = pathlib.Path(source).read_text()
    needle = 'I 5 263 6 4 59\n'
    assert text.count(needle) == 1
    pathlib.Path(target).write_text(text.replace(needle, 'I 5 255 6 6 19\nL 0xffff0000\n' + needle))


if __name__ == '__main__':
    {'suite': suite, 'swap-mutant': swap_mutant, 'truncate-mutant': truncate_mutant}[sys.argv[1]](*sys.argv[2:])
