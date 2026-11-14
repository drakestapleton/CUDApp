# Shared helpers for the report's recompute scripts (measuring tools, not Omega).
# Python standard library only. Reads raw evidence files, never a lane's prose.
# Evidence root: $AMD_REPORT_RAW, else ../raw beside this folder (the publication layout), else the handoff folder.
import hashlib, json, os, re, statistics, sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
_raw = HERE.parent / 'raw'
ROOT = Path(os.environ.get('AMD_REPORT_RAW') or (_raw if _raw.is_dir() else HERE.parent.parent)).resolve()
OUT = HERE / 'out'
OUT.mkdir(exist_ok=True)

STEP = re.compile(r'^OMEGA_STEP (\d+) tok=(-?\d+) us=(\d+)$', re.M)
IDS = re.compile(r'^OMEGA_PLAN ids=\[([0-9,\-]*)\]$', re.M)
PROMPT = 30                          # prompt steps of the lighthouse request


def sha256(path):
    h = hashlib.sha256()
    with open(path, 'rb') as f:
        for chunk in iter(lambda: f.read(1 << 20), b''):
            h.update(chunk)
    return h.hexdigest()


def rel(path):
    return str(Path(path).resolve().relative_to(ROOT))


def omega_run(path, prompt=PROMPT):
    """One launcher stdout capture: per-step microseconds and the emitted ids."""
    text = Path(path).read_text(errors='replace')
    us = [int(m.group(3)) for m in STEP.finditer(text)]
    m = IDS.search(text)
    ids = [int(x) for x in m.group(1).split(',')] if m and m.group(1) else None
    dec = us[prompt:]
    return dict(
        steps=len(us),
        decode_steps=len(dec),
        decode_ms_median=statistics.median(dec) / 1000 if dec else None,
        prefill_ms=sum(us[:prompt]) / 1000,
        ids=ids,
    )


def first_difference(a, b):
    if a is None or b is None:
        return 0
    for i, (x, y) in enumerate(zip(a, b)):
        if x != y:
            return i
    return None if len(a) == len(b) else min(len(a), len(b))


class Book:
    """Collects the numbers of one topic and writes out/<topic>.json and .txt."""

    def __init__(self, topic):
        self.topic = topic
        self.rows = []
        self.fail = []

    def add(self, id, value, unit, label, source, how, printed=None):
        # label: MEASURED, MODELED, DERIVED (arithmetic on measured values), COUNT, RECORD
        srcs = source if isinstance(source, list) else [source]
        files = []
        for s in srcs:
            p = ROOT / s
            if p.is_file():
                files.append(dict(path=s, sha256=sha256(p)))
            elif p.exists():
                files.append(dict(path=s, sha256=None, note='directory'))
            else:
                files.append(dict(path=s, sha256=None, note='MISSING'))
                self.fail.append(f'{id}: source missing {s}')
        self.rows.append(dict(id=f'{self.topic}.{id}', value=value, unit=unit, label=label,
                              printed=printed, sources=files, how=how,
                              recompute=f'recompute/{self.topic}.py'))
        return value

    def check(self, name, ok, detail=''):
        if not ok:
            self.fail.append(f'CHECK {name}: {detail}')
        return ok

    def close(self):
        (OUT / f'{self.topic}.json').write_text(json.dumps(self.rows, indent=1) + '\n')
        lines = []
        for r in self.rows:
            lines.append(f"{r['id']}\t{r['printed'] if r['printed'] is not None else r['value']}"
                         f"\t{r['unit']}\t{r['label']}\t{r['value']}")
        verdict = 'PASS' if not self.fail else 'FAIL'
        lines.append(f'RECOMPUTE {self.topic} {verdict} numbers={len(self.rows)} failures={len(self.fail)}')
        lines += self.fail
        (OUT / f'{self.topic}.txt').write_text('\n'.join(lines) + '\n')
        print(lines[len(self.rows)])
        for f in self.fail:
            print('  ' + f)
        sys.exit(0 if not self.fail else 1)


def fmt(x, nd):
    return f'{x:.{nd}f}'
