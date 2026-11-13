#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Independent checkpoint oracle and private-output acceptance for fp8gemv_b128.
Python is a measuring instrument only. Objects are emitted by seeded Omega hsaco.om.
Run under an externally held queen pool lock and nice; see fp8block.md.
"""
import argparse
from fractions import Fraction
import hashlib
import json
from pathlib import Path
import runpy
import shutil
import struct
import subprocess

PIN = '2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f'
NAME = 'fp8gemv_b128'
SHAPES = ((17408, 5120), (5120, 17408), (12288, 5120), (1024, 5120),
          (10240, 5120), (6144, 5120), (5120, 6144))


def sha(path):
    with Path(path).open('rb') as f:
        return hashlib.file_digest(f, 'sha256').hexdigest()


def ocp(byte, sg):
    """F8_E4M3: sign, exponent bias7, 3 fraction bits, signed zero, two NaNs.
    No executor/native-FNUZ conversion routine is used to construct this table.
    """
    sign, exponent, fraction = byte >> 7, (byte >> 3) & 15, byte & 7
    if exponent == 15 and fraction == 7:
        return 0x7fc00000
    if exponent == 0:
        value = Fraction(fraction, 512)
    else:
        value = Fraction(8 + fraction, 8) * Fraction(2) ** (exponent - 7)
    return sg['round32'](-value if sign else value) | (sign << 31)


def result(weights, x, scales, table, sg):
    # Explicit wave order, not a claim of bit equality with serial Rust dot().
    acc = 0
    for base in range(0, len(weights), 128):
        lanes = [0] * 64
        for lane in range(64):
            for col in (base + lane, base + lane + 64):
                if col < len(weights):
                    lanes[lane] = sg['fma'](table[weights[col]], x[col], lanes[lane])
        for xor in (32, 16, 8, 4, 2, 1):
            lanes = [sg['fma'](a, 0x3f800000, lanes[i ^ xor]) for i, a in enumerate(lanes)]
        acc = sg['fma'](scales[base // 128] << 16, lanes[0], acc)
    return acc


def fixture(m, k, rows, table, sg, real=None, all_bytes=False):
    nb, mb = (k + 127) // 128, (m + 127) // 128
    wp = 64
    xp = (wp + m * k + 63) & ~3
    sp = xp + k * 4 + 64
    yp = (sp + mb * nb * 2 + 67) & ~3
    data = bytearray(b'\xa5') * (yp + m * 4 + 64)
    struct.pack_into('<QQQQII', data, 0, wp, xp, sp, yp, m, k)
    x = [sg['round32'](Fraction(((i * 13 + 5) % 31) - 15, 16)) for i in range(k)]
    if all_bytes:
        x = [0x3f800000]
    for i, bits in enumerate(x):
        struct.pack_into('<I', data, xp + i * 4, bits)
    scales = [0x3f00 + ((i * 37 + 11) % 192) for i in range(mb * nb)]
    if all_bytes:
        scales = [0x3f80] * (mb * nb)
    if real:
        scales = list(struct.unpack('<' + 'H' * (mb * nb), real['scales']))
    for i, word in enumerate(scales):
        struct.pack_into('<H', data, sp + i * 2, word)
    expected_rows = {}
    for row in rows:
        weights = bytes((((i * 29 + row * 17 + i // 128) % 127) | ((i + row) % 2 << 7)) for i in range(k))
        if all_bytes:
            weights = bytes([row])
        if real:
            weights = real['rows'][row]
        data[wp + row * k:wp + (row + 1) * k] = weights
        expected_rows[row] = result(weights, x, scales[(row // 128) * nb:(row // 128 + 1) * nb], table, sg)
    expected = bytearray(data)
    for row, bits in expected_rows.items():
        struct.pack_into('<I', expected, yp + row * 4, bits)
    spans = [('kernarg', 0, 40), ('W', wp, m * k), ('x', xp, k * 4),
             ('scale_inv', sp, mb * nb * 2), ('y', yp, m * 4)]
    return data, expected, spans


def mutate(source):
    def replace(text, old, new):
        assert text.count(old) == 1, old
        return text.replace(old, new)
    per_row = replace(source, 'I 5 135 1 1 16\n', 'I 8 257 1 1\n')
    trans = replace(source, 'I 9 12 645 257 4\nI 9 13 646 257 4\n',
                    'I 8 257 1 12\nI 8 128 1 13\n')
    trans = replace(trans, 'I 3 13 0 16\n',
                    'I 2 12 129 4 1\nI 2 4 135 4 30\nI 2 4 129 4 0\n'
                    'I 8 4 1 20\nI 5 129 20 20 18\nI 3 13 0 16\n')
    trans = replace(trans, 'I 5 130 12 12 25\n', 'I 5 276 12 12 25\n')
    tail = replace(source, 'T 1\n', 'T 1\nI 2 16 255 17 1\nL 128\nI 4 @8 5\n')
    return {'per-row-scale': per_row, 'transposed-scale': trans, 'no-tail-block': tail}


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--omc', required=True, type=Path)
    p.add_argument('--weights', required=True, type=Path)
    p.add_argument('--out', required=True, type=Path)
    args = p.parse_args()
    if sha(args.omc) != PIN:
        raise SystemExit(69)
    root = Path(__file__).resolve().parents[2]
    out = args.out.resolve()
    out.mkdir(parents=True, exist_ok=False)
    snapshot = out / 'source'
    snapshot.mkdir()
    sources = [root / 'backend/amd/gfx942' / f for f in
               ('hsaco.om', NAME + '.isa', NAME + '-gfx942.s', 'exec/fp.om', 'exec/fma.om', 'exec/main.om')]
    sources += [Path(__file__).with_name(f) for f in ('sgemm.py', 'row-probe.om', 'token_access.py', 'fp8block.py')]
    for source in sources:
        shutil.copyfile(source, snapshot / source.name)
    source_hashes = {p.name: sha(p) for p in snapshot.iterdir()}
    sg = runpy.run_path(str(snapshot / 'sgemm.py'))
    instrument = runpy.run_path(str(snapshot / 'token_access.py'))['instrument']
    table = [ocp(i, sg) for i in range(256)]
    (out / 'ocp-e4m3-f32.json').write_text(json.dumps(table) + '\n')
    commands = []

    def run(argv, *, stdout=None, input=None, expected=0):
        proc = subprocess.run([str(a) for a in argv], input=input, capture_output=True, timeout=900)
        commands.append({'argv': [str(a) for a in argv], 'exit': proc.returncode,
                         'stdout_sha256': hashlib.sha256(proc.stdout).hexdigest(),
                         'stderr': proc.stderr.decode(errors='replace')})
        (out / 'commands.json').write_text(json.dumps(commands, indent=2) + '\n')
        assert proc.returncode == expected and not proc.stderr, commands[-1]
        if stdout:
            Path(stdout).write_bytes(proc.stdout)
        return proc.stdout

    def compile_source(name, source):
        src = out / (name + '.om')
        src.write_text(source)
        exe = out / name
        run([args.omc], input=source.encode(), stdout=exe)
        exe.chmod(0o755)
        return exe

    producer_source = (snapshot / 'hsaco.om').read_text()
    producer = compile_source('hsaco', producer_source)
    second = run([args.omc], input=producer_source.encode())
    assert second == producer.read_bytes()
    executor_source = ''.join((snapshot / f).read_text() for f in ('fp.om', 'fma.om'))
    executor_source += (snapshot / 'main.om').read_text().split('fn main to int\n')[0]
    executor_source += (snapshot / 'row-probe.om').read_text()
    executor = compile_source('row-exec', executor_source)
    assert run([args.omc], input=executor_source.encode()) == executor.read_bytes()

    def section(path, name):
        # Independent minimal ELF64 section reader, not seeded producer internals.
        blob = Path(path).read_bytes()
        shoff = struct.unpack_from('<Q', blob, 40)[0]
        entsize, count, stridx = struct.unpack_from('<HHH', blob, 58)
        entries = [struct.unpack_from('<IIQQQQIIQQ', blob, shoff + i * entsize) for i in range(count)]
        names = blob[entries[stridx][4]:entries[stridx][4] + entries[stridx][5]]
        for entry in entries:
            end = names.index(0, entry[0])
            if names[entry[0]:end].decode() == name:
                return blob[entry[4]:entry[4] + entry[5]]
        raise AssertionError(name)

    texts, objects = {}, {}
    original = (snapshot / (NAME + '.isa')).read_text()
    variants = {NAME: original, **{NAME + '-' + k: v for k, v in mutate(original).items()}}
    for label, text in variants.items():
        isa = out / (label + '.isa')
        isa.write_text(text)
        obj = out / (label + '.hsaco')
        run([producer, isa], stdout=obj)
        assert run([producer, isa]) == obj.read_bytes()
        raw = out / (label + '.text')
        raw.write_bytes(section(obj, '.text'))
        texts[label], objects[label] = raw, obj
    llvm_obj = out / 'llvm-oracle.o'
    run(['llvm-mc-18', '-triple=amdgcn-amd-amdhsa', '-mcpu=gfx942', '-filetype=obj',
         snapshot / (NAME + '-gfx942.s'), '-o', llvm_obj])
    for name in ('.text', '.note'):
        assert section(objects[NAME], name) == section(llvm_obj, name), name
    a, b = section(objects[NAME], '.rodata'), section(llvm_obj, '.rodata')
    assert len(a) == len(b) == 64 and a[:16] + a[24:] == b[:16] + b[24:], 'descriptor'
    print('PASS seeded deterministic producer/executor; LLVM text, metadata, descriptor (entry relocation excluded)', flush=True)
    cases, bounds, controls = [], [], []
    memory = out / 'memory.bin'

    def execute(exe, text, data, rows, m):
        memory.write_bytes(data)
        stdout = run([exe, text, memory, *rows, m])
        actual = memory.read_bytes()
        return stdout, actual

    def check(label, m, k, rows, real=None, all_bytes=False, measure=False):
        data, expected, spans = fixture(m, k, rows, table, sg, real, all_bytes)
        exe = executor
        if measure:
            exe = compile_source('access-' + label, instrument(executor_source, [(p, n) for _, p, n in spans]))
        stdout, actual = execute(exe, texts[NAME], data, rows, m)
        if actual != expected:
            first = next(i for i, (a, b) in enumerate(zip(actual, expected)) if a != b)
            raise AssertionError(f'{label} byte {first}: actual={actual[first:first+4].hex()} expected={expected[first:first+4].hex()}')
        if measure:
            hits = struct.unpack('<' + 'Q' * (2 * len(spans)), stdout)
            access = [{'name': name, 'bytes': n, 'max_read_offset': hits[i*2]-1 if hits[i*2] else None,
                       'max_write_offset': hits[i*2+1]-1 if hits[i*2+1] else None}
                      for i, (name, _, n) in enumerate(spans)]
            bounds.append({'case': label, 'M': m, 'K': k, 'sampled_rows': rows, 'buffers': access})
        else:
            assert not stdout
        cases.append({'case': label, 'M': m, 'K': k, 'rows': rows, 'overdispatch_row': m,
                      'comparison': 'every memory byte; exact RN-even Fraction wave oracle',
                      'checkpoint': real['evidence'] if real else None})
        print(f'PASS {label} M={m} K={k} rows={rows} overdispatch={m} all-memory-byte-equal', flush=True)
        return data, expected, spans

    check('all-ocp-bytes', 256, 1, list(range(256)), all_bytes=True)
    for m, k in ((0, 1), (3, 0), (3, 1), (3, 63), (3, 64), (3, 65), (3, 127),
                 (3, 128), (3, 129), (3, 191), (3, 255), (3, 256), (257, 259)):
        rows = list(range(m)) if m < 4 else [0, 1, 127, 128, 255, 256]
        check(f'tail-{m}-{k}', m, k, rows, measure=(m, k) == (257, 259))

    manifest = Path(str(args.weights) + '.sha256')
    pins = {line.split()[1]: line.split()[0] for line in manifest.read_text().splitlines()}
    checkpoint_evidence = {'manifest_sha256': sha(manifest), 'shards': []}
    found_shapes = set()
    for layer in (0, 3):
        shard = args.weights / f'layers-{layer}.safetensors'
        digest = sha(shard)
        assert digest == pins[shard.name], (shard.name, digest)
        checkpoint_evidence['shards'].append({'file': shard.name, 'sha256': digest})
        with shard.open('rb') as f:
            header_size, = struct.unpack('<Q', f.read(8))
            header = json.loads(f.read(header_size))
            start = header_size + 8
            for name, tensor in header.items():
                if not isinstance(tensor, dict) or tensor.get('dtype') != 'F8_E4M3':
                    continue
                shape = tuple(tensor['shape'])
                if shape not in SHAPES or shape in found_shapes:
                    continue
                m, k = shape
                rows = sorted({0, 1, 127, 128, m//2, m-1})
                scale_name = name + '_scale_inv'
                scale = header[scale_name]
                assert scale['dtype'] == 'BF16' and scale['shape'] == [(m+127)//128, (k+127)//128]
                lo, hi = scale['data_offsets']
                f.seek(start + lo)
                scales = f.read(hi-lo)
                weights = {}
                for row in rows:
                    f.seek(start + tensor['data_offsets'][0] + row * k)
                    weights[row] = f.read(k)
                    assert len(weights[row]) == k
                evidence = {'shard': shard.name, 'shard_sha256': digest, 'weight': name,
                            'weight_data_offsets': tensor['data_offsets'], 'scale': scale_name,
                            'scale_sha256': hashlib.sha256(scales).hexdigest(),
                            'row_sha256': {str(r): hashlib.sha256(w).hexdigest() for r, w in weights.items()}}
                real = {'rows': weights, 'scales': scales, 'evidence': evidence}
                check(f'checkpoint-{m}-{k}', m, k, rows, real=real, measure=True)
                found_shapes.add(shape)
    assert found_shapes == set(SHAPES), found_shapes

    # Non-square scale grid prevents a transposition from accidentally being the identity.
    for mutant in mutate(original):
        rows = [1] if mutant == 'per-row-scale' else [128, 256]
        mutant_data, mutant_expected, spans = fixture(257, 389, rows, table, sg)
        exe = compile_source('access-' + mutant, instrument(executor_source, [(p, n) for _, p, n in spans]))
        stdout, actual = execute(exe, texts[NAME + '-' + mutant], mutant_data, rows, 257)
        assert actual != mutant_expected, f'escaped {mutant}'
        hits = struct.unpack('<' + 'Q' * (2 * len(spans)), stdout)
        access = [{'name': name, 'bytes': n, 'max_read_offset': hits[i*2]-1 if hits[i*2] else None,
                   'max_write_offset': hits[i*2+1]-1 if hits[i*2+1] else None}
                  for i, (name, _, n) in enumerate(spans)]
        bounds.append({'case': mutant, 'M': 257, 'K': 389, 'sampled_rows': rows, 'buffers': access})
        controls.append({'mutant': mutant, 'exit': 0, 'result': 'CAUGHT numerical mismatch'})
        print(f'CONTROL {mutant} CAUGHT numerical mismatch, executor exit=0', flush=True)

    # Each buffer's measured last byte is protected by a separate one-byte-short boundary.
    data, expected, spans = fixture(257, 259, [0, 1, 127, 128, 255, 256], table, sg)
    measured = next(b for b in bounds if b['case'] == 'tail-257-259')['buffers']
    for index, (name, base, length) in enumerate(spans):
        last = max(x for x in (measured[index]['max_read_offset'], measured[index]['max_write_offset']) if x is not None)
        short = [(p, n) for _, p, n in spans]
        short[index] = (base, last)
        exe = compile_source('short-' + name, instrument(executor_source, short))
        memory.write_bytes(data)
        assert run([exe, texts[NAME], memory, 0, 1, 127, 128, 255, 256, 257], expected=68) == b''
        controls.append({'buffer': name, 'result': 'CAUGHT one-byte-short span', 'exit': 68})
        print(f'CONTROL short-{name} CAUGHT exit=68', flush=True)
    memory.unlink()
    report = {'schema': 'fp8block-acceptance-v1', 'status': 'EXERCISED', 'hardware': 'NOT_RUN',
              'performance': 'NOT_RUN', 'compiler_sha256': PIN, 'source_sha256': source_hashes,
              'checkpoint': checkpoint_evidence, 'decode_table_sha256': sha(out / 'ocp-e4m3-f32.json'),
              'kernarg_size': 40, 'grid_workgroups': ['M', 1, 1], 'block': [64, 1, 1],
              'objects': {k: {'file': v.name, 'sha256': sha(v)} for k, v in objects.items()},
              'cases': cases, 'bounds': bounds, 'controls': controls}
    (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(f'PASS fp8gemv_b128 cases={len(cases)} controls={len(controls)} hardware=NOT_RUN performance=NOT_RUN', flush=True)


if __name__ == '__main__':
    main()
