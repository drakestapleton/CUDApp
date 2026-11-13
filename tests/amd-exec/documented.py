#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Run the shell blocks in this directory's Markdown verbatim, from the repo root.
Caller supplies CPU isolation, PATH, OMEGA_OMC, AMD_WEIGHTS and AMD_DOC_OUT.
No safe-path flags or substitute interpreter are injected into documented commands.
"""
import hashlib
import json
import os
import pathlib
import re
import shutil
import subprocess
import sys


def main():
    root = pathlib.Path(__file__).resolve().parents[2]
    out = pathlib.Path(os.environ['AMD_DOC_OUT']).resolve()
    out.mkdir(parents=True, exist_ok=False)
    records = []
    failed = False
    not_run = False
    for doc in sorted((root / 'tests/amd-exec').glob('*.md')):
        text = doc.read_text()
        blocks = re.findall(r'^```sh\n(.*?)^```', text, re.M | re.S)
        assert blocks, f'no executable shell block: {doc}'
        for index, command in enumerate(blocks):
            assert not re.search(r'\bpython\S*\s+-(?:P|I)\b', command), 'documented safe-path workaround'
            name = f'{doc.stem}-{index}'
            log = out / (name + '.log')
            interpreter = shutil.which(command.split()[0])
            print(f'DOCUMENTED command={command!r} interpreter={interpreter}', flush=True)
            with log.open('wb') as f:
                p = subprocess.run(['sh', '-eu', '-c', command], cwd=root, stdout=f, stderr=subprocess.STDOUT)
            data = log.read_bytes()
            missing = b'ModuleNotFoundError:' in data or p.returncode == 127
            status = 'PASS' if p.returncode == 0 else 'NOT_RUN' if missing else 'FAIL'
            records.append({'document': str(doc.relative_to(root)), 'command': command,
                            'interpreter': interpreter, 'exit': p.returncode,
                            'status': status, 'log': str(log), 'sha256': hashlib.sha256(data).hexdigest()})
            (out / 'commands.json').write_text(json.dumps(records, indent=2) + '\n')
            print('DOCUMENTED ' + json.dumps(records[-1]), flush=True)
            failed |= status == 'FAIL'
            not_run |= status == 'NOT_RUN'
            if p.returncode:
                # Dependent bundle generation cannot produce evidence after a failed run.
                break
    return 1 if failed else 77 if not_run else 0


if __name__ == '__main__':
    sys.exit(main())
