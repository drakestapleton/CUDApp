#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
import os
from pathlib import Path
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1];OUT=Path(os.environ.get('OUT_DIR','build'))
BRIDGE=Path(os.environ['OMEGA_BRIDGE']);ENV=dict(os.environ,OMEGA_SEEDED_OMC=os.environ['OMEGA_OMC'])
source=(ROOT/'om/codegen/pf_admit.om').read_text()
mutants=[('mutant-limit','gt m 2','gt m 1'),('mc-ceiling','gt k 4096','gt k 2048'),('mc-width','lt e 4','lt e 8'),('rl-ceiling','gt k 16384','gt k 8192'),('rl-width','gt e 1024','gt e 128'),('tiling','ne rem t div e 32 0','ne rem t div e 16 0'),('shared','mul t 512','mul t 256')]
def run(args,**kw):return subprocess.run(args,cwd=ROOT,env=ENV,text=True,**kw)
link=['cc','-std=gnu11','-O2','-Isrc','tests/codegen_pfhost_admit_diff.c',str(OUT/'codegen_pfhost_oracle.o')]
with tempfile.TemporaryDirectory(prefix='pfhost-admit-mutants-') as td:
 tmp=Path(td)
 for name,before,after in mutants:
  if source.count(before)!=1:raise SystemExit('ambiguous site '+name)
  om=tmp/(name+'.om');c=tmp/(name+'.c');exe=tmp/name;om.write_text(source.replace(before,after))
  with c.open('w') as f:r=run(['sh',str(BRIDGE/'tools/bridge-seeded'),'c',str(om)],stdout=f)
  if r.returncode:raise SystemExit(name+' converter failed, not killed')
  r=run(link+[str(c),str(OUT/'libomega_gpu.a'),'-lpthread','-lm','-o',str(exe)])
  if r.returncode:raise SystemExit(name+' link failed, not killed')
  r=run([str(exe)],capture_output=True)
  if r.returncode!=1 or 'PFHOST-ADMIT-DIFF FAIL' not in r.stderr:raise SystemExit(name+' survived')
  print('PFHOST-ADMIT MUTANT '+name+' KILLED',flush=True)
 missing=tmp/'missing.o';archive=tmp/'missing.a'
 if run(['cc','-O2','-Isrc','-Domega_pf_admit=removed_omega_pf_admit','-c','src/cudapp_om_pf_admit.c','-o',str(missing)]).returncode:raise SystemExit('negative compile failed')
 objects=[str(p) for p in OUT.glob('*.o') if p.name not in ('cudapp_om_pf_admit.o','codegen_pfhost_oracle.o')]
 if run(['ar','rcs',str(archive),*objects,str(missing)]).returncode:raise SystemExit('negative archive failed')
 r=run(link+[str(archive),'-lpthread','-lm','-o',str(tmp/'missing')],capture_output=True)
 if r.returncode==0 or 'omega_pf_admit' not in r.stderr:raise SystemExit('strong-link negative failed')
 print('PFHOST-ADMIT STRONG-LINK PASS')
print('PFHOST-ADMIT MUTANTS PASS (7 killed)')
