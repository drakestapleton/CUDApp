# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Host-only word-policy differential, link refusal and source mutants."""
import os
from pathlib import Path
import subprocess
import sys
root = Path(__file__).resolve().parents[1]
os.chdir(root)
out = Path(sys.argv[1] if len(sys.argv) > 1 else 'build/dis-words').resolve()
out.mkdir(parents=True, exist_ok=True)
base = ['gcc','-std=gnu11','-O0','--coverage','-Wall','-Wextra','-Werror','-Isrc',
        'tests/dis_words_test.c','tests/dis_words_oracle.c','src/omega_sm121_disasm.c','src/cudapp_om_sm121_dis.c']
def build(where, binding):
    where.mkdir(parents=True, exist_ok=True)
    subprocess.run(base + binding + ['-o',str(where/'check')],check=True)
def run(where, args=(), good=True):
    p = subprocess.run([str(where/'check'),*args],stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
    (where/('red.log' if args else 'run.log')).write_text(p.stdout)
    if (p.returncode == 0) != good:
        raise RuntimeError(p.stdout)
    return p.stdout.strip()
build(out,['src/cudapp_om_words.c'])
print(run(out))
run(out,['--plant'],False)
print('RED planted difference rejected')
p = subprocess.run(base+['-o',str(out/'missing')],stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
(out/'link-negative.log').write_text(p.stdout)
assert p.returncode != 0 and 'omega_dis_words' in p.stdout
print('STRONG LINK missing Omega refused')
cli = ['gcc','-O2','-Isrc','tools/sm121dis/sm121dis.c','src/omega_sm121_disasm.c','src/cudapp_om_sm121_dis.c']
subprocess.run(cli+['src/cudapp_om_words.c','-o',str(out/'pairs-om')],check=True)
subprocess.run(cli+['-DCUDAPP_MIGRATION_PENDING_C_DISASM_WORDS=1','-o',str(out/'pairs-c')],check=True)
fixture = bytes(range(64))
(out/'words.bin').write_bytes(fixture)
for option in ([],['--pairs']):
    a = subprocess.run([str(out/'pairs-om'),*option,str(out/'words.bin')],capture_output=True)
    c = subprocess.run([str(out/'pairs-c'),*option,str(out/'words.bin')],capture_output=True)
    assert (a.returncode,a.stdout,a.stderr) == (c.returncode,c.stdout,c.stderr)
print('CLI listing and --pairs byte assembly differential PASS')
src = (root/'om/disasm/words.om').read_text()
mutants = [
 ('byte-shift','mul i 8','mul i 7'),
 ('opcode','and v 4095 2381','and v 4095 2380'),
 ('mask','and v 4095 2381','and v 4094 2381'),
 ('end','ret add at 16','ret add at 15'),
 ('retain','ret prev','ret 0'),
]
for name,old,new in mutants:
    assert src.count(old) == 1
    d = out/name
    d.mkdir(exist_ok=True)
    (d/'words.om').write_text(src.replace(old,new))
    env = dict(os.environ,OMEGA_UNITS=str(d/'words.om'),OMEGA_UNITS_DEST=str(d))
    subprocess.run(['sh','om/units.sh'],env=env,check=True,stdout=subprocess.DEVNULL)
    build(d,[str(d/'cudapp_om_words.c')])
    run(d,good=False)
    print('KILLED '+name)
subprocess.run(['gcov','-b','-c','-o',str(out),str(out/'check-dis_words_oracle.gcno')],check=True,cwd=out)
print('DISASM WORDS DIFFERENTIAL RED LINK MUTANTS PASS')
