#!/usr/bin/env python3
"""Compile each source mutant with the pinned seeded bridge; never edit generated C."""
import os
from pathlib import Path
import subprocess
import tempfile
root = Path(__file__).resolve().parents[2]
source = (root / 'om/components/cm_route.om').read_text()
mutants = (
    ('block-divisor', 'put data 1 div nbytes 64', 'put data 1 div nbytes 63'),
    ('tail-divisor', 'put data 2 rem nbytes 64', 'put data 2 rem nbytes 63'),
    ('tail-offset', 'put data 3 mul div nbytes 64 64', 'put data 3 mul div nbytes 64 63'),
    ('layout-count', 'for i ulong 6', 'for i ulong 5'),
    ('layout-add', 'total = add total get data i', 'total = sub total get data i'),
    ('dispatch', 'if eq mode 0', 'if eq mode 2'),
)
with tempfile.TemporaryDirectory(prefix='cm-route-mutants-') as temp:
    temp = Path(temp)
    for name, old, new in mutants:
        if source.count(old) != 1:
            raise SystemExit('FAIL ambiguous mutation site: ' + name)
        om = temp / (name + '.om')
        c = temp / (name + '.c')
        exe = temp / name
        om.write_text(source.replace(old, new))
        with c.open('w') as output:
            env = dict(os.environ, OMEGA_SEEDED_OMC=os.environ['OMEGA_OMC'])
            subprocess.run(['sh', os.environ['OMEGA_BRIDGE'] + '/tools/bridge-seeded',
                            'c', str(om)], stdout=output, env=env, check=True)
        subprocess.run(['cc','-std=c11','-O2','-ffp-contract=off','-I'+str(root/'src'),
                        '-DCM_ROUTE_IMPL="'+str(c)+'"', str(root/'tests/runtime/cm_route_unit.c'),
                        '-o',str(exe)],check=True)
        p=subprocess.run([str(exe)],capture_output=True,text=True)
        if p.returncode == 0 or 'FAIL cm route' not in p.stderr:
            raise SystemExit('FAIL survived or invalid mutant: ' + name + ': ' + p.stderr)
        print('KILLED ' + name)
print('PASS cm route mutants (6 killed)')
