#!/usr/bin/env python3
"""Source-derived lexical/record/capacity partition for dagsim.c's migration oracle.
No numeric oracle is implemented here: parse_diff compares C to generated Omega.
"""
from pathlib import Path
import sys
out = Path(sys.argv[1]); out.mkdir(parents=True, exist_ok=True)
for stale in out.glob('*.oev'): stale.unlink()
cases = []
header = 'oev version=1\nmachine id=m soc=s cores=8 mem_bytes=1024 prov=specified\nbuild repo=r sha=' + 'a'*40 + ' physics=p\n'
energy = 'energy instrument=none\n'
res = ('resource id=c kind=compute slots=2 prov=specified\n'
       'resource id=b kind=bandwidth cap=10 lo=8 hi=12 prov=specified\n'
       'resource id=v kind=capacity cap=100 prov=specified\n'
       'resource id=l kind=launch prov=specified\n')
ops = ('op id=a kind=compute engine=c t_ns=2 lo=1 hi=3 prov=specified\n'
       'op id=z kind=copy engine=c bytes=8 bw=b launch_ns=1 launcher=l prov=specified\n')
base = header + res + ops
records = [
 'oev version=1', 'machine id=m soc=s cores=8 mem_bytes=1024 prov=specified src=x',
 'build repo=r sha='+'a'*40+' physics=p',
 'calibration id=q taken=2026-11-06 valid_days=30 load1=0.5 src=x model=x',
 'resource id=extra kind=compute slots=1 prov=measured src=x',
 'op id=extra kind=compute engine=c t_ns=2 lo=1 hi=3 bytes=8 bw=b launch_ns=1 launcher=l M=1 N=1 core=0 arm=0 prov=measured src=x',
 'dep a z', 'buffer id=extra bytes=10 cap=v first=a last=z',
 'sync id=extra ops=a,z', 'energy instrument=m boundary=box joules=2 prov=measured src=x',
 'removed op=gone reason=reused ref=a proof=receipt',
 'energy instrument=m boundary=box power_w=2 prov=synthetic src=x',
 'resource id=extra kind=bandwidth cap=10 lo=8 hi=12 prov=measured src=x',
 'resource id=extra kind=capacity cap=100 prov=measured src=x']
def add(label, text): cases.append((label, text))
add('empty',''); add('valid',base+energy)
for ri, rec in enumerate(records):
    prefix = base if ri not in (0,1,2) else ''
    suffix = energy if not rec.startswith('energy ') else ''
    # singleton records also exercise missing mandatory records at EOF.
    add(f'record-{ri}', prefix+rec+'\n'+suffix)
    tokens = rec.split()
    for ti,t in enumerate(tokens[1:],1):
        if '=' not in t: continue
        key,_ = t.split('=',1)
        for kind, replacement in [('absent',None),('empty',key+'='),('bad',key+'=bogus')]:
            ts=tokens.copy()
            if replacement is None: del ts[ti]
            else: ts[ti]=replacement
            add(f'field-{ri}-{key}-{kind}',prefix+' '.join(ts)+'\n'+suffix)
    add(f'unknown-key-{ri}',prefix+rec+' extra=1\n'+suffix)
    add(f'duplicate-key-{ri}',prefix+rec+' '+tokens[1]+'\n'+suffix)
for kind in ['compute','copy','matvec','barrier','storage','persist','bogus']:
    add('op-kind-'+kind,base+f'op id=k kind={kind} engine=c t_ns=1 prov=specified\n'+energy)
    add('op-kind-no-time-'+kind,base+f'op id=k kind={kind} engine=c M=2 N=3\n'+energy)
for prov in ['unknown','synthetic','fitted','measured','specified','bogus']:
    for src in ['', ' src=x']:
        add('prov-'+prov+str(bool(src)),header+f'resource id=c kind=compute slots=1 prov={prov}'+src+'\n'+energy)
for reason in ['eliminated','reused','unchanged_dep','deferred','moved','shared','lookup','bogus']:
    add('removed-'+reason,base+f'removed op=gone reason={reason} ref=a proof=x\n'+energy)
for val in ['0','-0','+0','0.0','1','1.25','.5','1.','1e3','1E-3','+2','-2','inf','INF','Infinity','-inf','nan','NaN(x)','1e309','1e-500','-1e-500','1e','1x','0x1p0','0x1.8p+2','0x1p-1074','0x1p1024','2.2250738585072014e-308','4.9406564584124654e-324','1.00000000000000011102230246251565404236316680908203125','9007199254740993','1_0','0x','.', '+']:
    add('number-'+str(len(cases)),header+f'resource id=b kind=bandwidth cap={val} prov=specified\n'+energy)
    add('buffer-number-'+str(len(cases)),base+f'buffer id=q bytes={val} cap=v\n'+energy)
for val in ['0','1','-1','+2','2147483647','2147483648','4294967295','4294967296','18446744073709551615','18446744073709551616','99999999999999999999999999999999999999999','2x','x','--1']:
    add('integer-'+str(len(cases)),header+f'resource id=c kind=compute slots={val} prov=specified\n'+energy)
    add('uint-'+str(len(cases)),base+f'op id=q kind=matvec engine=c M={val} N=2\n'+energy)
for date in ['1970-01-01','1969-12-31','2026-11-06','2000-02-29','1900-03-01','2026-0-0','2026-13-32','2026-11-06junk','2026-11','+2026-+11-+6','-1-1-1','2026--1-1','2026-11-x']:
    add('date-'+str(len(cases)),header+'calibration id=q taken='+date+' valid_days=1 load1=0 src=x\n'+energy)
for date in ['0-1-1','9223372036854775808-1-1','-9223372036854775809-1-1','2026-9223372036854775808-1','2026-1-9223372036854775808']:
    add('date-overflow-'+str(len(cases)),header+'calibration id=q taken='+date+' valid_days=1 load1=0 src=x\n'+energy)
for sep in [' ', '\t', '\r', '\v', '\f', '\n']:
    add('whitespace-'+str(len(cases)),header+('resource id=q kind=compute slots=1 prov=specified'.replace(' ',sep))+'\n'+energy)
for tail in ['# ignored', ' # id=bad', '\x00garbage', '\n\n', '\r\n', ' unknown', '=bad', ' x=y=z']:
    add('lexical-'+str(len(cases)),base+energy.rstrip('\n')+tail+'\n')
for count in [7,8,9]: add('positions-'+str(count),base+'dep '+' '.join(['a','z']+['unused']*(count-2))+'\n'+energy)
for count in [23,24,25]: add('fields-'+str(count),header+'resource '+' '.join(['id=q','kind=compute','slots=1','prov=specified']+['slots=1']*(count-4))+'\n'+energy)
for length in [30,31,32]: add('keyword-'+str(length),header+'x'*length+'\n'+energy)
for length in [30,31,32]: add('key-'+str(length),header+'machine id=m prov=specified '+'x'*length+'=1\n'+energy)
for length in [38,39,40,41,46,47,48,126,127,128]:
    add('value-'+str(length),header+'machine id='+'x'*length+' src='+'y'*length+' prov=specified\n'+energy)
for length in [1022,1023,1024,1025]: add('line-'+str(length),header+'#'+'x'*(length-1)+'\n'+energy)
for count in [31,32,33]: add('resources-'+str(count),header+''.join(f'resource id=r{i} kind=compute slots=1 prov=specified\n' for i in range(count))+energy)
for count in [255,256,257]:
    add('ops-'+str(count),header+''.join(f'op id=o{i} kind=barrier\n' for i in range(count))+energy)
    add('buffers-'+str(count),base+''.join(f'buffer id=b{i} bytes=0 cap=v\n' for i in range(count))+energy)
    add('removed-'+str(count),base+''.join(f'removed op=r{i} reason=lookup\n' for i in range(count))+energy)
for count in [1023,1024,1025]: add('deps-'+str(count),base+'dep a z\n'*count+energy)
for text in ['sync id=q ops=a,,z,','sync id=a ops=a','sync id=q ops=q','dep z a','dep x y','resource id=c kind=compute slots=1 prov=specified','op id=a kind=barrier','energy instrument=none\nenergy instrument=none','removed op=a reason=lookup','calibration id=q taken=2026-11-06 valid_days=1 load1=0 src=x\ncalibration id=r']:
    add('refusal-'+str(len(cases)),base+text+'\n'+energy)
for length in [46,47,48,49,126,127,128,129]:
    add('physics-src-'+str(length),header+'build sha='+'a'*40+' physics='+'p'*length+'\nmachine id=m prov=measured src='+'s'*length+'\n'+energy)
    add('stored-src-'+str(length),header+'resource id=c kind=compute slots=1 prov=measured src='+'s'*length+'\n'+energy)
for space in ['\v','\f']:
    add('numeric-space-'+str(len(cases)),header+'machine id=m cores='+space+'1 mem_bytes='+space+'1.25 prov=specified\n'+energy)
    add('date-space-'+str(len(cases)),header+'calibration id=q taken='+space+'2026-11-06 valid_days='+space+'2 load1='+space+'0.5 src=x\n'+energy)
for n in [1048575,1048576,1048577]:
    text=header+energy
    while len(text)+1024<=n: text+='#'+'x'*1022+'\n'
    if len(text)<n: text+='#'+'x'*(n-len(text)-1)
    add('input-'+str(n),text)
for i,(label,text) in enumerate(cases): (out/f'{i:04d}-{label}.oev').write_bytes(text.encode())
print(f'parser partition: {len(cases)} cases written to {out}')
