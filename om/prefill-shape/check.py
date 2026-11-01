# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# External admission oracle, not a production dependency.
import ctypes,sys
lib=ctypes.CDLL(sys.argv[1]);f=lib.omega_native_qgshape
f.argtypes=[ctypes.POINTER(ctypes.c_uint64)]+[ctypes.c_uint64]*4;f.restype=ctypes.c_uint32
cases=[(4,8,1),(2,4,1),(0,0,0),(1,2,0),(2,8,0),(4,4,0),(4,16,1),(8,16,0),(8,8,1),(8,4,0),(8,32,0),(2,0,0),(0,4,0),(2,2,0),(16,32,0),(2**32-1,4,0),(2,2**32-1,0),(2**64-1,4,0),(4,2**64-1,0)]
cases += [(rt,warps,int((rt,warps) in {(4,8),(4,16),(2,4),(8,8)})) for rt in range(17) for warps in range(33)]
for rt,warps,want in cases:
 out=ctypes.c_uint64(999)
 assert f(ctypes.byref(out),rt,warps,128,0)==0
 assert out.value==want,(rt,warps,out.value,want)
assert f(None,2,4,128,0)==32
print('PASS shape admission and unsafe-shape refusal controls')

# Independent exact-integer enumeration of every emitted store immediate.
count=0
for rt in [0,2,4,8,9,2**64-1]:
 warps=4 if rt==2 else 8
 for epi in [0,1,2,2**64-1]:
  widths={0,1,128,5120,6144,16384,17408,34816,2**32-1,2**32,2**64-1}
  if 1<=rt<=8 and epi in (0,1):
   top=((1<<23)-1-(36 if epi==0 else 0))//((16*rt-8)*(4 if epi==0 else 1))
   widths.update([top-1,top,top+1])
  for n in sorted(widths):
   valid=1<=n<2**32 and rt in (2,4,8) and epi in (0,1)
   if valid:
    stride=n*(4 if epi==0 else 1)
    emitted=[(16*i+8*h)*stride+(32*j+4*q if epi==0 else 0) for i in range(rt) for h in range(2) for j in range(2) for q in range(2)]
    want=int(all(0<=x<(1<<23) for x in emitted))
   else:want=0
   out=ctypes.c_uint64(999)
   assert f(ctypes.byref(out),rt,warps,n,epi)==0
   assert out.value==want,(n,rt,epi,out.value,want)
   count+=1
assert f(None,8,8,16384,0)==32
print('PASS exact emitted-offset boundary oracle',count,'cases')
