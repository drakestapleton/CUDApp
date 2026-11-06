#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Compare real production nested recorders; fake resources, not fake recorders."""
import os
from pathlib import Path
import shlex
import subprocess

ROOT = Path(__file__).resolve().parents[1]
BUILD = Path(os.environ.get('OUT_DIR', ROOT / 'build'))
OUT = BUILD / 'model-graphs-nested'
CC = shlex.split(os.environ.get('CC', 'cc'))
FLAGS = ['-std=gnu11','-D_GNU_SOURCE','-O2','-Wall','-Wextra','-Werror','-Wno-unused-function','-ffunction-sections','-fdata-sections','-Isrc','-Itests','-Im16','-Invrm']

def run(cmd):
    subprocess.run([str(x) for x in cmd], cwd=ROOT, check=True)

binaries = {}
for variant in ('old','new','flip'):
    # These OUT_DIRs never share graph objects, including legacy-selector builds.
    out = OUT / variant
    out.mkdir(parents=True, exist_ok=True)
    objects=[]
    for family in ('FP8','BF16','GDN','ATTN','ATTN256','ROUTER'):
        obj=out / (family.lower()+'.o')
        run(CC+FLAGS+['-DMG_NEST_'+family,'-c','tests/model_graphs_nested_objects.c','-o',obj]);objects.append(obj)
    for family in ('q36','q38','decode'):
        obj=out / (family+'.o')
        defs=['-DMODEL_GRAPHS_NESTED']
        if variant=='old':defs+=['-DCUDAPP_MIGRATION_PENDING_C_GRAPHS=1']
        run(CC+FLAGS+defs+['-c',f'tests/model_graphs_{family}_fixture.c','-o',obj]);objects.append(obj)
    binary=out/'nested'
    neg=['-DMG_NEGATIVE','-Wl,--wrap=omega_mg_node'] if variant=='flip' else []
    run(CC+FLAGS+neg+['tests/test_model_graphs_nested.c','tests/model_graphs_nested_native.c',*objects,BUILD/'libomega_gpu.a','-Wl,--gc-sections','-lpthread','-lm','-o',binary])
    binaries[variant]=binary

# family, graph mode, attention, conversation heads, feature flags, position, layers.
# Process isolation also prevents one shape's pinned production kernel from
# invalidating another shape's residency contract.
cases=[]
for pos in (0,1,255,256,131064,131071):
    for attention in (0,1):
        cases += [(36,8,attention,1,0,pos,2),(38,2,attention,1,0,pos,2),(38,2,attention,1,1280,pos,2),(30,2,0,1,0,pos,2)]
# Batched independent conversations, contiguous prefill, expert/router GEMM,
# optional FP8 output head, BF16 9B split down, exact BF16 masked head.
for attention in (0,1):
    cases += [(36,2,attention,2,0,255,2),(36,3,attention,2,48,255,2),(36,4,attention,2,0,255,2),(36,6,attention,2,0,255,2),(36,7,attention,2,0,255,2),(36,8,attention,1,4,255,2),(38,2,attention,1,1290,255,2)]
cases += [(36,5,0,2,0,0,2),(38,1,0,1,512,0,2)]
for index,case in enumerate(cases):
    blobs=[]
    for variant in ('old','new'):
        capture=OUT/variant/'capture.bin'
        run([binaries[variant],*case,capture])
        blobs.append(capture.read_bytes())
    if blobs[0]!=blobs[1]:
        raise SystemExit(f'FAIL nested byte comparison case={index} spec={case}')
# A real plan descriptor is corrupted, not the comparison buffer or a recorder.
case=cases[0]
run([binaries['new'],*case,OUT/'new/negative-reference.bin'])
run([binaries['flip'],*case,OUT/'flip/negative.bin'])
if (OUT/'new/negative-reference.bin').read_bytes()==(OUT/'flip/negative.bin').read_bytes():
    raise SystemExit('FAIL nested plan-word flip survived')
print(f'MODEL-GRAPHS-NESTED PASS cases={len(cases)} real-recorders=GDN,attention,FP8,BF16,GEMM,router old-submissions=1 new-submissions=1 plan-word-flip=KILLED')
