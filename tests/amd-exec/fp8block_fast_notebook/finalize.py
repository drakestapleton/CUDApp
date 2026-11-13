#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Generate notebook evidence index and checksums from captured acceptance data."""
import argparse
import hashlib
import json
from pathlib import Path
import platform
import sys
import numpy

HERE=Path(__file__).resolve().parent

def digest(p): return hashlib.sha256(p.read_bytes()).hexdigest()

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--log',type=Path,action='append',required=True)
    parser.add_argument('--pool-lock',required=True,type=Path)
    args=parser.parse_args()
    report=json.loads((HERE/'selfcheck.json').read_text())
    out=Path(report['runs'][0]['output']).parent.parent
    record=dict(interpreter=sys.executable,python=platform.python_version(),numpy=numpy.__version__,
                cwd=str(HERE),cpu_lock=str(args.pool_lock.resolve()),nice=10,
                build_argv=[sys.executable,'build.py'],selfcheck_argv=[sys.executable,'selfcheck.py','--out',str(out)],
                child_exits=[dict(mode=r['mode'],exit=r['exit'],result=r['result']) for r in report['runs']],
                selfcheck_report=dict(file='selfcheck.json',sha256=digest(HERE/'selfcheck.json')),
                logs=[dict(path=str(p),sha256=digest(p)) for p in args.log],
                hardware='NOT_RUN',performance='NOT_RUN')
    (HERE/'cell-commands.json').write_text(json.dumps(record,indent=2)+'\n')
    (HERE/'selfcheck.log').write_bytes(args.log[-1].read_bytes())
    files=sorted(p for p in HERE.rglob('*') if p.is_file() and p.name!='SHA256SUMS' and '__pycache__' not in p.parts)
    (HERE/'SHA256SUMS').write_text(''.join('%s  %s\n' % (digest(p),p.relative_to(HERE)) for p in files))
    build=json.loads((HERE/'build.json').read_text())
    print('FINAL cell=%s bytes=%d sha256=%s checksummed_files=%d' % (build['file'],build['bytes'],build['sha256'],len(files)))

if __name__=='__main__': main()
