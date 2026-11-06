#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Focused differential/red/link/mutant acceptance; temporary mutants never alter sources."""
import os
from pathlib import Path
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]
WRAP=['bwrap','--dev-bind','/','/','--dev','/dev','--proc','/proc','--','taskset','-c','10-14','nice','-n','10']
ENV=dict(os.environ,OMEGA_SEEDED_OMC=os.environ.get('OMEGA_OMC',str(Path.home()/'work/cudastack/omega-mv/seed/out/omc')))
BRIDGE=Path('/tmp/bridge-71/tools/bridge-seeded')
RT=Path('/tmp/integ/bridge-73-79e8c36/tools/bridge-seeded')
def run(cmd,negative=False):
 p=subprocess.run(cmd,cwd=ROOT,env=ENV,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
 if (p.returncode==0)==negative: raise RuntimeError(f'{cmd}: rc={p.returncode}\n{p.stdout}')
 return p
with tempfile.TemporaryDirectory(prefix='api-hostrest-') as tmp:
 t=Path(tmp)
 def build(family,unit,extra=()):
  exe=t/(family+'test')
  src=['cc','-std=c11','-ffp-contract=off','-Isrc',*extra]
  if family=='host':src+=['tests/api_hostrest_test.c',str(unit)]
  else:
   run(['cc','-std=c11','-ffp-contract=off','-Isrc','-DCUDAPP_MIGRATION_PENDING_C_SCAN_REFERENCE_REST','-Domega_gpu_prefix_sum_reference=scan_c_prefix','-Domega_gpu_reduce_reference=scan_c_reduce','-c','src/omega_gpu_scan_ref.c','-o',str(t/'oracle.o')])
   src+=['tests/scan_rest_test.c','src/omega_gpu_scan_ref.c','src/cudapp_om_av_scan.c',str(t/'oracle.o'),str(unit),'-lm']
  run(src+['-o',str(exe)])
  return exe
 for family,file,bridge,changes in [
  ('host','hostrest.om',BRIDGE,[('eq c 48','eq c 49'),('eq c 50','eq c 51'),('rem k 512','rem k 511'),('decl 65536','decl 65535'),('div uint 48 groups','div uint 47 groups'),('ge n 1000','ge n 999')]),
  ('scan','scancarry.om',RT,[('ulong 32','ulong 33'),('lt e n','lt e sub n 1'),('div e 32 1','div e 32 0'),('add get src','sub get src'),('add e 1','add e 2'),('lt len src','le len src')])]:
  source=(ROOT/'om/apiviews'/file).read_text()
  original=ROOT/'src'/('cudapp_om_ah_rest.c' if family=='host' else 'cudapp_om_ar_carry.c')
  exe=build(family,original)
  p=run(WRAP+[str(exe)]);print(p.stdout.strip(),flush=True)
  if family=='host':
   exe=build(family,original,['-DCUDAPP_API_HOSTREST_RED'])
   p=run(WRAP+[str(exe)],True);print(f'host red killed rc={p.returncode}',flush=True)
  # Strong symbol must fail to link without its generated unit.
  missing=t/'missing.c';missing.write_text('/* deliberately no Omega entry */\n')
  try:build(family,missing)
  except RuntimeError:print(f'{family} strong-link negative PASS',flush=True)
  else:raise RuntimeError('missing Omega entry linked')
  for i,(old,new) in enumerate(changes):
   assert source.count(old)==1,(old,source.count(old))
   mutant=t/(family+str(i)+'.om');mutant.write_text(source.replace(old,new))
   cmd=[str(bridge)]+(['--runtime-views'] if family=='scan' else [])+['c',str(mutant)]
   p=run(cmd);unit=t/(family+str(i)+'.c');unit.write_text(p.stdout)
   exe=build(family,unit)
   p=run(WRAP+[str(exe)],True)
   print(f'{family} mutant {i+1} killed rc={p.returncode}: {old} -> {new}',flush=True)
 print('API_HOSTREST_SCAN acceptance PASS',flush=True)
