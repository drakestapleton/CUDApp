# launch3 notebook probe

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
timed out. These rows are labelled `REFERENCE (AMD runtime loading our object, not the Omega launcher)`. No new doorbell value,
extra Omega packet, alternative dispatch, other kernel, or Spark test is attempted.

Interpret observations together, not as a diagnosis: unchanged read pointer
with advanced write pointer may indicate CP did not consume the packet; expected
output with a nonzero signal may indicate execution without signal completion;
NaN fill means those output words were not observed written. Asynchronous reads
are not a coherent device-wide snapshot. HIP success alone does not establish
why direct dispatch failed. Host dispatch timing is not GPU event timing.

Cell bytes: 41990
Cell sha256: 58ad5598c5e1412320f841b7a605629ac6cdfa1ae3b905bed5d8704263b110b9
Binary sha256: 58b4d956b77394ef94509ebf3c42cbef81bb796642f4d9f554737b9c2a2cb2b9
Launcher source commit: 6e13387936a5b1ca56c6a4ed59bcec9a4b1cb2fb
Compiler source commit: 290ad5feb9fa34b47e3672f4921f16de7807fca1
Compiler sha256: ca20b6a5181d63b03d2dbbd2bdafd5cae225116e477997853f6f45d9d7d9641b

`bundle.json` records every object source commit and digest. `SHA256SUMS` covers
every delivered file, including this README and cell.py, except itself.
