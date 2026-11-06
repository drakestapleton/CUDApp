#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Mutation and strong-link controls for the TC host unit; run in the host sandbox."""
import os
from pathlib import Path
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]
OUT=Path(os.environ.get('OUT_DIR','build'))
BRIDGE=Path(os.environ['OMEGA_BRIDGE'])
ENV=dict(os.environ,OMEGA_SEEDED_OMC=os.environ['OMEGA_OMC'])
source=(ROOT/'om/codegen/tc_host.om').read_text()
numeric=(ROOT/'om/numeric/numeric.om').read_text()
mutants=[('k-floor','lt k 128','lt k 256'),('tc-k-ceiling','gt k 32768','gt k 16384'),('tc2-k-ceiling','gt k 16384','gt k 8192'),('output-bound','ge mul n mul rt 64 8388608','gt mul n mul rt 64 8388608'),('shared','mul get s 1 4096','mul get s 1 2048'),('weight','shl and get s 0 127 4','shl and get s 0 127 3'),('scale','e = add e 64','e = add e 63'),('permutation','shl tch_bf get s 2 16','shl tch_bf get s 1 16')]
def run(args,**kw):return subprocess.run(args,cwd=ROOT,env=ENV,text=True,**kw)
link=['cc','-std=gnu11','-O2','-ffp-contract=off','-Isrc','tests/codegen_tc_host_diff.c',str(OUT/'codegen_tc_oracle.o'),str(OUT/'codegen_tc2_oracle.o')]
with tempfile.TemporaryDirectory(prefix='tc-host-mutants-') as td:
 tmp=Path(td)
 for name,before,after in mutants:
  if source.count(before)!=1:raise SystemExit('ambiguous site '+name)
  om=tmp/(name+'.om'); c=tmp/(name+'.c');exe=tmp/name
  om.write_text(numeric+'\n'+source.replace(before,after))
  with c.open('w') as f:r=run(['sh',str(BRIDGE/'tools/bridge-seeded'),'c',str(om)],stdout=f)
  if r.returncode:raise SystemExit(name+' conversion failure, not a kill')
  r=run(link+[str(c),str(OUT/'libomega_gpu.a'),'-lpthread','-lm','-o',str(exe)])
  if r.returncode:raise SystemExit(name+' link failure, not a kill')
  r=run([str(exe)],capture_output=True)
  if r.returncode!=1 or 'TC-HOST-DIFF FAIL' not in r.stderr:raise SystemExit(name+' survived')
  print('TC-HOST MUTANT '+name+' KILLED',flush=True)
 # Hide the sole implementation by symbol renaming, not a weak fallback.
 missing=tmp/'missing.o'
 r=run(['cc','-O2','-Isrc','-Domega_tc_host=removed_omega_tc_host','-c','src/cudapp_om_tc_host.c','-o',str(missing)])
 if r.returncode:raise SystemExit('negative compile failed')
 archive=tmp/'missing.a'
 objects=[str(p) for p in OUT.glob('*.o') if p.name not in ('cudapp_om_tc_host.o','codegen_tc_oracle.o','codegen_tc2_oracle.o')]
 r=run(['ar','rcs',str(archive),*objects,str(missing)])
 if r.returncode:raise SystemExit('negative archive failed')
 r=run(link+[str(archive),'-lpthread','-lm','-o',str(tmp/'missing')],capture_output=True)
 if r.returncode==0 or 'omega_tc_host' not in r.stderr:raise SystemExit('strong-link negative failed')
 print('TC-HOST STRONG-LINK PASS')
print('TC-HOST MUTANTS PASS (8 killed)')
