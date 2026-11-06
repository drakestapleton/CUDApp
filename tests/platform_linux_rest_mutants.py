#!/usr/bin/env python3
"""Independent source mutants for residual platform policies; no device access."""
import os
from pathlib import Path
import subprocess
import tempfile
root = Path(__file__).resolve().parents[1]
source = (root / 'om/platform-linux/plat_rest.om').read_text()
mutants = (
    ('page-round', 'pages = and add get m 0 4095', 'pages = and add get m 0 0'),
    ('cursor-floor', 'if or lt cursor 65536', 'if or lt cursor 65535'),
    ('marker-slash', 'if eq get m 1 47', 'if eq get m 1 48'),
    ('marker-letter', 'ne get m pos 109', 'ne get m pos 110'),
    ('stale-generation', 'ne get m 3 get m 1', 'eq get m 3 get m 1'),
    ('time-scale', 'ret add mul get m 0 1000000000', 'ret add mul get m 0 1000000'),
    ('queue-bound', 'if ge add i 1 get m 0', 'if gt add i 1 get m 0'),
    ('sleep-remainder', 'put q 1 mul rem get m 0 1000', 'put q 1 mul div get m 0 1000'),
)
with tempfile.TemporaryDirectory(prefix='plat-rest-mutants-') as temp:
    temp=Path(temp)
    for name,old,new in mutants:
        if source.count(old)!=1: raise SystemExit('FAIL ambiguous mutation site: '+name)
        om=temp/(name+'.om'); c=temp/(name+'.c'); exe=temp/name
        om.write_text(source.replace(old,new))
        with c.open('w') as output:
            env=dict(os.environ,OMEGA_SEEDED_OMC=os.environ['OMEGA_OMC'])
            subprocess.run(['sh',os.environ['OMEGA_BRIDGE']+'/tools/bridge-seeded','c',str(om)],
                           stdout=output,env=env,check=True)
        subprocess.run(['cc','-std=c11','-O2','-ffp-contract=off','-I'+str(root/'src'),
                        '-DPLAT_REST_IMPL="'+str(c)+'"',str(root/'tests/platform_linux_rest.c'),
                        '-o',str(exe)],check=True)
        p=subprocess.run([str(exe)],capture_output=True,text=True)
        if p.returncode==0 or 'FAIL platform rest' not in p.stderr:
            raise SystemExit('FAIL survived or invalid mutant: '+name+': '+p.stderr)
        print('KILLED '+name)
print('PASS platform rest mutants (8 killed)')
