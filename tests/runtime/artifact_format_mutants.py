#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Compile three real .om mutants; compilation failure is not a killed mutant."""
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import artifact_format_gate as gate

root=gate.ROOT
bridge=Path(os.environ['OMEGA_BRIDGE'])
env=dict(os.environ,OMEGA_SEEDED_OMC=os.environ['OMEGA_OMC'])
for unit,old,new,parts in (
    ('artifact_format','if or eq ch 34 eq ch 92','if or eq ch 34 eq ch 93',()),
    ('artifact_admit','if not aa_digest b s 0','if aa_digest b s 0',('artifact_parse','artifact_policy','artifact_format')),
    ('artifact_elf','if ne ae_rd b 18 2 190','if ne ae_rd b 18 2 191',()),
):
    source=(root/'om/components'/f'{unit}.om').read_text()
    assert source.count(old)==1
    with tempfile.TemporaryDirectory(prefix='artifact-mutant-') as td:
        tmp=Path(td);om=tmp/f'{unit}.om'
        composed=''.join((root/'om/components'/f'{p}.om').read_text()+'\n' for p in parts)
        om.write_text(composed+source.replace(old,new)+'\n')
        generated=tmp/f'cudapp_om_{unit}.c'
        with generated.open('w') as out:
            subprocess.run(['sh',str(bridge/'tools/bridge-seeded'),'c',str(om)],env=env,stdout=out,check=True)
        lib=gate.build(tmp,[(generated.name,generated)])
        result=subprocess.run([sys.executable,str(root/'tests/runtime/artifact_format_gate.py'),'--library',str(lib)],capture_output=True,text=True)
        log=root/'build/artifact-format'/f'mutant-{unit}.log'
        log.parent.mkdir(parents=True,exist_ok=True);log.write_text(result.stdout+result.stderr)
        assert result.returncode==1 and 'AssertionError' in result.stderr,(unit,result.returncode,result.stderr)
        print(f'ARTIFACT-FORMAT mutant killed PASS {unit} (compiled; differential rejected)',flush=True)
print('ARTIFACT-FORMAT mutants PASS killed=3 compiled=3')
