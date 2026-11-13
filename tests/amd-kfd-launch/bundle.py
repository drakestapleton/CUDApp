#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# External notebook-test artifact packaging; not a production dependency.
import argparse,base64,hashlib,json,lzma,subprocess
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument('--gate',type=Path,required=True)
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
REFERENCE_LABEL='REFERENCE (AMD runtime loading our object, not the Omega launcher)'
def add(name,data,source):
    bundle['assets'][name]={'sha256':sha(data),'xz_base85':base64.b85encode(lzma.compress(data,preset=9|lzma.PRESET_EXTREME)).decode(),'source':source}
binary=(a.gate/'launch-x86').read_bytes()
assert sha(binary)==summary['binaries']['x86']
add('omega-launch',binary,{'gate':'tests/amd-kfd-launch/check.py'})
vadd=(a.gate/'vadd.hsaco').read_bytes()
assert sha(vadd)==summary['objects']['vadd']
add('vadd.hsaco',vadd,{'repository_head':bundle['sources']['launcher_head'],'producer':'backend/amd/gfx942/hsaco.om'})
a.out.mkdir(parents=True,exist_ok=False)
for name,record in bundle['assets'].items():
    path=a.out/name; path.write_bytes(lzma.decompress(base64.b85decode(record['xz_base85'])))
    if name=='omega-launch': path.chmod(0o700)
text=(root/'tests/amd-kfd-launch/notebook.py.in').read_text()
assert text.count('@BUNDLE@')==1
cell=text.replace('@BUNDLE@',json.dumps(bundle,separators=(',',':')))
(a.out/'cell.py').write_text(cell)
(a.out/'bundle.json').write_text(json.dumps(bundle,indent=2)+'\n')
readme=f"""# launch3 notebook probe

READY for the authorized MI300X notebook diagnostic run. CPU launcher gates passed;
actual KFD/GPU execution and the HIP comparison remain NOT_RUN.

Paste all of `cell.py` into one notebook cell. It needs x86-64 Linux, Python 3
standard library, executable temporary storage, permission to open `/dev/kfd`,
the readable gfx942 topology node and its exact DRM render device. The comparison
needs the installed ROCm torch package and the libamdhip64 that it loads; no torch
kernels are run. No package/model downloads, credentials, strace or dmesg are needed.

Download `amdserve-launch3.txt`, including on failure. CONTROLS is first, every
launcher stdout line is saved as it arrives, and RESULT is last. The direct vadd
uses its existing timeout, printed as `OMEGA diag wait_timeout_ns`, and existing
dispatch bytes. It reads queue pointers, signal, packet header/addresses, loaded
descriptor fields and first eight output words on success or timeout. Values
are decimal; create_queue_input_qword_N is the little-endian u64 at byte offset
8*N in the 88-byte ioctl input, including compiler-bound mapping addresses.
Returned queue ID and full doorbell offset are separate rows.

The notebook reads the child's KFD proc directory and queue/occupancy/VRAM files
while signal_wait runs. Missing/inaccessible files are reported, not inferred.
It then runs the identical vadd object and inputs once through HIP, even if Omega
timed out. These rows are labelled `{REFERENCE_LABEL}`. No new doorbell value,
extra Omega packet, alternative dispatch, other kernel, or Spark test is attempted.

Interpret observations together, not as a diagnosis: unchanged read pointer
with advanced write pointer may indicate CP did not consume the packet; expected
output with a nonzero signal may indicate execution without signal completion;
NaN fill means those output words were not observed written. Asynchronous reads
are not a coherent device-wide snapshot. HIP success alone does not establish
why direct dispatch failed. Host dispatch timing is not GPU event timing.

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
