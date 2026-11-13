#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Test-only LLVM decode of emitted objects; CDNA3 PDF pp28-29 Table11/12.
Checks the one-wait-state transcendental -> consuming non-trans VALU hazard.
Not a hardware timing model or an audit of the other Table11 hazards.
"""
import pathlib
import re
import subprocess
import sys

TRANS = set('''v_exp_f32 v_log_f32 v_rcp_f32 v_rcp_iflag_f32 v_rsq_f32
v_rcp_f64 v_rsq_f64 v_sqrt_f32 v_sqrt_f64 v_sin_f32 v_cos_f32 v_rcp_f16
v_sqrt_f16 v_rsq_f16 v_log_f16 v_exp_f16 v_sin_f16 v_cos_f16
v_exp_legacy_f32 v_log_legacy_f32'''.split())

def vgprs(operand):
    result = {int(n) for n in re.findall(r'\bv(\d+)\b', operand)}
    for first,last in re.findall(r'\bv\[(\d+):(\d+)\]',operand):
        result.update(range(int(first),int(last)+1))
    return result

def instructions(obj):
    result = subprocess.run(['llvm-objdump-18','--disassemble','--mcpu=gfx942',str(obj)],
                            capture_output=True,text=True,timeout=30)
    assert result.returncode == 0 and not result.stderr,result
    obj.with_suffix('.schedule.txt').write_text(result.stdout)
    rows = []
    for line in result.stdout.splitlines():
        if '//' not in line: continue
        text,encoded = line.split('//',1)
        if not re.match(r'\s*[0-9a-fA-F]+:',encoded): continue
        fields = text.strip().split(None,1)
        assert fields and re.match(r'^(s|v|ds|global)_\w+$',fields[0]),f'undecoded instruction: {line}'
        name = re.sub(r'_e(?:32|64)$','',fields[0])
        operands = fields[1] if len(fields)==2 else ''
        rows.append((name,operands,encoded.split(':',1)[0].strip()))
    assert rows,f'empty instruction stream {obj}'
    return rows

def hazards(rows):
    failures = []
    for producer,consumer in zip(rows,rows[1:]):
        op,operands,pc = producer
        nextop,nextargs,nextpc = consumer
        if op not in TRANS or not nextop.startswith('v_') or nextop in TRANS: continue
        written = vgprs(operands.split(',',1)[0])
        reads = nextargs.split(',',1)[1] if ',' in nextargs else ''
        # FMAC reads its destination accumulator as well as its explicit sources.
        if nextop.startswith(('v_fmac_','v_mac_')): reads = nextargs
        if written & vgprs(reads): failures.append(f'{pc}:{op} -> {nextpc}:{nextop}')
    return failures

def check(obj):
    rows = instructions(obj)
    failures = hazards(rows)
    assert not failures,f'missing transcendental wait: {obj.name}: {failures}'
    return sum(op in TRANS for op,_,_ in rows)

def suite(src,out):
    kernels = sorted(p.stem for p in src.glob('*.isa'))
    assert {'vadd','fsum16','sgemm','fp8gemv','bf16gemv','fp8gemv2','bf16gemv2'} <= set(kernels)
    for kernel in kernels:
        obj = out/('hsaco.hsaco' if kernel=='vadd' else kernel+'-hsaco.hsaco')
        count = check(obj)
        print(f'PASS scheduling {kernel} trans_instructions={count} rule=CDNA3-p28-29-trans-consumer',flush=True)
    for kernel in ('attn256_merge','attn256_gate'):
        source = (src/(kernel+'.isa')).read_text()
        assert '\nI 4 0 0\n' in source
        mutant = out/(kernel+'-no-trans-wait.isa')
        mutant.write_text(source.replace('\nI 4 0 0\n','\n'))
        obj = mutant.with_suffix('.hsaco')
        with obj.open('wb') as stream:
            result = subprocess.run([str(out/'hsaco'),str(mutant)],stdout=stream,stderr=subprocess.PIPE,timeout=30)
        assert result.returncode == 0 and not result.stderr,result
        try: check(obj)
        except AssertionError as error:
            assert str(error).startswith('missing transcendental wait:'),error
            print(f'CONTROL scheduling-{kernel}-no-wait CAUGHT {error}',flush=True)
        else: raise AssertionError(f'scheduling control escaped: {kernel}')
    print(f'PASS scheduling objects={len(kernels)} controls=2 hardware=NOT_RUN',flush=True)

if __name__=='__main__':
    suite(pathlib.Path(sys.argv[1]),pathlib.Path(sys.argv[2]))
