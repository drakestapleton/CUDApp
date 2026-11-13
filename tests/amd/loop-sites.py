#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Host measuring instrument for compiler-fuel exposure, not a product input cap.

Inventory every loop, including nested chunks. Constant-trip loops and explicitly
wire/buffer-bounded loops are excluded with their reason. Every remaining source
site has an alarm case. Parser alarms use a conservative input-byte upper bound,
not a claim to have executed that many iterations. Executor instruction/work and
ring argument demands have separate units. The execution gate additionally runs
past-fuel wave/grid programs and their compiled restored-bound mutants.

Changing loop structure requires updating its case/policy here: an unclassified
loop or a policy with no source site is a failure, never silently skipped.
"""
import pathlib
import re
import sys
import tempfile

FILES = ('backend/amd/gfx942/exec/main.om', 'backend/amd/gfx942/hsaco.om',
         'backend/amd/kfd/records.om', 'backend/amd/kfd/ring.om')
# These are semantic policies, not a stored line-number/count inventory.
POLICY = {
    'main.om': {
        ('rdword', 'while lt i 4 bound 999999'): 'fixed: four-byte word',
        ('mwrite', 'while lt i 4 bound 999999'): 'fixed: four-byte word',
        ('boundary', 'while lt pc targetpc bound 999999'): 'instructions',
        ('boundary', 'while and lt pc targetpc lt c 999998 bound 999999'): 'instructions',
        ('validate', 'while lt pc get st 1 bound 999999'): 'instructions',
        ('validate', 'while and lt pc get st 1 lt c 999998 bound 999999'): 'instructions',
        ('smexec', 'for i ulong n'): 'fixed: decoded scalar-memory opcode width',
        ('fp8bits', 'while lt f 8 bound 3'): 'fixed: FP8 mantissa normalization',
        ('wide', 'for i ulong n'): 'fixed: decoded vector-memory opcode width',
        ('vexec', 'while eq and shr get st 14 first 1 0 bound 64'): 'fixed: wave64 lane mask',
        ('wave', 'while true bound 999999'): 'iterations',
        ('wave', 'while lt c 999998 bound 999999'): 'iterations',
        ('gridarg', 'for i ulong ulong n'): 'fixed: argread buffer of 4096 bytes',
        ('main', 'while lt y ny bound 999999'): 'iterations',
        ('main', 'while and lt y ny lt c 999998 bound 999999'): 'iterations',
        ('main', 'while lt g groups bound 999999'): 'iterations',
        ('main', 'while and lt g groups lt d 999998 bound 999999'): 'iterations',
    },
    'hsaco.om': {
        ('labfind', 'for i ulong get gs 4'): 'bytes',
        ('token', 'while true bound 999999'): 'bytes',
        ('token', 'while ne get gs 1 10 bound 999999'): 'bytes',
        ('number', 'while true bound 999999'): 'bytes',
        ('format', 'for i ulong get gs 31'): 'bytes',
        ('inst', 'for k ulong fg f 2'): 'fixed: fieldidx checks the 64 instruction bits',
        ('namespan', 'while and gt c 32 ne c 256 bound 999999'): 'bytes',
        ('table', 'while true bound 999999'): 'bytes',
        ('flush', 'while lt p n bound 999999'): 'fixed: 4096-byte output buffer, positive write progress',
        ('wrle', 'for j ulong n'): 'fixed: ELF integer widths at call sites',
        ('zeros', 'for j ulong n'): 'bytes',
        ('bytes', 'for j ulong len v'): 'bytes',
        ('wrbe', 'for j ulong n'): 'fixed: MessagePack integer widths at call sites',
        ('span', 'while lt i n bound 999999'): 'bytes',
        ('span', 'for j ulong ulong r'): 'fixed: pread into 4096-byte name buffer',
    },
    'records.om': {
        ('tnum', 'while true bound 999999'): 'bytes',
        ('tnum', 'while ne c 10 bound 999999'): 'bytes',
        ('tnum', 'while and ge c 48 le c 57 bound 999999'): 'bytes',
        ('store', 'for i ulong wid'): 'fixed: validated u32/u64 wire width',
        ('fld', 'for j ulong cnt'): 'fixed: positive wire width and record extent checked per field',
        ('record', 'for f ulong nf'): 'bytes',
        ('main', 'for i ulong add idx 1'): 'bytes',
    },
    'ring.om': {('main', 'for i ulong sub ulong argcnt 1'): 'arguments'},
}
THRESHOLD = 500000


def inventory(root, replacements=None):
    sites = []
    for name in FILES:
        path = pathlib.Path(root) / name
        text = (replacements or {}).get(name, path.read_text())
        policy = POLICY[path.name]
        seen = set()
        function = None
        for line, statement in enumerate(text.splitlines(), 1):
            if statement.startswith('fn '):
                function = statement.split()[1]
            if not statement.startswith(('while ', 'for ')):
                continue
            if re.fullmatch(r'for \w+ ulong \d+', statement):
                continue
            key = function, statement
            assert key in policy, f'UNCLASSIFIED LOOP {name}:{line} {function}: {statement}'
            assert key not in seen, f'DUPLICATE LOOP needs a distinct case {name}:{line}'
            seen.add(key)
            if not policy[key].startswith('fixed:'):
                sites.append((name, line, function, statement, policy[key]))
        assert seen == policy.keys(), f'LOOP CASE WITHOUT SITE {name}: {policy.keys() - seen}'
    return sites


def pin(root):
    text = (pathlib.Path(root) / 'tests/multitarget/run.sh').read_text()
    return re.search(r'^omc_pin=(\w+)$', text, re.M).group(1)


def alarm(site, demand, compiler, source):
    name, line, function, statement, units = site
    if demand > THRESHOLD:
        print(f'LOOP-FUEL ALARM site={name}:{line} function={function} loop={statement!r} '
              f'demand_upper_bound={demand} units={units} threshold={THRESHOLD} '
              f'compiler={compiler} input={source}')
        return True
    return False


def test(root):
    sites = inventory(root)
    compiler = pin(root)
    cases = []
    with tempfile.TemporaryDirectory(prefix='loop-site-cases.') as tmp:
        # Materialized inputs, not a typed-in expected count. For parser sites the
        # byte count intentionally overestimates trips and alarms before fuel loss.
        payload = pathlib.Path(tmp) / 'past-fuel-input'
        payload.write_bytes(b'0\n' * 1000001)
        measured = {'bytes': payload.stat().st_size,
                    'instructions': len(payload.read_bytes().splitlines()),
                    'iterations': len(payload.read_bytes().splitlines()),
                    'arguments': len(payload.read_bytes().splitlines())}
        for site in sites:
            assert alarm(site, measured[site[4]], compiler, payload), ('missing alarm', site)
            cases.append(site)
        assert len(cases) == len(sites) and len(set(cases)) == len(sites)
        name = FILES[0]
        original = (pathlib.Path(root) / name).read_text()
        # A new bare input-growing loop cannot borrow an existing case.
        extra = original + '\nfn unmatched n ulong to void\nfor added ulong n\nend\nend\n'
        try:
            inventory(root, {name: extra})
        except AssertionError as exc:
            assert 'UNCLASSIFIED LOOP' in str(exc)
            print('CONTROL unmatched-loop CAUGHT scanner_exit=1', exc)
        else:
            raise AssertionError('new bare loop escaped inventory')
        # A restored flat dispatch loop also invalidates the source/case bijection.
        old = 'while lt y ny bound 999999'
        assert original.count(old) == 1
        try:
            inventory(root, {name: original.replace(old, 'for y ulong ny')})
        except AssertionError as exc:
            assert 'UNCLASSIFIED LOOP' in str(exc)
            print('CONTROL restored-bare-for CAUGHT scanner_exit=1', exc)
        else:
            raise AssertionError('restored bare for escaped inventory')
    print(f'LOOP-SITES PASS growing_sites={len(sites)} past_fuel_alarm_cases={len(cases)} compiler={compiler}')


if __name__ == '__main__':
    try:
        mode, root, *paths = sys.argv[1:]
        if mode == 'test':
            test(root)
        elif mode == 'files':
            sites, compiler = inventory(root), pin(root)
            failed = False
            for path in paths:
                n = pathlib.Path(path).stat().st_size
                for site in sites:
                    if site[4] == 'bytes':
                        failed |= alarm(site, n, compiler, path)
                print(f'LOOP-FUEL measured input={path} bytes={n} threshold={THRESHOLD}')
            sys.exit(1 if failed else 0)
        else:
            raise ValueError('unknown mode ' + mode)
    except (AssertionError, OSError, ValueError) as exc:
        print('LOOP-SITES FAIL', exc, file=sys.stderr)
        sys.exit(1)
