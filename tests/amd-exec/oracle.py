#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Test only. Python binary64 addition + struct binary32 rounding is the independent oracle.
Binary32 operands add exactly in binary64 when their exponents are close enough to
matter to binary32 rounding. At wider separation the small operand cannot change
round-to-nearest binary32. Overflow is signed infinity. NaNs are canonicalized to
0x7fc00000 (executor policy, not a claim about hardware NaN payload propagation).
No production code, LLVM disassembly, or .isa parsing implements the reference.
"""
import math
import pathlib
import random
import struct
import subprocess
import sys

COUNTS = (0, 1, 63, 64, 65, 200)
# Raw encodings: subnormal boundaries, both zeros, infinities, qNaN/sNaN,
# cancellation, overflow, half-ULP ties (even/odd), large exponent gaps.
PAIRS = (
    (0x00000001, 0x00000001), (0x007fffff, 0x00000001),
    (0x00800000, 0x807fffff), (0x80000001, 0x80000001),
    (0x00000000, 0x80000000), (0x80000000, 0x80000000),
    (0x00000000, 0x00000000), (0x7f800000, 0x3f800000),
    (0xff800000, 0xbf800000), (0x7f800000, 0xff800000),
    (0x7fc12345, 0x3f800000), (0x7f812345, 0xffc12345),
    (0x7f7fffff, 0x7f7fffff), (0xff7fffff, 0xff7fffff),
    (0x3f800000, 0xbf800000), (0x3f800000, 0x33800000),
    (0x3f800001, 0x33800000), (0xbf800001, 0xb3800000),
    (0x7f7fffff, 0x00000001), (0x00800001, 0x80000001),
    (0x3f800000, 0x80000001), (0x80800000, 0x007fffff),
    (0x00000001, 0x80000001), (0x7f800000, 0x7f800000),
)


def addref(a, b):
    x, = struct.unpack('<f', struct.pack('<I', a))
    y, = struct.unpack('<f', struct.pack('<I', b))
    z = x + y
    if math.isnan(z):
        return 0x7fc00000
    try:
        return struct.unpack('<I', struct.pack('<f', z))[0]
    except OverflowError:
        return 0xff800000 if z < 0 else 0x7f800000


def extract(source, target):
    data = pathlib.Path(source).read_bytes()
    assert data[:7] == b'\x7fELF\x02\x01\x01'
    off, = struct.unpack_from('<Q', data, 40)
    size, num, names = struct.unpack_from('<HHH', data, 58)
    assert size == 64 and names < num and off + size * num <= len(data)
    sections = [struct.unpack_from('<IIQQQQIIQQ', data, off + size * i) for i in range(num)]
    shstr = sections[names]
    strings = data[shstr[4]:shstr[4] + shstr[5]]
    matches = []
    for sh in sections:
        assert sh[0] < len(strings)
        end = strings.index(0, sh[0])
        if strings[sh[0]:end] == b'.text':
            assert sh[1] == 1 and sh[2] & 4 and sh[4] + sh[5] <= len(data)
            matches.append(data[sh[4]:sh[4] + sh[5]])
    assert len(matches) == 1 and matches[0] and len(matches[0]) % 4 == 0
    pathlib.Path(target).write_bytes(matches[0])


def fixture(n, pairs):
    capacity = max(64, ((n + 63) // 64) * 64) + 64
    a = 32
    b = a + capacity * 4
    c = b + capacity * 4
    data = bytearray([0xa5] * (c + capacity * 4 + 32))
    struct.pack_into('<QQQI', data, 0, a, b, c, n)
    expected = bytearray(data)
    for i in range(capacity):
        x, y = pairs[i % len(pairs)]
        struct.pack_into('<I', data, a + 4 * i, x)
        struct.pack_into('<I', data, b + 4 * i, y)
    expected[:] = data
    for i in range(n):
        x, y = pairs[i % len(pairs)]
        struct.pack_into('<I', expected, c + 4 * i, addref(x, y))
    return data, expected


def invoke(exe, text, mem, expected_rc=0):
    result = subprocess.run([exe, str(text), str(mem)], capture_output=True, timeout=60)
    assert result.returncode == expected_rc, (text, result.returncode, expected_rc, result.stderr)
    assert result.stdout == b'' and result.stderr == b'', (result.stdout, result.stderr)


def check(exe, text, mem, n, pairs):
    data, expected = fixture(n, pairs)
    mem.write_bytes(data)
    invoke(exe, text, mem)
    actual = mem.read_bytes()
    if actual != expected:
        offset = next((i for i, (x, y) in enumerate(zip(actual, expected)) if x != y), None)
        raise AssertionError(f'bit mismatch count={n} byte={offset} length={len(actual)}/{len(expected)}')
    # The whole-file comparison also enforces unchanged kernarg, inputs, past-count and guard bytes.


def suite(exe, text, flipped, out):
    out = pathlib.Path(out)
    mem = out / 'memory.bin'
    for n in COUNTS:
        check(exe, text, mem, n, PAIRS)
        print(f'PASS count={n} every-output-bit and past-count guards', flush=True)
    rng = random.Random(942)
    pairs = [(rng.getrandbits(32), rng.getrandbits(32)) for _ in range(4096)]
    check(exe, text, mem, len(pairs), pairs)
    print(f'PASS random binary32 pairs={len(pairs)} seed=942', flush=True)
    try:
        check(exe, flipped, mem, 200, PAIRS)
    except AssertionError as error:
        assert str(error).startswith('bit mismatch'), error
        print('CONTROL one-bit-vadd-operand CAUGHT bit mismatch', flush=True)
    else:
        raise AssertionError('one-bit mutant escaped')
    original = pathlib.Path(text).read_bytes()
    data, _ = fixture(0, PAIRS)
    words = list(struct.unpack('<' + 'I' * (len(original) // 4), original))
    branch_index = next(i for i, word in enumerate(words) if word >> 16 == 0xbf88)
    literal_index = next(i + 1 for i, word in enumerate(words)
                         if word >> 25 == 0x13 and word & 511 == 255)
    words[branch_index] = 0xbf880000 | ((literal_index - branch_index - 1) & 65535)
    bad_branch = struct.pack('<' + 'I' * len(words), *words)
    # Valid but unsupported s_nop. Full predecode must reject even after an endpgm.
    for label, code, rc in (
        ('outside-subset', struct.pack('<I', 0xbf800000) + original[4:], 66),
        ('unreachable-outside-subset', original + struct.pack('<I', 0xbf800000), 66),
        ('truncated-word', original[:-1], 65),
        ('branch-inside-literal', bad_branch, 65),
    ):
        bad = out / (label + '.text')
        bad.write_bytes(code)
        mem.write_bytes(data)
        invoke(exe, bad, mem, rc)
        assert mem.read_bytes() == data
        print(f'CONTROL {label} CAUGHT exit={rc} memory-unchanged', flush=True)
    # Invalid guest pointer cannot become a host access, or silently extend the memory file.
    data, _ = fixture(1, PAIRS)
    struct.pack_into('<Q', data, 0, 0xffffffffffffffff)
    mem.write_bytes(data)
    invoke(exe, text, mem, 67)
    assert mem.read_bytes() == data
    print('CONTROL memory-range CAUGHT exit=67 memory-unchanged', flush=True)


if __name__ == '__main__':
    if sys.argv[1] == 'extract':
        extract(*sys.argv[2:])
    elif sys.argv[1] == 'suite':
        suite(*sys.argv[2:])
    else:
        raise SystemExit('unknown oracle action')
