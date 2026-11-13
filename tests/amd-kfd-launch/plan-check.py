#!/usr/bin/env python3
# External CPU oracle for the native Omega plan executor.
import json,os,struct,subprocess,sys,tempfile,shutil
from pathlib import Path
root=Path(__file__).resolve().parents[2]
gate=Path(sys.argv[1]).resolve()
qemu=shutil.which('qemu-x86_64');assert qemu
cases=[]
def run(name,args,want):
 p=subprocess.run(list(map(str,args)),capture_output=True,timeout=90)
 assert p.returncode==want,(name,p.returncode,want,p.stdout[-3000:],p.stderr[-3000:])
 cases.append({'case':name,'exit':p.returncode,'expected':want})
 return p
with tempfile.TemporaryDirectory(prefix='plan-',dir='/tmp') as td:
 d=Path(td); nodes=d/'nodes'; node=nodes/'2';node.mkdir(parents=True)
 (node/'gpu_id').write_text('1\n')
 (node/'properties').write_text('gfx_target_version 90402\ndrm_render_minor 128\nsimd_count 32\nsimd_per_cu 4\nmax_waves_per_simd 8\narray_count 8\nsimd_arrays_per_engine 2\nnum_xcc 2\n')
 weight=d/'weights';weight.write_bytes(bytes(8)+struct.pack('<256H',*([0x3f80]*256))+bytes(8)+struct.pack('<256f',*([1.0]*256)))
 # Two independently parsed code objects, raw plan data, BF16 file widening,
 # zero remainder, 20 dependent dispatches per step, and three generated tokens.
 names=b'vaddvadd';data=struct.pack('<256f',*([1.0]*256))
 header=b'OMPLAN01'+struct.pack('<7Q',2,1,3,20,1,3,2)
 codes=struct.pack('<4Q',0,4,4,4)
 buffers=b''.join(struct.pack('<6Q',*b) for b in [(1024,3,0,0,1024,1),(1024,2,0,8,512,1),(1024,0,0,0,0,2)])
 prompt=struct.pack('<I',0)+bytes(4)
 entries=[]
 for i in range(20):
  entry=struct.pack('<8I',i%2,4,1,1,64,28,3,0)+bytes(24)+struct.pack('<I',256)
  entry+=b''.join(struct.pack('<2IQ',off,idx,0) for off,idx in [(0,0),(8,1),(16,2)])
  entries.append(entry+bytes((-len(entry))%8))
 prefix=header+codes+buffers+prompt+b''.join(entries)
 plan=prefix+names+data+struct.pack('<4Q',len(prefix),len(names),len(prefix)+len(names),len(data))
 pth=d/'valid.plan';pth.write_bytes(plan)
 argv=[pth,gate/'vadd.hsaco',gate/'vadd.hsaco',weight]
 for arch,pre in [('a64',[]),('x86',[qemu])]:
  prefix=[*pre,gate/('launch-'+arch),'plan']
  command=[*prefix,*argv]
  p=run(arch+' valid preflight absent KFD',command,77)
  assert b'OMEGA device=absent' in p.stdout
  for value in ('-1','not-a-number','18446744073709551616'):
   p=run(arch+' refuses malformed caller deadline '+value,[*command,value],2)
   assert b'OMEGA step open_kfd' not in p.stdout
  mutations={
   'bad-magic':(0,b'BADPLAN!'), 'zero-code':(8,struct.pack('<Q',0)),
   'count-overflow':(24,struct.pack('<Q',2**63)), 'zero-prompt':(40,bytes(8)),
   'bad-token':(56,struct.pack('<Q',3)), 'bad-name':(64,struct.pack('<Q',2**63)),
   'bad-kind':(96+8,struct.pack('<Q',4)), 'bad-flags':(96+40,struct.pack('<Q',3)),
   'bad-file-index':(96+48+16,struct.pack('<Q',1)),
   'bad-file-range':(96+48+24,struct.pack('<Q',2048)),
   'odd-widen':(96+48+32,struct.pack('<Q',511)),
   'token-not-GTT':(96+96+40,struct.pack('<Q',1)),
   'prompt-padding':(96+144+4,b'\x01'),
   'bad-code-index':(248,struct.pack('<I',2)),
   'bad-block':(248+16,struct.pack('<I',32)),
   'bad-kernarg-size':(248+20,struct.pack('<I',24)),
   'bad-reloc-alignment':(248+32+28,struct.pack('<I',4)),
   'duplicate-reloc':(248+32+28+16,bytes(4)),
   'bad-buffer-offset':(248+32+28+8,struct.pack('<Q',1024)),
   'nonzero-pointer':(248+32,struct.pack('<Q',1)),
   'dispatch-padding':(248+108,b'\x01'),
   'pool-overlap':(len(plan)-32,struct.pack('<Q',len(prefix)-1)),
  }
  too_large=bytearray(plan)
  for off in (96,144):struct.pack_into('<Q',too_large,off,2**63-4096)
  overflow=d/'arena-overflow.plan';overflow.write_bytes(too_large)
  p=run(arch+' refuses arena sum overflow',[*prefix,overflow,*argv[1:]],64)
  assert b'OMEGA step open_kfd' not in p.stdout
  for name,(off,value) in mutations.items():
   bad=d/(name+'.plan');bad.write_bytes(plan[:off]+value+plan[off+len(value):])
   p=run(arch+' refuses '+name,[*prefix,bad,*argv[1:]],64)
   assert b'OMEGA step open_kfd' not in p.stdout,name
 run('compile plan syscall oracle',['cc','-Wall','-Wextra','-Werror',root/'tests/amd-kfd-launch/trace.c','-o',gate/'trace'],0)
 p=run('native plan execution and ring wrap',['env','KFD_TEST_PLAN=1',gate/'trace',nodes,gate/'launch-a64','plan',*argv],0)
 assert b'packets=60 freed=all' in p.stderr,p.stderr[-2000:]
 assert b'OMEGA_PLAN ids=[1073741824,1073741824,1073741824]\n' in p.stdout,p.stdout[-2000:]
 # An accepted descriptor with nonzero LDS exercises plantemplate, not a handcrafted packet.
 ldsargv=[pth,gate/'vadd-lds.hsaco',gate/'vadd-lds.hsaco',weight]
 run('plan ELF nonzero LDS packet fields',['env','KFD_TEST_PLAN=1',gate/'trace',nodes,gate/'launch-a64','plan',*ldsargv],0)
 run('CONTROL plan swapped private-group',['env','KFD_TEST_PLAN=1',gate/'trace',nodes,gate/'plan-swapped-segments','plan',*ldsargv],1)
 (gate/'plan-simulation.stdout').write_bytes(p.stdout);(gate/'plan-simulation.stderr').write_bytes(p.stderr)
 # Raw file ranges share the same VRAM arena as the data-pool buffer.
 rawplan=bytearray(plan)
 struct.pack_into('<Q',rawplan,96+48+8,1)
 struct.pack_into('<Q',rawplan,96+48+24,528)
 struct.pack_into('<Q',rawplan,96+48+32,1024)
 rawpath=d/'raw.plan';rawpath.write_bytes(rawplan)
 p=run('raw file range in shared arena',['env','KFD_TEST_PLAN=1',gate/'trace',nodes,gate/'launch-a64','plan',rawpath,*argv[1:]],0)
 assert b'OMEGA_PLAN ids=[1073741824,1073741824,1073741824]\n' in p.stdout

 # The real model has 1504 buffers. Their count must not consume map handles.
 extra=1504-3
 hugehead=bytearray(header);struct.pack_into('<Q',hugehead,24,1504)
 hugeprefix=bytes(hugehead)+codes+buffers+struct.pack('<6Q',16,0,0,0,0,1)*extra+prompt+b''.join(entries)
 hugeplan=hugeprefix+names+data+struct.pack('<4Q',len(hugeprefix),len(names),len(hugeprefix)+len(names),len(data))
 hugepath=d/'many-buffers.plan';hugepath.write_bytes(hugeplan)
 p=run('1504 model buffers retain constant map count',['env','KFD_TEST_PLAN=1',gate/'trace',nodes,gate/'launch-a64','plan',hugepath,*argv[1:]],0)
 assert b'allocations=10 packets=60 freed=all' in p.stderr
 assert b'OMEGA_PLAN ids=[1073741824,1073741824,1073741824]\n' in p.stdout
 p=run('unlimited default permits slow completed step',['env','KFD_TEST_PLAN=1','KFD_TEST_EXPIRE=1',gate/'trace',nodes,gate/'launch-a64','plan',*argv],0)
 assert b'OMEGA alarm slow_dispatch_ns=' in p.stdout
 p=run('explicit zero timeout permits slow completed step',['env','KFD_TEST_PLAN=1','KFD_TEST_EXPIRE=1',gate/'trace',nodes,gate/'launch-a64','plan',*argv,'0'],0)
 assert b'OMEGA alarm slow_dispatch_ns=' in p.stdout
 run('caller deadline refuses slow completed step',['env','KFD_TEST_PLAN=1','KFD_TEST_EXPIRE=1',gate/'trace',nodes,gate/'launch-a64','plan',*argv,'10000000000'],1)
 p=run('missing completion refuses plan',['env','KFD_TEST_PLAN=1','KFD_TEST_NO_COMPLETE=1','KFD_TEST_EXPIRE=1',gate/'trace',nodes,gate/'launch-a64','plan',*argv,'10000000000'],1)
 assert b'child_exit=75' in p.stderr and b'OMEGA_PLAN complete' not in p.stdout
(gate/'plan-check.json').write_text(json.dumps({'cases':cases,'device':'NOT_RUN'},indent=2)+'\n')
print('PLAN_CPU PASS',len(cases),'cases; device NOT_RUN')
