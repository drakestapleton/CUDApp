#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Host only. Invoke under the mandatory device-hidden bwrap; never opens a GPU."""
from pathlib import Path
import os
import subprocess

root = Path(__file__).resolve().parents[1]
os.chdir(root)
out = root / 'build/fixture-verify'
out.mkdir(parents=True, exist_ok=True)
bridge = Path(os.environ.get('OMEGA_BRIDGE', str(Path.home()/'work/cudastack/omega-bridge')))
# OMEGA_SEED defaults to the bridge's own bootstrap compiler (tools/bootstrap.sh output), which is what the
# hosted job builds next to OMEGA_BRIDGE; it no longer falls back to a lane checkout under the home directory.
env = dict(os.environ, OMEGA_SEEDED_OMC=os.environ.get('OMEGA_OMC', str(Path.home()/'work/cudastack/omega-mv/seed/out/omc')), OMEGA_SEED=os.environ.get('OMEGA_SEED', str(bridge/'build/bootstrap/omc')))
cc = ['gcc','-std=gnu11','-O2','-Wall','-Wextra','-Werror','-ffp-contract=off','-Isrc']
unit = 'src/cudapp_freest_fixture_verify.c'
source = 'tests/om_fixture_verify_test.c'
rows = []
def run(name, args, expect=0):
    p = subprocess.run([str(x) for x in args], env=env, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    (out/(name+'.log')).write_bytes(p.stdout)
    rows.append(f'{name}\t{p.returncode}\t'+ ' '.join(map(str,args)))
    (out/'commands.tsv').write_text('\n'.join(rows)+'\n')
    print(name, 'rc', p.returncode, flush=True)
    if expect is not None and p.returncode != expect: raise SystemExit(p.stdout.decode(errors='replace'))
    return p
missing = run('missing-link', cc+[source,'-o',out/'missing'], None)
assert missing.returncode and b'undefined reference' in missing.stdout
print('PASS strong entry missing-link negative',flush=True)
run('build-omega',cc+[source,unit,'-o',out/'omega'])
run('build-oracle',cc+[source,'-DCUDAPP_MIGRATION_PENDING_C_ENGINE_FIXTURE_REST=1','-o',out/'oracle'])
base = run('omega',[out/'omega']).stdout
assert base == run('oracle',[out/'oracle']).stdout
print('PASS scalar and wrapper source-partition differential',flush=True)
text=(root/'om/codegen/fixture_verify.om').read_text()
mutants = [
 ('word_equal','if ne and a 4294967295 and c 4294967295','if eq and a 4294967295 and c 4294967295'),
 ('error_index','add mul a 2 b','add mul a 3 b'),
 ('status_equal','ret fv_bool eq a 0','ret fv_bool ne a 0'),
 ('bf16_bit','4294967295 262144','4294967295 262145'),
 ('bfly_bit','or c 288230376151711744','or c 288230376151711745'),
 ('shift_count','x = shl x 16','x = shl x 15'),
 ('optional_mask','mask = 15','mask = 7')]
for name, old, new in mutants:
    assert text.count(old)==1,(name,text.count(old))
    path=out/(name+'.om'); path.write_text(text.replace(old,new))
    p=run('generate-'+name,['sh',bridge/'tools/bridge-freest','c',path])
    gen=out/(name+'.c'); gen.write_bytes(p.stdout)
    run('build-'+name,cc+[source,gen,'-o',out/name])
    observed=run('run-'+name,[out/name],None)
    assert observed.returncode or observed.stdout != base, name+' SURVIVED'
    print('KILLED',name,flush=True)
# Every original fixture entrypoint, with only the predicate implementation switched.
bindings=sorted(root.glob('src/cudapp_om_*.c'))+sorted(root.glob('src/cudapp_freest_*.c'))
common=['tests/om_fixture_verify_calls.c','src/omega_blackwell_codegen.c','src/omega_blackwell_matmul.c','src/omega_gpu_om_ir.c','src/sha256.c']+bindings
run('build-calls',cc+common+['-o',out/'calls'])
run('build-calls-oracle',cc+['-DCUDAPP_MIGRATION_PENDING_C_ENGINE_FIXTURE_REST=1']+common+['-o',out/'calls-oracle'])
assert run('calls',[out/'calls']).stdout == run('calls-oracle',[out/'calls-oracle']).stdout
# Corrupt golden data in a private test copy: the actual exported verifier must reject it.
cg=(root/'src/omega_blackwell_codegen.c').read_text()
assert cg.count('0x00027308U')==1
red=out/'red-codegen.c'; red.write_text(cg.replace('0x00027308U','0x00027309U'))
red_common=[red if x=='src/omega_blackwell_codegen.c' else x for x in common]
run('build-red-calls',cc+red_common+['-o',out/'red-calls'])
p=run('red-calls',[out/'red-calls'],None)
assert p.returncode==1 and b'FAIL codegen fixture invocation' in p.stdout
# Oracle coverage records source-derived branch partition reachability, not hardware qualification.
run('build-coverage',['gcc','-std=gnu11','-O0','--coverage','-Isrc','-DCUDAPP_MIGRATION_PENDING_C_ENGINE_FIXTURE_REST=1',source,'-o',out/'coverage'])
run('coverage',[out/'coverage'])
run('gcov',['gcov','-b','-c','-o',out,out/'coverage-om_fixture_verify_test.gcno'])
print('PASS seven exported fixtures; red golden-data control; mutants 7/7 KILLED; HW NOT_RUN',flush=True)
