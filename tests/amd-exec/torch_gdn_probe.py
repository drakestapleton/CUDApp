#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Optional CPU-only independent reference instrument; never a production dependency."""
import pathlib
import runpy
import sys
# This directory's token.py is a GPU test module, not Python's token module.
sys.path=[p for p in sys.path if pathlib.Path(p).resolve()!=pathlib.Path(__file__).parent.resolve()]
sys.path.insert(0,sys.argv[1])
import torch
print('REFERENCE torch',torch.__version__,'device=cpu hardware=NOT_RUN')
torch.set_num_threads(1)
x=torch.tensor([1.0,2.0],dtype=torch.bfloat16)
print('DTYPE product/sum/epsilon/rsqrt/output',*(str(v.dtype) for v in (x*x,(x*x).sum(),(x*x).sum()+1e-6,torch.rsqrt((x*x).sum()+1e-6),x*torch.rsqrt((x*x).sum()+1e-6))))
x=torch.arange(0x7f80,dtype=torch.int16).view(torch.bfloat16)
a=x+1e-6
full=(x.float()+1e-6).bfloat16()
rounded=(x.float()+torch.tensor(1e-6,dtype=torch.bfloat16).float()).bfloat16()
f=int((a.view(torch.int16)!=full.view(torch.int16)).sum())
r=int((a.view(torch.int16)!=rounded.view(torch.int16)).sum())
print('EPSILON all-positive-finite-BF16',x.numel(),'full-FP32-scalar-mismatches',f,'BF16-scalar-mismatches',r)
print('EPSILON BF16 scalar bits',hex(int(torch.tensor(1e-6,dtype=torch.bfloat16).view(torch.int16))<<16))
assert r==0 and f>0
print('CONTROL unrounded-epsilon CAUGHT')
if len(sys.argv)>2 and sys.argv[2]=='suite':
    import base64
    import struct
    gdn=runpy.run_path(str(pathlib.Path(__file__).with_name('gdn.py')))
    check=gdn['ref']['check']
    scalar=torch.tensor([0x425e],dtype=torch.int16).view(torch.bfloat16)
    cpu=torch.rsqrt(scalar)
    correctly_rounded=torch.rsqrt(scalar.double()).bfloat16()
    cpu_bits=int(cpu.view(torch.int16)[0])<<16
    correct_bits=int(correctly_rounded.view(torch.int16)[0])<<16
    interval=gdn['ibf16'](gdn['rsqbound'](gdn['finite'](0x425e0000)))
    assert interval==(gdn['finite'](correct_bits),)*2
    assert cpu_bits==0x3e0a0000 and correct_bits==0x3e090000
    assert not interval[0]<=gdn['finite'](cpu_bits)<=interval[1]
    print('OBSERVED ARM torch BF16 rsqrt input=425e0000 CPU=3e0a0000 FP64-rounded=3e090000 Decimal120+ISA interval=',interval)
    print('CONTROL CPU-BF16-rsqrt-outside-ISA-envelope CAUGHT; CPU fallback bit parity is NOT claimed')
    for args in gdn['QK_CASES']:
        case=gdn['qk_fixture'](*args); memory=bytearray(base64.b64decode(case['memory']))
        xp,op=(a[1] for a in case['args'])
        x=torch.frombuffer(memory,dtype=torch.float32,count=4096,offset=xp).view(32,128).bfloat16()
        variance=(x*x).sum(dim=-1,keepdim=True)+1e-6
        cpu_inv=torch.rsqrt(variance)
        # This is explicitly a correctly-rounded primitive reference, NOT the
        # ARM BF16 torch fallback. Never widen the device oracle to admit it.
        inv=torch.rsqrt(variance.double()).bfloat16()
        print('OBSERVED CPU-rsqrt-vs-FP64-rounded differing-heads',int((cpu_inv!=inv).sum()))
        normalized=(x*inv).float()
        normalized[:16]=normalized[:16]*torch.tensor(struct.unpack('<f',struct.pack('<I',gdn['QSCALE']))[0])
        for head in range(args[0]):
            struct.pack_into('<128I',memory,op+head*512,*(b&0xffffffff for b in normalized[head].view(torch.int32).tolist()))
        check(memory,case)
        print('PASS torch-BF16-boundaries with FP64-rounded-rsqrt instrument',case['label'])
    # Torch's library reduction tree is not the declared Omega accumulation order.
    # Use independent Torch primitive operations in that explicitly declared order.
    for kind in ('zero','no-beta','no-decay','dense'):
        for head in range(3):
            old,values,expected,out_expected=gdn['state_head'](head,kind)
            def tensor(bits):
                data=bytearray(struct.pack('<'+str(len(bits))+'I',*bits))
                return torch.frombuffer(data,dtype=torch.float32).clone()
            state=tensor(old).view(128,128)
            value=tensor(values)
            decay,beta=tensor(gdn['state_scalars'](head,kind))
            key=tensor([gdn['qk_value'](head//3,r,False) for r in range(128)])
            query=tensor([gdn['qk_value'](head//3,r,True) for r in range(128)])
            state=state*decay
            kv=torch.zeros(128,dtype=torch.float32)
            for r in range(128): kv=kv+state[r]*key[r]
            delta=(value-kv)*beta
            output=torch.zeros(128,dtype=torch.float32)
            for r in range(128):
                state[r]=state[r]+key[r]*delta
                output=output+state[r]*query[r]
            output=output.bfloat16().float()
            actual=[b&0xffffffff for b in state.flatten().view(torch.int32).tolist()]
            out_actual=[b&0xffffffff for b in output.view(torch.int32).tolist()]
            assert actual==expected and out_actual==out_expected,(kind,head)
        print('PASS torch-recurrent-rounding-boundaries kind='+kind+' heads=3 every-state/output-bit')
