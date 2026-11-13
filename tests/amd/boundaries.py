#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Test-only MessagePack oracle: thresholds from the format's bit widths.
https://github.com/msgpack/msgpack/blob/master/spec.md#formats
Record indices use the unsigned 64-bit wire domain, not kernel-emitted values.
"""
import pathlib
import sys

UINT_WIDTHS = (7, 8, 16, 32, 64)
STR_WIDTHS = (5, 8, 16)


def edges(widths):
    return sorted({v for width in widths for v in ((1 << width) - 1, 1 << width)})


def record_indices():
    return tuple((1 << 64) - delta for delta in (2, 1))


def uint_bytes(value):
    if value < 1 << 7:
        return bytes([value])
    for width, tag in ((8, 0xcc), (16, 0xcd), (32, 0xce), (64, 0xcf)):
        if value < 1 << width:
            return bytes([tag]) + value.to_bytes(width // 8, 'big')
    raise ValueError(value)


def string_bytes(size, fill):
    if size < 1 << 5:
        prefix = bytes([0xa0 | size])
    else:
        for width, tag in ((8, 0xd9), (16, 0xda), (32, 0xdb)):
            if size < 1 << width:
                prefix = bytes([tag]) + size.to_bytes(width // 8, 'big')
                break
        else:
            raise ValueError(size)
    return prefix + bytes([fill]) * size


def generate(out):
    uints = [0] + [v for v in edges(UINT_WIDTHS) if v < 1 << 64]
    strings = edges(STR_WIDTHS)
    main = ['fn main to int'] + [f'mpuint {v}' for v in uints]
    expected = b''.join(uint_bytes(v) for v in uints)
    for i, size in enumerate(strings):
        fill = 65 + i
        main += [f's{i} = new uchar {size} {fill}', f'mpstr s{i}']
        expected += string_bytes(size, fill)
    main += ['flush', 'ret 0', 'end']
    (out / 'msgpack-main.om').write_text('\n'.join(main) + '\n')
    (out / 'msgpack.expected').write_bytes(expected)
    print(f'BOUNDARIES uint={uints} str={strings} record-index={record_indices()}')


if __name__ == '__main__':
    generate(pathlib.Path(sys.argv[1]))
