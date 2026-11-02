# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# External profile oracle, not a production dependency.
import ctypes,sys
lib=ctypes.CDLL(sys.argv[1])

# Independent profile table at every small boundary and representative large rows.
r=lib.omega_native_qgreading
r.argtypes=[ctypes.POINTER(ctypes.c_uint64),ctypes.c_uint64,ctypes.c_uint64];r.restype=ctypes.c_uint32
for rows in list(range(0,258))+[4095,4096,4097,2**32-1,2**32,2**64-1]:
 for index in [0,1,2,3,4,2**64-1]:
  out=ctypes.c_uint64(123)
  assert r(ctypes.byref(out),rows,index)==0
  expected=(4,8,45,0) if 0<rows<65 else (8,8,45,4)
  want=expected[index] if 0<rows<2**32 and index<4 else 2**32-1
  assert out.value==want,(rows,index,out.value,want)
assert r(None,64,0)==32
print('PASS adaptive profile boundaries, option order and refusal controls')
