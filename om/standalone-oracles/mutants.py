# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# External verification, never a production dependency. Run in device-empty bwrap.
import hashlib, os, subprocess, tempfile
from pathlib import Path
root=Path(__file__).resolve().parents[2]
lane=root/'om/standalone-oracles'
source=(lane/'oracles.om').read_text()
pins=dict(s.split('=',1) for s in (root/'src/cudapp_om_standalone_oracles.ident').read_text().split() if '=' in s)
bridge=Path(os.environ['OMEGA_BRIDGE'])/'tools/bridge-seeded'
assert hashlib.sha256(Path(os.environ['OMEGA_OMC']).read_bytes()).hexdigest()==pins['omc']
assert hashlib.sha256(bridge.read_bytes()).hexdigest()==pins['frontend']
assert hashlib.sha256((bridge.parent.parent/'src/toc.om').read_bytes()).hexdigest()==pins['converter']
assert not list(Path('/dev').glob('nvidia*')), 'run inside the documented device-empty bwrap'
env=dict(os.environ,OMEGA_SEEDED_OMC=os.environ['OMEGA_OMC'])
mutants={
 'vadd_multiplier':('2654435761','2654435760'),
 'grid_round_down':('ret div add a 255 256','ret div a 256'),
 'mirror_threshold':('ge b 1024','ge b 1025'),
 'cache_admit_extra':('if lt a 2','if lt a 3'),
 'reading_boundary':('if eq op 15\nif le a 64','if eq op 15\nif lt a 64'),
 'budget_short':('ret 2147483648','ret 2147483647'),
 'accept_infinity':('if lt and a 2147483647 2139095040','if le and a 2147483647 2139095040'),
 'rng_multiplier':('1664525','1664526'),
 'norm_sparse_tail':('if eq a 5119','if eq a 5120'),
}
with tempfile.TemporaryDirectory(prefix='standalone-mutants-') as directory:
    tmp=Path(directory)
    for name,(old,new) in mutants.items():
        assert source.count(old)==1,(name,'site not unique')
        om=tmp/(name+'.om');om.write_text(source.replace(old,new))
        c=tmp/(name+'.c');exe=tmp/name
        with c.open('w') as output:
            subprocess.run(['sh',str(bridge),'c',str(om)],env=env,stdout=output,check=True)
        subprocess.run([os.environ.get('CC','cc'),'-O2','-ffp-contract=off',str(lane/'scalar-check.c'),str(lane/'oracle-c.c'),str(c),'-o',str(exe)],check=True)
        result=subprocess.run([str(exe)],capture_output=True,text=True)
        assert result.returncode==1 and 'scalar mismatch' in result.stderr,(name,'escaped or failed for the wrong reason',result)
        print('KILLED '+name+': '+result.stderr.strip(),flush=True)
print('PASS standalone Omega mutants: all nine compiled source mutants killed',flush=True)
