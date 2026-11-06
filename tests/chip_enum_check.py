# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
import os
from pathlib import Path
import subprocess

root=Path.cwd()
out=root/'build/chip-enum'
out.mkdir(parents=True,exist_ok=True)
cc=os.environ.get('CC','cc')
base=[cc,'-std=gnu11','-O0','--coverage','-Wall','-Wextra','-Werror','-Isrc',str(root/'tests/chip_enum_diff.c'),str(root/'tests/chip_enum_oracle.c'),str(root/'src/omega_gpu_chip.c'),str(root/'src/cudapp_om_chip_admit.c')]
def build(dest,binding,defs=()):
    subprocess.run([*base,*defs,*binding,'-o',str(dest)],check=True)
def run(cmd,name,good=True):
    p=subprocess.run([str(x) for x in cmd],capture_output=True)
    (out/(name+'.log')).write_bytes(p.stdout+p.stderr)
    assert (p.returncode==0)==good,(name,p.returncode,p.stderr)
    return p.stdout
binding=str(root/'src/cudapp_om_chip_enum.c')
build(out/'om',[binding]);build(out/'c',[binding],['-DCUDAPP_MIGRATION_PENDING_C_CHIP_ENUM_POLICY=1'])
print(run([out/'om'],'diff').decode().strip())
run([out/'om','--plant'],'red',False)
missing=subprocess.run([*base,'-o',str(out/'missing')],capture_output=True)
(out/'link-negative.log').write_bytes(missing.stdout+missing.stderr)
assert missing.returncode and b'omega_chip_enum' in missing.stderr
print('RED and strong-link missing-unit controls PASS')
fixture=out/'root'
devices=fixture/'sys/bus/pci/devices'
devices.mkdir(parents=True,exist_ok=True)
vendors=[0,1,0x10dd,0x10de,0x10df,0xffff,0x10000,0xffffffff,0xffffffffffffffff]
classes=[0,1,0x2ffff,0x30000,0x30001,0x3ffff,0x40000,0xffffffff,0x100030000,0xffffffffffffffff]
ids=[0,0x2e11,0x2e12,0x2e13,0xffff,0x10000,0xffffffff]
i=0
for v in vendors:
    for c in classes:
        for d in ids:
            p=devices/f'fixture-{i:04d}'
            p.mkdir(exist_ok=True)
            for name,value in [('vendor',v),('class',c),('device',d)]: (p/name).write_text(f'{value:x}\n')
            i+=1
for name,files in [('missing',{}),('bad-vendor',{'vendor':'not-hex'}),('bad-class',{'vendor':'10de','class':'junk'}),('bad-device',{'vendor':'10de','class':'30000','device':'bad!'})]:
    p=devices/name;p.mkdir(exist_ok=True)
    for key,value in files.items():(p/key).write_text(value+'\n')
# A leading dot is excluded before any sysfs reads.
p=devices/'.hidden';p.mkdir(exist_ok=True)
for key,value in [('vendor','10de'),('class','30000'),('device','2e12')]:(p/key).write_text(value+'\n')
assert run([out/'om','--scan',fixture],'scan-om')==run([out/'c','--scan',fixture],'scan-c')
assert run([out/'om','--scan',out/'absent'],'missing-sysfs-om')==run([out/'c','--scan',out/'absent'],'missing-sysfs-c')
print('FIXTURE scan differential PASS vendor/class/device edges, capacities -1/0/1/2/3, null output, missing sysfs')
text=(root/'om/admission/chip_enum.om').read_text()
mutants=[('row-vendor','ce_eq a c','ce_eq a d'),('row-device','ce_eq b d','ce_eq b c'),('vendor','ce_eq a 4318','ce_eq a 4319'),('class','shr a 16 3','shr a 15 3'),('mask','ret and a b','ret or a b'),('capacity','lt a b','le a b'),('dot','ce_eq a 46','ce_eq a 47')]
for name,old,new in mutants:
    assert text.count(old)==1
    here=out/name;here.mkdir(exist_ok=True)
    unit=here/'chip_enum.om';unit.write_text(text.replace(old,new))
    env=dict(os.environ,OMEGA_UNITS=str(unit),OMEGA_UNITS_DEST=str(here))
    p=subprocess.run(['sh','om/units.sh'],env=env,capture_output=True)
    (here/'compile.log').write_bytes(p.stdout+p.stderr)
    assert p.returncode==0,p.stderr
    build(here/'check',[str(here/'cudapp_om_chip_enum.c')])
    run([here/'check'],name,False)
    print('KILLED',name)
cov=subprocess.run(['gcov','-b','-c','-t',str(out/'om-chip_enum_oracle.gcno')],capture_output=True)
(out/'oracle.gcov').write_bytes(cov.stdout);(out/'coverage.log').write_bytes(cov.stderr)
assert cov.returncode==0,cov.stderr
print('CHIP ENUM DIFFERENTIAL RED LINK SEVEN MUTANTS PASS')
