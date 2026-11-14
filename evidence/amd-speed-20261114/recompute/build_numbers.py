#!/usr/bin/env python3
# Merge out/<topic>.json into ../numbers.json, plus the quoted constants (constants.json).
# The output depends only on the evidence and these scripts: no clock, no dependence on the previous file, and the file is
# rewritten only when its bytes would change. A source whose sha256 differs from the previous numbers.json is printed
# (STALE), not stored.
import json
from common import HERE, OUT

p = HERE.parent / 'numbers.json'
old = p.read_text() if p.exists() else None
prev = {}
if old:
    for n in json.loads(old)['numbers']:
        for s in n.get('sources', []):
            prev[(n['id'], s['path'])] = s.get('sha256')
nums = []
for t in ['baseline', 'launcher', 'kernels', 'fourbit', 'specdecode', 'combined', 'sessions']:
    nums += json.loads((OUT / f'{t}.json').read_text())
for c in json.loads((HERE / 'constants.json').read_text()):
    c = dict(c)
    c['recompute'] = 'quoted, not recomputed'
    nums.append(c)
changed = sorted({f'{n["id"]} {s["path"]}' for n in nums for s in n.get('sources', [])
                  if (n['id'], s['path']) in prev and prev[(n['id'], s['path'])] != s.get('sha256')})
doc = dict(note='Every number checked in REPORT.md, with its label, unit, source files and the script that recomputes it. '
                'Source paths are relative to the evidence root: raw/ in the publication folder (the handoff folder where the '
                'report was built). A source with "base": "report folder" is relative to the folder that holds REPORT.md. '
                'MEASURED: read from a raw card or CPU log. DERIVED: arithmetic on MEASURED values. MODELED: computed from '
                'stated assumptions. COUNT: integer bookkeeping. RECORD: state or hash. QUOTED: a figure taken from a named '
                'source file and not recomputed here (constants.json). Where that source is a lane\'s notes (amd-launch-gap/RCCA.md, '
                'a lane\'s RESULT.md, spec-decode/model/), the file is not in the publication folder.',
           numbers=nums)
new = json.dumps(doc, indent=1) + '\n'
if new != old:
    p.write_text(new)
print(f'NUMBERS {len(nums)} {"unchanged" if new == old else "written"}; sources changed since last build: {len(changed)}')
for c in changed:
    print('  STALE', c)
