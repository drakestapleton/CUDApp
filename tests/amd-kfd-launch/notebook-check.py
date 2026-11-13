#!/usr/bin/env python3
# External CPU measuring harness only; never a launcher dependency.
import hashlib,json,os,platform,shutil,subprocess,sys,tempfile
from pathlib import Path
bundle=Path(sys.argv[1]).resolve(); gate=Path(sys.argv[2]).resolve()
assert not Path('/dev/kfd').exists(), 'CPU gate refuses a host with a KFD device'
qemu=shutil.which('qemu-x86_64'); assert qemu
cell=(bundle/'cell.py').read_text(); records=[]
with tempfile.TemporaryDirectory(prefix='launch3-cpu-') as tmp:
    prior=Path.cwd(); os.chdir(tmp)
    ns={'__name__':'__notebook_test__'}
    exec(compile(cell,'cell.py','exec'),ns)
    oldmachine=platform.machine; oldpopen=subprocess.Popen
    def emulated(args,*a,**k):
        args=list(args)
        if str(args[0])==str(ns['BIN']): args=[qemu,*args]
        return oldpopen(args,*a,**k)
    try:
        platform.machine=lambda:'x86_64'
        subprocess.Popen=emulated
        ns['run']()
    finally:
        subprocess.Popen=oldpopen; platform.machine=oldmachine
    binary=ns['BIN']
    report=Path('amdserve-launch3.txt').read_text()
    assert 'argument-header=64 PASS' in report and 'errno=2 exit=77 checked' in report
    assert report=='\n'.join(ns['ROWS'])+'\n'
    assert report.splitlines()[0].startswith('CONTROLS ') and report.splitlines()[-1]=='RESULT NOT_RUN'
    assert 'OMEGA step open_kfd fail errno=2 phase=after detail=0\n' in report
    (gate/'notebook-absent-receipt.txt').write_text(report)
    records.append({'case':'actual notebook x86 via qemu, absent KFD and refusals','exit':0})
    for name,path in ns['ASSETS'].items():
        if not name.endswith('.hsaco'): continue
        for target,command in (('a64',[gate/'launch-a64']),('x86',[qemu,binary])):
            command=[*command,'inspect',path,Path(name).stem]
            p=subprocess.run(list(map(str,command)),capture_output=True)
            assert p.returncode==0,(name,target,p.returncode,p.stdout,p.stderr)
            records.append({'case':target+' inspect '+name,'exit':p.returncode,'stdout':p.stdout.decode()})
    # The child holds itself alive until all scheduled read-only proc samples and
    # its first stdout line have reached disk. This rejects capture-after-exit.
    ns['BIN']=Path(sys.executable)
    ns['KFD_PROC_ROOT']=Path(tmp)/'proc-fixture'
    ns['SAMPLE_DELAYS']=(0.0,0.01,0.02)
    child=r'''import os,pathlib,sys,time
root=pathlib.Path(sys.argv[1])/str(os.getpid())
(root/'queues'/'7').mkdir(parents=True)
(root/'stats_37820').mkdir()
for path,value in [('queues/7/size','1024'),('queues/7/type','2'),('queues/7/gpuid','37820'),('stats_37820/cu_occupancy','0'),('vram_37820','4096')]:
    (root/path).write_text(value)
print('OMEGA streaming-first',flush=True)
print('OMEGA step signal_wait ok errno=0 phase=before detail=1',flush=True)
deadline=time.monotonic()+5
while True:
    receipt=pathlib.Path('amdserve-launch3.txt').read_text()
    if 'sample=2 pid='+str(os.getpid()) in receipt and 'OMEGA streaming-first\n' in receipt:
        break
    if time.monotonic()>deadline: sys.exit(91)
    time.sleep(0.01)
for path,value in [('queues/7/size','1024'),('queues/7/type','2'),('queues/7/gpuid','37820'),('stats_37820/cu_occupancy','0'),('vram_37820','4096')]:
    assert (root/path).read_text()==value
print('OMEGA streaming-last',flush=True)
sys.stderr.write('diagnostic-tail')
sys.exit(75)
'''
    p,_=ns['invoke'](['-c',child,ns['KFD_PROC_ROOT']],'streaming-proc-control',expected=None,sample=True)
    assert p.returncode==75 and p.stdout.endswith(b'OMEGA streaming-last\n')
    assert p.stderr==b'diagnostic-tail'
    report=Path('amdserve-launch3.txt').read_text()
    assert 'OMEGA stderr diagnostic-tail\n' in report and 'OMEGA proc samples=3;' in report
    for sample in range(3):
        lines=[s for s in report.splitlines() if s.startswith('OMEGA proc sample='+str(sample)+' ')]
        for field,value in [('size','1024'),('type','2'),('gpuid','37820'),('cu_occupancy','0'),('vram_37820','4096')]:
            assert any('/'+field+' value='+json.dumps(value) in s for s in lines),(sample,field,lines)
    ns['sample_kfd'](0,3)
    assert 'missing/unreadable errno=2' in Path('amdserve-launch3.txt').read_text()
    (gate/'notebook-proc-receipt.txt').write_text(Path('amdserve-launch3.txt').read_text())
    records.append({'case':'real child observes streaming and three proc samples while waiting; fields unchanged; nonzero exit retained; missing files explicit','exit':p.returncode,'expected':75})
    ns['BIN']=binary
    original_main=ns['main']
    def failure(): raise RuntimeError('CONTROL forced execution failure')
    ns['main']=failure
    try: ns['run']()
    except RuntimeError: pass
    else: raise AssertionError('forced failure survived')
    ns['main']=original_main
    report=Path('amdserve-launch3.txt').read_text()
    assert report.splitlines()[0].startswith('CONTROLS ') and report.splitlines()[-1]=='RESULT FAIL'
    (gate/'notebook-failure-receipt.txt').write_text(report)
    records.append({'case':'CONTROL execution failure keeps RESULT last','exit':1,'expected':1})
    digest=ns['BUNDLE']['assets']['omega-launch']['sha256']
    mutated=cell.replace(digest,'0'*64,1); assert mutated!=cell
    control=Path(tmp)/'digest-control'; control.mkdir(); os.chdir(control)
    try:
        badns={'__name__':'__notebook_test__'}
        exec(compile(mutated,'bad-cell.py','exec'),badns)
        badns['run']()
    except RuntimeError as exc:
        assert str(exc)=='asset digest omega-launch'
    else: raise AssertionError('asset digest mutant survived')
    report=Path('amdserve-launch3.txt').read_text()
    assert 'FAIL asset unpack: asset digest omega-launch' in report
    assert report.splitlines()[0].startswith('CONTROLS ') and report.splitlines()[-1]=='RESULT FAIL'
    (gate/'notebook-digest-receipt.txt').write_text(report)
    records.append({'case':'CONTROL corrupted binary digest refused before execution','exit':1,'expected':1})
    os.chdir(prior)
(gate/'notebook-check.json').write_text(json.dumps({'records':records,'device':'NOT_RUN','HIP_comparison':'NOT_RUN CPU host; no GPU runtime is imported by the CPU gate'},indent=2)+'\n')
print('NOTEBOOK_CPU PASS',len(records),'rows; all device rows NOT_RUN')
