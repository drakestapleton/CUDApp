#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Compile seeded source mutants; each must build, then fail the independent oracle."""
import os
from pathlib import Path
import subprocess
import sys
import tempfile
ROOT=Path(__file__).resolve().parents[1]
MUTANTS={
    'drop-grid-barrier': ('gridbar ins m\n', 'note mutant drops grid barrier\n'),
    'wrong-matvec': ('ef ins m FMUL r4 19 19 20 NONE 0', 'ef ins m FADD r4 19 19 20 NONE 0'),
    'descriptor-output-bound': ('if or not spanok a na size not spanok out rows size', 'if not spanok a na size'),
    'contracted-matvec': ('ef ins m FMUL r4 19 19 20 NONE 0', 'ef ins m 33 r4 19 19 20 18 0\nmov ins m 18 0'),
    'stale-generation': ('atomic ins m 27 27 3 26', 'atomic ins m 27 27 3 22'),
    'wrong-queue-stride': ('ef ins m IMADW r4 4 9 NONE 0 32', 'ef ins m IMADW r4 4 9 NONE 0 16'),
}
def main():
    bridge=os.environ.get('OMEGA_BRIDGE')
    omc=os.environ.get('OMEGA_OMC')
    if not bridge or not omc:
        print('NOT_RUN: OMEGA_BRIDGE and OMEGA_OMC are required');return 2
    env=dict(os.environ,OMEGA_SEEDED_OMC=omc)
    source=(ROOT/'om/resident/resident.om').read_text()
    check=(ROOT/'om/resident/check.om').read_text()
    archive=Path(sys.argv[1]).resolve()
    with tempfile.TemporaryDirectory(prefix='resident-mutants-') as tmp:
        t=Path(tmp)
        for name,(old,new) in MUTANTS.items():
            if source.count(old)!=1: raise RuntimeError(f'{name}: expected one mutation site')
            mutant=source.replace(old,new)
            outputs=[]
            for suffix,text in [('emit',mutant),('check',mutant+'\n'+check)]:
                om=t/f'{name}-{suffix}.om';c=om.with_suffix('.c');om.write_text(text)
                with c.open('w') as out:
                    subprocess.run(['sh',str(Path(bridge)/'tools/bridge-seeded'),'c',str(om)],stdout=out,env=env,check=True)
                outputs.append(str(c))
            exe=t/name
            subprocess.run([os.environ.get('CC','cc'),'-std=gnu11','-O2','-D_GNU_SOURCE','-ffp-contract=off','-I'+str(ROOT/'src'),'-I'+str(ROOT/'tests'),str(ROOT/'tests/gpu_resident_test.c'),str(ROOT/'src/omega_gpu_resident.c'),*outputs,str(archive),'-lpthread','-lm','-o',str(exe)],check=True)
            got=subprocess.run([str(exe),'--sim'],stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
            if got.returncode==0: raise RuntimeError(name+': SURVIVED')
            print(f'{name}: KILLED rc={got.returncode}',flush=True)
    print('GPU RESIDENT SOURCE MUTANTS PASS');return 0
if __name__=='__main__': sys.exit(main())
