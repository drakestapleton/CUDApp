#!/usr/bin/env python3
# External test/calculation oracle only; never a launcher dependency.
import hashlib,json,os,platform,shutil,struct,subprocess,sys,tempfile
from fractions import Fraction
from pathlib import Path
bundle=Path(sys.argv[1]).resolve(); gate=Path(sys.argv[2]).resolve()
assert not Path('/dev/kfd').exists(), 'CPU gate refuses a host with a KFD device'
qemu=shutil.which('qemu-x86_64'); assert qemu
cell=(bundle/'cell.py').read_text(); records=[]
with tempfile.TemporaryDirectory(prefix='launch1-cpu-') as tmp:
    prior=Path.cwd(); os.chdir(tmp)
    ns={'__name__':'__notebook_test__'}
    exec(compile(cell,'cell.py','exec'),ns)
    # Drive the exact notebook's x86 absent-device branch using real qemu execution.
    oldmachine=platform.machine; oldrun=subprocess.run
    binary=ns['BIN']
    def emulated(args,*a,**k):
        args=list(args)
        if str(args[0])==str(binary): args=[qemu,*args]
        return oldrun(args,*a,**k)
    try:
        platform.machine=lambda:'x86_64'
        subprocess.run=emulated
        ns['main']()
    finally:
        subprocess.run=oldrun; platform.machine=oldmachine
    report=Path('amdserve-launch1.txt').read_text()
    assert 'argument-header=64 PASS' in report and 'errno=2 exit=77 checked' in report
    assert report=='\n'.join(ns['ROWS'])+'\n' and len(report.splitlines())<60
    (gate/'notebook-absent-receipt.txt').write_text(report)
    records.append({'case':'actual notebook x86 via qemu, absent KFD and refusals','exit':0})
    for name,path in ns['ASSETS'].items():
        if not name.endswith('.hsaco'): continue
        for target,command in (('a64',[gate/'launch-a64']),('x86',[qemu,binary])):
            command=[*command,'inspect',path,Path(name).stem]
            p=subprocess.run(list(map(str,command)),capture_output=True)
            assert p.returncode==0,(name,target,p.returncode,p.stdout,p.stderr)
            records.append({'case':target+' inspect '+name,'exit':p.returncode,'stdout':p.stdout.decode()})
    for label,m,k in ns['SHAPES']:
        for kind in ('fp8','bf16'):
            paths,expected=ns['fixture'](kind,m,k)
            x=[Fraction((i*13)%31-15,16) for i in range(k)]
            for r in range(4):
                data,weights=ns['row'](kind,r,k)
                scale=(1.0,0.5,2.0,0.25)[r] if kind=='fp8' else 1.0
                exact=sum((Fraction(w)*v for w,v in zip(weights,x)),Fraction())*Fraction(scale)
                want=struct.pack('<f',float(exact))
                assert expected[4*r:4*r+4]==want,(kind,m,k,r)
            records.append({'case':label+' '+kind+' CPU rational scalar vs lane-FMA reference','m':m,'k':k,'expected_sha256':hashlib.sha256(expected).hexdigest(),'exit':0})
    # The real cell refuses a changed asset digest before any child is executed.
    digest=ns['BUNDLE']['assets']['omega-launch']['sha256']
    mutated=cell.replace(digest,'0'*64,1); assert mutated!=cell
    control=Path(tmp)/'digest-control'; control.mkdir(); os.chdir(control)
    try:
        exec(compile(mutated,'bad-cell.py','exec'),{'__name__':'__notebook_test__'})
    except RuntimeError as e:
        assert str(e)=='asset digest omega-launch'
    else: raise AssertionError('asset digest mutant survived')
    assert 'FAIL asset unpack: asset digest omega-launch' in Path('amdserve-launch1.txt').read_text()
    records.append({'case':'CONTROL corrupted binary digest refused before execution','exit':1,'expected':1})
    os.chdir(prior)
(gate/'notebook-check.json').write_text(json.dumps({'records':records,'device':'NOT_RUN','calculations':'fixture/reference in notebook.py.in; Fraction scalar sum then IEEE binary32 compared to fmaf lane reduction; quantities dimensionless'},indent=2)+'\n')
print('NOTEBOOK_CPU PASS',len(records),'rows; all device rows NOT_RUN')
