#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
import os
from pathlib import Path
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]
BRIDGE=Path(os.environ['OMEGA_BRIDGE'])
ENV=dict(os.environ,OMEGA_SEEDED_OMC=os.environ['OMEGA_OMC'])
host=(ROOT/'om/codegen/qm_host.om').read_text()
selfsrc=(ROOT/'om/codegen/qm_self.om').read_text()
# Link exactly what test-qmma-probe-rest links: the make recipe passes $(QMMA_REST_SRC).
sources=os.environ.get('QMMA_REST_SRC','').split()
for unit in ('src/cudapp_om_qm_host.c','src/cudapp_om_qm_self.c'):
 if unit not in sources:raise SystemExit('FAIL QMMA_REST_SRC lacks '+unit+'; run make test-qmma-probe-rest-mutants')
mutants=[('mark','host','eq a 16 eq b','eq a 15 eq b'),('read','host','shl b 8','shl b 9'),('f2-word','host','ne e 75526399','ne e 75526398'),('qmma-word','host','65535 29306','65535 29307'),('hmma-word','host','4294967040 268288','4294967040 268289'),('counts','host','add op 65536','add op 65535'),('self-store','self','put x 6 64','put x 6 68'),('self-f2','self','put x 13 1\nput x 6 1178940467','put x 13 0\nput x 6 1178940467')]
def run(args,**kwargs):return subprocess.run(args,cwd=ROOT,env=ENV,text=True,**kwargs)
def link(src,exe):return run(['cc','-std=gnu11','-O2','-Isrc','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','tests/qmma_probe_rest_test.c',*src,'-lm','-o',str(exe)],capture_output=True)
with tempfile.TemporaryDirectory(prefix='qmma-rest-mutants-') as td:
 tmp=Path(td)
 for name,unit,before,after in mutants:
  text=host if unit=='host' else selfsrc
  if text.count(before)!=1:raise SystemExit('ambiguous mutant '+name)
  om=tmp/(name+'.om');c=tmp/(name+'.c');exe=tmp/name
  changed=text.replace(before,after)
  if unit=='self':changed=(ROOT/'om/codegen/cg_qmma.om').read_text()+'\n'+changed
  om.write_text(changed)
  with c.open('w') as f:r=run(['sh',str(BRIDGE/'tools/bridge-seeded'),'c',str(om)],stdout=f)
  if r.returncode:raise SystemExit(name+' converter failed, not a kill')
  replaced='src/cudapp_om_qm_'+('host' if unit=='host' else 'self')+'.c'
  r=link([str(c) if p==replaced else p for p in sources],exe)
  if r.returncode:raise SystemExit(name+' link failed, not a kill: '+r.stderr)
  r=run([str(exe)],capture_output=True)
  if r.returncode!=1 or 'QMMA-REST' not in r.stderr or 'FAIL' not in r.stderr:raise SystemExit(name+' survived: '+r.stdout+r.stderr)
  print('QMMA-REST MUTANT '+name+' KILLED',flush=True)
 for unit in ('host','self'):
  symbol='omega_qm_'+unit
  r=link([p for p in sources if p!='src/cudapp_om_qm_'+unit+'.c'],tmp/('missing-'+unit))
  if r.returncode==0 or symbol not in r.stderr:raise SystemExit('strong-link '+unit+' failed')
  print('QMMA-REST STRONG-LINK '+unit+' PASS',flush=True)
print('QMMA-REST MUTANTS PASS (8 killed)')
