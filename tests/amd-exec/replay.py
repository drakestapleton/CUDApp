#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""External test instrument: exact replay parity, independent scalar controls, buffer bounds."""
import base64
import hashlib
import json
import pathlib
import runpy
import struct
import subprocess
import sys

HERE = pathlib.Path(__file__).parent
ROOT = HERE.parent.parent
SRC = ROOT/'backend/amd/gfx942'
PIN = '2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f'
NAMES = ('embed5120_step','attn256_prep_step','attn256_part_step','attn256_merge_step',
         'gdnconv4_step','gdnstate_step','step_advance','readfirst_probe')
BASELINES = ('embed5120','attn256_prep','attn256_part','attn256_merge','gdnconv4','gdnstate')
MUTANTS = {
 'embed5120_step-stale': ('embed5120_step','I 7 0 0 2 0 0 20 0 30 0 127 0 29','I 8 128 1 29'),
 'attn256_prep_step-row0': ('attn256_prep_step','I 8 22 1 45','I 8 128 1 45'),
 'attn256_part_step-length1': ('attn256_part_step','I 2 14 129 14 0','I 3 129 0 14'),
 'attn256_merge_step-length1': ('attn256_merge_step','I 2 14 129 14 0','I 3 129 0 14'),
 'gdnconv4_step-bank': ('gdnconv4_step','I 4 @901 5','I 4 @901 4'),
 'gdnstate_step-bank': ('gdnstate_step','I 4 @901 5','I 4 @901 4'),
 'step_advance-noinc': ('step_advance','I 5 129 8 8 52','I 8 264 1 8'),
 'readfirst_probe-zero': ('readfirst_probe','I 8 264 2 10','I 3 128 0 10'),
}

class Gate:
    def __init__(self, omc, out):
        self.omc, self.out = str(omc), out
        self.commands, self.results, self.access = [], [], []
        self.source = ''.join((SRC/'exec'/n).read_text() for n in ('fp.om','fma.om','main.om'))
        self.instrument = runpy.run_path(str(HERE/'token_access.py'))['instrument']
        self.extract = runpy.run_path(str(HERE/'oracle.py'))['extract']
        self.executors = {}
        self.arg_controls = set()
        self.spans = None
        self.argbytes = {n:int(next(x.split()[2] for x in (SRC/(n+'.isa')).read_text().splitlines()
                                  if x.startswith('K '))) for n in (*NAMES,*BASELINES)}
        paths = [SRC/'exec'/n for n in ('fp.om','fma.om','main.om')]
        paths += [SRC/(n+suffix) for n in NAMES for suffix in ('.isa','-gfx942.s')]
        paths += [SRC/(n+'.isa') for n in BASELINES]
        paths += [SRC/'hsaco.om',SRC/'rope.om']
        paths += [HERE/n for n in ('replay.py','semantics.py','semantics-probe.om','token_access.py',
                                  'oracle.py','gdn.py','gdnconv.py','token.py','attention.py','prep.py')]
        self.pins = {str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    def call(self, argv, data=None, rc=0):
        p = subprocess.run(list(map(str,argv)), input=data, capture_output=True, timeout=900)
        stdout_path = self.out/('command-'+str(len(self.commands))+'.stdout')
        stdout_path.write_bytes(p.stdout)
        self.commands.append(dict(argv=list(map(str,argv)),exit=p.returncode,
                                 stdout_path=str(stdout_path),stdout_sha256=hashlib.sha256(p.stdout).hexdigest(),stderr=p.stderr.decode(errors='replace')))
        (self.out/'commands.json').write_text(json.dumps(self.commands,indent=2)+'\n')
        assert p.returncode == rc, (argv,p.returncode,p.stderr.decode(errors='replace'))
        assert not p.stderr, (argv,p.stderr)
        return p.stdout
    def compile(self, name, source):
        (self.out/(name+'.om')).write_text(source)
        binary = self.call([self.omc],source.encode())
        path = self.out/name
        path.write_bytes(binary); path.chmod(0o700)
        return path
    def build(self):
        producer = self.compile('hsaco', (SRC/'hsaco.om').read_text())
        self.compile('exec',self.source)
        self.compile('rope',(SRC/'rope.om').read_text())
        sem = runpy.run_path(str(HERE/'semantics.py'))
        source = self.source[:self.source.index('fn main to int\n')] + (HERE/'semantics-probe.om').read_text()
        exe = self.compile('semantics',source)
        self.call([sys.executable,'-I',HERE/'semantics.py','suite',exe,self.out])
        old,new = sem['MUTATIONS']['readfirst-lane-zero']
        assert source.count(old)==1
        mutant = self.compile('readfirst-lane-zero',source.replace(old,new))
        self.call([sys.executable,'-I',HERE/'semantics.py','control',mutant,self.out,'readfirst-lane-zero'])
        for name in (*NAMES,*BASELINES):
            blob = self.call([producer,SRC/(name+'.isa')])
            path = self.out/(name+'.hsaco'); path.write_bytes(blob)
            self.extract(path,self.out/(name+'.text'))
            assert blob == self.call([producer,SRC/(name+'.isa')])
            if name in NAMES:
                oracle = self.out/(name+'.o')
                self.call(['llvm-mc-18','-triple=amdgcn-amd-amdhsa','-mcpu=gfx942','-filetype=obj',SRC/(name+'-gfx942.s'),'-o',oracle])
                self.extract(oracle,self.out/(name+'.llvm.text'))
                assert (self.out/(name+'.text')).read_bytes()==(self.out/(name+'.llvm.text')).read_bytes(), name+' encoding mismatch'
        for name,(base,old,new) in MUTANTS.items():
            source = (SRC/(base+'.isa')).read_text(); assert source.count(old)==1,(name,source.count(old))
            table = self.out/(name+'.isa'); table.write_text(source.replace(old,new))
            path = self.out/(name+'.hsaco'); path.write_bytes(self.call([producer,table]))
            self.extract(path,self.out/(name+'.text'))
        print('PASS build, independent LLVM code bytes, semantic oracle/control',flush=True)
    def layout(self, name, spans):
        self.layout_name, self.spans, self.executors = name, spans, {}
    def bounded(self, argbytes):
        spans=[(0,argbytes),*self.spans[1:]]
        if argbytes not in self.executors:
            self.executors[argbytes]=self.compile(self.layout_name+'-args'+str(argbytes),
                                                  self.instrument(self.source,spans))
        return self.executors[argbytes],spans
    def run(self, name, memory, gx, gy, rc=0):
        argbytes=self.argbytes[MUTANTS[name][0] if name in MUTANTS else name]
        exe,spans=self.bounded(argbytes)
        path = self.out/'replay.memory'
        if name in NAMES and name not in self.arg_controls and rc==0:
            short,_=self.bounded(argbytes-1)
            path.write_bytes(memory)
            self.call([short,self.out/(name+'.text'),path,gx*64,gy],rc=68)
            self.results.append(dict(case=name+' truncated kernargs',result='CAUGHT',exit=68))
            self.arg_controls.add(name)
            print('CAUGHT',name,'truncated kernargs',flush=True)
        path.write_bytes(memory)
        raw = self.call([exe,self.out/(name+'.text'),path,gx*64,gy],rc=rc)
        if rc: return None
        assert len(raw)==16*len(spans)
        hits=struct.unpack('<'+str(len(spans)*2)+'Q',raw)
        self.access.append(dict(kernel=name,buffers=[dict(start=p,bytes=n,
            max_read_byte=hits[2*i]-1 if hits[2*i] else None,
            max_write_byte=hits[2*i+1]-1 if hits[2*i+1] else None) for i,(p,n) in enumerate(spans)]))
        return bytearray(path.read_bytes())
    def equal(self, name, actual, expected, start):
        assert actual[start:]==expected[start:], name+' byte mismatch'
        self.results.append(dict(case=name,result='PASS',mismatched_bytes=0))
        print('PASS',name,flush=True)
    def caught(self, name, actual, expected, start):
        assert actual[start:]!=expected[start:], name+' ESCAPED'
        self.results.append(dict(case=name,result='CAUGHT'))
        print('CAUGHT',name,flush=True)


def append(memory, payload):
    memory.extend(bytes(64)); p=len(memory); memory.extend(payload); memory.extend(bytes(64)); return p


def advance(g,memory,pos,cap,variant='step_advance'):
    struct.pack_into('<QI',memory,0,pos,cap)
    return g.run(variant,memory,2,2)


def recurrence(g,base):
    ref = runpy.run_path(str(HERE/('gdn.py' if base=='gdnstate' else 'gdnconv.py')))
    fixture = ref['state_fixture'](16,'dense') if base=='gdnstate' else ref['fixture'](10240,'history')
    original = bytearray(base64.b64decode(fixture['memory']))
    ptr=[p for _,p in fixture['args']]
    sizes=([16384,24576,3145728,3145728,24576,192,192] if base=='gdnstate'
           else [40960,81920,81920,81920,40960])
    oldargs=8*len(ptr); position=append(original,bytes(4))
    spans=[(0,64),*zip(ptr,sizes),(position,4)]
    g.layout(base,spans)
    actual, expected = original[:],original[:]
    gx,gy=fixture['timing_grid']
    for step in range(3):
        args=ptr[:]
        if step&1: args[2],args[3]=args[3],args[2]
        struct.pack_into('<'+str(len(args))+'Q',expected,0,*args)
        baseline=g.run(base,expected,gx,gy)
        struct.pack_into('<'+str(len(ptr)+1)+'Q',actual,0,*ptr,position)
        observed=g.run(base+'_step',actual,gx,gy)
        if step==0:
            # The baseline's original interval/exact oracle also applies to the first step.
            check=runpy.run_path(str(HERE/'token.py'))['check']
            width=check(baseline[:len(base64.b64decode(fixture['memory']))],fixture)
            g.results.append(dict(case=base+' initial original oracle',result='PASS',
                                  interval_escapes=0,widest_interval=str(width)))
            wrong=g.run(base+'_step-bank',actual,gx,gy)
            g.caught(base+' wrong state bank',wrong,baseline,64)
        g.equal(base+f' consecutive step{step}',observed,baseline,64)
        expected=baseline; struct.pack_into('<I',expected,position,step+1)
        actual=advance(g,observed,position,3)
        g.equal(base+f' counter step{step}',actual,expected,64)
    # Shorten an actually accessed state buffer by one word: must refuse, not silently pad.
    spans[3]=(ptr[2],sizes[2]-4)
    g.layout(base+'-short',spans)
    struct.pack_into('<'+str(len(ptr)+1)+'Q',original,0,*ptr,position)
    g.run(base+'_step',original,gx,gy,rc=68)
    g.results.append(dict(case=base+' truncated old state',result='CAUGHT',exit=68))


def embedding(g):
    p,o,t=64,64+4*10240+64,64+4*10240+64+20480+64
    memory=bytearray(t+4+64)
    for row in range(4):
        struct.pack_into('<5120H',memory,p+row*10240,*[0x3f00+row*32+i%31 for i in range(5120)])
    position=append(memory,bytes(4))
    g.layout('embedding',[(0,64),(p,40960),(o,20480),(t,4),(position,4)])
    for step,token in enumerate((2,1,3)):
        struct.pack_into('<I',memory,t,token)
        expected=memory[:]; struct.pack_into('<2Q',expected,0,p+token*10240,o)
        expected=g.run('embed5120',expected,80,1)
        struct.pack_into('<3QI',memory,0,p,o,t,4)
        observed=g.run('embed5120_step',memory,81,2)
        g.equal('embedding device token '+str(token),observed,expected,64)
        for i in range(5120):
            assert struct.unpack_from('<I',observed,o+i*4)[0] == (0x3f00+token*32+i%31)<<16
        if step==0:
            g.caught('stale token',g.run('embed5120_step-stale',memory,80,1),expected,64)
            stale=advance(g,observed[:],position,3,'step_advance-noinc')
            good=advance(g,observed[:],position,3)
            g.caught('no increment',stale,good,64)
        memory=advance(g,observed,position,3)
        assert struct.unpack_from('<I',memory,position)[0]==step+1
    before=memory[:]; memory=advance(g,memory,position,3)
    g.equal('capacity exhausted no wrap',memory,before,64)
    struct.pack_into('<3QI',memory,0,p,o,t,4); struct.pack_into('<I',memory,t,4)
    g.equal('invalid token untouched',g.run('embed5120_step',memory,80,1),memory,64)


def attention(g,start):
    ref=runpy.run_path(str(HERE/'attention.py'))
    memory,*_=ref['fixture'](65 if start else 3)
    ptr=list(struct.unpack_from('<5Q',memory)); cap=struct.unpack_from('<I',memory,44)[0]
    tiles=(cap+63)//64
    sizes=[24576,4*cap*512,4*cap*512,24*tiles*1040,24576]
    position=append(memory,struct.pack('<I',start))
    g.layout('attention'+str(start),[(0,64),*zip(ptr,sizes),(position,4)])
    for step in range(start,start+3):
        expected=memory[:]; struct.pack_into('<5QII',expected,0,*ptr,step+1,cap)
        expected=g.run('attn256_part',expected,(step+64)//64,24)
        part=expected[:]
        expected=g.run('attn256_merge',expected,1,24)
        struct.pack_into('<6QI',memory,0,*ptr,position,cap)
        observed=g.run('attn256_part_step',memory,tiles+1,24)
        g.equal(f'attention part pos{step}',observed,part,64)
        premerge=observed[:]
        observed=g.run('attn256_merge_step',observed,1,24)
        g.equal(f'attention merge pos{step}',observed,expected,64)
        if step>0:
            g.caught(f'attention part stale length pos{step}',g.run('attn256_part_step-length1',memory,tiles,24),part,64)
            if step>=64:
                g.caught('attention merge stale tile stride',g.run('attn256_merge_step-length1',premerge,1,24),expected,64)
        memory=advance(g,observed,position,cap)
        assert struct.unpack_from('<I',memory,position)[0]==step+1


def preparation(g):
    ref=runpy.run_path(str(HERE/'prep.py'))
    rows=[g.call([g.out/'rope'],struct.pack('<I',p)) for p in range(5)]
    memory,ptr,*rest=ref['fixture'](2,struct.unpack('<64I',rows[2])); cap=rest[-1]
    table=append(memory,b''.join(rows)); position=append(memory,bytes(4))
    sizes=[49152,4096,4096,1024,1024,256,24576,4*cap*512,4*cap*512]
    g.layout('preparation',[(0,128),*zip(ptr,sizes),(table,cap*256),(position,4)])
    for step in range(3):
        expected=memory[:]; expected[ptr[5]:ptr[5]+256]=rows[step]
        struct.pack_into('<9QII',expected,0,*ptr,step,cap)
        expected=g.run('attn256_prep',expected,24,1)
        # The original row is immutable scratch from the caller's perspective, not a step output.
        memory[ptr[5]:ptr[5]+256]=rows[step]
        args=list(ptr); args[5]=table
        struct.pack_into('<10QI',memory,0,*args,position,cap)
        observed=g.run('attn256_prep_step',memory,25,2)
        g.equal('prep RoPE row pos'+str(step),observed,expected,128)
        if step:
            g.caught('stale RoPE row '+str(step),g.run('attn256_prep_step-row0',memory,24,1),expected,128)
        memory=advance(g,observed,position,cap)
    struct.pack_into('<I',memory,position,cap)
    struct.pack_into('<10QI',memory,0,*args,position,cap)
    g.equal('prep full cache untouched',g.run('attn256_prep_step',memory,24,1),memory,128)


def firstlane(g):
    memory=bytearray(448); struct.pack_into('<64I',memory,64,*[0xabc00000+i for i in range(64)])
    g.layout('firstlane',[(0,24),(64,256),(384,4)])
    rows=[]
    for mask,lane in ((0,0),(1,0),(2,1),(1<<63,63),((1<<63)|8,3),((1<<64)-1,0)):
        struct.pack_into('<3Q',memory,0,64,384,mask)
        expected=memory[:]; struct.pack_into('<I',expected,384,0xabc00000+lane)
        actual=g.run('readfirst_probe',memory,1,1)
        g.equal('readfirst mask '+str(mask),actual,expected,24)
        g.caught('readfirst no-read '+str(mask),g.run('readfirst_probe-zero',memory,1,1),expected,24)
        rows.append(dict(mask=mask,expected=0xabc00000+lane))
    (g.out/'readfirst-probe.json').write_text(json.dumps(dict(input=[0xabc00000+i for i in range(64)],rows=rows,
        positive='readfirst_probe.hsaco',negative='readfirst_probe-zero.hsaco',args=['input_ptr','output_ptr','mask_u64'],grid=[1,1],block=[64,1,1],buffers=[256,4]),indent=2)+'\n')


def main():
    omc,out=pathlib.Path(sys.argv[1]),pathlib.Path(sys.argv[2])
    assert hashlib.sha256(omc.read_bytes()).hexdigest()==PIN
    out.mkdir(parents=True,exist_ok=False)
    gate=Gate(omc,out)
    gate.build()
    firstlane(gate); embedding(gate); preparation(gate); attention(gate,0); attention(gate,62)
    recurrence(gate,'gdnconv4'); recurrence(gate,'gdnstate')
    objects=[]
    for name in NAMES:
        table=(SRC/(name+'.isa')).read_text().splitlines()
        k=next(x.split() for x in table if x.startswith('K '))
        args=[dict(name=x.split()[1],offset=int(x.split()[2]),bytes=int(x.split()[3]),kind='ptr' if x.split()[4]=='0' else 'scalar') for x in table if x.startswith('A ')]
        path=out/(name+'.hsaco')
        objects.append(dict(name=name,path=str(path),sha256=hashlib.sha256(path.read_bytes()).hexdigest(),kernarg_bytes=int(k[2]),args=args,block=[64,1,1]))
    report=dict(result='PASS',hardware='NOT_RUN',compiler_sha256=PIN,source_sha256=gate.pins,
                objects=objects,cases=gate.results,access=gate.access)
    (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print('PASS replay exact parity, controls, every-access bounds; hardware NOT_RUN',flush=True)

if __name__=='__main__': main()
