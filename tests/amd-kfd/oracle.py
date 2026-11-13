# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Independent host-test instrument only. Never imported by a producer or launcher.

AQL expected bytes: Python struct's standard little-endian sizes, independent of Omega's
layout lowering, applied to the public HSA hsa_kernel_dispatch_packet_t definition and enum
bit positions in ROCR-Runtime rocm-6.2.0 src/inc/hsa.h (digest in PROVENANCE.md).
Kernarg expected bytes: read the actual .args entries and segment size of the assembly oracle,
not a copied 28-byte golden. KFD expected bytes: use the live C sizeof/offsetof oracle's table,
not the table that the Omega serializer consumes. Neither test touches a device.
"""
import pathlib
import re
import struct
import subprocess
import runpy
import sys


def table(path):
    rows = []
    for line in pathlib.Path(path).read_text().splitlines():
        nums, label = line.split('#', 1)
        rows.append((tuple(map(int, nums.split())), label.strip()))
    result = []
    i = 0
    while i < len(rows):
        (req, size, count), name = rows[i]
        fields = rows[i + 1:i + 1 + count]
        assert len(fields) == count and size > 0
        for (off, width, repeat), _ in fields:
            assert width in (4, 8) and repeat > 0 and off + width * repeat <= size
        result.append((req, size, name, fields))
        i += count + 1
    return result


def exercise(root, out):
    root, out = pathlib.Path(root), pathlib.Path(out)
    events = []
    exits = {}
    controls = runpy.run_path(str(root / 'tests/amd-kfd/control.py'))
    matches = controls['matches']

    def caught(label, name):
        controls['prove'](out, label, 'exit', exits[name], 0)

    def run(name, exe, args, want=0, expected=None, timeout=10):
        cmd = [str(out / exe), *map(str, args)]
        p = subprocess.run(cmd, capture_output=True, timeout=timeout)
        (out / (name + '.stdout')).write_bytes(p.stdout)
        (out / (name + '.stderr')).write_bytes(p.stderr)
        (out / (name + '.command')).write_text(' '.join(cmd) + '\nexit=' + str(p.returncode) + '\n')
        exits[name] = p.returncode
        if want == 0:
            assert matches('exit', p.returncode, 0), (name, p.returncode, p.stderr)
        else:
            assert not matches('exit', p.returncode, 0), (name, 'unexpected acceptance')
        assert p.returncode == want, (name, p.returncode, want, p.stderr)
        if expected is not None:
            (out / (name + '.expected')).write_bytes(expected)
            assert p.stdout == expected, (name, p.stdout.hex(), expected.hex())
        events.append(name)
        return p.stdout

    # Two distinct fixed launches: all dispatch fields, high halves of every address,
    # all three dimensions, barrier 0/1 and the three fence-scope enums are represented.
    launches = [
        (1, 64, 1, 1, 1024, 1, 1, 0, 0, 0x0102030405060708, 0x1112131415161718,
         0x2122232425262728, 1, 2, 2),
        (3, 8, 4, 2, 32, 8, 6, 20, 4096, 0xfedcba9876543200, 0x8877665544332200,
         0xffeeddccbbaa9988, 0, 1, 0),
    ]
    for i, v in enumerate(launches):
        dims, wx, wy, wz, gx, gy, gz, priv, group, ko, ka, sig, barrier, acq, rel = v
        header = 2 | barrier << 8 | acq << 9 | rel << 11
        expected = struct.pack('<6H5I4Q', header, dims, wx, wy, wz, 0,
                               gx, gy, gz, priv, group, ko, ka, 0, sig)
        assert len(expected) == 64
        first = run('aql-' + str(i), 'aql', v, expected=expected)
        assert run('aql-repeat-' + str(i), 'aql.repeat', v) == first
    print('PASS AQL two fixed launches: independent HSA definition + struct.pack <6H5I4Q')
    for i, (field, value) in enumerate(((4, 0), (0, 0), (1, 0), (13, 3), (1, 65536),
                                       (7, 4294967296), (5, 2), (4, 1))):
        v = list(launches[0])
        v[field] = value
        run('aql-invalid-' + str(i), 'aql', v, 3, b'')
    caught('grid-size-zero', 'aql-invalid-0')
    for v in ('-1', '18446744073709551616', '0x10000000000000000', 'no', '0x', ''):
        args = list(launches[0])
        args[9] = v
        run('aql-number-' + str(len(events)), 'aql', args, 2, b'')
    print('PASS AQL field rules and overflowing/malformed numeric refusals')

    asm = (root / 'backend/amd/vadd-gfx1151.s').read_text()
    assert re.search(r'amdhsa.version:\s*\[\s*1,\s*2\s*\]', asm), 'requires v5 metadata 1.2'
    size = int(re.search(r'\.kernarg_segment_size:\s*(\d+)', asm).group(1))
    kdsize = int(re.search(r'\.amdhsa_kernarg_size\s+(\d+)', asm).group(1))
    assert size == kdsize == 28
    assert re.search(r'\.kernarg_segment_align:\s*8\b', asm)
    entries = re.findall(r'\{\s*\.name:\s*(\w+),\s*\.offset:\s*(\d+),\s*\.size:\s*(\d+),\s*\.value_kind:\s*(\w+)[^}]*\}', asm)
    assert [(n, int(o), int(s), k) for n, o, s, k in entries] == [
        ('a', 0, 8, 'global_buffer'), ('b', 8, 8, 'global_buffer'),
        ('c', 16, 8, 'global_buffer'), ('count', 24, 4, 'by_value')]
    assert 'hidden_' not in asm
    values = (0x0102030405060708, 0x8899aabbccddeeff, 0xfedcba9876543210, 1024)
    expected = bytearray(size)
    for (_, off, width, _), value in zip(entries, values):
        off, width = int(off), int(width)
        expected[off:off + width] = value.to_bytes(width, 'little')
    first = run('kernarg', 'kernarg', values, expected=expected)
    assert run('kernarg-repeat', 'kernarg.repeat', values) == first
    run('kernarg-short', 'kernarg', [*values, size - 1], 3, b'')
    run('kernarg-count-overflow', 'kernarg', [*values[:3], 4294967296], 3, b'')
    print('PASS kernarg assembly metadata: code-object-v5 metadata 1.2; four explicit arguments; no hidden args')
    caught('kernarg-one-byte-short', 'kernarg-short')

    src_table = root / 'backend/amd/kfd/requests.tsv'
    native_table = out / 'native-table.tsv'
    def matches_header(path):
        return matches('table', path, native_table)

    assert matches_header(src_table), 'header and repository table differ'
    oracle = table(native_table)
    with (out / 'request-list.tsv').open('w') as report:
        for idx, (req, size, name, fields) in enumerate(oracle):
            if req:
                report.write(f'{name}\t{req}\t0x{req:08x}\t{size}\n')
            values = []
            expected = bytearray(size)
            for j, ((off, width, repeat), label) in enumerate(fields):
                # Non-palindromic values prove byte order. Pads/union unused space remain zero.
                value = 0 if label == 'pad' else (0x0102030405060708 + j if width == 8 else 0x11223344 + j)
                for k in range(repeat):
                    values.append(value)
                    p = off + k * width
                    expected[p:p + width] = value.to_bytes(width, 'little')
            first = run('record-' + name, 'records', [src_table, idx, *values], expected=expected)
            assert run('record-repeat-' + name, 'records.repeat', [src_table, idx, *values]) == first
            run('zero-record-' + name, 'records', [src_table, idx], expected=bytes(size))
    print(f'PASS KFD {sum(bool(r[0]) for r in oracle)} requests + auxiliary pointed records: every serialized field vs live header oracle')

    # One request number is changed. Comparing the *same* complete live oracle catches it.
    lines = src_table.read_text().splitlines()
    nums, comment = lines[0].split('#', 1)
    a, b, c = map(int, nums.split())
    lines[0] = f'{a + 1}\t{b}\t{c}\t# {comment.strip()}'
    wrong_req = out / 'wrong-request.tsv'
    wrong_req.write_text('\n'.join(lines) + '\n')
    assert not matches_header(wrong_req)
    # records never emits the request number, so the header comparison is the only guard for it.
    controls['prove'](out, 'wrong-request-number', 'table', wrong_req, native_table)

    # One extra u32 field in the record extent: eight bytes becomes twelve, independently caught
    # both by the header table compare and the exact serialized record length/bytes comparison.
    lines = src_table.read_text().splitlines()
    lines[0] = f'{a}\t{b + 4}\t{c}\t# {comment.strip()}'
    wrong_size = out / 'one-field-too-large.tsv'
    wrong_size.write_text('\n'.join(lines) + '\n')
    assert not matches_header(wrong_size)
    changed = run('record-off-by-one-field', 'records', [wrong_size, 0, 1, 14])
    assert len(changed) == 12 and len(changed) != oracle[0][1]
    controls['prove'](out, 'record-off-by-one-field', 'table', wrong_size, native_table)
    lines = src_table.read_text().splitlines()
    nums, comment = lines[2].split('#', 1)
    off, width, count = map(int, nums.split())
    lines[2] = f'{off + 1}\t{width}\t{count}\t# {comment.strip()}'
    wrong_off = out / 'wrong-field-offset.tsv'
    wrong_off.write_text('\n'.join(lines) + '\n')
    assert not matches_header(wrong_off)
    run('record-field-offset', 'records', [wrong_off, 0, 1, 14], 3, b'')
    controls['prove'](out, 'field-offset-plus-one', 'table', wrong_off, native_table)
    # Generate both upper-domain edges, including the wrap case, from the wire width.
    indices = runpy.run_path(str(root / 'tests/amd/boundaries.py'))['record_indices']()
    for index in indices:
        run('record-index-' + str(index), 'records', [src_table, index], 3, b'')
    run('record-index-past-end', 'records', [src_table, len(oracle)], 3, b'')
    print(f'PASS records refuses generated upper-domain indices {indices} and first past-end index')
    source = (root / 'backend/amd/kfd/records.om').read_text()
    guard = 'if eq idx 18446744073709551615\nret 3\nend\n'
    assert source.count(guard) == 1, 'index guard mutant did not apply'
    mutant = out / 'records-index-mutant.om'
    mutant.write_text(source.replace(guard, '') + (root / 'backend/amd/kfd/cli.om').read_text())
    binary = out / 'records-index-mutant'
    p = subprocess.run([sys.argv[3]], input=mutant.read_bytes(), capture_output=True, timeout=120)
    (out / 'records-index-mutant.compile.log').write_bytes(p.stderr)
    assert p.returncode == 0 and not p.stderr, ('INVALID index mutant setup', p.returncode, p.stderr)
    binary.write_bytes(p.stdout)
    binary.chmod(0o700)
    p = subprocess.run([str(binary), str(src_table), str(indices[-1])], capture_output=True, timeout=10)
    assert p.returncode == 0 and not p.stdout and not p.stderr, ('INVALID index mutant outcome', p)
    print(f'CONTROL index-guard-removal CAUGHT actual_exit={p.returncode} expected_exit=3 empty_stdout')
    over_fuel = out / 'field-count-over-fuel.tsv'
    # Keep every preceding field valid: a truncated table would refuse 3 before exhausting fuel.
    over_fuel.write_text('0 4 1000001\n' + '0 4 1\n' * 1000001)
    run('record-field-count-over-fuel', 'records', [over_fuel, 0], 104, b'', timeout=60)
    caught('field-count-over-loop-fuel', 'record-field-count-over-fuel')

    steps = [1, 2, 3, 4, 5] * 40
    first = run('ring-wrap', 'ring', steps)
    assert first == b'RING ACCEPT body<header<write-index<doorbell<consume\n'
    assert run('ring-repeat', 'ring.repeat', steps) == first
    run('ring-four-steps-usage', 'ring', [1, 2, 3, 4], 64, b'')
    caught('ring-four-steps', 'ring-four-steps-usage')
    for name, values in [('header-before-body', [2, 1, 3, 4, 5]),
                         ('write-index-before-header', [1, 3, 2, 4, 5]),
                         ('doorbell-before-write-index', [1, 2, 4, 3, 5]),
                         ('consumer-before-doorbell', [1, 2, 3, 5, 4]),
                         ('body-reuse-before-consume', [1, 2, 3, 4, 1]),
                         ('incomplete', [1, 2, 3, 4, 5, 1])]:
        run('ring-' + name, 'ring', values, 3, b'')
    print('PASS ring single-producer/consumer order, 40 launches, 16 slots, wrap and byte visibility checked')
    caught('header-before-body', 'ring-header-before-body')
    print('PASS ring write-index/doorbell/consumer/reuse/incomplete-order refusals')
    print(f'ORACLE PASS subprocess_cases={len(events)}')


if __name__ == '__main__':
    try:
        exercise(sys.argv[1], sys.argv[2])
    except (AssertionError, ValueError, OSError, subprocess.SubprocessError) as exc:
        print('ORACLE FAIL', repr(exc), file=sys.stderr)
        sys.exit(1)
