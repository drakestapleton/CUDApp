#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Test oracle only: arbitrary rational arithmetic, one RN-even binary32 rounding.
No host float operation implements FMA. Independent of the executor's fixed limbs.
"""
from fractions import Fraction
import itertools
import pathlib
import random
import struct
import subprocess
import sys

SHAPES = ((1, 1, 1), (1, 3, 3), (3, 1, 65), (3, 65, 3),
          (64, 3, 1), (65, 1, 64), (1, 64, 65), (3, 65, 64),
          (65, 65, 3), (65, 65, 65), (1, 1, 0), (3, 65, 0),
          (0, 65, 3), (3, 0, 65))
VALUES = (0, 0x80000000, 1, 0x80000001, 0x007fffff, 0x00800000,
          0x3f800000, 0xbf800000, 0x3f800001, 0x3f7ffffe, 0x7f7fffff,
          0xff7fffff, 0x7f800000, 0xff800000, 0x7fc12345, 0x7f812345,
          0x00800001, 0x33000000, 0x4c000000, 0xcc000000)
FINITE = (0x3f800001, 0xbf800001, 0x3f7ffffe, 0x3eaaaaab, 0xbe4ccccd,
          0x33000000, 1, 0x80000001, 0x4c000000, 0xcc000000, 0x3f000000)


def finite(bits):
    exponent = (bits >> 23) & 255
    significand = (bits & 0x7fffff) | (0x800000 if exponent else 0)
    power = max(1, exponent) - 150
    value = Fraction(significand) * Fraction(2) ** power
    return -value if bits >> 31 else value


def round32(value):
    if not value:
        return 0
    sign = 0x80000000 if value < 0 else 0
    value = abs(value)
    e = value.numerator.bit_length() - value.denominator.bit_length()
    if value < Fraction(2) ** e:
        e -= 1
    quantum = max(-149, e - 23)
    scaled = value / (Fraction(2) ** quantum)
    q, r = divmod(scaled.numerator, scaled.denominator)
    if 2 * r > scaled.denominator or (2 * r == scaled.denominator and q & 1):
        q += 1
    if q == 1 << 24:
        q >>= 1
        quantum += 1
    if quantum > 104:
        return sign | 0x7f800000
    exponent = quantum + 150 if q >= 1 << 23 else 0
    return sign | exponent << 23 | (q & 0x7fffff)


def fma(a, b, c):
    ax, bx, cx = (x & 0x7fffffff for x in (a, b, c))
    if max(ax, bx, cx) > 0x7f800000:
        return 0x7fc00000
    product_sign = (a ^ b) & 0x80000000
    if ax == 0x7f800000 or bx == 0x7f800000:
        if not ax or not bx or (cx == 0x7f800000 and product_sign != c & 0x80000000):
            return 0x7fc00000
        return product_sign | 0x7f800000
    if cx == 0x7f800000:
        return c
    if (not ax or not bx) and not cx:
        return product_sign & c & 0x80000000
    return round32(finite(a) * finite(b) + finite(c))


def probe(exe):
    triples = list(itertools.product(VALUES, repeat=3))
    rng = random.Random(94232)
    triples += [tuple(rng.getrandbits(32) for _ in range(3)) for _ in range(10000)]
    # Deep product cancellation, overflow canceled before rounding, and subnormal halfway ties.
    triples += [(0x3f800001, 0x3f7ffffe, 0xbf800000),
                (0x7f7fffff, 0x40000000, 0xff7fffff),
                (1, 0x3f000000, 0), (3, 0x3f000000, 0),
                (0x80000001, 0x3f000000, 0x80000000)]
    data = b''.join(struct.pack('<III', *row) for row in triples)
    result = subprocess.run([exe], input=data, capture_output=True, timeout=60)
    assert result.returncode == 0 and result.stderr == b'', (result.returncode, result.stderr)
    expected = b''.join(struct.pack('<I', fma(*row)) for row in triples)
    if result.stdout != expected:
        index = next(i for i, (x, y) in enumerate(zip(result.stdout, expected)) if x != y) // 4
        actual, = struct.unpack_from('<I', result.stdout, index * 4)
        raise AssertionError(f'fma triple={triples[index]} observed={actual:08x} expected={fma(*triples[index]):08x}')
    print(f'PASS exact FMA rational-reference triples={len(triples)} seed=94232 RN-even denormals signed-zeros infinities NaNs', flush=True)


def fixture(m, n, k, sensitive=False, values=FINITE):
    # Guards immediately before and after every array; check the ENTIRE memory file.
    ap = 64
    bp = ap + 4 * m * k + 32
    cp = bp + 4 * k * n + 32
    data = bytearray([0xa5] * (cp + 4 * m * n + 64))
    struct.pack_into('<QQQIII', data, 0, ap, bp, cp, m, n, k)
    av = [values[(i * 7 + i // max(1, k)) % len(values)] for i in range(m * k)]
    bv = [values[(i * 13 + i // max(1, n) + 3) % len(values)] for i in range(k * n)]
    if sensitive:
        assert (m, n, k) == (1, 1, 2)
        av, bv = [0xbf800000, 0x3f800001], [0x3f800000, 0x3f7ffffe]
    for i, bits in enumerate(av):
        struct.pack_into('<I', data, ap + i * 4, bits)
    for i, bits in enumerate(bv):
        struct.pack_into('<I', data, bp + i * 4, bits)
    expected = bytearray(data)
    for row in range(m):
        for col in range(n):
            acc = 0
            for inner in range(k):
                acc = fma(av[row * k + inner], bv[inner * n + col], acc)
            struct.pack_into('<I', expected, cp + 4 * (row * n + col), acc)
    return data, expected


def check(exe, text, mem, m, n, k, sensitive=False, values=FINITE):
    data, expected = fixture(m, n, k, sensitive, values)
    mem.write_bytes(data)
    # Deliberate extra workgroup in BOTH dimensions checks masked columns and the s3 row guard.
    result = subprocess.run([exe, str(text), str(mem), str(n + 64), str(m + 1)],
                            capture_output=True, timeout=180)
    assert result.returncode == 0 and result.stdout == b'' and result.stderr == b'', result
    actual = mem.read_bytes()
    if actual != expected:
        offset = next((i for i, (a, b) in enumerate(zip(actual, expected)) if a != b), None)
        raise AssertionError(f'sgemm bit mismatch shape={(m,n,k)} byte={offset}')


def suite(exe, text, mutant_text, out):
    mem = pathlib.Path(out) / 'sgemm-memory.bin'
    for values, mode in ((FINITE, 'finite'), (VALUES, 'special')):
        for shape in SHAPES:
            check(exe, text, mem, *shape, values=values)
            print(f'PASS sgemm M={shape[0]} N={shape[1]} K={shape[2]} inputs={mode} ordered-fma every-output-bit and all guards', flush=True)
    check(exe, text, mem, 1, 1, 2, True)
    fused = fma(0x3f800001, 0x3f7ffffe, 0xbf800000)
    separate = fma(fma(0x3f800001, 0x3f7ffffe, 0), 0x3f800000, 0xbf800000)
    assert fused != separate
    try:
        check(exe, mutant_text, mem, 1, 1, 2, True)
    except AssertionError as error:
        assert str(error).startswith('sgemm bit mismatch'), error
        print(f'CONTROL sgemm-separate-mul-add CAUGHT fused={fused:08x} separate={separate:08x}', flush=True)
    else:
        raise AssertionError('unfused mutant escaped')
    data, _ = fixture(1, 1, 1)
    mem.write_bytes(data)
    code = bytearray(pathlib.Path(text).read_bytes())
    needle = struct.pack('<I', (28 << 25) | (11 << 17) | (11 << 9) | 263)
    assert code.count(needle) == 1
    index = code.index(needle)
    struct.pack_into('<I', code, index, (28 << 25) | (11 << 17) | (11 << 9) | 19)
    invalid = pathlib.Path(out) / 'constant-bus.text'
    invalid.write_bytes(code)
    result = subprocess.run([exe, str(invalid), str(mem), '1', '1'], capture_output=True, timeout=60)
    assert result.returncode == 66 and result.stdout == b'' and result.stderr == b'', result
    assert mem.read_bytes() == data
    print('CONTROL addc-second-scalar-constant-bus CAUGHT exit=66 memory-unchanged', flush=True)
    print(f'PASS sgemm shapes={len(SHAPES)} patterns=finite,special plus fused-sensitive shape and overdispatch X/Y guard groups hardware=NOT_RUN', flush=True)


def mutant(source, target):
    original = pathlib.Path(source).read_text()
    needle = 'I 5 270 6 4 0x3b'
    assert original.count(needle) == 1
    changed = original.replace(needle, 'I 5 270 6 14 0x05\nI 5 270 4 4 0x01')
    pathlib.Path(target).write_text(changed)


def summary(kernels):
    import runpy
    counts = runpy.run_path(str(pathlib.Path(__file__).with_name('oracle.py')))['COUNTS']
    semantics = runpy.run_path(str(pathlib.Path(__file__).with_name('semantics.py')))['CASES']
    fp8 = runpy.run_path(str(pathlib.Path(__file__).with_name('fp8gemv.py')))
    bf16 = runpy.run_path(str(pathlib.Path(__file__).with_name('bf16gemv.py')))
    gemv2 = runpy.run_path(str(pathlib.Path(__file__).with_name('gemv2.py')))
    attention = runpy.run_path(str(pathlib.Path(__file__).with_name('attention.py')))
    prep = runpy.run_path(str(pathlib.Path(__file__).with_name('prep.py')))
    print(f'AMD-GFX942-EXEC PASS kernels={kernels.replace(" ", ",")} '
          f'counts={",".join(map(str, counts))} sgemm_shapes={len(SHAPES)} '
          f'sgemm_cases={2*len(SHAPES)+1} semantics_cases={len(semantics)} '
          f'fp8gemv_shapes={len(fp8["SHAPES"])} fp8gemv_cases={2*len(fp8["SHAPES"])+1} '
          f'fp8_conversion_bytes={len(range(256))} '
          f'bf16gemv_shapes={len(bf16["SHAPES"])} bf16gemv_cases={2*len(bf16["SHAPES"])+1} '
          f'gemv2_shapes={len(gemv2["SHAPES"])} gemv2_cases_per_kernel={2*len(gemv2["SHAPES"])+1} '
          f'gemv2_sampled_shapes={len(gemv2["LARGE"])} '
          f'attention_T={",".join(map(str, attention["SHAPES"]))} '
          f'attention_cases={len(attention["SHAPES"])+1} '
          f'prep_positions={",".join(map(str, prep["POSITIONS"]))} '
          f'prep_cases={len(prep["POSITIONS"])+1} gate_cases=1 '
          f'rope_cases={len(prep["ROPE_POSITIONS"])} hardware=NOT_RUN')


if __name__ == '__main__':
    {'probe': probe, 'suite': suite, 'mutant': mutant, 'summary': summary}[sys.argv[1]](*sys.argv[2:])
