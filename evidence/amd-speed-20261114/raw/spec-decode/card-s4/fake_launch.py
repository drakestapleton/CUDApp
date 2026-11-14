#!/usr/bin/env python3
"""spec-decode S4 DRY rehearsal only (never in a card run): stands in for the launcher after the real launchers have
checked the same plan and arguments under qemu. Prints the launcher's output format with reference ids (published AMD
ids for lighthouse plans, the driver's golden ids for coding24 plans), one token per pass for OMPLAN02 (n=1 a=0).
Controls (plan directory ending -ctl) print the ids shifted by one. FAKE_MUTANT=1: OMPLAN02 runs print id 128 wrong.
  fake_launch.py PLAN ARGS... TIMEOUT_NS"""
import json, os, struct, sys
from pathlib import Path
plan = Path(sys.argv[1]); raw = plan.read_bytes(); magic = raw[:8]
ng, npm = struct.unpack_from('<Q', raw, 48)[0], struct.unpack_from('<Q', raw, 40)[0]
here = Path(__file__).resolve().parent
ids = json.loads((here / ('published-ids.json' if ng == 256 else 'golden24.json')).read_text())
if plan.parent.name.endswith('-ctl'): ids = ids[1:] + ids[:1]
if magic == b'OMPLAN02' and os.environ.get('FAKE_MUTANT') == '1': ids = ids[:128] + [ids[128] + 1] + ids[129:]
for s in range(npm + ng - 1):
    tok = ids[s - npm + 1] if s >= npm - 1 else 0
    print(f'OMEGA_STEP {s} tok={tok} us={26000 + s}' + (' n=1 a=0' if magic == b'OMPLAN02' else ''))
print('OMEGA_PLAN ids=[' + ','.join(map(str, ids)) + ']'); print('OMEGA_PLAN complete')
