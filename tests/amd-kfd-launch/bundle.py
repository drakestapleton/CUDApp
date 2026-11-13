#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# External notebook-test artifact packaging; not a production dependency.
import argparse,base64,hashlib,json,subprocess,zlib
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument('--gate',type=Path,required=True)
p.add_argument('--gemv-tree',type=Path,required=True)
p.add_argument('--attention-tree',type=Path,required=True)
p.add_argument('--out',type=Path,required=True)
a=p.parse_args()
root=Path(__file__).resolve().parents[2]
def sha(b): return hashlib.sha256(b).hexdigest()
def git(tree,*args): return subprocess.check_output(['git','-C',str(tree),*args])
summary=json.loads((a.gate/'summary.json').read_text())
assert summary['failures']==0
compiler=(root/'backend/amd/kfd/launch/COMPILER').read_text().splitlines()
assert compiler[1].split()[0]==summary['compiler']
bundle={'sources':{'launcher_head':git(root,'rev-parse','HEAD').decode().strip(),'compiler_head':compiler[0].split()[0],'compiler_sha256':summary['compiler'],'launcher_source_sha256':{str(f.relative_to(root)):sha(f.read_bytes()) for f in sorted((root/'backend/amd/kfd/launch').glob('*.om'))}},'assets':{}}
bundle['sources']['notebook_template_sha256']=sha((root/'tests/amd-kfd-launch/notebook.py.in').read_bytes())
def add(name,data,source):
    bundle['assets'][name]={'sha256':sha(data),'zlib_base64':base64.b64encode(zlib.compress(data,9)).decode(),'source':source}
binary=(a.gate/'launch-x86').read_bytes()
assert sha(binary)==summary['binaries']['x86']
add('omega-launch',binary,{'gate':'tests/amd-kfd-launch/check.py'})
vadd=(a.gate/'vadd.hsaco').read_bytes()
assert sha(vadd)==summary['objects']['vadd']
add('vadd.hsaco',vadd,{'repository_head':bundle['sources']['launcher_head'],'producer':'backend/amd/gfx942/hsaco.om'})
def committed(tree,path,name):
    commit=git(tree,'log','-1','--format=%H','--',path).decode().strip()
    assert commit
    data=git(tree,'show',commit+':'+path)
    # The exact committed object is used, never a moving uncommitted substitute.
    add(name,data,{'commit':commit,'path':path})
for kind in ('fp8','bf16'):
    committed(a.gemv_tree,'evidence/AMD-GFX942-GEMV2/objects/'+kind+'gemv2.hsaco',kind+'gemv2.hsaco')
for kernel in ('part','merge','prep','gate'):
    committed(a.attention_tree,'evidence/AMD-GFX942-PREP/objects/attn256_'+kernel+'.hsaco','attn256_'+kernel+'.hsaco')
for group,name in (('GDNCONV4','gdnconv4'),('RMSNORM5120','rmsnorm5120'),('MLPSILU','mlpsilu')):
    committed(a.attention_tree,'evidence/AMD-GFX942-'+group+'/objects/'+name+'.hsaco',name+'.hsaco')
# Exact device-qualified object choices specified by the integrating lane.
required={'fp8gemv2.hsaco':'99eaf23f9af959db5a16cc5d7e6392dc32cfb2ee6b721797554e6bd62eecd0a0','bf16gemv2.hsaco':'4ef73f616043b8bf007b025d1ec62635be26c80a3eeb09a681d0f38386e9aaeb','attn256_part.hsaco':'9d2599e672abe87082f885f46870c7d31fddd3dff8e5e43b069e32a59a8f5f95','attn256_merge.hsaco':'a721017369917d2b757095198877f16f9aa41a8955dbf4bf55d73e29f83b2ec3'}
for name,digest in required.items(): assert bundle['assets'][name]['sha256']==digest,(name,'not the approved object')
a.out.mkdir(parents=True,exist_ok=False)
for name,record in bundle['assets'].items():
    path=a.out/name; path.write_bytes(zlib.decompress(base64.b64decode(record['zlib_base64'])))
    if name=='omega-launch': path.chmod(0o700)
text=(root/'tests/amd-kfd-launch/notebook.py.in').read_text()
assert text.count('@BUNDLE@')==1
cell=text.replace('@BUNDLE@',json.dumps(bundle,separators=(',',':')))
(a.out/'cell.py').write_text(cell)
(a.out/'bundle.json').write_text(json.dumps(bundle,indent=2)+'\n')
readme=f"""# launch2 notebook probe

READY for the authorized MI300X notebook run. CPU launcher gates passed;
actual KFD/GPU execution, timings and full 27B serving remain NOT_RUN.

Paste all of `cell.py` into one notebook cell. It needs x86-64 Linux, Python 3
standard library, libm, executable temporary storage, and permission to open
`/dev/kfd`, the readable gfx942 topology node and its exact DRM render device.
The render minor comes from sysfs, not a guessed device index. A missing render
device is reported with errno and stops the run; no other GPU is substituted.
The optional comparison needs libamdhip64. No packages or model downloads,
credentials, strace, dmesg, or shell paste are required.

Download `amdserve-launch2.txt` afterward, including on failure. CONTROLS is first,
the observed topology table and every launcher stdout line are persisted as they
arrive, and RESULT is last. Temporary inputs, outputs and raw stdout/stderr remain
at the path printed in the receipt. Direct launches are labelled OMEGA; same-object
HIP measurements are labelled `OMEGA objects via AMD HIP loader`. There are no
torch rows or Spark measurements. Host dispatch timing includes diagnostic writes
between dispatch and signal-wait boundaries and is not GPU event time.

Cell bytes: {len(cell.encode())}
Cell sha256: {sha(cell.encode())}
Binary sha256: {sha(binary)}
Launcher source commit: {bundle['sources']['launcher_head']}
Compiler source commit: {bundle['sources']['compiler_head']}
Compiler sha256: {summary['compiler']}

`bundle.json` records every object source commit and digest. `SHA256SUMS` covers
every delivered file, including this README and cell.py, except itself.
"""
(a.out/'README.md').write_text(readme)
files=sorted(f for f in a.out.iterdir() if f.is_file())
(a.out/'SHA256SUMS').write_text(''.join(sha(f.read_bytes())+'  '+f.name+'\n' for f in files))
print(json.dumps({'cell_bytes':len(cell.encode()),'cell_sha256':sha(cell.encode()),'binary_sha256':sha(binary),'assets':{k:v['sha256'] for k,v in bundle['assets'].items()},'device':'NOT_RUN'},sort_keys=True))
