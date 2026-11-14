# Instrument (not Omega): OMPLAN01 launch micro-benchmark. Every step is N dispatches of the model's
# step_advance kernel (the same object real2 uses: driver-real2.py OBJ, sha256-checked), with
# kernargs (position pointer, u32 cap) exactly as real2 passes them; the pointer is relocated to a
# private VRAM buffer. No weights are read.
# Usage: gap-microplan.py DRIVER N N_GEN OUT.plan OUT_DIR   (writes OUT_DIR/step_advance.hsaco)
import base64,hashlib,lzma,re,struct,sys
from pathlib import Path
drv,N,G,out,od=Path(sys.argv[1]),int(sys.argv[2]),int(sys.argv[3]),Path(sys.argv[4]),Path(sys.argv[5])
m=re.search(r"'step_advance': \['([0-9a-f]{64})', '([^']+)'",drv.read_text())
want,data=m.group(1),m.group(2)
obj=lzma.decompress(base64.b85decode(data));assert hashlib.sha256(obj).hexdigest()==want
od.mkdir(parents=True,exist_ok=True);(od/'step_advance.hsaco').write_bytes(obj)
# kernarg size from the kernel descriptor step_advance.kd (ELF64 symtab lookup; descriptor offset 8, u32)
shoff=struct.unpack_from('<Q',obj,40)[0];shnum,shstr=struct.unpack_from('<HH',obj,60)
sec=[struct.unpack_from('<IIQQQQIIQQ',obj,shoff+64*i) for i in range(shnum)]
def sect(i):return sec[i][4],sec[i][5]
kd=None
for s in sec:
    if s[1]==2:  # SHT_SYMTAB
        stroff,_=sect(s[6])
        for k in range(s[5]//24):
            nm,info,oth,shn,val,size=struct.unpack_from('<IBBHQQ',obj,s[4]+24*k)
            name=obj[stroff+nm:obj.index(b'\0',stroff+nm)]
            if name==b'step_advance.kd':
                vaddr,off=None,None
                for t in sec:
                    if t[3]<=val<t[3]+t[5] and t[1]!=8:off=t[4]+val-t[3]
                kd=off
assert kd is not None
K=struct.unpack_from('<I',obj,kd+8)[0]
assert K>=12,K
raw=bytes(8)+struct.pack('<I',131072)+bytes(K-12)
name=b'step_advance'
hdr=b'OMPLAN01'+struct.pack('<7Q',1,0,2,N,1,G,1)
codes=struct.pack('<2Q',0,len(name))
bufs=struct.pack('<6Q',4096,0,0,0,0,1)+struct.pack('<6Q',4096,0,0,0,0,2)
prompt=struct.pack('<i',0)+bytes(4)
one=struct.pack('<8I',0,1,1,1,64,K,1,0)+raw+struct.pack('<IIQ',0,0,0)
one+=bytes((-len(one))%8)
prefix=hdr+codes+bufs+prompt+one*N
plan=prefix+name+struct.pack('<4Q',len(prefix),len(name),len(prefix)+len(name),0)
out.write_bytes(plan)
print('MICROPLAN',out,'N',N,'gen',G,'kernarg',K,'bytes',len(plan),'object',want)
