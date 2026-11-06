#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Attributed source mutants; compile common objects once, no checked-in mutation."""
import os
from pathlib import Path
import shlex
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
bridge = Path(os.environ['OMEGA_BRIDGE'])
env = dict(os.environ, OMEGA_SEEDED_OMC=os.environ['OMEGA_OMC'])
cc = shlex.split(os.environ.get('CC', 'cc'))
flags = ['-std=gnu11', '-O2', '-Wall', '-Wextra', '-Werror', '-I' + str(root / 'src')]
math = ['om/components/cm_math.om', 'om/components/cm_math_extra.om', 'om/components/cm_math_erfc.om']
units = {name: math + ['om/components/' + name + '.om'] for name in (
    'cm_fit', 'cm_stats', 'cm_features', 'cm_quantile', 'cm_decision', 'cm_prediction')}
units['cm_math'] = math
units['cm_sort'] = ['om/components/cm_sort.om']
units['cm_calibration'] = math + ['om/components/cm_quantile.om', 'om/components/cm_calibration.om']
units['cm_blob'] = ['om/components/cm_blob.om']
units['cm_digest'] = ['om/cache/digest.om', 'om/components/cm_digest.om']
units['cm_route'] = ['om/components/cm_route.om']
mutants = (
    ('cm_features', 'cm_features', 'ret 4294967295', 'ret 0', 'rx_costmodel_unit', ['refuse']),
    ('cm_stats', 'cm_stats', 'put out 45 add get out 45 1', 'put out 45 add get out 45 2', 'rx_costmodel_unit', []),
    ('cm_fit', 'cm_fit', 'sigma = div b a', 'sigma = mul b a', 'rx_costmodel_unit', []),
    ('cm_quantile', 'cm_quantile', 'ret bitcast ulong add add z a b', 'ret bitcast ulong neg add add z a b', 'rx_costmodel_unit', []),
    ('cm_prediction', 'cm_prediction', 'put words 17 bitcast ulong mul sd scale', 'put words 17 bitcast ulong add sd scale', 'rx_costmodel_unit', []),
    ('cm_decision', 'cm_decision', 'put data 100 ulong 2', 'put data 100 ulong 3', 'rx_costmodel_unit', []),
    ('cm_calibration', 'cm_calibration', 'if lt scale f64 1.0', 'if gt scale f64 1.0', 'rx_costmodel_unit', []),
    ('cm_blob', 'cm_blob', 'put words 16 ulong 5591222095', 'put words 16 ulong 5591222094', 'cm_blob_unit', []),
    ('cm_digest', 'cm_digest', 'put data q uchar and v 255', 'put data q uchar and v 254', 'cm_blob_unit', []),
    ('cm_math', 'cm_math_erfc', 'ret bitcast ulong cm_log bitcast f64 bits false', 'ret bitcast ulong cm_log bitcast f64 bits true', 'cm_math_unit', []),
)

with tempfile.TemporaryDirectory(prefix='cudapp-cm-mutants-') as temporary:
    tmp = Path(temporary)
    def compile_object(source, name, extra=()):
        obj = tmp / (name + '.o')
        subprocess.run(cc + flags + list(extra) + ['-c', str(source), '-o', str(obj)], check=True)
        return str(obj)
    originals = {name: compile_object(root / 'src' / ('cudapp_om_' + name + '.c'), name) for name in units}
    common = [compile_object(root / 'src/runtime/rx_costmodel.c', 'model', ['-ffp-contract=off']),
              compile_object(root / 'src/sha256.c', 'sha'),
              compile_object(root / 'tests/runtime/cm_oracle.c', 'oracle', ['-ffp-contract=off'])]
    tests = {name: compile_object(root / 'tests/runtime' / (name + '.c'), name) for name in (
        'rx_costmodel_unit', 'cm_blob_unit', 'cm_math_unit')}
    verifier = compile_object(root / 'tests/runtime/cm_decision_diff.c', 'decision-verifier', ['-DCM_DIFF_REPLAY_MAIN'])
    disabled = subprocess.run(cc + flags + ['-DNDEBUG', '-c', str(root / 'tests/runtime/cm_decision_diff.c'),
                              '-o', str(tmp / 'disabled.o')], capture_output=True, text=True)
    assert disabled.returncode != 0 and 'requires enabled assertions' in disabled.stderr
    print('CM DECISION verifier control PASS: assertion-disabled build refused', flush=True)
    for name, part, old, new, test, args in mutants:
        contents = []
        changed = 0
        for path in units[name]:
            source = (root / path).read_text()
            if Path(path).stem == part:
                assert source.count(old) == 1, (name, old, source.count(old))
                source = source.replace(old, new)
                changed += 1
            contents.append(source)
        assert changed == 1
        om = tmp / (name + '-mutant.om')
        om.write_text('\n'.join(contents) + '\n')
        generated = tmp / (name + '-mutant.c')
        with generated.open('w') as output:
            subprocess.run(['sh', str(bridge / 'tools/bridge-seeded'), 'c', str(om)],
                           env=env, stdout=output, check=True)
        mutated = compile_object(generated, name + '-mutant')
        objects = [mutated] if test == 'cm_math_unit' else common + [
            mutated if key == name else value for key, value in originals.items()]
        executable = tmp / (name + '-unit')
        subprocess.run(cc + [tests[test]] + objects + ['-lm', '-o', str(executable)], check=True)
        result = subprocess.run([str(executable)] + args, capture_output=True, text=True)
        failures = [line for line in result.stderr.splitlines() if 'FAIL' in line or 'Assertion' in line]
        assert result.returncode != 0 and failures, (name, result.returncode, result.stdout, result.stderr)
        print('CM SOURCE mutant caught:', name, 'rc=' + str(result.returncode), failures[0], flush=True)
        if name == 'cm_decision':
            executable = tmp / 'decision-verifier-mutant'
            wraps = ['-Wl,--wrap=' + name for name in ('rx_cm_decide', 'rx_cm_predict',
                     'rx_cm_init', 'rx_cm_observe', 'rx_cm_calibrate', 'rx_cm_charge')]
            subprocess.run(cc + [verifier] + objects + wraps + ['-lm', '-o', str(executable)], check=True)
            result = subprocess.run([str(executable)], capture_output=True, text=True)
            assert result.returncode != 0 and 'out->action == expected.action' in result.stderr, result
            print('CM DECISION verifier control PASS: discrete action mutant caught', flush=True)
print('CM SOURCE mutants PASS (10 attributed arithmetic/header/digest/decision mutants)')
