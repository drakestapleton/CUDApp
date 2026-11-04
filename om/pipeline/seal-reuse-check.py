# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# External independent SHA256 oracle for the Omega normalized-command memo.
import ctypes,hashlib,json,random,sys,time
from pathlib import Path
l=ctypes.CDLL(sys.argv[1]);fn=l.omega_native_mcreseal
fn.argtypes=[ctypes.POINTER(ctypes.c_uint64),ctypes.c_void_p,ctypes.c_uint64,ctypes.c_void_p,ctypes.c_uint64,ctypes.c_uint64,ctypes.c_void_p,ctypes.c_uint64,ctypes.c_void_p,ctypes.c_uint64]
U=ctypes.c_ubyte
b=(U*1024)();q=(U*1024)();c=(U*2048)();o=(U*32)();r=ctypes.c_uint64()
rng=random.Random(710)
for a in (b,q):
 for i in range(1024):a[i]=rng.randrange(256)
def digest(n):
 v=bytearray(q[:384]);v[40:44]=bytes(4);v[48:52]=bytes(4)
 return hashlib.sha256(bytes(b[:896+n*4])+v+bytes(q[512:896])).digest()
cases=0
def check(n):
 global cases
 for i in range(32):o[i]=255
 rc=fn(ctypes.byref(r),b,1024,q,1024,n,c,2048,o,32)
 assert rc==0,(rc,n)
 if 1<=n<=32:assert r.value==0 and bytes(o)==digest(n),(n,r.value,bytes(o).hex(),digest(n).hex())
 else:assert r.value==2**64-1 and bytes(o)==bytes(32)
 cases+=1
for n in (0,1,2,10,31,32,33,2**64-1,10):check(n);check(n)
# Every byte of both inputs: covered mutations must refresh, ignored bytes must retain exact digest.
for a in (b,q):
 for i in range(1024):
  a[i]^=1;check(32);check(32);a[i]^=1;check(32)
# Same bytes interpreted with each argument count, both directions.
for n in list(range(1,33))+list(range(32,0,-1)):check(n);check(n)
# Zero commands have equal overlapping prefixes across argument counts.
for a in (b,q):
 for i in range(1024):a[i]=0
for n in (32,1,32,2,31,10):check(n);check(n)
check(10);start=time.perf_counter_ns()
for i in range(256):
 assert fn(ctypes.byref(r),b,1024,q,1024,10,c,2048,o,32)==0 and r.value==0
elapsed=time.perf_counter_ns()-start
# Refresh cold memo as well, proving no prior command is required.
for i in range(2048):c[i]=0
check(10)
print(json.dumps({'pass':True,'oracle_cases':cases,'warm_calls':256,'warm_total_ns':elapsed,'all_input_byte_mutations':2048,'oracle':'Python hashlib.sha256 over original normalized byte contract'}))
