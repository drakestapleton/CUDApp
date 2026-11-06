# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
import os
from pathlib import Path
import subprocess

root = Path.cwd()
out = root / 'build/dagsim-jstr'
out.mkdir(parents=True, exist_ok=True)
cc = os.environ.get('CC', 'cc')
base = [cc, '-std=gnu11', '-O0', '--coverage', '-ffunction-sections', '-fdata-sections', '-Isrc', '-Itools/dagsim', str(root/'tests/dagsim/jstr_diff.c')]
def build(dest, binding, defs=()):
    subprocess.run([*base, *defs, *binding, '-Wl,--gc-sections', '-lm', '-o', str(dest)], check=True)
def run(cmd, name, good=True):
    p = subprocess.run([str(v) for v in cmd], capture_output=True)
    (out/(name+'.log')).write_bytes(p.stdout+p.stderr)
    assert (p.returncode == 0) == good, (name, p.returncode, p.stderr)
    return p.stdout
binding = str(root/'src/cudapp_om_dag_jstr.c')
build(out/'om', [binding])
build(out/'c', [binding], ['-DCUDAPP_MIGRATION_PENDING_C_DAGSIM_JSON_ESCAPE=1'])
print(run([out/'om'], 'diff').decode().strip())
assert run([out/'om', '--stream'], 'stream-om') == run([out/'c', '--stream'], 'stream-c')
print('JSON STREAM DIFFERENTIAL PASS empty string and bytes 1..255')
run([out/'om', '--plant'], 'red', False)
missing = subprocess.run([*base, '-Wl,--gc-sections', '-lm', '-o', str(out/'missing')], capture_output=True)
(out/'link-negative.log').write_bytes(missing.stdout+missing.stderr)
assert missing.returncode and b'omega_dag_jstr' in missing.stderr
print('RED and strong-link missing-unit controls PASS')
text = (root/'om/tools/dag_jstr.om').read_text()
mutants = [('quote','eq c 34','eq c 35'), ('slash','eq c 92','eq c 93'), ('control','lt c 32','lt c 31'), ('hex','and uint c 15','and uint c 14'), ('length','ret 6','ret 5')]
for name, old, new in mutants:
    assert text.count(old) == 1
    here = out/name
    here.mkdir(exist_ok=True)
    unit = here/'dag_jstr.om'
    unit.write_text(text.replace(old,new))
    env = dict(os.environ, OMEGA_UNITS=str(unit), OMEGA_UNITS_DEST=str(here))
    p = subprocess.run(['sh','om/units.sh'], env=env, capture_output=True)
    (here/'compile.log').write_bytes(p.stdout+p.stderr)
    assert p.returncode == 0, p.stderr
    build(here/'check', [str(here/'cudapp_om_dag_jstr.c')])
    run([here/'check'], name, False)
    print('KILLED', name)
cov = subprocess.run(['gcov','-b','-c','-t',str(out/'c-jstr_diff.gcno')],capture_output=True)
(out/'oracle.gcov').write_bytes(cov.stdout)
(out/'coverage.log').write_bytes(cov.stderr)
assert cov.returncode == 0, cov.stderr
print('JSON DIFFERENTIAL RED LINK FIVE MUTANTS PASS')
