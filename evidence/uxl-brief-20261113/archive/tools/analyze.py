#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""analyze.py: read the raw files of a hand-run Level 3 card session and print the numbers. Python 3 stdlib only.

Usage:
  analyze.py OUT_DIR CUDAPP [--json results.json]                 notebook layout (files written by the parent):
        uxl-native-{warmup,run1..run5,control}.txt (+ .err), uxl-l3.log, uxl-graph-fresh.json,
        uxl-reference-fresh.json, uxl-stock.json, uxl-env.log      (missing files are reported, not fatal)
  analyze.py --published CUDAPP [--json results.json]             self-test on the published receipt:
        evidence/amd-qwen38-native-refusal-20261113/native/{warmup,sample1..5,control}.stdout; must reproduce
        calculations.json (decode medians 26.731 26.642 26.675 26.726 26.760, median prefill 779.042 ms)

Definitions are those of evidence/amd-qwen38-native-refusal-20261113/oracle/benchmark-qualified.py at
dc536882 (the same file in the brief's folder amd-qwen38-native-20261113/oracle is byte-identical):
  line 8   P=F['prompt_ids'];G=F['generated_ids'];N=len(P)+len(G)-1
  line 66  steps = every '^OMEGA_STEP (\\d+) tok=(\\d+) us=(\\d+)$' line
  line 67  ids   = the '^OMEGA_PLAN ids=[...]' list
  line 68  shape = exit==0 and step indices == range(N) and ids == tok of steps[len(P)-1:]
           (the exit code is not in a .txt capture; this script uses the rest of line 68 and reports that)
  line 69  prefill_ms = sum(u/1000 for steps[:len(P)]);  decode_ms_median = statistics.median(u/1000 for steps[len(P):])
Aggregates follow evidence/amd-qwen38-native-refusal-20261113/calculate.py: median of the five timed decode medians,
median of the five prefill values, 1000/median, and spark-baseline/qualification.json graph_ms_per_token / median.
Exit status 0 when every check passes, 1 otherwise.
"""
import hashlib, json, re, statistics, sys
from pathlib import Path

COMMIT = 'dc536882c06a692991be97dc65cd2d037d82b129'
PLAN_SHA = '2a292dadc236f224005ed4e2d8ac2ba8b33dc71d79dabcc8497d177561f9c012'
CONTROLS_SHA = '40701b9a81ef55c3dc92620df801d595cd1a10360ae047e14e76de37e279228e'
TIMED = ['run1', 'run2', 'run3', 'run4', 'run5']
LABELS = ['warmup'] + TIMED + ['control']
STEP = re.compile(r'^OMEGA_STEP (\d+) tok=(\d+) us=(\d+)$', re.M)
IDS = re.compile(r'^OMEGA_PLAN ids=\[([\d, ]+)\]$', re.M)

def sha(p): return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def first_div(a, b): return next((i for i, (x, y) in enumerate(zip(a, b)) if x != y), None)
def compare_stock(ids, native, G):
    if not isinstance(ids, list): return {'n_ids': 0}
    return {'n_ids': len(ids), 'ids_equal_native': ids == native, 'first_divergence_from_native': first_div(ids, native),
            'ids_equal_spark': ids == G, 'first_divergence_from_spark': first_div(ids, G)}
def load(p):
    try: return json.loads(Path(p).read_text())
    except (OSError, ValueError) as e: return {'_error': repr(e)}

def parse_run(text, P, G):
    """benchmark-qualified.py lines 66-69, unchanged apart from the exit code."""
    N = len(P) + len(G) - 1
    steps = [tuple(map(int, v)) for v in STEP.findall(text)]
    m = IDS.search(text); ids = list(map(int, m.group(1).split(','))) if m else []
    shape = [i for i, t, u in steps] == list(range(N)) and ids == [t for i, t, u in steps[len(P) - 1:]]
    return {'steps': len(steps), 'expected_steps': N, 'prompt_steps': len(P), 'n_ids': len(ids), 'shape_ok': shape,
            'prefill_ms': sum(u / 1000 for i, t, u in steps[:len(P)]) if shape else None,
            'decode_ms_median': statistics.median(u / 1000 for i, t, u in steps[len(P):]) if shape else None,
            'first_divergence_from_spark': first_div(ids, G) if ids else None,
            'ids_match_spark': sum(a == b for a, b in zip(ids, G)), 'ids_equal_spark': ids == G,
            'step_us': [u for i, t, u in steps], 'ids': ids}

def parse_l3(path):
    """Tolerant reader of the shell trace: sha256sum lines for the two plans, and per-run start/end epoch seconds
    on lines that name a run label and the word start/begin or end/done/finish (any order, any separators)."""
    out = {'plan_sha256_lines': [], 'run_epochs': {}, 'unparsed_epoch_lines': []}
    for line in Path(path).read_text(errors='replace').splitlines():
        m = re.search(r'\b([0-9a-f]{64})\b\s+\S*?(real2(?:-controls)?\.plan)\b', line)
        if m: out['plan_sha256_lines'].append({'plan': m.group(2), 'sha256': m.group(1), 'line': line.strip()})
        ep = re.findall(r'(?<![\d.])(1[6-9]\d{8}(?:\.\d+)?)(?![\d.])', line)
        if not ep: continue
        low = line.lower()
        lab = next((l for l in LABELS if re.search(r'(?<![a-z0-9])' + l + r'(?![0-9])', low)), None)
        kind = 'start' if re.search(r'start|begin', low) else 'end' if re.search(r'\bend|done|finish|stop', low) else None
        if lab and kind and len(ep) == 1: out['run_epochs'].setdefault(lab, {})[kind] = float(ep[0])
        elif lab and len(ep) == 2: out['run_epochs'].setdefault(lab, {}).update(start=float(ep[0]), end=float(ep[1]))
        else: out['unparsed_epoch_lines'].append(line.strip())
    for lab, d in out['run_epochs'].items():
        if 'start' in d and 'end' in d: d['wall_s'] = d['end'] - d['start']
    return out

def find_id_lists(obj, path='', found=None):
    """Every list of 256 non-negative ints anywhere in a JSON document (stock script format is not fixed)."""
    found = [] if found is None else found
    if isinstance(obj, dict):
        for k, v in obj.items(): find_id_lists(v, path + '/' + str(k), found)
    elif isinstance(obj, list):
        if len(obj) == 256 and all(isinstance(x, int) and not isinstance(x, bool) and x >= 0 for x in obj): found.append((path, obj))
        else:
            for i, v in enumerate(obj): find_id_lists(v, '%s/%d' % (path, i), found)
    return found

def scalars(obj, path='', out=None, depth=0):
    out = {} if out is None else out
    if isinstance(obj, dict) and depth < 4:
        for k, v in obj.items(): scalars(v, path + '/' + str(k), out, depth + 1)
    elif isinstance(obj, (int, float, str, bool)) or obj is None:
        out[path] = obj if not isinstance(obj, str) else obj[:200]
    return out

def main():
    a = sys.argv[1:]; js = None
    if '--json' in a: i = a.index('--json'); js = Path(a[i + 1]); del a[i:i + 2]
    published = '--published' in a
    if published: a.remove('--published')
    if (published and len(a) != 1) or (not published and len(a) != 2): sys.exit(__doc__)
    repo = Path(a[-1]).resolve(); out = None if published else Path(a[0]).resolve()
    D = repo / 'evidence' / 'amd-qwen38-native-refusal-20261113'
    F = load(D / 'oracle' / 'lighthouse256.json'); P, G = F['prompt_ids'], F['generated_ids']
    GRAPH = load(D / 'numerical' / 'graph.json')['ids']
    spark_ms = load(D / 'spark-baseline' / 'qualification.json').get('graph_ms_per_token')
    R = {'mode': 'published self-test' if published else 'notebook out/', 'cudapp': str(repo), 'out_dir': str(out) if out else None,
         'definitions_source': 'evidence/amd-qwen38-native-refusal-20261113/oracle/benchmark-qualified.py lines 8, 66-69; aggregates as calculate.py',
         'reference_files': {'lighthouse256.json': sha(D / 'oracle' / 'lighthouse256.json'), 'numerical/graph.json': sha(D / 'numerical' / 'graph.json'),
                             'benchmark-qualified.py': sha(D / 'oracle' / 'benchmark-qualified.py')},
         'inputs': {}, 'missing': [], 'checks': {}, 'runs': {}}
    try:
        import subprocess
        R['cudapp_head'] = subprocess.run(['git', '-C', str(repo), 'rev-parse', 'HEAD'], capture_output=True, text=True).stdout.strip()
    except Exception as e: R['cudapp_head'] = repr(e)
    R['checks']['cudapp_head_is_dc536882'] = R['cudapp_head'] == COMMIT

    def src(label, ext):
        if published:
            return D / 'native' / ({'run%d' % i: 'sample%d' % i for i in range(1, 6)}.get(label, label) + ('.stdout' if ext == 'txt' else '.stderr'))
        return out / ('uxl-native-%s.%s' % (label, ext))
    for lab in LABELS:
        f = src(lab, 'txt'); e = src(lab, 'err')
        if not f.is_file(): R['missing'].append(str(f)); continue
        r = parse_run(f.read_text(errors='replace'), P, G)
        r['file'] = str(f); r['sha256'] = sha(f)
        if e.is_file():
            t = e.read_text(errors='replace'); r['err_file'] = str(e); r['err_sha256'] = sha(e); r['err_bytes'] = len(t.encode()); r['err_tail'] = t[-300:]
        r['ids_equal_published_graph'] = bool(r['ids']) and r['ids'] == GRAPH
        r['first_divergence_from_published_graph'] = first_div(r['ids'], GRAPH) if r['ids'] else None
        R['inputs'][f.name] = r['sha256']; R['runs'][lab] = r

    if not published:
        for name in ('uxl-l3.log', 'uxl-graph-fresh.json', 'uxl-reference-fresh.json', 'uxl-stock.json', 'uxl-env.log'):
            p = out / name
            if p.is_file(): R['inputs'][name] = sha(p)
            else: R['missing'].append(str(p))
        if (out / 'uxl-l3.log').is_file():
            l3 = parse_l3(out / 'uxl-l3.log'); R['l3_log'] = l3
            for lab, d in l3['run_epochs'].items():
                if lab in R['runs'] and 'wall_s' in d: R['runs'][lab]['process_wall_s_from_l3_log'] = d['wall_s']
            shas = {}
            for x in l3['plan_sha256_lines']: shas.setdefault(x['plan'], set()).add(x['sha256'])
            R['plan_sha256_seen'] = {k: sorted(v) for k, v in shas.items()}
            R['checks']['plan_sha256_equals_brief'] = shas.get('real2.plan') == {PLAN_SHA}
            R['checks']['controls_plan_sha256_equals_readme'] = shas.get('real2-controls.plan') == {CONTROLS_SHA}

    # ---- checks on the native runs
    timed = [R['runs'].get(l) for l in TIMED]; have = all(timed)
    allrun = [R['runs'][l] for l in ['warmup'] + TIMED if l in R['runs']]
    C = R['checks']
    C['all_7_native_files_present'] = all(l in R['runs'] for l in LABELS)
    C['every_run_285_steps_shape_ok'] = len(allrun) == 6 and all(r['shape_ok'] and r['steps'] == len(P) + len(G) - 1 for r in allrun)
    C['every_run_256_ids'] = len(allrun) == 6 and all(r['n_ids'] == len(G) for r in allrun)
    C['every_run_ids_equal_published_graph'] = len(allrun) == 6 and all(r['ids_equal_published_graph'] for r in allrun)
    ctl = R['runs'].get('control')
    if ctl:
        C['control_shape_ok'] = ctl['shape_ok']
        C['control_ids_differ_from_published_graph'] = bool(ctl['ids']) and ctl['ids'] != GRAPH
        C['control_ids_differ_from_every_native_run'] = bool(ctl['ids']) and all(ctl['ids'] != r['ids'] for r in allrun)
        ctl['first_divergence_from_native_run1'] = first_div(ctl['ids'], timed[0]['ids']) if timed[0] else None

    agg = {}
    if have and all(r['shape_ok'] for r in timed):
        dm = [r['decode_ms_median'] for r in timed]; pm = [r['prefill_ms'] for r in timed]
        agg = {'timed_decode_ms_medians': dm, 'median_decode_ms': statistics.median(dm), 'timed_prefill_ms': pm,
               'median_prefill_ms': statistics.median(pm), 'warmup_decode_ms_median': R['runs'].get('warmup', {}).get('decode_ms_median'),
               'derived_reciprocal_median_step_tokens_s': 1000 / statistics.median(dm),
               'historical_spark_graph_ms': spark_ms,
               'historical_spark_graph_time_ratio': spark_ms / statistics.median(dm) if spark_ms else None,
               'first_divergence_from_spark': [r['first_divergence_from_spark'] for r in timed]}
        walls = [r.get('process_wall_s_from_l3_log') for r in timed]
        if all(w is not None for w in walls): agg['timed_process_wall_s_from_l3_log'] = walls; agg['median_process_wall_s_from_l3_log'] = statistics.median(walls)
    R['aggregate'] = agg
    native = timed[0]['ids'] if have and timed[0]['ids'] else GRAPH

    if published:
        calc = load(D / 'calculations.json')
        C['reproduces_published_decode_medians'] = agg.get('timed_decode_ms_medians') == calc['accepted_decode_ms']
        C['reproduces_published_median_decode_ms'] = agg.get('median_decode_ms') == calc['median_decode_ms']
        C['reproduces_published_median_prefill_ms'] = agg.get('median_prefill_ms') == calc['median_prefill_ms']
        C['reproduces_published_ratio'] = agg.get('historical_spark_graph_time_ratio') == calc['historical_spark_graph_time_ratio']
        for i, l in enumerate(TIMED, 1):
            s = load(D / 'native' / ('sample%d.json' % i))
            C['%s_matches_sample%d.json' % (l, i)] = (R['runs'][l]['prefill_ms'] == s['prefill_ms'] and R['runs'][l]['decode_ms_median'] == s['decode_ms_median']
                                                      and R['runs'][l]['first_divergence_from_spark'] == s['first_divergence'])
        # negative control: one altered id must be refused by the shape check
        bad = IDS.sub(lambda m: m.group(0).replace('[', '[9', 1), src('run1', 'txt').read_text(), count=1)
        C['negative_control_altered_id_refused'] = not parse_run(bad, P, G)['shape_ok']
    else:
        g = load(out / 'uxl-graph-fresh.json')
        if 'ids' in g:
            R['graph_fresh'] = {k: v for k, v in g.items() if k not in ('ids', 'top5')}
            R['graph_fresh'].update(n_ids=len(g['ids']), ids_equal_published_graph=g['ids'] == GRAPH, first_divergence_from_spark=first_div(g['ids'], G))
            C['graph_fresh_256_ids'] = len(g['ids']) == len(G)
            C['every_run_ids_equal_graph_fresh'] = len(allrun) == 6 and all(r['ids'] == g['ids'] for r in allrun)
            if ctl: C['control_ids_differ_from_graph_fresh'] = ctl['ids'] != g['ids']
        else: R['graph_fresh'] = g
        ref = load(out / 'uxl-reference-fresh.json')
        if 'ids' in ref:
            R['reference_fresh'] = {k: v for k, v in ref.items() if k not in ('ids', 'top5')}
            R['reference_fresh'].update(n_ids=len(ref['ids']), ids_equal_native=ref['ids'] == native, first_divergence_from_native=first_div(ref['ids'], native),
                                        ids_equal_spark=ref['ids'] == G, first_divergence_from_spark=first_div(ref['ids'], G))
            C['reference_fresh_256_ids'] = len(ref['ids']) == len(G)
        else: R['reference_fresh'] = ref
        st = load(out / 'uxl-stock.json')
        R['stock'] = {'scalars': scalars(st), 'id_lists': [{'path': p, 'ids_equal_native': v == native, 'first_divergence_from_native': first_div(v, native),
                                                             'ids_equal_spark': v == G, 'first_divergence_from_spark': first_div(v, G)} for p, v in find_id_lists(st)]}
        # stock-cell.py rows: decode_ms = intervals between StoppingCriteria calls after tokens 1..256 (255 values),
        # first_token_s = call start to the first StoppingCriteria call; values are the cell's own, copied, not recomputed
        rows = st.get('rows') if isinstance(st.get('rows'), list) else []
        R['stock']['rows'] = [{k: x.get(k) for k in ('label', 'total_s', 'first_token_s', 'decode_ms_median', 'decode_ms_min', 'decode_ms_max', 'prompt_tokens', 'calls')}
                              | {'n_decode_ms': len(x.get('decode_ms') or []), **compare_stock(x.get('generated_ids'), native, G)} for x in rows if isinstance(x, dict)]
        tr = [x for x in R['stock']['rows'] if x['label'] in TIMED and isinstance(x.get('decode_ms_median'), (int, float))]
        if len(tr) == 5: R['stock']['median_of_timed_decode_ms_median'] = statistics.median(x['decode_ms_median'] for x in tr); R['stock']['median_of_timed_total_s'] = statistics.median(x['total_s'] for x in tr)
    for r in R['runs'].values(): r.pop('ids', None)          # ids live in the input files; per-step us are kept
    R['all_checks_pass'] = all(v is True for v in C.values())

    # ---- text table
    w = print
    w('analyze.py  %s  CUDApp %s  definitions: benchmark-qualified.py lines 8, 66-69' % (R['mode'], R['cudapp_head']))
    w('%-8s %6s %5s %8s %13s %17s %10s %10s %11s' % ('run', 'steps', 'ids', 'shape', 'prefill_ms', 'decode_median_ms', '=graph', 'spark_div', 'wall_s_l3'))
    for l in LABELS:
        r = R['runs'].get(l)
        if not r: w('%-8s MISSING' % l); continue
        f = lambda v, d: ('%.' + str(d) + 'f') % v if isinstance(v, (int, float)) else '-'
        w('%-8s %6d %5d %8s %13s %17s %10s %10s %11s' % (l, r['steps'], r['n_ids'], r['shape_ok'], f(r['prefill_ms'], 3), f(r['decode_ms_median'], 3),
                                                       r['ids_equal_published_graph'], r['first_divergence_from_spark'], f(r.get('process_wall_s_from_l3_log'), 3)))
    for k in ('median_decode_ms', 'median_prefill_ms', 'derived_reciprocal_median_step_tokens_s', 'historical_spark_graph_ms', 'historical_spark_graph_time_ratio',
              'median_process_wall_s_from_l3_log'):
        if k in agg: w('%-42s %s' % (k, agg[k]))
    for name in ('graph_fresh', 'reference_fresh'):
        x = R.get(name)
        if isinstance(x, dict) and 'n_ids' in x:
            w('%-16s ids=%d decode_ms_median=%s first_div_spark=%s %s' % (name, x['n_ids'], x.get('decode_ms_median'), x.get('first_divergence_from_spark'),
              ('equal_native=%s first_div_native=%s' % (x['ids_equal_native'], x['first_divergence_from_native'])) if 'ids_equal_native' in x else 'equal_published_graph=%s' % x['ids_equal_published_graph']))
    if 'stock' in R:
        w('stock            %d id list(s) of 256 found; scalars: %d' % (len(R['stock']['id_lists']), len(R['stock']['scalars'])))
        for x in R['stock'].get('rows', []):
            w('stock %-8s total_s=%s first_token_s=%s decode_ms_median=%s n_decode_ms=%s prompt_tokens=%s ids=%s first_div_native=%s first_div_spark=%s' % (
                x['label'], x['total_s'], x['first_token_s'], x['decode_ms_median'], x['n_decode_ms'], x['prompt_tokens'], x['n_ids'],
                x.get('first_divergence_from_native'), x.get('first_divergence_from_spark')))
        for k in ('median_of_timed_decode_ms_median', 'median_of_timed_total_s'):
            if k in R['stock']: w('stock %-36s %s' % (k, R['stock'][k]))
    for m in R['missing']: w('MISSING %s' % m)
    for k, v in C.items(): w('CHECK %-48s %s' % (k, 'PASS' if v is True else 'FAIL'))
    w('ANALYZE %s' % ('PASS' if R['all_checks_pass'] and not R['missing'] else 'FAIL'))
    if js: js.write_text(json.dumps(R, indent=1) + '\n'); w('wrote %s sha256=%s' % (js, sha(js)))
    sys.exit(0 if R['all_checks_pass'] and not R['missing'] else 1)

if __name__ == '__main__':
    main()
