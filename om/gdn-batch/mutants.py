# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# External numerical-free policy mutation oracle. No production dependency.
import hashlib,os,resource,subprocess,tempfile
from pathlib import Path
resource.setrlimit(resource.RLIMIT_CORE,(0,0))
root=Path(__file__).resolve().parents[2]
bridge=Path(os.environ['OMEGA_BRIDGE'])/'tools/bridge-seeded'
pins=dict(x.split('=',1) for x in (root/'src/cudapp_om_gdn_batch_plan.ident').read_text().split() if '=' in x)
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
assert sha(Path(os.environ['OMEGA_OMC']))==pins['omc']
assert sha(bridge)==pins['frontend'] and sha(bridge.parent.parent/'src/toc.om')==pins['converter']
env=dict(os.environ,OMEGA_SEEDED_OMC=os.environ['OMEGA_OMC'])
mutants={
 'large_rows':('plan','gt rows 65536','gt rows 65537'),
 'tiny_rows':('plan','lt rows 3','lt rows 2'),
 'wrong_mode':('plan','gt mode 1','gt mode 2'),
 'layer_bound':('plan','ge layer layers','gt layer layers'),
 'ignore_failed_operation':('step','ne ok 1','gt ok 1'),
 'ignore_missing_completion':('step','ne missing 18446744073709551615','gt missing 18446744073709551615'),
 'ignore_completed_count':('step','ne done 3','gt done 3'),
 'ignore_submitted_count':('step','ne launched 3','gt launched 3'),
 'skip_operation':('step','ret add phase 1','ret add phase 2'),
 'equal_stamps':('stamp','if ge get ts i get ts add i 1','if gt get ts i get ts add i 1'),
 'wrong_interval':('stamp','ret sub get ts add which 1 get ts which','ret sub get ts add which 1 get ts 0'),
}
with tempfile.TemporaryDirectory(prefix='gdn-batch-mutants-') as td:
 t=Path(td)
 for name,(unit,old,new) in mutants.items():
  source=(root/f'om/gdn-batch/{unit}.om').read_text();assert source.count(old)==1,(name,source.count(old))
  om=t/(name+'.om');om.write_text(source.replace(old,new));c=t/(name+'.c');so=t/(name+'.so')
  with c.open('w') as f:subprocess.run(['sh',str(bridge),'c',str(om)],env=env,stdout=f,check=True)
  rest=[str(root/f'src/cudapp_om_gdn_batch_{u}.c') for u in ['plan','step','stamp'] if u!=unit]
  subprocess.run([os.environ.get('CC','cc'),'-O2','-shared','-fPIC',str(c),*rest,'-o',str(so)],check=True)
  r=subprocess.run(['python3',str(root/'om/gdn-batch/check.py'),str(so)],capture_output=True,text=True)
  assert r.returncode!=0 and 'AssertionError' in r.stderr,(name,r.returncode,r.stderr)
  print(name+': caught',flush=True)
