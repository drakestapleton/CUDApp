#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Read-only oracle/mutation runner. Invoke inside the documented device-hidden wrapper."""
import os
import pathlib
import shutil
import subprocess
import sys

root = pathlib.Path(__file__).resolve().parents[1]
os.chdir(root)
d = root / 'build/gemm-ex-policy'
d.mkdir(parents=True, exist_ok=True)
log = (d / 'check.log').open('w')
def run(cmd, *, env=None, reject=False):
    p = subprocess.run([str(x) for x in cmd], text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, env=env)
    text = p.stdout
    log.write('$ ' + ' '.join(map(str,cmd)) + '\nrc=' + str(p.returncode) + '\n' + text + '\n');log.flush()
    if (p.returncode == 0) == reject:
        print(text);raise SystemExit('FAIL command expectation: ' + ' '.join(map(str,cmd)))
    return p
for key in ('OMEGA_OMC','OMEGA_BRIDGE','OMEGA_SEED'):
    if not os.environ.get(key): raise SystemExit('NOT_RUN: missing ' + key)
run(['sh','om/units.sh','--check'])
run(['make','-j3','build/libomega_gpu.a'])
cc = [os.environ.get('CC','cc'),'-std=gnu11','-O0','-ffp-contract=off','-Isrc']
lib = ['build/libomega_gpu.a','-lpthread','-lm']
base = d / 'base'
for old in d.glob('base-*.gcda'):
    old.unlink()
run(cc + ['--coverage','tests/gemm_ex_policy_test.c',*lib,'-o',base])
p=run([base]); print(p.stdout.strip())
run(cc + ['-DCUDAPP_MIGRATION_PENDING_C_GEMM_EX_HOST_POLICY=0','tests/gemm_ex_policy_test.c',*lib,'-o',d/'shell'])
p=run([d/'shell']);print('DEFAULT SHELL: '+p.stdout.strip())
e=os.environ.copy();e['GEMM_POLICY_RED']='1'
p=run([base],env=e,reject=True)
if 'FAIL' not in p.stdout: raise SystemExit('FAIL red control did not reach comparator')
print('RED COMPARATOR: PASS (nonzero mismatch)')
# Real production archive, not the oracle harness, must refuse to link without this unit.
missing=d/'missing.a';shutil.copyfile('build/libomega_gpu.a',missing)
run(['ar','d',missing,'cudapp_om_gemm_ex_policy.o'])
p=run(cc+['-Itests','tests/gpu_gemm_ex_test.c',missing,'-lpthread','-lm','-o',d/'missing'],reject=True)
if 'omega_gemm_ex_policy' not in p.stdout: raise SystemExit('FAIL unrelated missing-unit failure')
print('STRONG LINK NEGATIVE: PASS (omega_gemm_ex_policy undefined)')
source=pathlib.Path('om/gemmex/policy.om').read_text()
mutants=[
 ('minimum','if lt a b','if gt a b'),
 ('transpose-index','ret add mul d b c','ret add mul c b d'),
 ('row-stride','if eq op 2\nif ne a 0\nret b','if eq op 2\nif ne a 0\nret 1'),
 ('plane-pack','if and ne a 5 ne a 6','if and ne a 4 ne a 6'),
 ('window-quota','ret div 65536 w','ret div 65535 w'),
 ('contiguous','if or eq a 1 eq b mul c d','if or eq a 2 eq b mul c d'),
 ('row-offset','ret mul mul a b c','ret add mul a b c'),
 ('column-stride','if eq op 7\nif ne a 0','if eq op 7\nif eq a 0'),
]
for name,old,new in mutants:
    if source.count(old)!=1: raise SystemExit('FAIL nonunique mutant '+name)
    om=d/(name+'.om');om.write_text(source.replace(old,new))
    env=os.environ.copy();env['OMEGA_SEEDED_OMC']=env['OMEGA_OMC']
    p=run(['sh',pathlib.Path(env['OMEGA_BRIDGE'])/'tools/bridge-seeded','c',om],env=env)
    c=d/(name+'.c');c.write_text(p.stdout)
    exe=d/name
    run(cc+['tests/gemm_ex_policy_test.c',c,*lib,'-o',exe])
    p=run([exe],reject=True)
    if 'FAIL' not in p.stdout: raise SystemExit('FAIL mutant not killed by comparison: '+name)
    print('MUTANT '+name+': KILLED '+p.stdout.splitlines()[0])
run(['gcov','-b','-c','-o',d,str(d/'base-gemm_ex_policy_test.gcno')])
targets={'gx_min','gx_arow','gx_bcol','gx_brow','gx_pack','gx_conv'}
coverage=pathlib.Path('omega_gpu_gemm_ex.c.gcov').read_text()
summary=[line for line in coverage.splitlines() if line.startswith('function ') and line.split()[1] in targets]
(d/'coverage.txt').write_text('\n'.join(summary)+'\n')
for path in pathlib.Path('.').glob('*.gcov'):
    shutil.move(str(path),d/path.name)
print('\n'.join(summary))
for target in ('test-gpu-gemm-ex','test-gpu-gemm-ex-sim'):
    p=run(['make','-j3',target]);print(target+': rc=0 '+'; '.join(line for line in p.stdout.splitlines() if 'PASS' in line))
print('GEMM EX HOST POLICY ACCEPTANCE PASS (8 mutants, red comparator, missing-unit link)')
