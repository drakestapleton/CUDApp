#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Notebook-glue only: derive the immutable v13 ABI/evidence bundle from a run."""
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys


def sha(path):
    with Path(path).open('rb') as f:
        return hashlib.file_digest(f, 'sha256').hexdigest()


def main(run_dir, destination, baseline_dir):
    run, dest, base = map(lambda p: Path(p).resolve(), (run_dir, destination, baseline_dir))
    report = json.loads((run / 'report.json').read_text())
    assert report['status'] == 'EXERCISED'
    root = Path(__file__).resolve().parents[2]
    head = subprocess.check_output(['git', '-C', str(root), 'rev-parse', 'HEAD'], text=True).strip()
    own_paths = {'fp8gemv_b128f.isa': 'backend/amd/gfx942/fp8gemv_b128f.isa',
                 'fp8gemv_b128f-gfx942.s': 'backend/amd/gfx942/fp8gemv_b128f-gfx942.s',
                 'fp8block_fast.py': 'tests/amd-exec/fp8block_fast.py'}
    for name, path in own_paths.items():
        committed = subprocess.check_output(['git', '-C', str(root), 'show', head + ':' + path])
        assert hashlib.sha256(committed).hexdigest() == report['source_sha256'][name], path
    # Input validation happens before creating the destination. Never overwrite a bundle.
    for entry in report['objects'].values():
        assert sha(run / entry['file']) == entry['sha256'], entry
    baseline = json.loads((base / 'list.json').read_text())
    assert baseline['compiler_sha256'] == report['compiler_sha256']
    previous = next(k for k in baseline['kernels'] if k['name'] == 'fp8gemv_b128')
    previous['objects'] = [o for o in previous['objects'] if o['role'] == 'kernel']
    assert len(previous['objects']) == 1
    for obj in previous['objects']:
        assert sha(base / obj['file']) == obj['sha256']
        table = Path(obj['file']).with_suffix('.isa')
        assert sha(base / table) == obj['table_sha256']
        committed = subprocess.check_output(['git', '-C', str(root), 'show',
                                            baseline['source_head'] + ':backend/amd/gfx942/' + str(table)])
        assert hashlib.sha256(committed).hexdigest() == obj['table_sha256']
    notebook = Path(__file__).with_name('fp8block_fast_notebook')
    for p in notebook.iterdir():
        if p.is_file():
            committed = subprocess.check_output(['git', '-C', str(root), 'show', head + ':' + str(p.relative_to(root))])
            assert hashlib.sha256(committed).hexdigest() == sha(p)
    dest.mkdir(parents=True, exist_ok=False)
    shutil.copytree(run / 'source', dest / 'source')
    shutil.copytree(base / 'source', dest / 'baseline/source')
    for file in ('list.json', 'report.json', 'commands.json'):
        shutil.copyfile(base / file, dest / 'baseline' / file)
    for obj in previous['objects']:
        shutil.copyfile(base / obj['file'], dest / 'baseline' / obj['file'])
        table = Path(obj['file']).with_suffix('.isa')
        shutil.copyfile(base / table, dest / 'baseline' / table)
        obj['file'] = 'baseline/' + obj['file']
    for p in notebook.iterdir():
        if p.is_file():
            shutil.copyfile(p, dest / p.name)
    for file in ('report.json', 'commands.json', 'ocp-e4m3-f32.json'):
        shutil.copyfile(run / file, dest / file)
    source = (run / 'source/fp8gemv_b128f.isa').read_text()
    args, kernarg = [], None
    for line in source.splitlines():
        words = line.split()
        if words and words[0] == 'K':
            kernarg = int(words[2])
        if words and words[0] == 'A':
            args.append({'name': words[1], 'offset': int(words[2]), 'size': int(words[3]),
                         'kind': 'global_buffer' if words[4] == '0' else 'by_value'})
    assert kernarg == report['kernarg_size']
    objects = []
    for name, entry in report['objects'].items():
        shutil.copyfile(run / entry['file'], dest / entry['file'])
        shutil.copyfile(run / (name + '.isa'), dest / (name + '.isa'))
        suffix = name.removeprefix('fp8gemv_b128f-')
        access = [b for b in report['bounds'] if b['case'] == suffix] if name != 'fp8gemv_b128f' else [
            b for b in report['bounds'] if b['case'].startswith(('checkpoint-', 'tail-'))]
        assert access
        objects.append({**entry, 'entry': 'fp8gemv_b128f', 'bytes': (dest / entry['file']).stat().st_size,
                        'role': 'kernel' if name == 'fp8gemv_b128f' else 'deliberately_wrong_control',
                        'table_sha256': sha(dest / (name + '.isa')), 'access': access})
    manifest = {'schema': 'gfx942-notebook-bundle-v13', 'source_head': head,
                'compiler_sha256': report['compiler_sha256'], 'hardware': 'NOT_RUN',
                'performance': 'NOT_RUN', 'evidence': {
                    file: {'file': file, 'sha256': sha(dest / file)}
                    for file in ('report.json', 'commands.json', 'ocp-e4m3-f32.json')},
                'kernels': [{'name': 'fp8gemv_b128f', 'kernarg_size': kernarg, 'kernarg': args,
                             'grid_workgroups': report['grid_workgroups'], 'block': report['block'],
                             'scale_layout': 'BF16 [ceil(M/128)][ceil(K/128)] row-major',
                             'weight_encoding': 'OCP F8_E4M3, unmodified checkpoint bytes',
                             'cases': report['cases'], 'controls_caught': report['controls'],
                             'objects': objects}]}
    manifest['baseline_source_head'] = baseline['source_head']
    manifest['kernels'].append(previous)
    (dest / 'list.json').write_text(json.dumps(manifest, indent=2) + '\n')
    lines = ['# gfx942 checkpoint block-FP8 kernel bundle', '', f'Source head: `{head}`.',
             f"Seeded compiler SHA256: `{report['compiler_sha256']}`.",
             'Hardware: **NOT_RUN**. Performance: **NOT_RUN**.', '',
             'Objects are emitted by seeded Omega `hsaco.om`; LLVM is an independent encoding oracle.',
             'Real-shape tests retain full dimensions/strides but execute the listed sampled rows.',
             'The full acceptance report, executed commands and exact input source snapshots are included.', '',
             '| Object | Role | Bytes | SHA256 |', '|---|---|---:|---|']
    for obj in objects:
        lines.append(f"| {obj['file']} | {obj['role']} | {obj['bytes']} | `{obj['sha256']}` |")
    for obj in previous['objects']:
        lines.append(f"| {obj['file']} | baseline kernel | {obj['bytes']} | `{obj['sha256']}` |")
    lines += ['', f"Baseline source head: `{baseline['source_head']}`; original acceptance in `baseline/`."]
    lines += ['', '## fp8gemv_b128f ABI', '', f'Kernarg bytes: **{kernarg}**.',
              f"Workgroup grid: `{report['grid_workgroups']}`. Threads per workgroup: `{report['block']}`.",
              'W: raw OCP E4M3; x/y: FP32; scale_inv: packed BF16 row-major block grid.', '',
              '| Argument | Offset | Bytes | Kind |', '|---|---:|---:|---|']
    for arg in args:
        lines.append(f"| {arg['name']} | {arg['offset']} | {arg['size']} | {arg['kind']} |")
    lines += ['', '## Measured per-buffer bounds', '',
              'Offsets are inclusive bytes from each buffer base; `None` means no access.',
              'Every measurement is executor evidence, not a device memory trace.', '']
    for obj in objects:
        lines += [f"### {obj['file']}", '']
        for bound in obj['access']:
            lines += [f"Case `{bound['case']}`: M={bound['M']}, K={bound['K']}; rows={bound['sampled_rows']}.", '',
                      '| Buffer | Allocated bytes | Last read | Last write |', '|---|---:|---:|---:|']
            for buf in bound['buffers']:
                lines.append(f"| {buf['name']} | {buf['bytes']} | {buf['max_read_offset']} | {buf['max_write_offset']} |")
            lines.append('')
    lines += ['## Negative controls', '']
    for control in report['controls']:
        lines.append(f"- {control.get('mutant', control.get('buffer'))}: {control['result']}; exit {control['exit']}.")
    (dest / 'list.md').write_text('\n'.join(lines) + '\n')
    files = sorted(p for p in dest.rglob('*') if p.is_file())
    (dest / 'SHA256SUMS').write_text(''.join(f'{sha(p)}  {p.relative_to(dest)}\n' for p in files))
    print(json.dumps({'bundle': str(dest), 'source_head': head, 'objects': report['objects']}, indent=2))


if __name__ == '__main__':
    main(*sys.argv[1:])
