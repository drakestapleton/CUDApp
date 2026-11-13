# launch2 CPU acceptance

Intent: select the readable gfx942 device in a container, expose each launcher step and errno, and preserve the notebook's complete diagnostic receipt.

`receipt.json` is the generated command/exit/source/compiler/binary/bundle record. `summary.json` and `notebook-check.json` are the gate outputs; `logs/` retains the commands' output and earlier failed attempts. `bin/` and `bundle/` contain the measured artifacts, not a production installation.

`node-3-only.txt` is generated from the actual native syscall-instrument stdout. The instrument supplies a sorted fake topology, CPU nodes, and an EPERM node before the only readable gfx942 node. It independently checks the selected GPU ID and render minor at the ioctl/device-open boundary. Both gpu_id-open and properties-open denial are exercised. Abort-on-CPU and abort-on-EPERM mutants must fail; a missing selected render device must report its errno rather than substitute another GPU.

Notebook acceptance executes the actual x86 binary under qemu for malformed-input and absent-device controls. A real Python child waits until its first stdout line appears in the receipt before exiting, distinguishing streaming from capture-after-exit. Forced execution failure and corrupted-asset controls require CONTROLS first and RESULT last. Reference arithmetic is checked independently with exact rational sums.

The earlier compile failure exhausted Omega's local register pool; the node-number accumulator was extracted into its own function. The subsequent fixture failure exposed filesystem directory ordering: it reached node 3 before any skipped node. The test instrument now orders its supplied records, without changing or hard-coding production discovery. Original logs and failing source/results are retained under `investigation/` and `logs/`.

EXERCISED: CPU instrumentation and emulated x86 paths only. Actual MI300X KFD execution, GPU timings, model serving, independent final-source review, merge and deployment remain NOT_RUN. READY in the generated bundle README means ready for the already-authorized notebook probe, not hardware-qualified. The generated cell size is recorded in that README and receipt; it exceeds the preferred size, with no truncation of objects or diagnostics.
