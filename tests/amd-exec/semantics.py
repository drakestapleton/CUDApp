#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Independent manual-derived single-instruction expectations, CDNA3 PDF pages in each case.
Python is a test instrument only. No executor source is read to derive an expectation.
Integer arithmetic below is the recorded calculation of the manual's pseudocode.
"""
import pathlib
import struct
import subprocess
import sys

U32 = (1 << 32) - 1
U64 = (1 << 64) - 1
CASES = []

def s(r): return (0, r)
def v(r, lane=0): return (1, r * 64 + lane)
def t(r): return (2, r)
def sop(op, a=4, b=5, d=8): return [0x80000000 | op << 23 | d << 16 | b << 8 | a]
def sop1(op, a=4, d=8): return [0xbe800000 | d << 16 | op << 8 | a]
def vop(op, a=257, b=2, d=3): return [op << 25 | d << 17 | b << 9 | a]
def sm(op, d=8, offset=0): return [0xc0020000 | op << 18 | d << 6, offset]
def glob(op, offset=0, addr=4):
    return [0xdc008000 | op << 18 | (offset & 8191),
            addr | (3 << 8 if op == 28 else 0) | 127 << 16 | (3 << 24 if op == 20 else 0)]

def case(name, page, words, seed=None, result=None, *, ex=1, vc=U64, cc=1,
         nex=None, nvc=None, ncc=None, pc=None, running=1, mem=None, memory=None, refusal=0):
    expect = {t(14): ex if nex is None else nex, t(15): vc if nvc is None else nvc,
              t(16): cc if ncc is None else ncc, t(5): len(words)*4 if pc is None else pc}
    expect.update(seed or {})  # inputs and untouched registers must survive unless overwritten
    expect.update(result or {})
    CASES.append(dict(name=name, page=page, words=words, seed=seed or {}, expect=expect,
                      ex=ex, vc=vc, cc=cc, running=running, mem=mem, memory=memory, refusal=refusal))

# Scalar arithmetic: full unsigned temporary, destination low32; SUB sets borrow, not carry/no-borrow.
for a, b in [(0, 1), (1, 1), (2, 1), (U32, 1), (0, U32)]:
    case(f'sub-{a}-{b}', 97, sop(1), {s(4): a, s(5): b}, {s(8): (a-b)&U32}, ncc=int(b>a))
for op, label, carry_in in [(0, 'add', 0), (4, 'addc', 1)]:
    for a, b in [(0, 0), (U32, 0), (U32, 1), (U32, U32)]:
        total = a+b+carry_in
        case(f'{label}-{a}-{b}', 97 if op == 0 else 98, sop(op), {s(4): a, s(5): b},
             {s(8): total&U32}, cc=carry_in, ncc=int(total>U32))
for width, op in [(32, 28), (64, 29)]:
    for a, b in [(0, 1), (1, width), (0x80000001, width+1), (U32, width-1)]:
        value = a | (0x12345678 << 32 if width == 64 else 0)
        shifted = (value << (b & (width-1))) & ((1 << width)-1)
        seed = {s(4): a, s(6): b}
        result = {s(8): shifted & U32}
        if width == 64:
            seed[s(5)] = value >> 32
            result[s(9)] = shifted >> 32
        case(f'lshl{width}-{a}-{b}', 103, sop(op, b=6), seed, result, ncc=int(shifted != 0))
for op, label in [(36, 'mul-low'), (44, 'mul-high')]:
    for a, b in [(U32, U32), (0x80000000, 3), (0, 17)]:
        product = a*b
        result = product & U32 if op == 36 else product >> 32
        case(f'{label}-{a}-{b}', 104 if op == 36 else 107, sop(op), {s(4): a, s(5): b}, {s(8): result})
case('mov-preserves-scc', 115, sop1(0), {s(4): U32}, {s(8): U32})
case('mov-literal', 115, sop1(0, a=255)+[0x81234567], result={s(8): 0x81234567})
for mask in [0, 0x8000000000000001]:
    case(f'saveexec-{mask}', 125, sop1(32, a=106), result={s(8): 3, s(9): 0x80000000},
         ex=0x8000000000000003, vc=mask, nex=mask & 0x8000000000000003, ncc=int(mask != 0))
case('saveexec-source-destination-alias', 125, sop1(32, a=8), {s(8): 2, s(9): 0},
     {s(8): 3, s(9): 0}, ex=3, nex=2)
# Branch targets are signed DWORD displacements relative to next PC; padding is never executed.
for op, label in [(4, 'scc0'), (5, 'scc1')]:
    for cc in [0, 1]:
        take = (cc == (op-4))
        case(f'branch-{label}-{cc}', 139, [0xbf800001 | op << 16, 0xbf810000, 0xbf810000], cc=cc,
             pc=8 if take else 4)
case('branch-negative', 139, [0xbf84ffff], cc=0, pc=0)
for ex in [0, 1]:
    case(f'branch-execz-{ex}', 140, [0xbf880001, 0xbf810000, 0xbf810000], ex=ex, pc=4 if ex else 8)
for wait in [0xc07f, 0x0f70]:
    case(f'wait-{wait}', 141, [0xbf8c0000 | wait])
case('endpgm', 138, [0xbf810000], running=0)
# Scalar memory components individually ignore low two bits (p68), not just the final sum.
mem = struct.pack('<16I', *range(0x12340000, 0x12340010))
for op in [0, 1, 2]:
    case(f'smem-x{1<<op}', 146 if op < 2 else 147, sm(op, offset=4), {s(0): 4, s(1): 0},
         {s(8+i): 0x12340002+i for i in range(1 << op)}, mem=mem)
case('smem-component-low-bits', 68, sm(0, offset=3), {s(0): 3, s(1): 0}, {s(8): 0x12340000}, mem=mem)
for op, dest in [(1, 9), (2, 10)]:
    case(f'smem-bad-destination-{op}', 70, sm(op, d=dest), refusal=66)
case('smem-negative-undefined', 68, sm(0, offset=1 << 20), refusal=66)
# Vector integer operations: lane0/63 active, lane1 destination is untouched.
for op, label, a, b, answer in [
    (19, 'and', 0xa5, 0x5a, 0), (52, 'add-wrap', U32, 2, 1),
    (18, 'lshl-mask', 33, 0x80000001, 2), (16, 'lshr-mask', 33, 0x80000001, 0x40000000)]:
    case(label, {19:174, 52:183, 18:173, 16:173}[op], vop(op),
         {v(1): a, v(2): b, v(3,1): 0xdeadbeef, v(1,63): a, v(2,63): b},
         {v(3): answer, v(3,63): answer}, ex=(1 << 63)|1)
for op, label in [(25, 'add-co'), (28, 'addc-co')]:
    carry_in = 1 if op == 28 else 0
    case(label, 175 if op == 25 else 176, vop(op),
         {v(1): U32, v(2): 1, v(1,63): 0, v(2,63): 0, v(3,1): 0xdeadbeef},
         {v(3): carry_in, v(3,63): carry_in}, ex=(1 << 63)|1, nvc=1)
    case(label+'-zero-exec', 22, vop(op), ex=0, nvc=0)
# Input VCC must be captured before the instruction writes it, including scalar VCC sources.
case('add-co-vcc-source-snapshot', 22, vop(25, a=106),
     {v(2): U32, v(2,1): 0}, {v(3): 1, v(3,1): 2}, ex=3, vc=2, nvc=1)
cmpword = 0x7c000000 | 204 << 17 | 2 << 9
case('cmp-unsigned-and-exec', 251, [cmpword | 257],
     {v(1): U32, v(2): 0, v(1,63): 0, v(2,63): U32}, ex=(1 << 63)|1, nvc=1)
case('cmp-vcc-source-snapshot', 22, [cmpword | 106], {v(2): 0}, vc=2, nvc=1)
case('cmp-zero-exec', 22, [cmpword | 257], ex=0, nvc=0)
case('vmov-inline-negative', 186, [0x7e000200 | 3 << 17 | 193], result={v(3): U32})
case('vmov-literal', 186, [0x7e000200 | 3 << 17 | 255, 0x80000001], result={v(3): 0x80000001})
case('vmov-exec-hi', 186, [0x7e000200 | 3 << 17 | 127], result={v(3): 0x80000000, v(3,63): 0x80000000}, ex=(1 << 63)|1)
# FP bit patterns: exact denormal addition, halfway-to-even, signed zero, exact fused cancellation.
# The exhaustive Fraction oracle separately covers the numerical algorithms and special values.
for name, op, a, b, c, answer, page in [
    ('fadd-denorm', 1, 1, 1, 0, 2, 169),
    ('fadd-halfway-even', 1, 0x3f800000, 0x33800000, 0, 0x3f800000, 169),
    ('fadd-minus-zero', 1, 0x80000000, 0x80000000, 0, 0x80000000, 169),
    ('fmul-denorm', 5, 1, 0x3f800000, 0, 1, 170),
    ('fmul-negative-zero', 5, 0x80000000, 0x3f800000, 0, 0x80000000, 170),
    ('fmac-fused', 59, 0x3f800001, 0x3f7ffffe, 0xbf800000, 0xa8800000, 185)]:
    case(name, page, vop(op), {v(1): a, v(2): b, v(3): c}, {v(3): answer})
for offset, address in [(4, 4), (-4, 12)]:
    case(f'global-load-{offset}', 504, glob(20, offset), {v(4): address, v(5): 0},
         {v(3): 0x12340002}, mem=mem)
    changed = bytearray(mem)
    struct.pack_into('<I', changed, 8, 0xabcdef01)
    case(f'global-store-{offset}', 505, glob(28, offset), {v(4): address, v(5): 0, v(3): 0xabcdef01},
         mem=mem, memory=bytes(changed))
case('global-odd-address-pair', 21, glob(20, addr=5), refusal=66)
# FP8 additions are separately derived from the manual, not the executor implementation.
import runpy
fp8_decode = runpy.run_path(str(pathlib.Path(__file__).with_name('fp8gemv.py')))['decode']
cv = 0x7e000000 | 3 << 17 | 84 << 9 | 249
for byte in range(256):
    case(f'fp8-byte-{byte:02x}', '58,211-212', [cv, 0x601],
         {v(1): 0xabcdef00 | byte, v(3,1): 0xdeadbeef, v(1,63): byte},
         {v(3): fp8_decode(byte), v(3,63): fp8_decode(byte)}, ex=(1 << 63)|1)
case('fp8-zero-exec', 211, [cv, 0x601], {v(3): 0xdeadbeef}, ex=0)
for control in [0x10601, 0x20601, 0x30601, 0x1601, 0x800601]:
    case(f'fp8-unsupported-sdwa-{control:x}', 212, [cv, control], refusal=66)
case('fp8-non-sdwa', 211, [(cv & ~511) | 257], refusal=66)
for op in [645, 646]:
    for a, b in [(U32, U32), (0x80000000, 3), (0, 17)]:
        answer = (a*b >> (32 if op == 646 else 0)) & U32
        case(f'vmul-{op}-{a}-{b}', 361, [0xd0000003 | op << 16, 257 | 4 << 9],
             {v(1): a, s(4): b, v(3,1): 0xdeadbeef}, {v(3): answer})
case('vmul-vector-source', 361, [0xd2850003, 257 | 258 << 9],
     {v(1): U32, v(2): 2}, {v(3): 0xfffffffe})
case('vmul-modifier-refused', 361, [0xd2850103, 257 | 4 << 9], refusal=66)
case('vmul-src2-refused', 361, [0xd2850003, 257 | 4 << 9 | 1 << 18], refusal=66)
case('global-ubyte-last-byte', 503, [0xdc408000, 3 << 24 | 127 << 16 | 4],
     {v(4): 0, v(5): 0}, {v(3): 255}, mem=b'\xff')
case('global-ubyte-negative-offset', 503, [0xdc409fff, 3 << 24 | 127 << 16 | 4],
     {v(4): 1, v(5): 0}, {v(3): 128}, mem=b'\x80')
case('global-ubyte-range-refused', 503, [0xdc408000, 3 << 24 | 127 << 16 | 4],
     {v(4): 1, v(5): 0}, mem=b'\xff', refusal=67)

MUTATIONS = {
    'sub-no-borrow': ('put st 16 fmbool lt a b', 'put st 16 fmbool ge a b'),
    'inactive-vcc': ('if or eq cl 6 and eq cl 5 or eq get st 10 25 eq get st 10 28', 'if eq cl 6'),
    'vcc-source': ('get st 17', 'get st 15'),
    'smem-low-bits': ('p = or and get sr sb 0xfffffffc shl get sr add sb 1 32', 'p = or get sr sb shl get sr add sb 1 32'),
    'smem-offset-low-bits': ('put st 12 and x 0xfffffffc', 'put st 12 x'),
    'smem-dest-alignment': ('if ne and sd sub shl ulong 1 op 1 0', 'if false'),
    'smem-negative': ('if ne shr x 20 0', 'if ne shr x 21 0'),
    'vgpr-pair-alignment': ('if or ge va 255 ne and va 1 0', 'if ge va 255'),
}
MUTANT_CASE = {
    'sub-no-borrow': 'sub-0-1', 'inactive-vcc': 'add-co',
    'vcc-source': 'cmp-vcc-source-snapshot', 'smem-low-bits': 'smem-component-low-bits',
    'smem-offset-low-bits': 'smem-component-low-bits',
    'smem-dest-alignment': 'smem-bad-destination-1', 'smem-negative': 'smem-negative-undefined',
    'vgpr-pair-alignment': 'global-odd-address-pair',
}

def run_case(exe, out, c):
    code, memory, state = [out / ('single.'+suffix) for suffix in ['text', 'memory', 'state']]
    code.write_bytes(struct.pack('<'+'I'*len(c['words']), *c['words']))
    before = c['mem'] if c['mem'] is not None else bytes(64)
    memory.write_bytes(before)
    init = [c['ex'] & U32, c['ex'] >> 32, c['vc'] & U32, c['vc'] >> 32, c['cc'], len(c['seed'])]
    for (bank, index), value in c['seed'].items(): init += [bank, index, value]
    init += [len(c['expect'])]
    for bank, index in c['expect']: init += [bank, index]
    state.write_bytes(struct.pack('<'+'I'*len(init), *init))
    result = subprocess.run([exe, str(code), str(memory), str(state)], capture_output=True, timeout=15)
    if result.returncode != c['refusal']:
        return f'exit={result.returncode} expected={c["refusal"]}'
    if c['refusal']: return None
    expected = [c['running'], *c['expect'].values()]
    expected_bytes = struct.pack('<'+'Q'*len(expected), *expected)
    if result.stdout != expected_bytes:
        actual = struct.unpack('<'+'Q'*(len(result.stdout)//8), result.stdout) if len(result.stdout)%8 == 0 else result.stdout
        return f'state={actual} expected={expected}'
    if memory.read_bytes() != (c['memory'] if c['memory'] is not None else before): return 'memory differs'
    return None

def main():
    mode = sys.argv[1]
    if mode == 'mutate':
        source, out = pathlib.Path(sys.argv[2]), pathlib.Path(sys.argv[3])
        text = source.read_text()
        for name, (old, new) in MUTATIONS.items():
            if old not in text: raise AssertionError(f'mutation missing: {name}')
            (out / (name+'.om')).write_text(text.replace(old, new))
        print(' '.join(MUTATIONS))
        return
    exe, out = sys.argv[2], pathlib.Path(sys.argv[3])
    if mode == 'control':
        name = sys.argv[4]
        c = next(c for c in CASES if c['name'] == MUTANT_CASE[name])
        error = run_case(exe, out, c)
        if error is None: raise AssertionError(f'control not caught: {name}')
        print(f'CONTROL manual-{name} CAUGHT case={c["name"]} pdf-page={c["page"]} {error}')
        return
    for c in CASES:
        error = run_case(exe, out, c)
        if error: raise AssertionError(f'{c["name"]} PDF page {c["page"]}: {error}')
    print(f'PASS manual single-instruction semantics cases={len(CASES)} all implemented instructions flags/masks/addressing hardware=NOT_RUN')

if __name__ == '__main__': main()
