# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Outside notebook verification/timing instrument, not an Omega runtime dependency.
import base64, ctypes, ctypes.util, glob, hashlib, json, math, os, struct, subprocess, sys, time, traceback, zlib
DATA = json.loads(zlib.decompress(base64.b85decode('@DATA@')))
GROUP = os.environ.get('AMDSERVE_GROUP', '')
OUTF = 'amdserve-probe-v13.txt'
STATEF, MARKF = OUTF + '.state', OUTF + '.mark'
T_START = float(os.environ.get('AMDSERVE_T0', time.time()))
SLACK = 1 << 20
CACHE_BYTES = 256 * (1 << 20)
COLD_BYTES = 1100000000
MIN_CALLS = 200
SIMULATED = False

def say(text):
    line = '%s [t=%ds]' % (text, time.time()-T_START)
    print(line, flush=True)
    with open(OUTF, 'a') as f: f.write(line+'\n')

def summarize_controls(records):
    controls = {}
    for record in records: controls.update(record.get('controls', {}))
    expected = [g['id'][4:] for g in DATA['groups'] if g['kind']=='control']
    good = set(controls)==set(expected) and all(v is True for v in controls.values())
    say('CONTROLS %s %s' % ('all CAUGHT' if good else 'ESCAPED or NOT_RUN', '; '.join('%s=%s' % (n,controls.get(n,'NOT_RUN')) for n in expected)))
    return good

def read_records():
    records = []
    for line in open(STATEF): records.append(json.loads(line))
    return records

def parent():
    open(OUTF,'w').close(); open(STATEF,'w').close()
    faults = []; interrupted = False
    say('AMDSERVE-PROBE-V11 source=%s groups=%d; controls first; isolated children' % (DATA['source_head'],len(DATA['groups'])))
    try:
        host = subprocess.run("rocminfo | grep -m2 -E 'Marketing Name: +AMD|Name: +gfx'; cat /opt/rocm/.info/version",shell=True,capture_output=True,text=True,timeout=30)
        say('HOST '+ ' | '.join(host.stdout.splitlines()))
        controls_done = False
        for group in DATA['groups']:
            if group['kind']!='control' and not controls_done:
                summarize_controls(read_records()); controls_done=True
            if os.path.exists(MARKF): os.remove(MARKF)
            errfile = OUTF+'.stderr-'+group['id']
            try:
                with open(errfile,'w') as err:
                    rc = subprocess.run([sys.executable,os.path.abspath(__file__)],env=dict(os.environ,AMDSERVE_GROUP=group['id'],AMDSERVE_T0=str(T_START),OPENBLAS_NUM_THREADS='1',OMP_NUM_THREADS='1'),stdout=subprocess.DEVNULL,stderr=err,timeout=900).returncode
            except subprocess.TimeoutExpired:
                rc='TIMEOUT(900s)'
            if rc!=0:
                faults.append(group['id'])
                marker=open(MARKF).read().strip() if os.path.exists(MARKF) else 'none'
                tail=' | '.join(open(errfile).read().splitlines()[-2:])[:300]
                say('FAULT group %s exit %s; last launch: %s; stderr: %s' % (group['id'],rc,marker,tail))
    except BaseException:
        interrupted=True
        say('ERROR parent '+ ' | '.join(traceback.format_exc().splitlines()[-3:]))
    finally:
        try:
            records=read_records(); seen=[r['group'] for r in records]
            missing=[g['id'] for g in DATA['groups'] if g['id'] not in seen]
            caught=summarize_controls(records)
            ok=not interrupted and not faults and not missing and len(seen)==len(set(seen))==len(DATA['groups']) and caught and all(r['ok'] for r in records)
            say('RESULT overall %s%s' % ('PASS' if ok else 'FAIL_OR_INCOMPLETE','; missing='+','.join(missing) if missing else ''))
        except BaseException as error:
            say('CONTROLS ESCAPED or NOT_RUN; malformed or absent state')
            say('RESULT overall FAIL_OR_INCOMPLETE; summary='+str(error))
        say('NOT_RUN full-model logits/tokens, native Omega launcher; simulated-device results are not hardware or performance evidence')


def marker(text):
    with open(MARKF,'w') as f:
        f.write(text+' in group '+GROUP); f.flush(); os.fsync(f.fileno())


def setup_hip():
    for lib in [ctypes.util.find_library('amdhip64')]+sorted(glob.glob('/opt/rocm*/lib/libamdhip64.so*')):
        if lib:
            try:
                device=ctypes.CDLL(lib)
                say('HIP_LIBRARY '+lib)
                return device
            except OSError: pass
    raise RuntimeError('libamdhip64 unavailable; nothing launched')


def chk(rc,label):
    if rc!=0: raise RuntimeError('%s rc=%s' % (label,rc))


def put(buf):
    p=ctypes.c_void_p()
    chk(hip.hipMalloc(ctypes.byref(p),ctypes.c_size_t(len(buf)+SLACK)),'hipMalloc')
    try:
        if buf: chk(hip.hipMemcpy(p,buf,ctypes.c_size_t(len(buf)),1),'H2D')
        chk(hip.hipMemset(ctypes.c_void_p(p.value+len(buf)),0,ctypes.c_size_t(SLACK)),'zero slack')
    except BaseException:
        hip.hipFree(p); raise
    return p


def get(p,n):
    out=ctypes.create_string_buffer(n)
    chk(hip.hipMemcpy(out,p,ctypes.c_size_t(n),2),'D2H')
    return out.raw


def fp8_table():
    # Independent OCP sign/exponent/fraction definition; NOT native FNUZ.
    table=[]
    for byte in range(256):
        e,f=(byte>>3)&15,byte&7
        value=math.nan if e==15 and f==7 else math.ldexp(f,-9) if e==0 else math.ldexp(8+f,e-10)
        table.append(-value if byte&128 else value)
    return np.array(table,dtype=np.float64)


def fixture(m,k,special=False):
    rng=np.random.default_rng(942128+m*65537+k)
    w=rng.integers(0,256,size=(m,k),dtype=np.uint8)
    # Keep numerical coverage: uniform NaNs at long K would make almost every output NaN.
    # Finite rows retain all 254 finite encodings, including OCP -0 and +/-448.
    w[(w&127)==127]=126
    if special:
        assert (m,k)==(256,1)
        w[:,0]=np.arange(256,dtype=np.uint8)
    elif m and k:
        w[0,0]=127
        if m>1: w[-1,-1]=255
        if m>2: w[1,0]=128
        if k>1 and m>2: w[1,1]=126
        if k>2 and m>2: w[1,2]=254
    xwords=rng.integers(0x3c00,0x3f81,size=k,dtype=np.uint16)
    xwords|=rng.integers(0,2,size=k,dtype=np.uint16)<<15
    x=(xwords.astype(np.uint32)<<16).view(np.float32)
    scales=rng.integers(0x3800,0x4081,size=((m+127)//128,(k+127)//128),dtype=np.uint16)
    if special: x[:]=1; scales[:]=0x3f80
    image=[w.tobytes(),x.tobytes(),scales.tobytes(),b'\xa5'*(m*4)]
    h=hashlib.sha256()
    h.update(struct.pack('<II',m,k))
    for b in image[:3]: h.update(b)
    return dict(m=m,k=k,w=w,x=x,scales=scales,buffers=image,digest=h.hexdigest())


def gamma(n,u):
    return n*u/(1-n*u)


def oracle(case,bf16=False):
    m,k=case['m'],case['k']; table=fp8_table(); x=case['x'].astype(np.float64)
    ref=np.zeros(m,dtype=np.float64); mag=np.zeros(m,dtype=np.float64)
    scales=(case['scales'].astype(np.uint32)<<16).view(np.float32).astype(np.float64)
    for row in range(0,m,128):
        end=min(row+128,m)
        w=table[case['w'][row:end]]
        s=np.repeat(scales[row//128],128)[:k]
        w*=s
        if bf16:
            bits=w.astype(np.float32).view(np.uint32)
            rounded=((bits+0x7fff+((bits>>16)&1))>>16).astype(np.uint16)
            # NaN payloads must remain NaNs after rounding, never turn into infinity.
            rounded[np.isnan(w)]=0x7fc0
            w=(rounded.astype(np.uint32)<<16).view(np.float32).astype(np.float64)
        ref[row:end]=w@x
        mag[row:end]=np.abs(w)@np.abs(x)
    # FP32 path depth: two products/FMA, six tree adds, then ceil(K/128) block FMAs.
    # Float64 calculator error is included; test inputs avoid finite under/overflow.
    n=(k+127)//128+8
    bound=(gamma(n,2.0**-24)+gamma(2*k+n,2.0**-53))*mag+2.0**-126
    if bf16:
        # AMD library uses BF16 inputs/output; permit FP32 accumulation then BF16 rounding.
        bound=(gamma(k+1,2.0**-24)+gamma(2*k+1,2.0**-53))*mag
        bound+=2.0**-8*(np.abs(ref)+bound)+2.0**-126
    return ref,bound


def mismatches(raw,expected):
    ref,bound=expected
    y=np.frombuffer(raw,dtype='<f4').astype(np.float64)
    assert y.shape==ref.shape
    finite=np.isfinite(ref)
    good=np.where(finite,np.isfinite(y)&(np.abs(y-ref)<=bound),np.isnan(ref)&np.isnan(y))
    return int(np.count_nonzero(~good))


def require_output(raw,expected,label):
    bad=mismatches(raw,expected)
    if bad: raise AssertionError('%s all-row oracle mismatches=%d' % (label,bad))


class Module:
    def __init__(self,name):
        self.name=name; self.module=ctypes.c_void_p(); self.fn=ctypes.c_void_p()
        item=DATA['objects'][name]; self.sha=item['sha256']
        raw=base64.b64decode(item['base64'])
        assert hashlib.sha256(raw).hexdigest()==self.sha,'object digest mismatch'
        chk(hip.hipModuleLoadData(ctypes.byref(self.module),ctypes.create_string_buffer(raw,len(raw))),'module load')
        chk(hip.hipModuleGetFunction(ctypes.byref(self.fn),self.module,item['entry'].encode()),'entry')
    def close(self):
        if self.module.value: chk(hip.hipModuleUnload(self.module),'unload')
    def mark(self): marker(self.name+'.hsaco sha256 '+self.sha)


class Buffers:
    def __init__(self,case,copies=1):
        self.case=case; self.ptrs=[]; self.weights=[]; self.outputs=[]; self.params=[]; self.values=[]
        try:
            def alloc(buf):
                p=put(buf); self.ptrs.append((p,buf)); return p
            x=alloc(case['buffers'][1]); scales=alloc(case['buffers'][2])
            for _ in range(copies):
                w=alloc(case['buffers'][0]); y=alloc(case['buffers'][3])
                self.weights.append(w); self.outputs.append(y)
                vals=[w,x,scales,y,ctypes.c_uint32(case['m']),ctypes.c_uint32(case['k'])]
                self.values.append(vals)
                self.params.append((ctypes.c_void_p*6)(*[ctypes.cast(ctypes.byref(v),ctypes.c_void_p) for v in vals]))
        except BaseException: self.close(); raise
    def launch(self,module,index=0,overdispatch=False):
        global CURRENT_CASE,CURRENT_BUFFERS,CURRENT_INDEX
        CURRENT_CASE,CURRENT_BUFFERS,CURRENT_INDEX=self.case,self,index
        chk(hip.hipModuleLaunchKernel(module.fn,max(1,self.case['m']+int(overdispatch)),1,1,64,1,1,0,None,self.params[index],None),'launch')
    def check(self,expected,inputs=True):
        chk(hip.hipDeviceSynchronize(),'sync')
        for y in self.outputs: require_output(get(y,self.case['m']*4),expected,'Omega')
        outputs={p.value for p in self.outputs}
        for p,buf in self.ptrs:
            if get(ctypes.c_void_p(p.value+len(buf)),SLACK)!=bytes(SLACK): raise AssertionError('nonzero allocation slack')
            if inputs and p.value not in outputs and get(p,len(buf))!=buf: raise AssertionError('input buffer modified')
    def close(self):
        for p,_ in reversed(self.ptrs): chk(hip.hipFree(p),'free')
        self.ptrs=[]


def wall_ms(action,sync,copies):
    calls=((max(MIN_CALLS,copies)+copies-1)//copies)*copies
    for i in range(copies): action(i)
    sync(); start=time.perf_counter()
    for i in range(calls): action(i%copies)
    sync(); elapsed=(time.perf_counter()-start)*1000/calls
    if not elapsed>0: raise AssertionError('nonpositive timing interval')
    return elapsed,calls


def correctness(case, name):
    expected=oracle(case)
    module=Module(name); bufs=None
    try:
        bufs=Buffers(case); module.mark(); bufs.launch(module,overdispatch=True); bufs.check(expected)
    finally:
        if bufs: bufs.close()
        module.close()
    say('OMEGA CORRECTNESS M=%d K=%d all_rows=%d PASS finite_rows=%d NaN_rows=%d same_data_sha256=%s kernel=%s' % (case['m'],case['k'],case['m'],np.count_nonzero(np.isfinite(expected[0])),np.count_nonzero(np.isnan(expected[0])),case['digest'],name))


def control(name):
    case=fixture(257,389); expected=oracle(case)
    baseline=Module('fp8gemv_b128f'); mutant=Module('fp8gemv_b128f-'+name); bufs=None
    try:
        bufs=Buffers(case); baseline.mark(); bufs.launch(baseline); bufs.check(expected)
        mutant.mark(); bufs.launch(mutant); chk(hip.hipDeviceSynchronize(),'sync')
        bad=mismatches(get(bufs.outputs[0],case['m']*4),expected)
        for p,buf in bufs.ptrs:
            if get(ctypes.c_void_p(p.value+len(buf)),SLACK)!=bytes(SLACK): raise AssertionError('mutant wrote slack')
        if not bad: raise AssertionError(name+' ESCAPED')
        say('CONTROL '+name+' CAUGHT numerical_mismatches='+str(bad))
        return True
    finally:
        if bufs: bufs.close()
        baseline.close(); mutant.close()


def omega_timing(case, name):
    weight_bytes=len(case['buffers'][0]); copies=COLD_BYTES//weight_bytes+1
    assert copies*weight_bytes>CACHE_BYTES
    expected=oracle(case); module=Module(name); bufs=None
    try:
        bufs=Buffers(case,copies); module.mark()
        sync=lambda: chk(hip.hipDeviceSynchronize(),'sync')
        warm,wn=wall_ms(lambda i:bufs.launch(module,0),sync,1)
        cold,cn=wall_ms(lambda i:bufs.launch(module,i),sync,copies)
        bufs.check(expected)
        say('OMEGA TIMING kernel=%s M=%d K=%d warm_ms=%.6f cold_rotation_ms=%.6f warm_GB_per_s=%.3f cold_GB_per_s=%.3f weight_bytes=%d copies=%d rotation_bytes=%d warm_calls=%d cold_calls=%d all_rows=PASS same_data_sha256=%s%s' % (name,case['m'],case['k'],warm,cold,weight_bytes/warm/1e6,weight_bytes/cold/1e6,weight_bytes,copies,weight_bytes*copies,wn,cn,case['digest'],' SIMULATED_NOT_MEASURED' if SIMULATED else ''))
    finally:
        if bufs: bufs.close()
        module.close()


def reference(case):
    import torch
    if not torch.cuda.is_available() or not torch.version.hip: raise RuntimeError('ROCm torch unavailable: REFERENCE NOT_RUN')
    say('REFERENCE backend torch=%s HIP=%s device=%s' % (torch.__version__,torch.version.hip,torch.cuda.get_device_name()))
    marker('REFERENCE torch.matmul (AMD library, not Omega) same_data_sha256='+case['digest'])
    m,k=case['m'],case['k']; copies=COLD_BYTES//(m*k*2)+1
    assert copies*m*k*2>CACHE_BYTES
    backing=[]
    def padded(n):
        whole=torch.zeros(n+SLACK//2,dtype=torch.bfloat16,device='cuda'); backing.append((whole,n)); return whole[:n]
    # Dequantize exactly once before timing; all copies have identical BF16 weights.
    host=np.empty((m,k),dtype=np.uint16); table=fp8_table()
    scales=(case['scales'].astype(np.uint32)<<16).view(np.float32)
    for row in range(0,m,128):
        w=(table[case['w'][row:row+128]]*np.repeat(scales[row//128],128)[:k]).astype(np.float32)
        bits=w.view(np.uint32); words=((bits+0x7fff+((bits>>16)&1))>>16).astype(np.uint16)
        words[np.isnan(w)]=0x7fc0; host[row:row+128]=words
    first=padded(m*k).reshape(m,k)
    first.copy_(torch.from_numpy(host).view(torch.bfloat16))
    matrices=[first]
    for _ in range(copies-1):
        copy=padded(m*k).reshape(m,k); copy.copy_(first); matrices.append(copy)
    x=padded(k); x.copy_(torch.from_numpy(case['x']).to(torch.bfloat16))
    ys=[padded(m) for _ in range(copies)]
    warm,wn=wall_ms(lambda i:torch.matmul(matrices[0],x,out=ys[0]),torch.cuda.synchronize,1)
    cold,cn=wall_ms(lambda i:torch.matmul(matrices[i],x,out=ys[i]),torch.cuda.synchronize,copies)
    expected=oracle(case,bf16=True)
    for y in ys: require_output(y.float().cpu().numpy().tobytes(),expected,'REFERENCE BF16')
    for whole,n in backing:
        if torch.count_nonzero(whole[n:]).item(): raise AssertionError('REFERENCE slack corruption')
    weight_bytes=m*k*2
    say('REFERENCE torch.matmul BF16 (AMD library, not Omega) M=%d K=%d warm_ms=%.6f cold_rotation_ms=%.6f warm_GB_per_s=%.3f cold_GB_per_s=%.3f weight_bytes=%d copies=%d rotation_bytes=%d warm_calls=%d cold_calls=%d all_rows=PASS same_data_sha256=%s%s' % (m,k,warm,cold,weight_bytes/warm/1e6,weight_bytes/cold/1e6,weight_bytes,copies,weight_bytes*copies,wn,cn,case['digest'],' SIMULATED_NOT_MEASURED' if SIMULATED else ''))

# SELFCHECK_INSERT

def child():
    global np,hip
    ok=False; controls={}; group=next(g for g in DATA['groups'] if g['id']==GROUP)
    try:
        import numpy as np
        say('GROUP '+GROUP+' numpy='+np.__version__+(' SIMULATED_DEVICE' if SIMULATED else ''))
        if group['kind']=='reference':
            reference(fixture(*group['shape']))
        else:
            hip=setup_hip(); chk(hip.hipInit(0),'hipInit')
            if group['kind']=='control': controls[GROUP[4:]]=control(GROUP[4:])
            elif group['kind']=='correctness':
                case=fixture(*group['shape'])
                for name in DATA['timed_kernels']: correctness(case,name)
            elif group['kind']=='tails':
                for m,k in DATA['tails']:
                    case=fixture(m,k)
                    for name in DATA['timed_kernels']: correctness(case,name)
                case=fixture(256,1,special=True)
                for name in DATA['timed_kernels']: correctness(case,name)
            elif group['kind']=='timing': omega_timing(fixture(*group['shape']),group['kernel'])
            else: raise ValueError('unknown group kind')
        ok=True
    except BaseException:
        if group['kind']=='reference':
            say('REFERENCE (AMD library, not Omega) M=%d K=%d NOT_RUN_OR_FAILED' % tuple(group['shape']))
        say('ERROR group '+GROUP+': '+' | '.join(traceback.format_exc().splitlines()[-4:]))
    finally:
        with open(STATEF,'a') as f: f.write(json.dumps(dict(group=GROUP,ok=ok,controls=controls))+'\n')

if __name__=='__main__':
    if GROUP: child()
    else: parent()
