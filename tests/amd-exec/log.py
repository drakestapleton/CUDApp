#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Test instrument: CDNA3 V_LOG_F32 base2, 1ULP, FTZ (ISA p187)."""
from decimal import Decimal, localcontext
from fractions import Fraction
from functools import lru_cache
import pathlib
import random
import runpy
import struct
import subprocess
import sys
ref=runpy.run_path(str(pathlib.Path(__file__).with_name('attention.py')))
finite,enclosure,ftz= (ref[n] for n in ('finite','enclosure','ftz'))
@lru_cache(None)
def logbound(bits):
    x=finite(ftz(bits))
    assert x>0
    if x==1: return Fraction(0),Fraction(0)
    with localcontext() as ctx:
        ctx.prec=120
        value=(Decimal(x.numerator)/Decimal(x.denominator)).ln()/Decimal(2).ln()
    return enclosure(Fraction(value))
def suite(exe):
    rng=random.Random(942033)
    values=list(range(0x3f7fff00,0x3f800100))+[rng.randrange(0x800000,0x7f800000) for _ in range(4096)]
    result=subprocess.run([exe],input=b''.join(struct.pack('<II',33,x) for x in values),capture_output=True,timeout=60)
    assert result.returncode==0 and not result.stderr and len(result.stdout)==4*len(values),result
    worst=Fraction(0)
    for i,x in enumerate(values):
        y=finite(struct.unpack_from('<I',result.stdout,4*i)[0]); lo,hi=logbound(x)
        assert lo<=y<=hi,f'LOG interval escape input={x:08x} value={y} bound={lo,hi}'
        worst=max(worst,abs(y-(lo+hi)/2))
    print(f'PASS LOG Decimal120 inputs={len(values)} interval_escapes=0 worst_midpoint_error={float(worst):.12g} seed=942033')
if __name__=='__main__': suite(sys.argv[1])
