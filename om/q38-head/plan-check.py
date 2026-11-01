# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# External verification of checked portable Omega bindings.
import ctypes as C,struct,sys
L=C.CDLL(sys.argv[1]);F=L.omega_q38_runtime;F.restype=C.c_uint32
F.argtypes=[C.POINTER(C.c_uint64),C.c_void_p,C.c_uint64,C.c_void_p,C.c_uint64,C.c_void_p,C.c_uint64]
m=(C.c_uint64*16)();b=C.create_string_buffer(struct.pack('<5120f',*([2.7109375]*5120)),20480);q=(C.c_uint64*192)();r=C.c_uint64()
def run():
 return F(C.byref(r),m,16,b,20480,q,192),r.value
m[0]=1;m[1]=248320
assert run()==(0,1)
info=list(q);hn=struct.unpack('<f',struct.pack('<I',q[0]))[0]
assert hn==195.37811279296875 and q[2]==10384
assert q[10]==2*1024**3
m[0]=3
assert run()==(0,1) and q[10]==2*1024**3
q[0]=info[0]
p=((248320+2047)//2048)*2048
assert info[3:10]==[0,20480,20480+248320*4,20480+248320*8,20480+248320*8+p*4,20480+248320*12+p*4,p]
m[0]=2;m[2]=q[0]
for j in range(3,9):m[j]=j*0x1000000
assert run()==(0,6)
base=m[3];h=m[4];lo=m[5];av=m[6];ai=m[7];tab=m[8]
expected=[
[1,20,5120,h,0,base,0,0,5120],
[2,0,0,tab,base,base+info[4],0,0,0],
[3,970,248320,base+info[4],base+info[5],base+info[6],base+info[7],info[0],248320],
[4,p//2048,p,base+info[6],0,av,ai,0,0],
[5,1,2048,av,ai,base+info[8],base+info[8]+4,0,0],
[6,0,0,h,base+info[7],lo,base+info[8],0,0]]
for i,row in enumerate(expected):assert list(q)[i*16:(i+1)*16]==row+[0]*7,(i,list(q)[i*16:(i+1)*16],row)
for index,wrong in [(1,0),(1,248321),(2,0),(2,0x7f800000),(2,0x80000000)]+[(j,v) for j in range(3,9) for v in [0,1,1<<48]]:
 saved=m[index];m[index]=wrong;before=list(q);assert run()==(0,0);assert list(q)==before;m[index]=saved
E=L.omega_q38_emit;E.restype=C.c_uint32;E.argtypes=[C.POINTER(C.c_uint64),C.c_void_p,C.c_uint64]
code=(C.c_ubyte*16384)();assert E(C.byref(r),code,16384)==0
frame=open(sys.argv[2],'rb').read()
head=struct.unpack_from('<6Q',frame)
offset=48+16*(head[1]+head[2])+64*head[0]
assert bytes(code[:r.value])==frame[offset:]
print('PASS: layout, six complete descriptors, admission refusals, unchanged output on refusal, emitted kernel byte identity')
