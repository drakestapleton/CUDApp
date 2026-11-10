#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Host-only verification: retained C, exact state traces, source mutants, link negatives."""
import hashlib
import os
from pathlib import Path
import shlex
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]
BRIDGE=Path(os.environ['OMEGA_BRIDGE'])
OMC=Path(os.environ['OMEGA_OMC'])
# Pins come from om/units.sh, the one place they are recorded (a copied toc.om pin went stale at the bridge repin).
UNITS_PIN={k:v for k,v in (l.split('=',1) for l in (ROOT/'om/units.sh').read_text().splitlines() if l.split('=',1)[0] in ('OMC_PIN','CONVERTER_PIN','FRONTEND_PIN'))}
for p,h in [(OMC,UNITS_PIN['OMC_PIN']),(BRIDGE/'src/toc.om',UNITS_PIN['CONVERTER_PIN']),(BRIDGE/'tools/bridge-seeded',UNITS_PIN['FRONTEND_PIN'])]:
    if hashlib.sha256(p.read_bytes()).hexdigest()!=h: raise SystemExit('NOT_RUN unpinned input '+str(p))
if Path('/dev/nvidiactl').exists(): raise SystemExit('REFUSED: isolate /dev')
# Default to the vendored 580.173.02 headers the Makefile builds against; NVRM_VENDOR still overrides.
VENDOR=Path(os.environ.get('NVRM_VENDOR',str(ROOT/'third_party/nvidia-open-580.173.02')))
INCS=[ROOT/'src',ROOT/'nvrm']+[VENDOR/p for p in ('src/common/sdk/nvidia/inc','kernel-open/common/inc','kernel-open/nvidia-uvm','src/nvidia/arch/nvalloc/unix/include')]
CC=shlex.split(os.environ.get('CC','cc'))+['-std=gnu11','-D_GNU_SOURCE','-O2','-Wall','-Wextra','-Werror','-ffp-contract=off','-ffunction-sections','-fdata-sections','-Wl,--gc-sections']+['-I'+str(p) for p in INCS]
UNITS=['nvrm_va','nvrm_alloc_scalar','nvrm_counts','nvrm_alloc_push']
SRCS=[ROOT/'src'/('cudapp_om_'+u+'.c') for u in UNITS]+[ROOT/'src/cudapp_om_nvrm_policy.c',ROOT/'src/omega_ampere_qmd.c',ROOT/'src/cudapp_om_amp_runtime.c',ROOT/'src/cudapp_om_amp_qmd.c']
ENV=dict(os.environ,OMEGA_SEEDED_OMC=str(OMC))
def run(cmd,**kw): return subprocess.run([str(c) for c in cmd],check=True,timeout=180,**kw)
def same(a,b):
    with a.open('rb') as x,b.open('rb') as y:
        while True:
            c=x.read(65536);d=y.read(65536)
            if c!=d:return False
            if not c:return True
MUTANTS=[
 ('overlap-edge','nvrm_va','if gt na_end a sub i 1 va','if ge na_end a sub i 1 va'),
 ('coalesce-next','nvrm_va','if eq vend get a mul add i 1 2','if ne vend get a mul add i 1 2'),
 ('exact-fit','nvrm_va','if eq vend rend','if ne vend rend'),
 ('split-tail','nvrm_va','put s 0 add n 1\nend\nend\nput s 3 va','put s 0 n\nend\nend\nput s 3 va'),
 ('pool-reuse','nvrm_va','ret get a n','ret 0'),
 ('counter-step','nvrm_alloc_scalar','ret add a 4294967296','ret add a 4294967295'),
 ('method-mode','nvrm_alloc_scalar','if eq mode 1','if eq mode 2'),
 ('packet-envelope','nvrm_alloc_push','if or ne shr h 28 2 eq cnt 0','if or ne shr h 28 3 eq cnt 0'),
 ('decimal-digit','nvrm_counts','v = add mul v 10 d','v = add mul v 10 0'),
 ('order-edge','nvrm_counts','if ge o get state 0','if gt o get state 0'),
]
PATHS={
 'nvrm_va':['native/nvrm/alloc_profile.om','native/nvrm/alloc.om'],
 'nvrm_alloc_scalar':['native/nvrm/policy.om','native/nvrm/alloc_scalar.om'],
 'nvrm_alloc_push':['native/nvrm/alloc_profile.om','om/encoder/amp_runtime.om','native/nvrm/alloc_push.om'],
 'nvrm_counts':['native/nvrm/counts.om'],
}
with tempfile.TemporaryDirectory(prefix='nvrm-alloc-check-') as tmp:
    tmp=Path(tmp); oracle=tmp/'oracle'; normal=tmp/'normal'
    driver=ROOT/'tests/nvrm_alloc_test.c'; counter=ROOT/'tests/nvrm_counts_test.c'
    run(CC+['-DCUDAPP_MIGRATION_PENDING_C_NVRM_ALLOC=1',driver]+SRCS+['-o',oracle])
    run(CC+[driver]+SRCS+['-o',normal])
    ctrace=tmp/'c.trace';otrace=tmp/'om.trace'
    with ctrace.open('wb') as f:run([oracle],stdout=f)
    with otrace.open('wb') as f:run([normal],stdout=f)
    if not same(ctrace,otrace):raise SystemExit('FAIL C/OMEGA allocator/packet/method/slot state differential')
    print('NVRM ALLOC DIFFERENTIAL PASS (32768 randomized operations plus source boundaries; exact active state)',flush=True)
    with otrace.open('wb') as f:run([normal,'--red'],stdout=f)
    if same(ctrace,otrace):raise SystemExit('FAIL red trace accepted')
    print('NVRM ALLOC RED CONTROL KILLED',flush=True)
    counts=tmp/'counts';run(CC+[counter,SRCS[2],'-o',counts]);run([counts])
    # The driver and counter executable jointly require every strong export.
    for unit in UNITS:
        target=counter if unit=='nvrm_counts' else driver
        sources=[p for p in SRCS if p.name!='cudapp_om_'+unit+'.c']
        r=subprocess.run([str(c) for c in CC+[target]+sources+['-o',tmp/'missing']],stdout=subprocess.PIPE,stderr=subprocess.PIPE,timeout=180)
        if r.returncode==0 or ('omega_'+unit).encode() not in r.stderr:raise SystemExit('FAIL missing-unit negative '+unit)
        print('NVRM ALLOC STRONG LINK NEGATIVE PASS '+unit,flush=True)
    for name,unit,before,after in MUTANTS:
        src='\n'.join((ROOT/p).read_text() for p in PATHS[unit])+'\n'
        if src.count(before)!=1:raise SystemExit('FAIL mutant site count '+name)
        om=tmp/(name+'.om');om.write_text(src.replace(before,after));generated=tmp/(name+'.c')
        cmd=['sh',BRIDGE/'tools/bridge-seeded']+([] if unit=='nvrm_alloc_scalar' else ['--runtime-views'])+['c',om]
        with generated.open('wb') as f:run(cmd,env=ENV,stdout=f)
        exe=tmp/name
        target=counter if unit=='nvrm_counts' else driver
        sources=[generated if p.name=='cudapp_om_'+unit+'.c' else p for p in SRCS]
        run(CC+[target]+sources+['-o',exe])
        with otrace.open('wb') as f:r=subprocess.run([str(exe)],stdout=f,stderr=subprocess.PIPE,timeout=180)
        if unit=='nvrm_counts':
            killed=r.returncode!=0 and b'Assertion' in r.stderr
        else:killed=(r.returncode==0 and not same(ctrace,otrace)) or (r.returncode!=0 and b'Assertion' in r.stderr)
        if not killed:raise SystemExit('FAIL semantic mutant not killed '+name+' rc='+str(r.returncode))
        print('NVRM ALLOC MUTANT KILLED '+name,flush=True)
print('NVRM ALLOC MIGRATION PASS (10 compiled semantic mutants; HOST_ONLY)')
