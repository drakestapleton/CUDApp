# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# External fake-transport mutation checker. No GPU or production dependency.
from pathlib import Path
import os,resource,subprocess,sys,tempfile
resource.setrlimit(resource.RLIMIT_CORE,(0,0))
root=Path(__file__).resolve().parents[2]
source=(root/'src/omega_gpu_gdn_api.c').read_text()
test=(root/'om/gdn-batch/dispatch-check.c').read_text()
wrap=['omega_gpu_session_lock','omega_gpu_session_unlock','omega_gpu_session_open','omega_gpu_session_on_close','omega_gpu_session_scratch','omega_gpu_session_scratch_free','omega_gpu_session_sync_to_gpu','omega_gpu_session_sync_to_cpu','omega_gpu_cmdlist_create','omega_gpu_cmdlist_free','omega_gpu_cmdlist_reset','omega_gpu_cmdlist_count','omega_gpu_cmdlist_add','omega_gpu_cmdlist_submit','omega_gpu_cmdlist_stamps','omega_gpu_session_launch']
mutants={
 'ignore_failed_submit':('pf_batch_action(batch, 4, &batch_stats, submitted)','pf_batch_action(batch, 4, &batch_stats, 1)'),
 'ignore_completed_count':('const uint64_t done = stats ? stats->sems_done : 0;','const uint64_t done = stats ? 3 : 0;'),
 'ignore_missing_completion':('stats ? (uint64_t)(int64_t)stats->first_missing : UINT64_MAX','UINT64_MAX'),
 'ignore_submitted_count':('stats ? stats->launches : 0','stats ? 3 : 0'),
 'ignore_timestamp_refusal':('ns == UINT64_MAX) rc = OMEGA_GPU_GDN_CHIP_FAIL','ns == UINT64_MAX && 0) rc = OMEGA_GPU_GDN_CHIP_FAIL'),
 'ignore_readback_failure':('!rec && omega_gpu_session_sync_to_cpu(&g->pf.mem) != 0)', '!rec && omega_gpu_session_sync_to_cpu(&g->pf.mem) != 0 && 0)'),
 'ignore_poison':('if (unchanged) { omega_gpu_session_set_error("gdn prefill: output poison survived")','if (unchanged && 0) { omega_gpu_session_set_error("gdn prefill: output poison survived")'),
 'skip_list_release':('    omega_gpu_cmdlist_free(g->pf_batch);','    (void)g->pf_batch;'),
}
with tempfile.TemporaryDirectory(prefix='gdn-batch-dispatch-mutants-') as td:
 t=Path(td)
 for name,(old,new) in mutants.items():
  assert source.count(old)==1,(name,source.count(old))
  c=t/(name+'.c');c.write_text(source.replace(old,new));check=t/(name+'-check.c');check.write_text(test.replace('#include "../../src/omega_gpu_gdn_api.c"','#include "'+str(c)+'"'));exe=t/name
  subprocess.run([os.environ.get('CC','cc'),'-std=gnu11','-O2','-I'+str(root/'src'),str(check),sys.argv[1],*['-Wl,--wrap='+x for x in wrap],'-lpthread','-lm','-o',str(exe)],check=True)
  r=subprocess.run([str(exe)],capture_output=True,text=True)
  assert r.returncode!=0 and 'Assertion' in r.stderr,(name,r.returncode,r.stderr)
  print(name+': caught',flush=True)
