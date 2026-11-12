#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Test only: exercise signed SOPP range boundaries against the ISA definition.
No generated table is input to a production build. The production SGEMM backward
branch is separately compared to LLVM's complete .text bytes by run.sh.
"""
import pathlib
import struct
import subprocess
import sys

exe, source, output = sys.argv[1:]
out = pathlib.Path(output)
lines = pathlib.Path(source).read_text().splitlines()
header = '\n'.join(lines[:next(i for i, line in enumerate(lines) if line.startswith('I '))]) + '\n'
wait = 'I 4 0xc07f 0x0c\n'
end = 'I 4 0 0x01\n'


def text(data):
    shoff, = struct.unpack_from('<Q', data, 40)
    entsize, count = struct.unpack_from('<HH', data, 58)
    sections = [struct.unpack_from('<IIQQQQIIQQ', data, shoff + i * entsize) for i in range(count)]
    code = [data[row[4]:row[4] + row[5]] for row in sections if row[1] == 1 and row[2] & 4]
    assert len(code) == 1
    return code[0]


for label, body, expected, index, imm in (
    ('forward-max', 'I 4 @99 0x05\n' + wait * 32767 + 'T 99\n' + end, 0, 0, 32767),
    ('backward-min', 'T 99\n' + wait * 32767 + 'I 4 @99 0x05\n' + end, 0, 32767, 32768),
    ('forward-overflow', 'I 4 @99 0x05\n' + wait * 32768 + 'T 99\n' + end, 4, 0, 0),
    ('backward-overflow', 'T 99\n' + wait * 32768 + 'I 4 @99 0x05\n' + end, 4, 0, 0),
    ('undefined-label', 'I 4 @99 0x05\n' + end, 4, 0, 0),
    ('duplicate-label', 'T 99\nT 99\n' + end, 4, 0, 0),
):
    table = out / (label + '.isa')
    table.write_text(header + body)
    result = subprocess.run([exe, str(table)], capture_output=True, timeout=60)
    assert result.returncode == expected, (label, result.returncode, expected, result.stderr)
    if expected:
        assert result.stdout == b'', (label, 'bytes escaped before refusal')
        print(f'CONTROL {label} CAUGHT exit=4 before output', flush=True)
    else:
        assert result.stderr == b''
        code = text(result.stdout)
        word, = struct.unpack_from('<I', code, index * 4)
        assert word == 0xbf850000 | imm, (label, hex(word))
        print(f'PASS SOPP {label} exact signed-16-bit word={word:08x}', flush=True)
