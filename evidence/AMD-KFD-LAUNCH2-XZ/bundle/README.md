# launch2 notebook probe

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

Cell bytes: 55308
Cell sha256: d8a27241ac7b000d9dda5384b929de6985112c96d2cfd83e8c31a6b1502e9d7f
Binary sha256: 70e63de39a28be6c8aa1bde9ac46ca93797e5bd4ea8633fddd5be3dae3e0070d
Launcher source commit: e3d6c654f4c836f623bf7261addf2ae838c486c4
Compiler source commit: 290ad5feb9fa34b47e3672f4921f16de7807fca1
Compiler sha256: ca20b6a5181d63b03d2dbbd2bdafd5cae225116e477997853f6f45d9d7d9641b

`bundle.json` records every object source commit and digest. `SHA256SUMS` covers
every delivered file, including this README and cell.py, except itself.
