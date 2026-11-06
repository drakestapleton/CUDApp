#!/usr/bin/env python3
"""Run only under the required bwrap CPU envelope (see lane constraints)."""
from pathlib import Path
import hashlib, os, subprocess, sys
root=Path(__file__).resolve().parents[2]
os.chdir(root)
out=Path('build/dagsim-parse'); out.mkdir(parents=True,exist_ok=True)
for stale in out.glob('*.gcda'): stale.unlink()
def run(cmd, **kw):
    print('+ '+' '.join(map(str,cmd)),flush=True)
    return subprocess.run(list(map(str,cmd)),check=True,**kw)
cc=os.environ.get('CC','cc')
flags=['-std=gnu11','-O2','-ffp-contract=off','-D_GNU_SOURCE','-Isrc','-Itools/dagsim']
exports=['ds_status_name','ds_prov_name','ds_day_from_date','ds_find_op','ds_find_res','ds_parse_text','ds_parse_file','ds_resolved_pack','ds_eval_c','ds_eval','ds_eval_interval','ds_counterfactual','ds_diff_models']
run([cc,*flags,'--coverage','-DCUDAPP_MIGRATION_PENDING_C_DAGSIM_PARSE=1',*[f'-D{x}=oracle_{x}' for x in exports],'-c','tools/dagsim/dagsim.c','-o',out/'oracle.o'])
run([cc,*flags,'--coverage','-c','tools/dagsim/dagsim.c','-o',out/'parser.o'])
sources=['tests/dagsim/parse_diff.c',out/'oracle.o',out/'parser.o','src/runtime/rx_costmodel.c','src/sha256.c','src/cudapp_om_dag_eval.c','src/cudapp_om_dag_parse.c','src/cudapp_om_dag_fmt.c','src/cudapp_om_dag_res.c','src/cudapp_om_dag_diff.c']
# Match the integrated RX_CM_BINDING and DS_SHA_SRC providers, including route.
bindings=['src/cudapp_om_core_sha.c',*sorted(Path('src').glob('cudapp_om_cm_*.c'))]
sources.extend(bindings)
run([cc,*flags,*sources,'-lm','-lgcov','-o',out/'diff'])
run([sys.executable,'tests/dagsim/parse_partition.py',out/'corpus'])
files=sorted((out/'corpus').glob('*.oev'))
run([out/'diff',*files])
red=subprocess.run([str(out/'diff'),str(out/'corpus'/'0001-valid.oev')],env=dict(os.environ,DS_PARSE_RED='1'))
if red.returncode != 1: raise SystemExit('red control did not fail with rc=1')
print('parser red control: caught (rc=1)',flush=True)
run([cc,*flags,'tests/dagsim/format_diff.c',*sources[1:],'-lm','-lgcov','-o',out/'format-diff'])
run([out/'format-diff'])
cli_sources=['tools/dagsim/dagsim_cli.c','tools/dagsim/dagsim.c','src/runtime/rx_costmodel.c','src/sha256.c','src/cudapp_om_dag_eval.c','src/cudapp_om_dag_parse.c','src/cudapp_om_dag_fmt.c','src/cudapp_om_dag_res.c','src/cudapp_om_dag_diff.c']
cli_sources.extend(bindings)
cli_sources.append('src/cudapp_om_dag_jstr.c')
for name,defs in [('cli',[]),('cli-oracle',['-DCUDAPP_MIGRATION_PENDING_C_DAGSIM_PARSE=1'])]:
    run([cc,*flags,*defs,*cli_sources,'-lm','-o',out/name])
fixtures=sorted(Path('tests/dagsim/fixtures').glob('*.oev'))
cmds=[['eval',str(p),'--asof','2026-10-20'] for p in fixtures]
cmds += [['pareto',*map(str,fixtures),'--asof','2026-10-20']]
cmds += [['diff',str(p),str(p),'--asof','2026-10-20'] for p in fixtures]
for args in cmds:
    actual=subprocess.run([str(out/'cli'),*args],capture_output=True)
    oracle=subprocess.run([str(out/'cli-oracle'),*args],capture_output=True)
    if (actual.returncode,actual.stdout,actual.stderr)!=(oracle.returncode,oracle.stdout,oracle.stderr):
        print('CLI differential failed:',args,actual,oracle); raise SystemExit(1)
print(f'CLI differential: {len(cmds)} commands, 0 failures',flush=True)
omc=os.environ.get('OMEGA_OMC',str(Path.home()/'work/cudastack/omega-mv/seed/out/omc'))
bridge=Path(os.environ.get('OMEGA_BRIDGE',str(Path.home()/'work/cudastack/omega-bridge')))
if hashlib.sha256(Path(omc).read_bytes()).hexdigest()!='2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f':
    raise SystemExit('pinned seeded compiler mismatch')
source=Path('om/tools/dag_parse.om').read_text()
mutants=[
 ('resource-bound','if ge get m 11 32','if ge get m 11 31',1),
 ('op-bound','fn dp_op t view uchar 1048576 m edit ulong 16384 to ulong\nif ge get m 12 256','fn dp_op t view uchar 1048576 m edit ulong 16384 to ulong\nif ge get m 12 255',1),
 ('line-bound','if ge sub stop p 1024','if ge sub stop p 1023',1),
 ('sha-length','if ne get m 14011 40','if ne get m 14011 39',1),
 ('unknown-field','if dp_unused m','if false',1),
 ('finite-buffer','if ge and get m add b 2 9223372036854775807 9218868437227405312','if false',1)]
for name,old,new,count in mutants:
    if source.count(old)!=count: raise SystemExit(f'mutant {name}: expected {count} sites, found {source.count(old)}')
    mdir=out/('mutant-'+name); mdir.mkdir(exist_ok=True)
    path=mdir/'dag_parse.om'; path.write_text(source.replace(old,new))
    emitted=run(['sh',bridge/'tools/bridge-seeded','c',path],env=dict(os.environ,OMEGA_SEEDED_OMC=omc),capture_output=True)
    (mdir/'dag_parse.c').write_bytes(emitted.stdout)
    changed=[s if str(s)!='src/cudapp_om_dag_parse.c' else mdir/'dag_parse.c' for s in sources]
    run([cc,*flags,*changed,'-lm','-lgcov','-o',mdir/'diff'])
    result=subprocess.run([str(mdir/'diff'),*map(str,files)],capture_output=True)
    (mdir/'out.log').write_bytes(result.stdout+result.stderr)
    if result.returncode!=1: raise SystemExit(f'mutant {name}: not killed by a differential assertion (rc={result.returncode})')
    print(f'parser mutant {name}: killed (rc=1); {mdir}/out.log',flush=True)
print(f'parser mutants: {len(mutants)} killed',flush=True)
for name in ['oracle','parser']:
    cov=run(['gcov','-b','-c','-t','-o',out/(name+'.gcno'),'tools/dagsim/dagsim.c'],capture_output=True)
    (out/(name+'.gcov')).write_bytes(cov.stdout)
    (out/(name+'-coverage.log')).write_bytes(cov.stderr)
print('gcov: oracle and unpacker branch receipts written',flush=True)
