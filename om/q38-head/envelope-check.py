# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Independent external numerical verification; not a runtime dependency.
import math
import random
import struct
import subprocess
import sys
from fractions import Fraction


def fbits(f):
    return struct.unpack('<I', struct.pack('<f', f))[0]


def value(b):
    return struct.unpack('<f', struct.pack('<I', b))[0]


def run(bits):
    assert len(bits) == 5120
    p = subprocess.run([sys.argv[1]], input=struct.pack('<5120I', *bits), capture_output=True)
    assert p.returncode == 0, (p.returncode, p.stderr)
    assert len(p.stdout) == 8
    return struct.unpack('<Q', p.stdout)[0]


rng = random.Random(710)
cases = [[0] * 5120, [fbits(1.0)] * 5120, [fbits(2.7109375)] * 5120]
cases += [[fbits(-1.0)] * 5120, [0] * 5119 + [fbits(-2.7109375)]]
for _ in range(25):
    cases.append([rng.randrange(0, 0x7d000000) | (rng.randrange(2) << 31) for _ in range(5120)])
for b in [1, 0x007fffff, 0x00800000, 0x7e000000, 0x7f7fffff]:
    a = [0] * 5120
    a[-1] = b
    cases.append(a)
for exponent in range(1, 255):
    for mantissa in [0, 1, 0x7ffffe, 0x7fffff, rng.randrange(0x800000)]:
        a = [0] * 5120
        a[rng.randrange(5120)] = (exponent << 23) | mantissa
        cases.append(a)
checks = 0
for a in cases:
    w = max(abs(value(b)) for b in a)
    r = run(a)
    if w < value(0x00800000):
        assert r == fbits(1.0), (w, r)
    else:
        exact = Fraction(w) * 72 * Fraction(1025, 1024)
        if exact > Fraction(value(0x7f7fffff)):
            assert r == 0
        else:
            assert 0 < r < 0x7f800000
            assert Fraction(value(r)) >= exact
            assert Fraction(value(r - 1)) < exact
            assert value(r) >= math.sqrt(5120) * w
    checks += 1
for b in [0x7f800000, 0xff800000, 0x7fc00000, 0xffc00001]:
    a = [fbits(1.0)] * 5120
    a[-1] = b
    assert run(a) == 0
    checks += 1
print(f'PASS: {checks} envelope cases, exact rational upward rounding, tail invalid-input refusal')
