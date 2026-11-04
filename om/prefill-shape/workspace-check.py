# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Independent capacity oracle, not production.
import ctypes,sys,random
lib=ctypes.CDLL(sys.argv[1]);f=lib.omega_native_qgworkspace
f.argtypes=[ctypes.POINTER(ctypes.c_uint64)]+[ctypes.c_uint64]*5;f.restype=ctypes.c_uint32
cases=[]
for rows in [0,1,3,30,63,64,65,128,129,512,4096,2**32-1,2**32,2**64-1]:
 for bm in [0,32,64,128,2**32-1,2**32,2**64-1]:
  for delta in [-1,0,1]:
   cap=max(0,rows+delta);pad=min(2**64-1,rows+bm+delta);tiles=(rows+bm-1)//bm+1 if bm else 0
   if cap<2**64 and pad>=0:cases += [(cap,pad,tiles,rows,bm),(cap,pad,max(0,tiles-1),rows,bm)]
cases += [(4096,4224,33,30,64),(4096,4224,33,4096,128),(4096,4224,33,4096,64)]
rng=random.Random(710)
for _ in range(1000):cases.append(tuple(rng.randrange(8193) for _ in range(5)))
for args in cases:
 capacity,padded,tiles,rows,bm=args
 want=int(0<rows<2**32 and 0<bm<2**32 and rows<=capacity and rows+bm<=padded and (rows+bm-1)//bm+1<=tiles)
 out=ctypes.c_uint64(9);assert f(ctypes.byref(out),*args)==0;assert out.value==want,(args,out.value,want)
assert f(None,4096,4224,33,30,64)==32
print('PASS independent workspace capacity oracle',len(cases),'cases')
