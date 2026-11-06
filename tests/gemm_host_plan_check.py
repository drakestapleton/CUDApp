#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""C-oracle proof runner; run inside the device-hidden wrapper documented in evidence."""
import os
import pathlib
import shutil
import subprocess
root=pathlib.Path(__file__).resolve().parents[1];os.chdir(root)
d=root/'build/gemm-host-plan';d.mkdir(parents=True,exist_ok=True)
log=(d/'check.log').open('w')
def run(cmd,*,env=None,reject=False):
    p=subprocess.run([str(x) for x in cmd],text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,env=env)
    log.write('$ '+' '.join(map(str,cmd))+'\nrc='+str(p.returncode)+'\n'+p.stdout+'\n');log.flush()
    if (p.returncode==0)==reject:
        print(p.stdout);raise SystemExit('FAIL command expectation: '+' '.join(map(str,cmd)))
    return p
for key in ('OMEGA_OMC','OMEGA_BRIDGE'):
    if not os.environ.get(key):raise SystemExit('NOT_RUN: missing '+key)
run(['sh','om/units.sh','--check']);run(['make','-j3','build/libomega_gpu.a'])
cc=[os.environ.get('CC','cc'),'-std=gnu11','-O0','-ffp-contract=off','-Isrc']
lib=['build/libomega_gpu.a','-lpthread','-lm'];base=d/'base'
run(cc+['--coverage','tests/gemm_host_plan_test.c',*lib,'-o',base])
for old in d.glob('base-*.gcda'):
    old.unlink()
p=run([base]);print(p.stdout.strip())
e=os.environ.copy();e['GEMM_POLICY_RED']='1'
p=run([base],env=e,reject=True)
if 'FAIL' not in p.stdout:raise SystemExit('FAIL red control missed comparator')
print('RED COMPARATOR: PASS (nonzero mismatch)')
missing=d/'missing.a';shutil.copyfile('build/libomega_gpu.a',missing)
run(['ar','d',missing,'cudapp_om_gemm_host.o','cudapp_om_gemm_value.o','cudapp_om_gemm_cfg.o','cudapp_om_gemm_plan.o','cudapp_om_gemm_span.o'])
p=run(cc+['-Itests','tests/gpu_gemm_test.c',missing,'-lpthread','-lm','-o',d/'missing'],reject=True)
if 'omega_gemm_' not in p.stdout:raise SystemExit('FAIL unrelated missing-unit error')
print('STRONG LINK NEGATIVE: PASS (Omega planner symbols undefined)')
source=pathlib.Path('om/gemm-any/host.om').read_text()
mutants=[
 ('tile-set','ret or eq v 1 or eq v 2 eq v 4','ret or eq v 1 or eq v 2 eq v 3'),
 ('dimension-edge','if gt get x add i 8 16384','if ge get x add i 8 16384'),
 ('rounding','ret mul div add v sub m 1 m m','ret mul div v m m'),
 ('pack-padding','ret mul mul add mul groups ks2 2 frag 512','ret mul mul add mul groups ks2 1 frag 512'),
 ('span-edge','if gt sp 4294967295','if gt sp 65535'),
 ('band-grid','put p 12 mul band gy','put p 12 mul band gx'),
 ('alignment','ret and add a 255 18446744073709551360','ret and a 18446744073709551360'),
 ('main-registers','ret and mul 4 add add mul 4 a mul 2 b mul a b 4294967295','ret and mul 4 add add mul 3 a mul 2 b mul a b 4294967295'),
]
wrappers=[('gemm_host',None),('gemm_value','value.om'),('gemm_cfg','config.om'),('gemm_plan','plan.om'),('gemm_span','span.om')]
for name,old,new in mutants:
    if source.count(old)!=1:raise SystemExit('FAIL nonunique mutant '+name)
    changed=source.replace(old,new);files=[]
    env=os.environ.copy();env['OMEGA_SEEDED_OMC']=env['OMEGA_OMC']
    for unit,wrapper in wrappers:
        om=d/(name+'-'+unit+'.om');om.write_text(changed+('\n'+pathlib.Path('om/gemm-any',wrapper).read_text() if wrapper else ''))
        p=run(['sh',pathlib.Path(env['OMEGA_BRIDGE'])/'tools/bridge-seeded','c',om],env=env)
        c=d/(name+'-'+unit+'.c');c.write_text(p.stdout);files.append(c)
    exe=d/name;run(cc+['tests/gemm_host_plan_test.c',*files,*lib,'-o',exe])
    p=run([exe],reject=True)
    if 'FAIL' not in p.stdout:raise SystemExit('FAIL mutant missed comparator '+name)
    print('MUTANT '+name+': KILLED '+p.stdout.splitlines()[0])
run(['gcov','-b','-c','-o',d,str(d/'base-gemm_host_plan_test.gcno')])
targets={'esz','kstep','pow124','type_ok','band_ok','main_regs','omega_gpu_gemm_cfg_ok','roundup','omega_gpu_gemm_plan','ld_span','omega_gpu_gemm_span','omega_gpu_gemm_drive_ld','packed_locked','up16'}
coverage=pathlib.Path('omega_gpu_gemm.c.gcov').read_text()
summary=[line for line in coverage.splitlines() if line.startswith('function ') and line.split()[1] in targets]
(d/'coverage.txt').write_text('\n'.join(summary)+'\n')
for path in pathlib.Path('.').glob('*.gcov'):
    shutil.move(str(path),d/path.name)
print('\n'.join(summary))
run(['make','-j3','build/gpu_gemm_test'])
for tier in ('--host-only','--sim'):
    p=run(['build/gpu_gemm_test',tier]);print('GEMM '+tier+': rc=0 '+p.stdout.splitlines()[-1])
p=run(['make','-j3','test-gpu-gemm-any'])
print('GEMM ANY: rc=0 '+'; '.join(line for line in p.stdout.splitlines() if 'PASS' in line))
print('GEMM HOST PLAN ACCEPTANCE PASS (8 mutants, red comparator, missing-unit link)')
