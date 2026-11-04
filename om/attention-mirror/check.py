# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Independent external oracle. Not a production dependency.
import ctypes,itertools,random,sys
lib=ctypes.CDLL(sys.argv[1]);U=ctypes.c_uint64;bad=2**64-1
plan=lib.omega_native_attnmirplan;plan.restype=ctypes.c_uint32;plan.argtypes=[ctypes.POINTER(U),ctypes.POINTER(U),U]
step=lib.omega_native_attnmirnext;step.restype=ctypes.c_uint32;step.argtypes=[ctypes.POINTER(U)]+[U]*4

def expected(a):
 mode,qh,kh,limit,pos,rows,plane=a
 if not (mode in (0,1,2) and qh==24 and kh==4 and 0<limit<=131072 and 0<rows<=32768 and pos+rows<=limit and plane==limit*4*512+4*4*512+64):return bad
 return 2*plane if mode==1 or (mode==2 and rows>=1024) else 0

cases=[]
for limit in (1,30,1023,1024,4096,40960,131072):
 for rows in (1,30,1023,1024,1025,4096,32768):
  for pos in (0,max(0,limit-rows),limit):
   for mode in (0,1,2,3):cases.append((mode,24,4,limit,pos,rows,limit*2048+8256))
base=(2,24,4,40960,0,4096,40960*2048+8256)
for i in range(7):
 for v in (0,1,3,23,25,1023,1024,32768,32769,131072,131073,2**32,2**64-1):
  a=list(base);a[i]=v;cases.append(tuple(a))
rng=random.Random(710)
for _ in range(1000):
 limit=rng.randrange(1,131073);rows=rng.randrange(1,min(limit,32768)+1);pos=rng.randrange(limit-rows+1)
 cases.append((rng.randrange(3),24,4,limit,pos,rows,limit*2048+8256))
for a in cases:
 out=U(99);assert plan(ctypes.byref(out),(U*7)(*a),7)==0;assert out.value==expected(a),('plan',a,out.value,expected(a))
assert plan(None,(U*7)(*base),7)==32
assert plan(ctypes.byref(U()),None,7)==32
assert plan(ctypes.byref(U()),(U*7)(*base),6)==32

def next_expected(phase,need,have,ok):
 if ok!=1 or (need and (need<20608 or need>536887424 or need%16)):return 0
 if phase==0:return 1 if need>have else 2
 if phase==1:return 2 if have>=need else 0
 if phase==2:return 3 if need else 4
 if phase==3:return 4 if need else 0
 if phase==4:return 5
 return 0
n=0
for a in itertools.product(range(7),(0,16,20608,20609,33570944,536887424,536887440,2**64-1),(0,16,20608,33570944,536887424,2**64-1),(0,1,2)):
 out=U(99);assert step(ctypes.byref(out),*a)==0;assert out.value==next_expected(*a),('step',a,out.value,next_expected(*a));n+=1
assert step(None,0,20608,0,1)==32
print('PASS independent attention mirror oracle',len(cases),'plans',n,'transitions')
