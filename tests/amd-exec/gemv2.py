#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Test-only exact Fraction oracle. No kernel or executor code derives expectations.
Recorded arithmetic: chunk=256, lane owns 4 consecutive columns; ascending k per lane;
simultaneous XOR stages 32,16,8,4,2,1, each with IEEE nearest-even binary32 addition.
Large fixtures retain actual M/K and sample first/middle/last rows through row-probe.
"""
import pathlib
import runpy
import struct
import subprocess
import sys

HERE = pathlib.Path(__file__).parent
SG = runpy.run_path(str(HERE/'sgemm.py'))
FP = runpy.run_path(str(HERE/'fp8gemv.py'))
BF = runpy.run_path(str(HERE/'bf16gemv.py'))
SHAPES = tuple(sorted(set(FP['SHAPES']) | set(BF['SHAPES']) |
                      {(m, k) for m in (1, 2, 65) for k in (255, 256, 257, 1023, 1024, 1025, 2049)}))
LARGE = ((17408, 5120), (5120, 17408), (5120, 5120))
TREE = (32, 16, 8, 4, 2, 1)


def result(weights, xs, decoder, scale=None):
    partial = [0]*64
    for lane in range(64):
        for chunk in range(0, len(xs), 256):
            for j in range(4):
                k = chunk+4*lane+j
                if k < len(xs):
                    partial[lane] = SG['fma'](decoder(weights[k]), xs[k], partial[lane])
    for distance in TREE:
        before = partial
        partial = [SG['fma'](before[lane], 0x3f800000, before[lane ^ distance]) for lane in range(64)]
    answer = partial[0]
    if scale is not None:
        answer = SG['fma'](scale, answer, (scale ^ answer) & 0x80000000)
    return answer


def fixture(kind, m, k, special=False, rows=None, witness=False):
    fp = kind == 'fp8gemv2'
    size = 1 if fp else 2
    wp = 64 if fp else 66
    xp = (wp + m*k*size+19) & ~3
    sp = xp+4*k+16
    yp = sp+4*m+16 if fp else sp
    data = bytearray([0xa5])*(yp+4*m+64)
    if fp:
        struct.pack_into('<QQQQII', data, 0, wp, xp, sp, yp, m, k)
    else:
        struct.pack_into('<QQQII', data, 0, wp, xp, yp, m, k)
    rows = range(m) if rows is None else rows
    finite = (0x3f800000, 0xbf800000, 0x3f800001, 0x3eaaaaab, 1, 0x80000001, 0, 0x80000000)
    vals = SG['VALUES'] if special else finite
    xs = [vals[(i*3+1) % len(vals)] for i in range(k)]
    if witness:
        assert (m, k) == (1, 129)
        xs = [0]*k
        xs[0], xs[64], xs[128] = 0x4b800000, 0x3f800000, 0xcb800000
    for i, value in enumerate(xs):
        struct.pack_into('<I', data, xp+4*i, value)
    expected_values = {}
    for row in rows:
        if fp:
            weights = [(i*13+row*7+56) & 255 for i in range(k)]
            if not special:
                weights = [0x7f if w == 0x80 else w for w in weights]
            scale = (SG['VALUES'] if special else (0x3f800000, 0xbf000000, 0x40000000))[row % (len(SG['VALUES']) if special else 3)]
            if witness:
                weights, scale = [0x40]*k, 0x3f800000
            data[wp+row*k:wp+(row+1)*k] = bytes(weights)
            struct.pack_into('<I', data, sp+4*row, scale)
            decoder = FP['decode']
        else:
            values = BF['VALUES'] if special else (0x3f80, 0xbf80, 0x3f81, 0x3eab, 1, 0x8001, 0, 0x8000)
            weights = [values[(row*7+i*11) % len(values)] for i in range(k)]
            if witness:
                weights = [0x3f80]*k
            for i, w in enumerate(weights):
                struct.pack_into('<H', data, wp+2*(row*k+i), w)
            scale, decoder = None, BF['decode']
        expected_values[row] = result(weights, xs, decoder, scale)
    expected = bytearray(data)
    for row, bits in expected_values.items():
        struct.pack_into('<I', expected, yp+4*row, bits)
    return data, expected, expected_values


def check(exe, text, out, kind, m, k, special=False, rows=None, witness=False):
    memory = pathlib.Path(out)/'gemv2-memory.bin'
    data, expected, values = fixture(kind, m, k, special, rows, witness)
    memory.write_bytes(data)
    args = [str((m+1)*64), '1'] if rows is None else list(map(str, (*rows, m)))
    completed = subprocess.run([exe, str(text), str(memory), *args], capture_output=True, timeout=600)
    assert completed.returncode == 0 and not completed.stdout and not completed.stderr, completed
    actual = memory.read_bytes()
    if actual != expected:
        offset = next(i for i, (a, b) in enumerate(zip(actual, expected)) if a != b)
        raise AssertionError(f'{kind} bit mismatch M={m} K={k} byte={offset} got={actual[offset]:02x} expected={expected[offset]:02x}')
    return values


def mutate(source, out):
    text = pathlib.Path(source).read_text()
    stem = pathlib.Path(source).stem
    first = 'I 5 255 2 15 21\nL 128\nI 11 0 15 4 5\nI 4 0xc07f 12\nI 5 260 5 4 1\n'
    second = 'I 5 192 2 15 21\nI 11 0 15 4 5\nI 4 0xc07f 12\nI 5 260 5 4 1\n'
    changes = {
        'order': (first+second, second+first),
        'tail': ('I 2 16 129 17 1\nI 4 @3 5', 'I 2 16 129 17 1\nI 4 @3 4'),
        'stride': ('I 9 6 645 257 13', 'I 9 6 645 257 12'),
    }
    for name, (old, new) in changes.items():
        assert text.count(old) == 1, (name, text.count(old))
        (pathlib.Path(out)/(stem+'-'+name+'.isa')).write_text(text.replace(old, new))


def suite(exe, probe, out):
    out = pathlib.Path(out)
    rows_receipt = ['kernel\tM\tK\trow\tFP32_bits']
    for kind in ('fp8gemv2', 'bf16gemv2'):
        code = out/(kind+'.text')
        for special in (False, True):
            for m, k in SHAPES:
                check(exe, code, out, kind, m, k, special)
                print(f'PASS {kind} M={m} K={k} inputs={"special" if special else "finite"} every-output-bit all-memory-guards', flush=True)
        check(exe, code, out, kind, 1, 129, witness=True)
        for label, shape in [('order', (1, 129)), ('tail', (1, 3)), ('stride', (3, 257))]:
            try:
                check(exe, out/(kind+'-'+label+'.text'), out, kind, *shape, witness=label == 'order')
            except AssertionError as error:
                assert str(error).startswith(kind+' bit mismatch'), error
                print(f'CONTROL {kind}-{label} CAUGHT {error}', flush=True)
            else:
                raise AssertionError(f'escaped {kind}-{label}')
        for m, k in LARGE:
            rows = (0, m//2, m-1)
            values = check(probe, code, out, kind, m, k, rows=rows)
            rows_receipt += [f'{kind}\t{m}\t{k}\t{r}\t{b:08x}' for r, b in values.items()]
            print(f'PASS {kind} M={m} K={k} sampled-rows={",".join(map(str, rows))} actual-full-row-stride every-sampled-bit all-unsampled-outputs-untouched', flush=True)
    (out/'gemv2-sampled-rows.tsv').write_text('\n'.join(rows_receipt)+'\n')
    print(f'PASS gemv2 kernels=fp8gemv2,bf16gemv2 shapes={len(SHAPES)} cases-per-kernel={2*len(SHAPES)+1} sampled-shapes={len(LARGE)} sampled-rows-per-shape=3 hardware=NOT_RUN', flush=True)


if __name__ == '__main__':
    {'mutate': mutate, 'suite': suite}[sys.argv[1]](*sys.argv[2:])
