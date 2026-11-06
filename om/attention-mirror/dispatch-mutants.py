# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# External transport mutants: the actual-dispatch checker must reject each fault.
from pathlib import Path
import os,subprocess,sys,tempfile
root=Path(__file__).resolve().parents[2]
source=(root/'src/omega_gpu_prefill_attn256_api.c').read_text()
test=(root/'om/attention-mirror/dispatch-check.c').read_text()
wrap=['omega_gpu_session_lock','omega_gpu_session_unlock','omega_gpu_session_open','omega_gpu_session_on_close','omega_gpu_decode_attn256_cache_view','omega_gpu_session_sync_to_gpu','omega_gpu_session_sync_to_cpu','omega_gpu_session_free','omega_gpu_pf_alloc_dev_locked','omega_gpu_pf_copy_va_locked','omega_gpu_session_launch']
mutants={
 'copy_failure_ignored':('if (omega_gpu_pf_copy_va_locked(g.mirror.va, v.k_va, (size_t)mirror_bytes, copy_ns) != OMEGA_GPU_PF_OK)','if (omega_gpu_pf_copy_va_locked(g.mirror.va, v.k_va, (size_t)mirror_bytes, copy_ns) != OMEGA_GPU_PF_OK && 0)'),
 'wrong_k_pointer':('attend[1] = g.mirror.va;', 'attend[1] = v.k_va;'),
 'scoped_option_ignored':('uint64_t mirror_bytes = 0;', 'mirror_mode = 0; uint64_t mirror_bytes = 0;'),
 'failed_hook_ignored':('if (omega_gpu_session_on_close(on_session_close) != 0)', 'if (omega_gpu_session_on_close(on_session_close) != 0 && 0)'),
}
with tempfile.TemporaryDirectory(prefix='attention-dispatch-mutants-') as td:
 t=Path(td)
 for name,(old,new) in mutants.items():
  assert source.count(old)==1,(name,source.count(old))
  c=t/(name+'.c');c.write_text(source.replace(old,new));check=t/(name+'-check.c');check.write_text(test.replace('#include "../../src/omega_gpu_prefill_attn256_api.c"','#include "'+str(c)+'"'))
  exe=t/name
  subprocess.run([os.environ.get('CC','cc'),'-std=gnu11','-O2','-I'+str(root/'src'),'-I'+str(root/'om/attention-mirror'),str(check),sys.argv[1],*['-Wl,--wrap='+x for x in wrap],'-lpthread','-lm','-o',str(exe)],check=True)
  p=subprocess.run([str(exe)],capture_output=True,text=True)
  assert p.returncode!=0 and 'Assertion' in p.stderr,(name,p.returncode,p.stderr)
  print(name+': caught',flush=True)
