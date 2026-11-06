#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Source-derived C oracle differential; run inside the bwrap CPU envelope."""
from pathlib import Path
import hashlib, os, subprocess, sys
root=Path(__file__).resolve().parents[2];os.chdir(root)
out=Path('build/dagsim-resolve');out.mkdir(parents=True,exist_ok=True)
def run(cmd,**kw):
    print('+ '+' '.join(map(str,cmd)),flush=True)
    return subprocess.run(list(map(str,cmd)),check=True,**kw)
cc=os.environ.get('CC','cc')
flags=['-std=gnu11','-O2','-ffp-contract=off','-D_GNU_SOURCE','-Isrc','-Itools/dagsim']
exports=['ds_status_name','ds_prov_name','ds_day_from_date','ds_find_op','ds_find_res','ds_parse_text','ds_parse_file','ds_resolved_pack','ds_eval_c','ds_eval','ds_eval_interval','ds_counterfactual','ds_diff_models']
for stale in out.glob('*.gcda'):stale.unlink()
run([cc,*flags,'-O0','--coverage','-DCUDAPP_MIGRATION_PENDING_C_DAGSIM_RESOLVE=1',*[f'-D{x}=oracle_{x}' for x in exports],'-c','tools/dagsim/dagsim.c','-o',out/'oracle.o'])
run([cc,*flags,'--coverage','-c','tools/dagsim/dagsim.c','-o',out/'resolve.o'])
sources=['tests/dagsim/resolve_diff.c',out/'oracle.o',out/'resolve.o','src/runtime/rx_costmodel.c','src/sha256.c','src/cudapp_om_dag_eval.c','src/cudapp_om_dag_parse.c','src/cudapp_om_dag_fmt.c','src/cudapp_om_dag_res.c','src/cudapp_om_dag_diff.c']
sources.extend(['src/cudapp_om_core_sha.c',*sorted(Path('src').glob('cudapp_om_cm_*.c'))])
def link(src,dest):run([cc,*flags,*src,'-Wl,--wrap=rx_cm_predict','-Wl,--wrap=malloc','-lm','-lgcov','-o',dest])
link(sources,out/'diff');run([out/'diff'])
for name in ['oracle','resolve']:
    cov=run(['gcov','-b','-c','-t','-o',out/(name+'.gcno'),'tools/dagsim/dagsim.c'],capture_output=True)
    (out/(name+'.gcov')).write_bytes(cov.stdout);(out/(name+'-coverage.log')).write_bytes(cov.stderr)
missing=[cc,*flags,'tools/dagsim/dagsim_cli.c',out/'resolve.o',*sources[3:5],*sources[6:],'-lm','-lgcov','-o',out/'missing-scheduler']
missing.insert(-3, 'src/cudapp_om_dag_jstr.c')
neg=subprocess.run([str(x) for x in missing],capture_output=True)
symbols=run(['nm',out/'resolve.o'],capture_output=True).stdout
(out/'scheduler-boundary.log').write_bytes(('COMMAND '+' '.join(str(x) for x in missing)+'\n').encode()+neg.stdout+neg.stderr+f'\nRC {neg.returncode}\nDEFAULT OBJECT SYMBOLS\n'.encode()+symbols)
assert neg.returncode!=0 and b'omega_dag_eval' in neg.stderr,'missing Omega scheduler must fail linkage'
assert b' T ds_eval_c' not in symbols and b' U omega_dag_eval' in symbols,'default scheduler must have no C fallback'
print('scheduler boundary: PASS (missing Omega scheduler link refused; no C fallback symbol)',flush=True)
red=subprocess.run([str(out/'diff')],env=dict(os.environ,DS_RESOLVE_RED='1'),capture_output=True)
(out/'red.log').write_bytes(red.stdout+red.stderr)
if red.returncode!=1:raise SystemExit(f'resolve red control did not return 1: {red.returncode}')
print('resolve red control: caught (rc=1)',flush=True)
omc=os.environ.get('OMEGA_OMC',str(Path.home()/'work/cudastack/omega-mv/seed/out/omc'))
bridge=Path(os.environ.get('OMEGA_BRIDGE',str(Path.home()/'work/cudastack/omega-bridge')))
if hashlib.sha256(Path(omc).read_bytes()).hexdigest()!='2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f':raise SystemExit('pinned seeded compiler mismatch')
res=Path('om/tools/dag_res.om').read_text();diff=Path('om/tools/dag_diff.om').read_text()
mutants=[
 ('unknown-resource','res','if eq prov 0','if false'),
 ('stale-boundary','res','if gt dr_i get a 6 expiry','if ge dr_i get a 6 expiry'),
 ('pessimistic-time','res','v = dr_f m add p 4','v = dr_f m add p 3'),
 ('prediction-n','res','ret eq get a 27 0','ret false'),
 ('bandwidth-double','res','bitcast ulong mul dr_f m add add p 2 k 2.0','bitcast ulong mul dr_f m add add p 2 k 3.0'),
 ('eliminated-proof','diff','ret or not used ne get t add key 80 0','ret true'),
 ('deferred-consumer','diff','ret not used','ret true'),
 ('removed-weight','diff','w = add timev launchv','w = timev'),
 ('changed-membership','res','put m add 7600 i 1','put m add 7600 i 0')]
if '--no-mutants' not in sys.argv:
    for name,unit,old,new in mutants:
        text=res if unit=='res' else diff
        if text.count(old)!=1:raise SystemExit(f'mutant {name}: expected unique site, got {text.count(old)}')
        mr=res.replace(old,new) if unit=='res' else res
        md=diff.replace(old,new) if unit=='diff' else diff
        dest=out/('mutant-'+name);dest.mkdir(exist_ok=True)
        replacements={}
        for un,source in [('dag_res',mr),('dag_diff',mr+'\n'+md)]:
            p=dest/(un+'.om');p.write_text(source)
            emitted=run(['sh',bridge/'tools/bridge-seeded','c',p],env=dict(os.environ,OMEGA_SEEDED_OMC=omc),capture_output=True)
            c=dest/(un+'.c');c.write_bytes(emitted.stdout);replacements['src/cudapp_om_'+un+'.c']=c
        link([replacements.get(str(s),s) for s in sources],dest/'diff')
        r=subprocess.run([str(dest/'diff')],capture_output=True);(dest/'out.log').write_bytes(r.stdout+r.stderr)
        if r.returncode!=1:raise SystemExit(f'mutant {name}: not killed by assertions (rc={r.returncode})')
        print(f'resolve mutant {name}: killed (rc=1); {dest}/out.log',flush=True)
    print(f'resolve mutants: {len(mutants)} killed',flush=True)
print('dagsim resolve Omega differential: PASS',flush=True)
