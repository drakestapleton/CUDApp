# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# External call-isolation fault tests; not a production dependency.
from pathlib import Path
import os,subprocess,sys,tempfile
r=Path(__file__).resolve().parents[2]
archive=Path(sys.argv[1]).resolve();src=(r/'src/omega_gpu_pfgemm.c').read_text()
mutants={
 'ignore_scoped_options':('if (scoped) s = *scoped;','if (scoped && 0) s = *scoped;'),
 'leak_options_to_global':('const OmegaQgSpec s = qg_scoped_spec(rt, warps, wait, group);','omega_gpu_pf_qgemm_set_shape(rt, warps, wait); omega_gpu_pf_qgemm_set_group(group);\n    const OmegaQgSpec s = qg_scoped_spec(rt, warps, wait, group);'),
 'wrong_tile_height':('uint64_t p[32] = {5, 0, 16, s.rt, 1};','uint64_t p[32] = {5, 0, 8, s.rt, 1};'),
}
with tempfile.TemporaryDirectory(prefix='qg-scoped-faults-') as d:
 d=Path(d)
 for name,(old,new) in mutants.items():
  assert src.count(old)==1,name
  f=d/(name+'.c');f.write_text(src.replace(old,new));exe=d/name
  subprocess.run([os.environ.get('CC','cc'),'-std=gnu11','-O2','-Wall','-Wextra','-Werror','-I'+str(r/'src'),str(f),str(r/'om/prefill-shape/scoped-check.c'),str(archive),'-Wl,--wrap=omega_gpu_pf_open_locked','-Wl,--wrap=omega_gpu_pf_kernel_locked','-lpthread','-lm','-o',str(exe)],check=True)
  q=subprocess.run([str(exe)],capture_output=True,text=True)
  assert q.returncode==1 and 'scoped qgemm isolation: FAIL' in q.stdout,(name,q.returncode,q.stdout,q.stderr)
  print(name+': caught',flush=True)
