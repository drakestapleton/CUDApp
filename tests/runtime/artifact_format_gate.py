#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Host-only byte/refusal differential. JSON is an output mirror, never admission."""
import argparse
import ctypes as C
import hashlib
import json
import os
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import random
import shlex
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
SUFFIX = ('.manifest', '.text.bin', '.cubin', '.qmd0.bin', '.qmd1.bin', '.cbank_args.bin', '.json', '.sha256')
SEED = 0xA17FAC7
class Param(C.Structure):
    _fields_ = [('offset', C.c_uint32), ('size', C.c_uint32), ('pointer', C.c_uint8)]
class Artifact(C.Structure):
    _fields_ = [(k, C.c_char_p) for k in ('op','shape','bounds','contract_name','contract_id_hex','semantic_spec_id_hex','kernel_function','precision','accumulation','tolerance')] + [
        ('text', C.POINTER(C.c_uint8)), ('text_len', C.c_size_t), ('params', C.POINTER(Param)), ('nparams', C.c_size_t)] + [
        (k,C.c_uint32) for k in ('gpr_count','grid_x','grid_y','block_x','block_y','block_z','shared_bytes','cbank_offset')] + [
        ('cbank_args',C.POINTER(C.c_uint32)),('cbank_words',C.c_size_t),('qmd0',C.POINTER(C.c_uint32)),('qmd1',C.POINTER(C.c_uint32))] + [
        (k,C.c_char_p) for k in ('min_driver','requires','source_commit','tool_version','qualification')]

def strict_pairs(items):
    out={}
    for k,v in items:
        if k in out: raise ValueError('duplicate JSON key: '+k)
        out[k]=v
    return out

def parse_json(raw):
    return json.loads(raw, object_pairs_hook=strict_pairs,
                      parse_constant=lambda x: (_ for _ in ()).throw(ValueError(x)))

def fixture():
    text=(C.c_uint8*16)()
    params=(Param*1)(Param(896,8,1))
    q0=(C.c_uint32*96)();q1=(C.c_uint32*96)();cb=(C.c_uint32*2)()
    q1[52]=12; q1[34]=1|(1<<16);q1[35]=1|(16<<8)|(1<<17)
    q1[36]=(0x04b44808 & ~0x7ff)|8;q1[39]=q1[40]=q1[41]=1
    a=Artifact(b'matmul',b'm=1,k=1,n=1',b'm<=16',b'fixture',b'a'*64,b'b'*64,b'fixture',b'bf16',b'fp32',b'0',text,16,params,1,
               16,1,1,1,1,1,0,896,cb,2,q0,q1,b'580.173.02',b'sm_121',b'UNKNOWN',b'artifact-format/1',b'host-only')
    return a,(text,params,q0,q1,cb)

def run(cmd):
    print('+',shlex.join(map(str,cmd)),flush=True)
    subprocess.run(cmd,cwd=ROOT,check=True)

def build(out, replacements=()):
    lib=out/'artifact.so'
    cc=shlex.split(os.environ.get('CC','cc'))
    sources=['src/omega_native_artifact.c','tests/runtime/artifact_oracle.c','src/omega_cubin.c','src/omega_blackwell_realize.c','src/omega_blackwell_encoder.c','src/sha256.c']
    repl=dict(replacements)
    sources += [str(repl.get(p.name,p)) for p in sorted((ROOT/'src').glob('cudapp_om_*.c'))]
    sources += ['src/cudapp_freest_revision.c','src/cudapp_freest_epoch.c','src/cudapp_freest_f16.c']
    flags=['-fPIC','-O0','-std=c11','-D_GNU_SOURCE','-Isrc']
    cache=ROOT/'build/artifact-format/objects';cache.mkdir(parents=True,exist_ok=True)
    deps=b''.join(p.read_bytes() for p in sorted((ROOT/'src').glob('*.h')))
    deps+=(ROOT/'src/omega_native_artifact.c').read_bytes()+(ROOT/'src/omega_native_artifact_om.inc').read_bytes()
    def compile_one(source):
        path=ROOT/source
        digest=hashlib.sha256(path.read_bytes()+deps+repr(cc+flags).encode()).hexdigest()
        obj=cache/(path.stem+'-'+digest+'.o')
        if not obj.exists():run(cc+flags+['-c',str(path),'-o',str(obj)])
        return str(obj)
    with ThreadPoolExecutor(max_workers=3) as workers:objects=list(workers.map(compile_one,sources))
    run(cc+['-shared','-Wl,-z,defs','-o',str(lib)]+objects+['-lpthread','-lm'])
    return lib

def bind(lib):
    dll=C.CDLL(str(lib))
    for name in ('omega_native_artifact_export','artifact_c_export'):
        f=getattr(dll,name);f.argtypes=[C.POINTER(Artifact),C.c_char_p,C.c_void_p,C.c_void_p,C.c_size_t];f.restype=C.c_int
    for name in ('omega_native_artifact_admit','artifact_c_admit'):
        f=getattr(dll,name);f.argtypes=[C.c_char_p,C.c_char_p,C.c_void_p,C.c_void_p,C.c_size_t];f.restype=C.c_int
    return dll

def invoke(f,*args):
    ident=C.create_string_buffer(65);err=C.create_string_buffer(256)
    rc=f(*args,ident,err,256)
    return rc,ident.value

def remove(p):
    for s in SUFFIX: Path(str(p)+s).unlink(missing_ok=True)

def rehash(raw):
    try:
        start=raw.index(b'\nop=')+1;end=raw.index(b'provenance_tool=',start)
        p=raw.index(b'artifact_id=')+12; e=raw.index(b'\n',p)
        if e-p!=64:return raw
        return raw[:p]+hashlib.sha256(raw[start:end]).hexdigest().encode()+raw[e:]
    except ValueError:return raw

def codec(dll):
    f=dll.omega_artifact_format
    f.argtypes=[C.POINTER(C.c_uint64),C.c_void_p,C.c_uint64,C.c_void_p,C.c_uint64]
    c=dll.artifact_c_json_escape
    c.argtypes=[C.c_char_p,C.c_void_p,C.c_size_t];c.restype=C.c_size_t
    for raw in [b'']+[bytes([i]) for i in range(1,256)]+[b'\xc3\xa9',b'quote"slash\\']:
        b=(C.c_uint8*65536)();s=(C.c_uint64*128)();ok=C.c_uint64()
        for i,v in enumerate(raw):b[i]=v
        s[0]=2;s[10]=0;s[50]=len(raw)
        assert f(C.byref(ok),b,65536,s,128)==0 and ok.value==1
        emitted=bytes(b[32768:32768+s[1]])
        expected=(C.c_uint8*2048)()
        n=c(raw,expected,2048)
        assert b'  "shape": '+bytes(expected[:n])+b',\n' in emitted
        try:decoded=raw.decode('utf-8')
        except UnicodeDecodeError:
            try:parse_json(emitted)
            except UnicodeDecodeError:pass
            else:raise AssertionError('legacy invalid UTF-8 became valid')
        else:assert parse_json(emitted)['shape']==decoded
    for invalid in (b'{"x":1,"x":2}',b'{"x":"\\q"}',b'{"x":"\\uZZZZ"}'):
        try:parse_json(invalid)
        except (ValueError,UnicodeDecodeError):pass
        else:raise AssertionError('strict independent parser accepted adversarial JSON')
    print('ARTIFACT-FORMAT codec PASS byte-domain=256 valid-utf8 strict-escapes duplicate-keys invalid-utf8-classified',flush=True)

def corpus(dll,directory,flip=False,cases=6000):
    rng=random.Random(SEED); a,keep=fixture()
    om=directory/'om';oracle=directory/'cc'
    mutations=[b'plain',b'quote"slash\\',b'x'*127,b'x'*128,b'x'*8192,b'bad\nvalue',b'bad\rvalue',b'\x7f',b'\x01',b'\xc3\xa9']
    fields=('shape','bounds','contract_name','precision','accumulation','tolerance','requires','source_commit','tool_version','qualification','kernel_function','contract_id_hex','semantic_spec_id_hex','min_driver','op')
    valid=[]
    for i in range(256):
        a,keep=fixture();key=fields[i%len(fields)]
        value=mutations[(i//len(fields))%len(mutations)] if i else b'm=1,k=1,n=1'
        if i:setattr(a,key,value)
        ro=invoke(dll.omega_native_artifact_export,C.byref(a),os.fsencode(om));rc=invoke(dll.artifact_c_export,C.byref(a),os.fsencode(oracle))
        assert ro==rc,('export',i,key,value[:40],ro,rc)
        if ro[0]==0:
            for s in SUFFIX:
                ob=Path(str(om)+s).read_bytes();cb=Path(str(oracle)+s).read_bytes()
                if flip and i==0 and s=='.manifest':cb=bytes([cb[0]^1])+cb[1:]
                assert ob==cb,('bytes',i,s)
            js=parse_json(Path(str(om)+'.json').read_bytes())
            assert js['artifact_id'].encode()==ro[1]
            if key in js:assert js[key]==value.decode('utf-8')
            valid.append(Path(str(om)+'.manifest').read_bytes())
        remove(om);remove(oracle)
    a,keep=fixture();assert invoke(dll.omega_native_artifact_export,C.byref(a),os.fsencode(om))[0]==0
    base=Path(str(om)+'.manifest').read_bytes();mp=Path(str(om)+'.manifest')
    lines=base.splitlines(keepends=True)
    for i in range(cases):
        kind=i%12;r=bytearray(base)
        if kind==0:r=bytearray(valid[rng.randrange(len(valid))])
        elif kind==1:r[rng.randrange(len(r))]^=rng.randrange(1,256)
        elif kind==2:r=r[:rng.randrange(len(r))]
        elif kind==3:r+=rng.choice([b'\n',b'{}',b'\x00',b'op=matmul\n'])
        elif kind==4:
            j=rng.randrange(1,len(lines));r=bytearray(b''.join(lines[:j]+[lines[j]]+lines[j:]))
        elif kind==5:r=bytearray(base.replace(b'contract_id='+b'a'*64,b'contract_id='+b'G'*64))
        elif kind==6:r=bytearray(base.replace(b'shape=m=1,k=1,n=1',b'shape='+b'x'*rng.randrange(127,9000)))
        elif kind==7:r=bytearray(rng.choice([b'{',b'{"a":1,"a":2}',b'"\\uZZZZ"',b'[NaN]',b'{"bad":"\\q"}']))
        elif kind==8:
            j=rng.randrange(1,len(lines));k=rng.randrange(1,len(lines));z=lines.copy();z[j],z[k]=z[k],z[j];r=bytearray(b''.join(z))
        elif kind==9:r=bytearray(base.replace(b'grid=1,1,1',b'grid='+rng.choice([b'-1,1,1',b'4294967296,1,1',b'01,1,1',b'1,1,2'])))
        elif kind==10:r=bytearray(base.replace(b'text_sha256=',b'text_sha256=z'))
        if kind in (0,5,6,9,10):r=bytearray(rehash(bytes(r)))
        mp.write_bytes(r)
        driver=rng.choice([None,b'580.173.02',b'1.0.0',b'bad',b'999.0.0',b'1.'+b'1'*100+b'.0'])
        ro=invoke(dll.omega_native_artifact_admit,os.fsencode(om),driver);rc=invoke(dll.artifact_c_admit,os.fsencode(om),driver)
        assert ro==rc,('admit',i,kind,ro,rc,bytes(r)[:100])
    print(f'ARTIFACT-FORMAT PASS export=256 admission={cases} seed={SEED:#x} byte-exact strict-json',flush=True)

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--library',type=Path);ap.add_argument('--flip',action='store_true');args=ap.parse_args()
    out=ROOT/'build/artifact-format';out.mkdir(parents=True,exist_ok=True)
    lib=args.library or build(out)
    dll=bind(lib);codec(dll)
    with tempfile.TemporaryDirectory(prefix='artifact-format-') as td:corpus(dll,Path(td),args.flip)
    if not args.library and not args.flip:
        red=subprocess.run([os.sys.executable,__file__,'--library',str(lib),'--flip'],cwd=ROOT,capture_output=True,text=True)
        (out/'negative-control.log').write_text(red.stdout+red.stderr)
        assert red.returncode==1 and "('bytes', 0, '.manifest')" in red.stderr,'flipped expected byte was not caught at the expected comparison'
        print('ARTIFACT-FORMAT negative-control PASS expected-byte-flip rc=1')
if __name__=='__main__':main()
