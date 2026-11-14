#!/usr/bin/env python3
# Every MI300X notebook session behind this report: id, UTC start (encoded in the id as YYMMDDhhmmss),
# lane, what ran, evidence file and its sha256. A session counts only if its id is written in its evidence
# (in a file, or in the name of the folder the results were stored under).
import json, re
from common import ROOT, OUT, Book, sha256

b = Book('sessions')
CB = 'amd-launch-gap/src/CUDApp-base/evidence'   # CUDApp dc536882 clone
SESSIONS = [
    ('261113072240-bb2a389c', 'earlier qualification (Main)', 'launcher 373e76cd, warmup + 5 lighthouse runs',
     f'{CB}/amd-qwen38-native-final-20261113/host/qualification.json', 'COMPLETE'),
    ('261113080038-5a0c78a1', 'earlier qualification (Main)', 'launcher f646ea5d, warmup + 5 lighthouse runs',
     f'{CB}/amd-qwen38-native-compatible-20261113/host/qualification.json', 'COMPLETE'),
    ('261113101013-76e03a28', 'published run (Main)', 'launcher 08a50491, warmup + 5 lighthouse runs, controls',
     f'{CB}/amd-qwen38-native-refusal-20261113/host/qualification.json', 'COMPLETE'),
    ('261113185025-46d1b9c5', 'UXL brief repeat (Main)', 'public-clone repeat, HIP graph, PyTorch FP8, stock BF16, control',
     'card-evidence/uxl-evidence.tar.gz', 'COMPLETE'),
    ('261113232737-7605956f', 'GpuBenchSync', 'per-kernel timing, first build of the cells',
     'amd-kernel-speed/card/GpuBenchSync-20261113T232737Z/aks-results.tar.gz', 'COMPLETE'),
    ('261114001549-e787e65d', 'AmdKernelSpeed', 'per-kernel timing, v2 kernels, cap286, v1all ladder',
     'amd-kernel-speed/card/261114001549-e787e65d/aks-results.tar.gz', 'COMPLETE'),
    ('261114005045-52ef268c', 'AmdLaunchGap', 'run 1: plan, plan2, planscope, planstage, HIP graph, controls',
     'amd-launch-gap/card/gap-evidence.tar.gz', 'COMPLETE'),
    ('261114011836-f591aa44', 'FourBit', 'q4gemv_d8 and q4gemv_d8r2 bit checks and timing; ended early at about 01:26Z (section 13)',
     'four-bit/evidence/card-partial-20261114/card_q4.log.partial', 'PARTIAL: 5 of 10 shapes'),
    ('261114012602-e8cddcb1', 'AmdLaunchGap', 'run 2: plan, plan3, HIP graph, plan3 control, plan2',
     'amd-launch-gap/card/run2/gap-evidence.tar.gz', 'COMPLETE'),
    ('261114014516-c715b669', 'FourBit', 'rerun: q4gemv_d8, q4gemv_d8r2 and FP8/BF16 baseline on 10 shapes, two launches hung and were killed',
     'four-bit/evidence/card-20261114/q4card.tar.gz', 'COMPLETE for the 4-bit rerun, 2 timings missing'),
    ('261114020544-6e585e21', 'AmdKernelSpeed', 'combined run: 7 arms (launcher, v2 kernels, FP8 v2b1/v2b2, cap286) x 3 interleaved runs, control',
     'amd-kernel-speed/combined/card-2026-11-14T0206Z/akc-results.tar.gz', 'COMPLETE'),
    ('261114022348-7f66e322', 'AmdLaunchGap', 'run 3: launch micro-benchmark; 9 paired cycles plan3, plan3c, HIP; plan3t phases; published launcher; control',
     'amd-launch-gap/card/run3/gap-evidence.tar.gz', 'COMPLETE'),
    ('261114025237-3a2e29e5', 'SpecDecode2', 'extra-row timing of the R-row kernels (fp8 R=1,2,3,8; bf16 R=1)',
     'spec-decode/card-s4/results/jupyter-user-261114025237-3a2e29e5/rho-results.json', 'PARTIAL: BF16 rows at R > 1 and the speculative step not reached'),
]
# where the session id itself is written down (the evidence file may be a tarball)
ID_SOURCE = {
    '261113185025-46d1b9c5': 'card-evidence/uxl-card.txt',
    '261113232737-7605956f': 'amd-kernel-speed/card/GpuBenchSync-20261113T232737Z/SESSION.txt',
    '261114001549-e787e65d': 'amd-kernel-speed/card/launch-log.jsonl',
    '261114005045-52ef268c': 'amd-launch-gap/card/final/logs/card.txt',
    '261114011836-f591aa44': 'four-bit/evidence/card-partial-20261114/README.md',
    '261114012602-e8cddcb1': 'amd-launch-gap/card/run2/final/logs/card.txt',
    '261114014516-c715b669': 'four-bit/evidence/card-20261114/card_q4.state.json',
    '261114020544-6e585e21': 'amd-kernel-speed/combined/card-2026-11-14T0206Z/driver.jsonl',
    '261114022348-7f66e322': 'amd-launch-gap/card/run3/final/logs/card.txt',
}
rows = []
for sid, lane, what, ev, state in SESSIONS:
    full = 'jupyter-user-' + sid
    t = sid[:12]
    start = f'20{t[0:2]}-{t[2:4]}-{t[4:6]}T{t[6:8]}:{t[8:10]}:{t[10:12]}Z'
    p = ROOT / ev
    h = sha256(p) if p.exists() else None
    idsrc = ID_SOURCE.get(sid, ev)
    if sid in ID_SOURCE or full not in ev:
        text = (ROOT / idsrc).read_text(errors='replace') if (ROOT / idsrc).exists() else ''
        seen = sid in text or sid[-8:] in text
        b.check(f'{sid} named in {idsrc}', seen)
    else:
        # the relay stored the results under a folder named after the session
        b.check(f'{sid} named in the evidence path', p.exists())
    b.check(f'{sid} evidence present', h is not None, ev)
    rows.append(dict(session=full, utc_start=start, lane=lane, ran=what, evidence=ev, sha256=h, state=state))
    b.add(f'{sid}.sha256', h or 'PENDING', '', 'RECORD', [ev] if h else [], 'sha256 of the evidence file')
# The published card-side tarball hashes, where a lane recorded one beside the tarball
for ev, side in [('amd-kernel-speed/card/261114001549-e787e65d/aks-results.tar.gz', 'amd-kernel-speed/card/261114001549-e787e65d/aks-results.sha256'),
                 ('amd-kernel-speed/card/GpuBenchSync-20261113T232737Z/aks-results.tar.gz', 'amd-kernel-speed/card/GpuBenchSync-20261113T232737Z/aks-results.sha256'),
                 ('amd-launch-gap/card/gap-evidence.tar.gz', 'amd-launch-gap/card/gap-evidence.tar.gz.sha256'),
                 ('amd-launch-gap/card/run2/gap-evidence.tar.gz', 'amd-launch-gap/card/run2/gap-evidence.tar.gz.sha256'),
                 ('amd-kernel-speed/combined/card-2026-11-14T0206Z/akc-results.tar.gz', 'amd-kernel-speed/combined/card-2026-11-14T0206Z/SHA256SUMS'),
                 ('amd-launch-gap/card/run3/gap-evidence.tar.gz', 'amd-launch-gap/card/run3/gap-evidence.tar.gz.sha256')]:
    b.check(f'{ev} equals card-side hash', sha256(ROOT / ev) == (ROOT / side).read_text().split()[0])
b.check('uxl-evidence.tar.gz equals the hash in card-evidence/README.md',
        sha256(ROOT / 'card-evidence/uxl-evidence.tar.gz') in (ROOT / 'card-evidence/README.md').read_text())
# The FourBit script that ran in sessions f591aa44 and c715b669 (bundle 10c93d3d), recovered from the lane's git
# history: its hash must equal the bundle's, and the rerun evidence must name that bundle.
X = 'amd-report/extracted/four-bit-bundle-10c93d3d'
bj = json.loads((ROOT / f'{X}/BUNDLE.json').read_text())
b.check('extracted card_q4.py is the one in bundle 10c93d3d', bj['bundle_sha256'].startswith('10c93d3d')
        and sha256(ROOT / f'{X}/card_q4.py') == bj['files']['card_q4.py'])
b.check('the rerun evidence names bundle 10c93d3d', '10c93d3d' in (ROOT / 'four-bit/evidence/card-20261114/README.md').read_text())
classes_src = (ROOT / f'{X}/card_q4.py').read_text().split('CLASSES = [', 1)[1].split('\n]', 1)[0]
planned = re.findall(r'^\s+\("([^"]+)", ', classes_src, re.M)
b.add('fourbit.shapes_planned', len(planned), 'shapes', 'COUNT', f'{X}/card_q4.py', 'entries of CLASSES')
b.add('count', len(rows), 'sessions', 'COUNT', [r['evidence'] for r in rows], 'card sessions behind this report')
(OUT / 'sessions-table.json').write_text(json.dumps(rows, indent=1) + '\n')
b.close()
