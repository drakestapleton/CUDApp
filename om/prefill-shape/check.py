# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# External admission oracle, not a production dependency.
import ctypes,sys
lib=ctypes.CDLL(sys.argv[1]);f=lib.omega_native_qgshape
f.argtypes=[ctypes.POINTER(ctypes.c_uint64),ctypes.c_uint64,ctypes.c_uint64];f.restype=ctypes.c_uint32
cases=[(4,8,1),(2,4,1),(0,0,0),(1,2,0),(2,8,0),(4,4,0),(4,16,0),(8,16,0),(2,0,0),(0,4,0),(2,2,0),(16,32,0),(2**32-1,4,0),(2,2**32-1,0),(2**64-1,4,0),(4,2**64-1,0)]
for rt,warps,want in cases:
 out=ctypes.c_uint64(999)
 assert f(ctypes.byref(out),rt,warps)==0
 assert out.value==want,(rt,warps,out.value,want)
assert f(None,2,4)==32
print('PASS shape admission and unsafe-shape refusal controls')
