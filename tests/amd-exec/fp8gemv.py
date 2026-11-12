#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Test only. FP8 format definition -> exact Fraction -> nearest-even binary32.
The table, fixtures and results are recorded by the gate; no host FP8 library is used.
"""
from fractions import Fraction
import itertools
import pathlib
import runpy
import struct
import subprocess
import sys

SG = runpy.run_path(str(pathlib.Path(__file__).with_name('sgemm.py')))
SHAPES = tuple(itertools.product((1, 63, 64, 65), (0, 1, 3, 64, 65))) + ((0, 3),)
SCALES = (0x40000000, 0xbf000000, 0, 0x80000000, 1, 0x80000001, 0x7f7fffff)


def decode(byte):
    # MI300 ISA PDF p58: bias8, unsigned zero, sole NaN0x80; exponent15 is finite.
    if byte == 0x80:
        return 0x7fc00000  # documented host canonical-NaN policy
    exponent, fraction = (byte >> 3) & 15, byte & 7
    value = Fraction(fraction if exponent == 0 else 8 + fraction, 8)
    value *= Fraction(2) ** ((exponent if exponent else 1) - 8)
    if byte & 128:
        value = -value
    return SG['round32'](value)


def fixture(m, k, special=False, all_bytes=False):
    wp = 64
    xp = (wp + m*k + 19) & ~3
    sp = xp + 4*k + 16
    yp = sp + 4*m + 16
    data = bytearray([0xa5] * (yp + 4*m + 64))
    struct.pack_into('<QQQQII', data, 0, wp, xp, sp, yp, m, k)
    weights = [((i*13 + i//max(k, 1) + 56) & 255) for i in range(m*k)]
    if not special:
        weights = [0x7f if w == 0x80 else w for w in weights]
    values = SG['VALUES'] if special else SG['FINITE']
    xs = [values[(i*3+3) % len(values)] for i in range(k)]
    scales = [(SG['VALUES'] if special else SCALES)[i % len(SG['VALUES'] if special else SCALES)] for i in range(m)]
    if all_bytes:
        assert (m, k) == (256, 1)
        weights, xs, scales = list(range(256)), [0x3f800000], [0x3f800000]*256
    data[wp:wp+m*k] = bytes(weights)
    for i, bits in enumerate(xs): struct.pack_into('<I', data, xp+4*i, bits)
    for i, bits in enumerate(scales): struct.pack_into('<I', data, sp+4*i, bits)
    expected = bytearray(data)
    for row in range(m):
        acc = 0
        for inner in range(k):
            acc = SG['fma'](decode(weights[row*k+inner]), xs[inner], acc)
        answer = SG['fma'](scales[row], acc, (scales[row]^acc)&0x80000000)
        struct.pack_into('<I', expected, yp+4*row, answer)
    return data, expected


def check(exe, text, out, m, k, special=False, all_bytes=False):
    memory = pathlib.Path(out)/'fp8gemv-memory.bin'
    data, expected = fixture(m, k, special, all_bytes)
    memory.write_bytes(data)
    result = subprocess.run([exe, str(text), str(memory), str(m+64), '1'], capture_output=True, timeout=180)
    assert result.returncode == 0 and not result.stdout and not result.stderr, result
    actual = memory.read_bytes()
    if actual != expected:
        offset = next(i for i, (a,b) in enumerate(zip(actual, expected)) if a != b)
        raise AssertionError(f'fp8gemv bit mismatch M={m} K={k} byte={offset}')


def suite(exe, text, ocp_exe, no_scale, out):
    for special in (False, True):
        for m, k in SHAPES:
            check(exe, text, out, m, k, special)
            print(f'PASS fp8gemv M={m} K={k} inputs={"special" if special else "finite"} ordered-FMA row-scale every-bit all-guards', flush=True)
    check(exe, text, out, 256, 1, all_bytes=True)
    table = pathlib.Path(out)/'fp8-all256.tsv'
    table.write_text('byte\tE4M3FNUZ_FP32_bits\n' + ''.join(f'{b:02x}\t{decode(b):08x}\n' for b in range(256)))
    print('PASS fp8gemv all256 E4M3FNUZ table exact Fraction oracle including sole-NaN80 unsigned-zero subnormals exponent15 finite', flush=True)
    for label, binary, code in [('OCP-E4M3FN-bias7', ocp_exe, text), ('dropped-row-scale', exe, no_scale)]:
        try:
            check(binary, code, out, 1, 1)
        except AssertionError as error:
            assert str(error).startswith('fp8gemv bit mismatch'), error
            print(f'CONTROL fp8gemv-{label} CAUGHT {error}', flush=True)
        else:
            raise AssertionError(f'escaped {label}')


def mutate(source, target):
    text = pathlib.Path(source).read_text()
    begin = text.index('fn fp8bits ')
    end = text.index('fn vecalu ', begin)
    # Complete OCP E4M3FN mutant: bias7, signed zero and NaN +/-0x7f (not FNUZ).
    replacement = '''fn fp8bits a ulong to ulong
v = and a 255
m = and v 127
if eq m 127
ret 0x7fc00000
end
if eq m 0
ret shl and v 128 24
end
e = shr m 3
f = and m 7
if eq e 0
e = 121
while lt f 8 bound 3
f = shl f 1
e = sub e 1
end
else
e = add e 120
end
ret or shl and v 128 24 or shl e 23 shl and f 7 20
end
'''
    pathlib.Path(target).write_text(text[:begin]+replacement+text[end:])


def scale_mutant(source, target):
    text = pathlib.Path(source).read_text()
    needle = 'I 5 263 4 4 5\n'
    assert text.count(needle) == 1
    pathlib.Path(target).write_text(text.replace(needle, 'I 8 260 1 4\n'))


NESTED_WAVE = """while true bound 999999
c = ulong 0
while lt c 999998 bound 999999
if not step pc
ret
end
pc = get st 5
c = add c 1
end
end
"""
FLAT_WAVE = """while true bound 999999
if not step pc
ret
end
pc = get st 5
end
"""


def flat_mutant(source, target):
    # The executor before the fix: one wave loop bound 999999 (the pinned compiler's largest bound).
    text = pathlib.Path(source).read_text()
    assert text.count(NESTED_WAVE) == 1
    pathlib.Path(target).write_text(text.replace(NESTED_WAVE, FLAT_WAVE))


def long_wave(exe, flat_exe, text, out):
    # K=100000: eleven wave steps per k (two loads, wait, convert, FMA, four address adds, counter, branch), so one wave runs past 999999 steps.
    check(exe, text, out, 1, 100000)
    print('PASS fp8gemv M=1 K=100000 one wave past 999999 steps every-bit all-guards', flush=True)
    memory = pathlib.Path(out)/'fp8gemv-memory.bin'
    memory.write_bytes(fixture(1, 100000)[0])
    result = subprocess.run([flat_exe, str(text), str(memory), '65', '1'], capture_output=True, timeout=600)
    assert result.returncode == 104, result.returncode
    print('CONTROL single-loop executor CAUGHT trap exit=104 on the same run', flush=True)


if __name__ == '__main__':
    {'suite': suite, 'mutate': mutate, 'scale-mutant': scale_mutant, 'flat-mutant': flat_mutant, 'long-wave': long_wave}[sys.argv[1]](*sys.argv[2:])
