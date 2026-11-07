# REPRO-COST-20261107: author-side clean-room reproduction of the portable application (DRA-202)

Card cuda202 (kanban t_c2fbba86). Followed `docs/repro/README.md` exactly, from fresh clones of the pushed
branch `hive/t_c2fbba86-repro-cost` at `047b1d3e8e2085ba95c09e0617b07c650443ea56` and of omega-mv at
`369dbc96`, with a cleared environment (`env -i`, `PATH=/usr/bin:/bin`, an empty `HOME`), on the two
machines DRA-201 used. The exact driver scripts are in `driver/`. This is the AUTHOR reproducing his own
package on his own machines: it shows that the written steps are complete for these machines. It is not
an independent reproduction (NOT_RUN; nobody outside AIEN was contacted).

## Results

| Step | Machine | Result | Receipt |
|---|---|---|---|
| 1 sources | Spark | omc sha256 `2c82480596bb…029f` = pin; CUDApp `047b1d3e` | `spark-producer/stepA.log` |
| 2 `fetch-tools.sh` | Spark (spark-b87b, Ubuntu 24.04.5, kernel 7.0.0-1019-nvidia) | `FETCH PASS`, 3/3 packages equal their pinned sha256 | `spark-producer/stepA.log` |
| 3 `produce.sh` | Spark CPU (cores 10-11, pool lock), lavapipe | `MULTITARGET PASS pass=51 fail=0 not_run=3`; `REPRO PRODUCE PASS files=14` (all 14 producer outputs equal the DRA-201 digests) | `spark-producer/run.log`, `produce.check` |
| cost meter | Spark CPU | `cost.sh` output byte-equal to `tests/repro/cost.expect`; `REPRO-GATE PASS pass=42 fail=0` | `spark-producer/cost.out`, `repro-gate.log` |
| 4 `replay-nvidia.sh` | **GB10**, NVIDIA driver 580.173.02, window `20261107T030800Z-377574` (`--gb10 exclusive --quiet exclusive`, open 03:08:10Z, close 03:08:41Z, held 41 s, "quiet machine") | `NVIDIA-GPU PASS pass=38 fail=0 not_run=3`; `REPRO NVIDIA PASS files=6` | `gb10-gpu/nvidia-gpu.log`, `replay.check` |
| 5 `replay-apple.sh` | **Apple M2** (Mac14,2, 8 GB, macOS 26.6.2), MoltenVK 1.4.2 downloaded by the script (tar sha256 = pin) | `APPLE-GPU PASS pass=20 fail=0`; `REPRO APPLE PASS files=6` | `m2-gpu/apple-gpu.log`, `replay.check` |
| cross-target | GB10 vs M2 outputs of this run | all 6 output files have the same sha256 on both GPUs (and equal the DRA-201 digests) | `outputs.sha256`, `m2-gpu/outputs.sha256` |

NOT_RUN rows inside the passing gates, and why:

- producer (3): the OpenCL runs on PoCL (`POCL_LIBRARY` not set; the Vulkan runs on lavapipe did run), the
  AMDGPU_HSACO offline checks (no `llvm-mc`/`ld.lld` given) and the `hello_gpu --sim` cross-check
  (`HELLO_GPU` not set). All three are optional for the GPU replays; DRA-201's producer run had PoCL off too.
- GB10 (3): the CUDA++ native baseline `hello_gpu` and its control (optional, not built for this run; DRA-201
  ran them 10/10), and the in-script cross-target compare (`MT_COMPARE_DIR` not set; the cross-target
  comparison was done by digest instead, row above).

## What this does and does not show

- Shows: on these two machines, the committed steps alone, with no state from the author's account,
  rebuild the application from Omega source and reproduce DRA-201's outputs bit for bit on both GPU
  vendors, and the cost table recomputes byte for byte.
- Does not show: that another person, another NVIDIA GPU, another Mac or an x86 host can do it (see the
  README's support matrix and open limits), any own-engine execution path (both GPU routes are vendor
  runtimes), or anything about the cost ratio R (NOT_RUN).

## Incident, recorded

The first GB10 attempt (window `20261107T025904Z-d0e9ce`) took the CPU pool lock inside the GPU window, waited
on it and was cut by the agent's tool timeout; no GPU job ran and the window was closed unused after 4 min
(BOARD 03:06Z). `driver/stepB.sh` now takes the pool lock before the window opens.

## Packaging cost (this card), MEASURED snapshot at 2026-11-07T03:10:57Z

From the Hermes record of this card's session (claimed 1794017990): 3067 s wall-clock (0.85 h), 117 model
calls, output tokens 130749, cache-read 30915357, cache-write 971252, uncached input 236, estimated USD
13.6552554 (list price, `cost_status estimated`), exclusive GB10 time 41 s (plus 4 min of the unused first
window). The session continued after this snapshot (commit, review handoff, issue updates); the final
figure is in the DRA-202 issue comment. Human hands-on time: none recorded.

Arithmetic: `wolfram.txt`.
