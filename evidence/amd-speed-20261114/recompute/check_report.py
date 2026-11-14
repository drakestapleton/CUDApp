#!/usr/bin/env python3
# Check REPORT.md against numbers.json. What is checked, exactly:
#  1. Every decimal number outside code spans equals, to its printed precision, some value in numbers.json.
#  2. A decimal followed by a unit word (ms, us, percent, times, TB/s, GB, tokens per second, tokens per pass) must equal a
#     value of that unit class, so a figure cannot be matched by an unrelated value in another unit.
#  3. Bound claims (claims.json): each regex must match the report, and every captured number (integers included) must
#     equal the named numbers.json value. These cover the headline figures and ratios and the counts of runs, launches
#     and gate cases.
#  4. No em or en dashes.
# Integers other than the bound claims, and decimals inside tables without a unit word, are checked only by rule 1 or not
# at all. --selftest applies known one-number mutations to a copy and requires each to fail.
# Usage: check_report.py REPORT.md numbers.json claims.json [--selftest]
import json, re, sys
from pathlib import Path

UNITS = [(r'ms\b', {'ms/token', 'ms', 'ms/row'}), (r'(?:us|\u00b5s)\b', {'us', 'us/step'}), (r'percent\b', {'%'}),
         (r'times\b', {'x'}), (r'TB/s', {'TB/s'}), (r'GB\b', {'GB', 'B'}), (r'tokens per second\b', {'tokens/s'}),
         (r'tokens per pass\b', {'tokens/pass'})]


def load_values(nums):
    vals = []
    for n in nums:
        v = n['value']
        for x in (v if isinstance(v, list) else [v]):
            if isinstance(x, (int, float)) and not isinstance(x, bool):
                vals.append((float(x), n.get('unit', '')))
                if n.get('unit') == 'B':
                    vals.append((float(x) / 1e9, 'B'))
    return vals


def check(report, nums, claims, quiet=False):
    vals = load_values(nums)
    byid = {n['id']: n['value'] for n in nums}
    text = re.sub(r'```.*?```', lambda m: ' ' * len(m.group(0)), report, flags=re.S)
    text = re.sub(r'`[^`\n]*`', lambda m: ' ' * len(m.group(0)), text)
    bad, seen, bound = [], 0, 0
    for m in re.finditer(r'(?<![\w.-])(-?\d+\.\d+)(?![\w.]*\d)', text):
        tok, b = m.group(1), m.end()
        if text[b:b + 1] == '.' and text[b + 1:b + 2].isdigit():
            continue
        seen += 1
        nd = len(tok.split('.')[1])
        x = float(tok)
        ok = lambda v: abs(x - v) <= 0.5 * 10 ** -nd + 1e-12
        line = report.count('\n', 0, m.start()) + 1
        if not any(ok(v) for v, _ in vals):
            bad.append(f'NO SOURCE {tok} (line {line})')
            continue
        after = text[b:b + 24].lstrip()
        for pat, cls in UNITS:
            if re.match(pat, after):
                bound += 1
                if not any(ok(v) and u in cls for v, u in vals):
                    bad.append(f'WRONG UNIT {tok} {pat} (line {line})')
                break
    nclaims = 0
    for c in claims:
        ms = list(re.finditer(c['pattern'], report))
        if not ms:
            bad.append(f'CLAIM NOT FOUND {c["pattern"][:70]}')
            continue
        for m in ms:
            for g, cid in zip(m.groups(), c['ids']):
                nclaims += 1
                want = byid.get(cid)
                if isinstance(want, list) and len(want) == 1:
                    want = want[0]
                got = float(g.replace(',', ''))
                nd = len(g.split('.')[1]) if '.' in g else 0
                if not isinstance(want, (int, float)) or abs(got - want) > 0.5 * 10 ** -nd + 1e-12:
                    bad.append(f'CLAIM MISMATCH {g} != {cid}={want} (line {report.count(chr(10), 0, m.start()) + 1})')
    dash = [i + 1 for i, l in enumerate(report.splitlines()) if '\u2013' in l or '\u2014' in l]
    if not quiet:
        print(f'CHECK-REPORT decimals={seen} unit-bound={bound} claims={nclaims} MISMATCH {len(bad)} dashes={len(dash)}')
        for x in bad:
            print('  ' + x)
        for d in dash:
            print('  DASH line', d)
    return not bad and not dash


MUTATIONS = [('16.226', '16.326'), ('1.674 times', '1.829 times'), ('173 completed', '171 completed'),
             ('Spark gate: 318 checks', 'Spark gate: 316 checks'), ('5 of the 10 planned', '6 of the 10 planned')]

if __name__ == '__main__':
    report = Path(sys.argv[1]).read_text()
    nums = json.loads(Path(sys.argv[2]).read_text())['numbers']
    claims = json.loads(Path(sys.argv[3]).read_text())
    if '--selftest' in sys.argv:
        ok = check(report, nums, claims, quiet=True)
        caught = 0
        for old, new in MUTATIONS:
            if old not in report:
                print(f'  SELFTEST mutation target missing: {old}')
                continue
            caught += not check(report.replace(old, new, 1), nums, claims, quiet=True)
        res = ok and caught == len(MUTATIONS)
        print(f'CHECK-SELFTEST {"PASS" if res else "FAIL"} original={"pass" if ok else "fail"} mutations caught={caught}/{len(MUTATIONS)}')
        sys.exit(0 if res else 1)
    sys.exit(0 if check(report, nums, claims) else 1)
