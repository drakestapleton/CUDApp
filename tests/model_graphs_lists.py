#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Build both actual graph TUs; compare actual list banks/QMDs on a fake platform.
Run only in the device-free namespace documented in om/graph/CHIP.md.
"""
import os
from pathlib import Path
import shlex
import subprocess
ROOT=Path(__file__).resolve().parents[1]
os.chdir(ROOT)
OUT=ROOT/'build/model-graphs-lists'
OUT.mkdir(parents=True,exist_ok=True)
CC=shlex.split(os.environ.get('CC','cc'))
flags=['-std=gnu11','-D_GNU_SOURCE','-O2','-Wall','-Wextra','-Werror','-Wno-unused-function','-ffunction-sections','-fdata-sections','-Isrc','-Itests','-Im16','-Invrm']
def run(cmd):
    print('+',shlex.join(map(str,cmd)),flush=True)
    subprocess.run(list(map(str,cmd)),check=True)
objects=[]
for family in ['q36','q38','decode']:
    for mode in ['old','new']:
        obj=OUT/f'{family}-{mode}.o'
        run(CC+flags+(['-DCUDAPP_MIGRATION_PENDING_C_GRAPHS=1'] if mode=='old' else [])+['-c',f'tests/model_graphs_{family}_fixture.c','-o',obj])
        symbols=subprocess.check_output(['nm','--defined-only','--extern-only',str(obj)],text=True)
        args=[]
        for line in symbols.splitlines():
            name=line.split()[-1]
            args+=['--redefine-sym',f'{name}={mode}_{name}']
        run(['objcopy',*args,obj])
        objects.append(obj)
wrappers=(ROOT/'tests/model_graphs_wraps.txt').read_text().splitlines()
binary=OUT/'check'
run(CC+flags+['tests/test_model_graphs_lists.c',*objects,'build/libomega_gpu.a','-Wl,--gc-sections',*[f'-Wl,--wrap={s}' for s in wrappers],'-lpthread','-lm','-o',binary])
run([binary])
