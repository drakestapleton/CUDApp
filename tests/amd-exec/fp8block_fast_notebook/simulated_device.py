# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# TEST INJECTION ONLY. No hardware, driver, GPU, or AMD library is exercised.
import signal, resource, types
resource.setrlimit(resource.RLIMIT_CORE,(0,0))
SIMULATED=True
_original_run=subprocess.run

def _simulation_run(argv,*args,**kwargs):
    if isinstance(argv,str) and argv.startswith('rocminfo '):
        return subprocess.CompletedProcess(argv,0,'SIMULATED host; no device query\n','')
    return _original_run(argv,*args,**kwargs)
subprocess.run=_simulation_run


def _simulation_result(case,mode):
    m,k=case['m'],case['k']; tab=fp8_table(); x=case['x'].astype(np.float64)
    flat=(case['scales'].astype(np.uint32)<<16).view(np.float32).ravel().astype(np.float64)
    nb=(k+127)//128; mb=(m+127)//128; out=np.zeros(m,dtype=np.float64)
    stop=k//128*128 if mode=='no-tail-block' else k
    for row in range(m):
        columns=np.arange(stop)//128
        indices=(row*nb+columns if mode=='per-row-scale' else columns*mb+row//128 if mode=='transposed-scale' else (row//128)*nb+columns)
        scales=np.zeros(stop,dtype=np.float64)
        valid=indices<len(flat); scales[valid]=flat[indices[valid]]
        out[row]=np.dot(tab[case['w'][row,:stop]]*scales,x[:stop])
    return out.astype(np.float32).tobytes()


class Fake:
    def __init__(self):
        self.mem={}; self.next=1<<20; self.modules={}; self.functions={}; self.cache={}
    def hipInit(self,n): return 0
    def locate(self,p,n):
        address=p.value if hasattr(p,'value') else p
        for base,record in self.mem.items():
            if base<=address and address+n<=base+record['size']: return record,address-base
        raise AssertionError('simulated allocation overrun')
    def read(self,p,n):
        record,off=self.locate(p,n); data=record['data']
        return data[off:off+n]+bytes(max(0,off+n-max(off,len(data))))
    def hipMalloc(self,pp,n):
        pp._obj.value=self.next; self.mem[self.next]=dict(size=n.value,data=b'')
        self.next+=n.value+4096; return 0
    def hipMemset(self,p,value,n):
        record,off=self.locate(p,n.value)
        assert value==0 and off==len(record['data']) and n.value==SLACK
        return 0
    def hipMemcpy(self,dst,src,n,kind):
        if kind==1:
            record,off=self.locate(dst,n.value); assert off==0
            record['data']=src if isinstance(src,bytes) else ctypes.string_at(src,n.value)
            assert len(record['data'])==n.value
        else: ctypes.memmove(dst,self.read(src,n.value),n.value)
        return 0
    def hipFree(self,p): del self.mem[p.value]; return 0
    def hipModuleLoadData(self,pm,data):
        digest=hashlib.sha256(data.raw).hexdigest()
        names=[name for name,item in DATA['objects'].items() if item['sha256']==digest]
        assert len(names)==1
        handle=len(self.modules)+1; self.modules[handle]=names[0]; pm._obj.value=handle; return 0
    def hipModuleGetFunction(self,pf,module,name):
        assert name==DATA['objects'][self.modules[module.value]]['entry'].encode()
        handle=len(self.functions)+1; self.functions[handle]=self.modules[module.value]; pf._obj.value=handle; return 0
    def hipModuleUnload(self,module): return 0
    def hipDeviceSynchronize(self): return 0
    def hipModuleLaunchKernel(self,fn,gx,gy,gz,bx,by,bz,shared,stream,params,extra):
        case=CURRENT_CASE; bufs=CURRENT_BUFFERS
        assert (gy,gz,bx,by,bz)==(1,1,64,1,1)
        assert gx in (max(1,case['m']),case['m']+1)
        for i,value in enumerate(bufs.values[CURRENT_INDEX]):
            if i<4:
                p=ctypes.cast(params[i],ctypes.POINTER(ctypes.c_void_p)).contents.value
                assert p==value.value
                record=self.mem[p]; expected=case['buffers'][i]
                assert record['size']==len(expected)+SLACK
                assert self.read(p+len(expected),SLACK)==bytes(SLACK)
                if i<3: assert record['data']==expected
            else: assert ctypes.cast(params[i],ctypes.POINTER(ctypes.c_uint32)).contents.value==value.value
        if GROUP==os.environ.get('SELFCHECK_FAULT_GROUP'): os.kill(os.getpid(),signal.SIGSEGV)
        name=self.functions[fn.value]; mode=name.removeprefix('fp8gemv_b128f-') if name.startswith('fp8gemv_b128f-') else ''
        group=next(g for g in DATA['groups'] if g['id']==GROUP)
        if group['kind']=='timing': assert name==group['kernel'],'timing dispatched the wrong kernel'
        if GROUP==os.environ.get('SELFCHECK_ESCAPE_GROUP'): mode=''
        key=(case['digest'],mode)
        if key not in self.cache: self.cache[key]=_simulation_result(case,mode)
        output=self.cache[key]
        if GROUP==os.environ.get('SELFCHECK_CORRUPT_GROUP'):
            output=bytearray(output); index=max(0,case['m']-2)
            struct.pack_into('<f',output,index*4,1e30); output=bytes(output)
        self.mem[bufs.outputs[CURRENT_INDEX].value]['data']=output
        return 0

setup_hip=lambda: Fake()


class Storage:
    def __init__(self,size,dtype,data=None): self.size=size; self.dtype=dtype; self.data=data


class Tensor:
    def __init__(self,storage,shape=None,start=0,count=None):
        self.storage=storage; self.start=start
        self.count=storage.size-start if count is None else count
        self.shape=(self.count,) if shape is None else tuple(shape)
    def array(self):
        data=self.storage.data
        if data is None or self.start>=len(data): return np.zeros(self.shape,dtype=self.storage.dtype)
        assert self.start+self.count<=len(data)
        return data[self.start:self.start+self.count].reshape(self.shape)
    def __getitem__(self,item):
        assert isinstance(item,slice) and item.step is None
        start=0 if item.start is None else item.start; end=self.count if item.stop is None else item.stop
        return Tensor(self.storage,start=self.start+start,count=end-start)
    def reshape(self,*shape):
        assert math.prod(shape)==self.count
        return Tensor(self.storage,shape,self.start,self.count)
    def copy_(self,other):
        assert self.start==0 and self.count==other.count and self.storage.dtype==other.storage.dtype
        self.storage.data=other.array().reshape(-1)
        return self
    def view(self,dtype):
        assert dtype=='bf16' and self.storage.dtype==np.uint16
        return self
    def to(self,dtype):
        assert dtype=='bf16'
        value=self.array().astype(np.float32); bits=value.view(np.uint32)
        data=((bits+0x7fff+((bits>>16)&1))>>16).astype(np.uint16)
        data[np.isnan(value)]=0x7fc0
        return Tensor(Storage(data.size,np.uint16,data.ravel()),data.shape)
    def float(self):
        assert self.storage.dtype==np.uint16
        data=(self.array().astype(np.uint32)<<16).view(np.float32)
        return Tensor(Storage(data.size,np.float32,data.ravel()),data.shape)
    def cpu(self): return self
    def numpy(self): return self.array()


class Torch:
    bfloat16='bf16'; __version__='SIMULATED'; version=types.SimpleNamespace(hip='SIMULATED')
    cuda=types.SimpleNamespace(is_available=lambda:True,synchronize=lambda:None,get_device_name=lambda:'SIMULATED_gfx942')
    def __init__(self): self.cache={}
    def zeros(self,n,dtype,device):
        assert dtype=='bf16' and device=='cuda' and n>=SLACK//2
        return Tensor(Storage(n,np.uint16))
    def from_numpy(self,data): return Tensor(Storage(data.size,data.dtype,data.ravel()),data.shape)
    def count_nonzero(self,t): return types.SimpleNamespace(item=lambda:int(np.count_nonzero(t.array())))
    def matmul(self,a,x,out):
        # All matrix copies alias immutable simulation storage, not real device allocations.
        base=a.storage.data
        while getattr(base,'base',None) is not None: base=base.base
        key=(id(base),id(x.storage.data),a.shape)
        if key not in self.cache:
            w=a.array(); xf=x.float().array().astype(np.float64); result=np.zeros(a.shape[0],dtype=np.float32)
            for r in range(0,a.shape[0],128):
                wf=(w[r:r+128].astype(np.uint32)<<16).view(np.float32).astype(np.float64)
                result[r:r+128]=wf@xf
            bits=result.view(np.uint32)
            words=((bits+0x7fff+((bits>>16)&1))>>16).astype(np.uint16); words[np.isnan(result)]=0x7fc0
            self.cache[key]=words
        out.storage.data=self.cache[key]
        return out

sys.modules['torch']=Torch()
