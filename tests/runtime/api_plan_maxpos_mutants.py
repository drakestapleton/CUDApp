#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Keep the seven max-position controls after their C predicates move to Omega.

This portable gate mutates checked generated bindings, not production sources.
The independent test-api-plan-mutants target mutates and recompiles canonical
.om source with the pinned compiler. Shared span admission now serves read,
write and step: one span mutant must trigger all three distinct expectations.
"""
import os
from pathlib import Path
import shlex
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
lib = str(Path(sys.argv[1]).resolve())
flags = shlex.split(sys.argv[2])
cc = shlex.split(os.environ.get('CC', 'cc'))
source = (root / 'src/cudapp_om_api_da_plan.c').read_text()
revert_first = len(sys.argv) == 4 and sys.argv[3] == '--revert-first'
assert len(sys.argv) == 3 or revert_first, 'usage: library cflags [--revert-first]'
cap = '_a=v3;uint64_t _b=131072ULL;o_op(10, 5, _a,_b);'
pos = '_a=o_get(v1, v1_n, 3ULL, 5);uint64_t _b=o_get(v1, v1_n, 5ULL, 5);o_op(10, 5, _a,_b);'
span = '_a=o_get(v1, v1_n, 5ULL, 5);uint64_t _b=o_get(v1, v1_n, 3ULL, 5);o_op(2, 5, _a,_b);'
rope = '_i=10ULL;uint64_t _v=({uint64_t _a=v3;uint64_t _b=512ULL;o_op(3, 5, _a,_b);});'
mutants = (
    ('cap-off-by-one', cap, cap.replace('o_op(10,', 'o_op(11,'),
     (('cap-off-by-one', 'FAIL cache of 131072 positions'),)),
    ('cap-one-chunk-over', cap, cap.replace('131072ULL', '131328ULL'),
     (('cap-one-chunk-over', 'FAIL cache of 131072 positions'),)),
    ('write-pos-bound', pos, '_a=0ULL;uint64_t _b=1ULL;o_op(6, 5, _a,_b);',
     (('write-pos-bound', 'omega_gpu_decode_attn_cache_write(c, 0, 0, 131073, 1, k, v)'),)),
    ('shared-span-bound', span, span.replace('o_op(2, 5, _a,_b);', 'o_op(1, 5, o_op(2, 5, _a,_b),1ULL);'),
     (('write-count-bound', 'omega_gpu_decode_attn_cache_write(c, 0, 3, 131071, 2, k2, v2)'),
      ('read-pos-bound', 'omega_gpu_decode_attn_cache_read(c, 0, 0, 131072, kr, vr)'),
      ('step-pos-bound', 'omega_gpu_decode_attn(c, 0, 131072, g_qkv, 1e-6f, 1.0f, g_out, &info)'))),
    ('rope-table-short', rope, rope.replace('_a=v3;', '_a=o_op(2,5,v3,8ULL);'),
     (('rope-table-short', 'FAIL RoPE rows up to 131071'),)),
)
controls = 0
with tempfile.TemporaryDirectory(prefix='cudapp-maxpos-') as tmp:
    tmp = Path(tmp)
    for name, old, new, required in mutants:
        assert source.count(old) == 1, (name, 'generated mutation site changed')
        binding = tmp / (name + '.c')
        exe = tmp / name
        # The baseline target still exercises the requested migration switch.
        if revert_first and name == 'cap-off-by-one':
            print('NEGATIVE CONTROL: reverting cap-off-by-one mutation; gate must fail', flush=True)
            new = old
        binding.write_text(source.replace(old, new))
        # These binding-specific mutants must select the Omega adapter path.
        subprocess.run(cc + flags + ['-O1', '-g', '-fsanitize=address,undefined',
            '-fno-sanitize-recover=all', '-UCUDAPP_MIGRATION_PENDING_C_API_PLAN',
            '-DCUDAPP_MIGRATION_PENDING_C_API_PLAN=0', '-o', str(exe),
            str(root / 'tests/test_omega_gpu_decode_attn_maxpos.c'),
            str(root / 'src/omega_gpu_decode_attn_api.c'), str(binding), lib,
            '-lpthread', '-lm'], cwd=root, check=True)
        result = subprocess.run([str(exe)], cwd=root, capture_output=True, text=True)
        (tmp / (name + '.log')).write_text(result.stdout + result.stderr)
        if revert_first:
            assert result.returncode == 0, ('negative-control baseline failed', result)
            print('NEGATIVE CONTROL: unchanged binding survives; gate rejects missing mutation', flush=True)
            raise SystemExit(1)
        assert result.returncode != 0 and '\nFAIL ' in '\n' + result.stdout, (name, result)
        for control, expected in required:
            assert expected in result.stdout, (control, result.stdout, result.stderr)
            print('maxpos control killed:', control, flush=True)
            controls += 1
assert controls == 7
print('test-gpu-decode-attn-maxpos: 7 of 7 controls killed (5 binding mutants)')
