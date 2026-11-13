#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Test-only LLVM decode of emitted objects; CDNA3 PDF pp28-29 Table11/12.
Checks the one-wait-state transcendental -> consuming non-trans VALU hazard,
and v_readlane/v_readfirstlane SGPR writes -> VALU constant read (2 wait
states), v_readlane/v_writelane lane select (4) and VMEM read (5), following
fall-through and branch paths. Other SGPR-writing VALU ops (v_cmp, carry-out)
and the other Table11 rows are not audited. Not a hardware timing model.
"""
import pathlib
import re
import subprocess
import sys

TRANS = set('''v_exp_f32 v_log_f32 v_rcp_f32 v_rcp_iflag_f32 v_rsq_f32
v_rcp_f64 v_rsq_f64 v_sqrt_f32 v_sqrt_f64 v_sin_f32 v_cos_f32 v_rcp_f16
v_sqrt_f16 v_rsq_f16 v_log_f16 v_exp_f16 v_sin_f16 v_cos_f16
v_exp_legacy_f32 v_log_legacy_f32'''.split())

SGPR_WRITERS = ('v_readfirstlane_b32','v_readlane_b32')
LANE_SELECT = {'v_readlane_b32':2,'v_writelane_b32':2}
CARRY_IN = ('v_addc_co_u32','v_subb_co_u32','v_subbrev_co_u32')
CARRY_OUT = ('v_addc_co_u32','v_subb_co_u32','v_subbrev_co_u32','v_div_scale_')
VMEM = ('global_','buffer_','scratch_','flat_')

def vgprs(operand):
    result = {int(n) for n in re.findall(r'\bv(\d+)\b', operand)}
    for first,last in re.findall(r'\bv\[(\d+):(\d+)\]',operand):
        result.update(range(int(first),int(last)+1))
    return result

def sgprs(operand):
    result = {int(n) for n in re.findall(r'\bs(\d+)\b',operand)}
    for first,last in re.findall(r'\bs\[(\d+):(\d+)\]',operand):
        result.update(range(int(first),int(last)+1))
    if re.search(r'\bvcc\b',operand): result.update((106,107))
    if re.search(r'\bvcc_lo\b',operand): result.add(106)
    if re.search(r'\bvcc_hi\b',operand): result.add(107)
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

def sgpr_need(op,operands,written):
    """Wait states CDNA3 p29 requires before this instruction reads `written`."""
    args = [arg.strip() for arg in operands.split(',')] if operands else []
    if op.startswith(VMEM): return 5 if written & sgprs(operands) else 0
    if not op.startswith('v_'): return 0
    need = 0
    for i,arg in enumerate(args):
        if i == 0 or not written & sgprs(arg): continue
        if LANE_SELECT.get(op) == i: need = max(need,4)
        elif i == 1 and ('_co_' in op or op.startswith(CARRY_OUT)): continue
        elif op.startswith(CARRY_IN) and i == len(args)-1: continue
        else: need = max(need,2)
    return need

def waits(row):
    op,operands,_ = row
    return int(operands,0)+1 if op == 's_nop' else 1

def successors(rows,index,at):
    op,operands,pc = rows[at]
    if op == 's_endpgm': return []
    if op == 's_branch' or op.startswith('s_cbranch_'):
        assert re.fullmatch(r'-?\d+',operands.split()[0]),f'unknown branch target: {pc}:{op} {operands}'
        simm = int(operands.split()[0])
        if simm >= 0x8000: simm -= 0x10000
        target = int(pc,16)+4+4*simm
        assert target in index,f'branch target outside object: {pc}:{op} {operands}'
        rest = [at+1] if op != 's_branch' and at+1 < len(rows) else []
        return [index[target]]+rest
    assert op not in ('s_setpc_b64','s_swappc_b64'),f'indirect branch in SGPR window: {pc}:{op}'
    return [at+1] if at+1 < len(rows) else []

def sgpr_hazards(rows):
    index = {int(pc,16):i for i,(_,_,pc) in enumerate(rows)}
    failures = []
    for start,(op,operands,pc) in enumerate(rows):
        if op not in SGPR_WRITERS: continue
        written = sgprs(operands.split(',',1)[0])
        assert written,f'no SGPR destination: {pc}:{op} {operands}'
        pending,seen = [(start,0)],set()
        while pending:
            at,count = pending.pop()
            if (at,count) in seen: continue
            seen.add((at,count))
            count += 0 if at == start else waits(rows[at])
            for nxt in successors(rows,index,at):
                name,args,where = rows[nxt]
                need = sgpr_need(name,args,written)
                if need > count: failures.append(f'{pc}:{op} -> {where}:{name} waits={count} need={need}')
                if count < 5: pending.append((nxt,count))
    return failures

# Checker controls on hand-written decoded rows: each must report its hazard,
# and the safe rows must not.
SYNTHETIC = {
    'constant': ([('v_readfirstlane_b32','s8, v1','0'),('s_nop','0','4'),('v_mov_b32','v2, s8','8')],'waits=1 need=2'),
    'lane-select': ([('v_readfirstlane_b32','s8, v1','0'),('s_nop','1','4'),('v_readlane_b32','s9, v2, s8','8')],'waits=2 need=4'),
    'vmem': ([('v_readlane_b32','s9, v1, s2','0'),('s_nop','2','4'),('global_load_dword','v2, v3, s[8:9]','8')],'waits=3 need=5'),
    'branch-path': ([('v_readfirstlane_b32','s8, v1','0'),('s_cbranch_scc0','2','4'),('s_nop','4','8'),('s_nop','4','c'),('v_mov_b32','v2, s8','10')],'waits=1 need=2'),
}
SAFE = {
    'constant-after-nop1': [('v_readfirstlane_b32','s8, v1','0'),('s_nop','1','4'),('v_mov_b32','v2, s8','8')],
    'carry-in': [('v_readfirstlane_b32','s8, v1','0'),('v_addc_co_u32','v1, vcc, v2, v3, s[8:9]','4')],
    'salu-read': [('v_readfirstlane_b32','s8, v1','0'),('s_sub_u32','s28, s8, s10','4')],
}

def checker_controls():
    for name,(rows,expect) in SYNTHETIC.items():
        failures = sgpr_hazards(rows)
        assert len(failures) == 1 and failures[0].endswith(expect),(name,failures)
        print(f'CONTROL scheduling-sgpr-{name} CAUGHT {failures[0]}',flush=True)
    for name,rows in SAFE.items():
        assert sgpr_hazards(rows) == [],(name,sgpr_hazards(rows))
        print(f'PASS scheduling-sgpr-{name} no hazard',flush=True)
    return len(SYNTHETIC)

def check(obj):
    rows = instructions(obj)
    failures = hazards(rows)
    assert not failures,f'missing transcendental wait: {obj.name}: {failures}'
    failures = sgpr_hazards(rows)
    assert not failures,f'missing VALU SGPR-write wait: {obj.name}: {failures}'
    return sum(op in TRANS for op,_,_ in rows),sum(op in SGPR_WRITERS for op,_,_ in rows)

def suite(src,out):
    kernels = sorted(p.stem for p in src.glob('*.isa'))
    assert {'vadd','fsum16','sgemm','fp8gemv','bf16gemv','fp8gemv2','bf16gemv2'} <= set(kernels)
    for kernel in kernels:
        obj = out/('hsaco.hsaco' if kernel=='vadd' else kernel+'-hsaco.hsaco')
        count,writes = check(obj)
        print(f'PASS scheduling {kernel} trans_instructions={count} sgpr_writes={writes} rule=CDNA3-p28-29-trans-consumer,valu-sgpr-write',flush=True)
    mutants = (('attn256_merge','\nI 4 0 0\n','no-trans-wait','no-wait','missing transcendental wait:'),
               ('attn256_gate','\nI 4 0 0\n','no-trans-wait','no-wait','missing transcendental wait:'),
               ('readfirst_probe','\nI 4 1 0\n','no-sgpr-wait','no-sgpr-wait','missing VALU SGPR-write wait:'))
    for kernel,site,label,shown,message in mutants:
        source = (src/(kernel+'.isa')).read_text()
        assert site in source,(kernel,site)
        mutant = out/(kernel+'-'+label+'.isa')
        mutant.write_text(source.replace(site,'\n'))
        obj = mutant.with_suffix('.hsaco')
        with obj.open('wb') as stream:
            result = subprocess.run([str(out/'hsaco'),str(mutant)],stdout=stream,stderr=subprocess.PIPE,timeout=30)
        assert result.returncode == 0 and not result.stderr,result
        try: check(obj)
        except AssertionError as error:
            assert str(error).startswith(message),error
            print(f'CONTROL scheduling-{kernel}-{shown} CAUGHT {error}',flush=True)
        else: raise AssertionError(f'scheduling control escaped: {kernel} {label}')
    controls = len(mutants)+checker_controls()
    print(f'PASS scheduling objects={len(kernels)} controls={controls} hardware=NOT_RUN',flush=True)

if __name__=='__main__':
    suite(pathlib.Path(sys.argv[1]),pathlib.Path(sys.argv[2]))
