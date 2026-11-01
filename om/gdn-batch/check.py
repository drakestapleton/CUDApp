# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# External independent host oracle, not a production dependency.
import ctypes,itertools,json,sys
U=ctypes.c_uint64; MAX=2**64-1
lib=ctypes.CDLL(sys.argv[1])
plan=lib.omega_native_gdnbplan;plan.argtypes=[ctypes.POINTER(U),ctypes.POINTER(U),U];plan.restype=ctypes.c_uint32
step=lib.omega_native_gdnbnext;step.argtypes=[ctypes.POINTER(U)]+[U]*6;step.restype=ctypes.c_uint32
stamp=lib.omega_native_gdnbstamp;stamp.argtypes=[ctypes.POINTER(U),ctypes.POINTER(U),U,U];stamp.restype=ctypes.c_uint32
plans=steps=stamps=0
for mode,rows,layer,layers in itertools.product([0,1,2,MAX],[0,1,2,3,4,4096,65535,65536,65537,MAX],[0,1,47,48,2**32-1,MAX],[0,1,48,2**32-1,2**32,MAX]):
    want=MAX
    if mode in [0,1] and 3<=rows<=65536 and 1<=layers<=2**32-1 and layer<layers:want=3 if mode else 0
    out=U(123);args=(U*4)(mode,rows,layer,layers)
    assert plan(ctypes.byref(out),args,4)==0 and out.value==want,(mode,rows,layer,layers,want,out.value)
    plans+=1
valid={(phase,phase,0,MAX,1,0):phase+1 for phase in range(4)}
valid.update({(4,3,3,MAX,1,3):5,(5,3,3,MAX,1,3):6})
for row in itertools.product([0,1,2,3,4,5,6,MAX],[0,1,2,3,4,MAX],[0,1,2,3,4,MAX],[0,1,2,3,MAX],[0,1,2,MAX],[0,1,2,3,4,MAX]):
    out=U(123);assert step(ctypes.byref(out),*row)==0 and out.value==valid.get(row,0),(row,out.value)
    steps+=1
for values in itertools.product([0,1,2,3,7,MAX-1,MAX],repeat=4):
    for index in [0,1,2,3,MAX]:
        want=MAX
        if index<3 and values[-1]!=MAX and all(a<b for a,b in zip(values,values[1:])):want=values[index+1]-values[index]
        out=U(123);args=(U*4)(*values)
        assert stamp(ctypes.byref(out),args,4,index)==0 and out.value==want,(values,index,want,out.value)
        stamps+=1
for fn,extra in [(plan,[]),(stamp,[0])]:
    for n in [0,3,5,MAX]:
        out=U(123);args=(U*5)(0,1,2,3,4)
        assert fn(ctypes.byref(out),args,n,*extra)==32 and out.value==123
print(json.dumps({'status':'PASS_GDN_BATCH_POLICY','admission_cases':plans,'transition_cases':steps,'timestamp_cases':stamps,'wrong_lengths_refused':8}))
