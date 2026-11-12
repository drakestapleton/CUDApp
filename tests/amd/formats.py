#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Test-only growable format registry checks; no producer implementation here."""
import pathlib
import subprocess
import sys

producer, source, expected, output = sys.argv[1:]
text = pathlib.Path(source).read_text()
out = pathlib.Path(output)
reference = pathlib.Path(expected).read_bytes()
# The SOPP branch-reference format remains ID4 by schema; all other IDs are names, not indexes.
rows = []
for line in text.splitlines():
    cells = line.split()
    if cells and cells[0] in ('F', 'B', 'D', 'I') and cells[1] != '4':
        cells[1] = str(int(cells[1])+1000000)
        line = ' '.join(cells)
    rows.append(line)
variants = {
    'sparse-format-ids': '\n'.join(rows)+'\n',
    'growing-format-count': text + ''.join(f'F {i} 1\nB {i} 0 0 32 0\n' for i in range(11, 35)),
    'undefined-format': text + 'I 1000002\n',
}
for name, body in variants.items():
    table = out/(name+'.isa')
    table.write_text(body)
    result = subprocess.run([producer, str(table)], capture_output=True, timeout=60)
    if name == 'undefined-format':
        assert result.returncode == 4 and not result.stdout, result
        print('CONTROL undefined-format CAUGHT exit=4 no object')
    else:
        assert result.returncode == 0 and not result.stderr and result.stdout == reference, (name, result)
        print(f'PASS {name} byte-identical FP8 object (no capacity ceiling)')
