# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Independent external oracle. Not a production dependency.
import ctypes,itertools,sys
lib=ctypes.CDLL(sys.argv[1]);fn=lib.omega_native_qgorder
fn.argtypes=[ctypes.POINTER(ctypes.c_uint64),ctypes.c_void_p]+[ctypes.c_uint64]*5
fn.restype=ctypes.c_uint32

def plan(group,cols,tiles,guard=1):
    out=(ctypes.c_uint64*64)();result=ctypes.c_uint64(99)
    rc=fn(ctypes.byref(result),out,64,group,cols,tiles,guard)
    assert rc==0,rc
    return result.value,list(out)

def execute(p,x,y):
    r=[x,y,0,0];pred=False
    for i in range(p[2]):
        op,d,a,b,imm=p[3+5*i:8+5*i]
        assert max(d,a,b)<4
        if op==1:r[d]=r[a]&imm
        elif op==2:r[d]=r[a]>>imm
        elif op==3:r[d]=r[a]*imm+r[b]
        elif op==4:
            assert imm==896+11*4
            r[d]=execute.tiles
        elif op==5:pred=r[a]>=r[b]
        elif op==6:
            if pred:return None
        else:raise AssertionError('unknown opcode')
    return (r[0],r[1])

def cover(p,cols,tiles):
    execute.tiles=tiles;seen=set()
    for x,y in itertools.product(range(p[0]),range(p[1])):
        mapped=execute(p,x,y)
        if mapped is None:continue
        if mapped in seen or not (0<=mapped[0]<cols and 0<=mapped[1]<tiles):return False
        seen.add(mapped)
    return seen==set(itertools.product(range(cols),range(tiles)))

cases=0
for g in range(6):
    shapes=list(itertools.product([1,3,17],[1,15,16,17,65]))+list(itertools.product([40,128,272],[64,65]))
    for c,t in shapes:
        ok,p=plan(g,c,t);assert ok==1
        assert (p[0],p[1])==(c*(1<<g),(t+(1<<g)-1)//(1<<g))
        assert cover(p,c,t),(g,c,t)
        ok,e=plan(g,c,t,0);assert ok==1
        assert e[2]==(0 if g==0 else 3)
        assert e[3:18]==p[3:18]
        cases+=1
for g,c,t,guard,want in [(5,2047,65535,1,1),(5,2048,65535,1,0),(6,1,1,1,0),(0,0,1,1,0),(0,1,0,1,0),(0,65536,1,1,0),(0,1,65536,1,0),(0,1,1,2,0),(2**64-1,1,1,1,0),(0,2**64-1,1,1,0),(0,1,2**64-1,1,0)]:
    ok,p=plan(g,c,t,guard);assert ok==want
    if not ok:assert p[:3]==[0,0,0]
    cases+=1
out=(ctypes.c_uint64*64)();v=ctypes.c_uint64()
assert fn(None,out,64,1,1,1,1)==32
assert fn(ctypes.byref(v),None,64,1,1,1,1)==32
assert fn(ctypes.byref(v),out,63,1,1,1,1)==32
ok,p=plan(4,3,17)
for name,mutate in [('drop_guard',lambda q:q.__setitem__(2,3)),('wrong_mask',lambda q:q.__setitem__(7,7)),('wrong_row_stride',lambda q:q.__setitem__(17,8)),('floor_grid',lambda q:q.__setitem__(1,1)),('skip_row',lambda q:q.__setitem__(14,0))]:
    q=p.copy();mutate(q);assert not cover(q,3,17),name
    print(name+': caught')
print('PASS grouped-grid coverage and boundary oracle',cases,'cases')
