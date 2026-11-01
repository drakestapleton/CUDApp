# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# External regression mutant: losing an explicit timing request must be detected.
from pathlib import Path
import os,resource,subprocess,sys,tempfile
resource.setrlimit(resource.RLIMIT_CORE,(0,0))
root=Path(__file__).resolve().parents[2]
source=(root/'om/gdn-batch/chip-check.c').read_text()
old='OmegaGpuGdnPfInfo *timing = info ? info : (batch_timing ? &local : NULL);'
new='OmegaGpuGdnPfInfo *timing = batch_timing ? (info ? info : &local) : NULL;'
assert source.count(old)==1
with tempfile.TemporaryDirectory(prefix='gdn-selector-mutant-') as td:
    t=Path(td);unit=t/'chip.c';check=t/'check.c';binary=t/'check'
    unit.write_text(source.replace(old,new).replace('#include "../../tests/gpu_gdn_test.c"','#include "'+str(root/'tests/gpu_gdn_test.c')+'"'))
    check.write_text((root/'om/gdn-batch/selector-check.c').read_text().replace('#include "chip-check.c"','#include "'+str(unit)+'"'))
    subprocess.run([os.environ.get('CC','cc'),'-std=gnu11','-O2','-ffp-contract=off','-I'+str(root/'src'),'-I'+str(root/'tests'),str(check),sys.argv[1],'-Wl,--wrap=omega_gpu_gdn_prefill_scoped','-lpthread','-lm','-o',str(binary)],check=True)
    result=subprocess.run([str(binary)],capture_output=True,text=True)
    assert result.returncode!=0 and 'Assertion' in result.stderr,(result.returncode,result.stderr)
    print('drop_requested_timing: caught')
