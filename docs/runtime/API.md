# Omega Runtime API v0

Omega Runtime API version: 0.18.0

Status: first written version, 2026-10-19 (slice R1, omega-mv#594, tracking #593). Source of truth is the C headers in
`src/`. This file describes them; `docs/runtime/omega_runtime_api.manifest` lists them; `make test-runtime-api` checks the
list against the headers (a host check, no GPU). Review state: reviewed by Opus 5.5 against the headers and the code on 2026-10-22 (L386); its findings are fixed
in this version. The check proves the list and every quoted C signature match the headers; prose is checked by review.

The version number lives in one header, `src/omega_runtime_api.h` (`OMEGA_RUNTIME_API_VERSION_MAJOR`, `_MINOR`, `_PATCH`).

## 1. What this is

Omega is a GPU compute runtime that does not use CUDA. This file is the written contract for the part a program calls
directly: find and open the GPU, get memory, copy data in and out, run a kernel, run many kernels in one submission, run a
packaged job, and read errors. GB10 is the historical device target. The Linux reference now also has
an experimental sm_86 direct-driver path for GA102/GA104; both Ampere devices remain NOT_RUN.

Compatibility preflight has two parts. `make doctor` checks the kernel-module version, the PCI target and the rest of
the machine, and `make hello-gpu` runs it first. Since 0.14.0 the library also admits the GPU itself at device open:
before `nvrm_open` opens any `/dev/nvidia` node it reads `/sys/bus/pci/devices`, and `om/admission/chip_admit.om`
decides whether the GPU is one CUDA++ targets (GB10 `0x2e12` as sm_121; the prepared, NOT_RUN Ampere ids `0x2204`,
`0x2486`, `0x2489` as sm_86). An unknown or unqualified PCI id is refused with one sentence naming the address and id,
and no node is opened. `CUDAPP_GPU_BDF` (for example `000f:01:00.0`) picks one GPU when several are present; without
it every NVIDIA display GPU must be a target. The driver's card list is checked again against the same decision before
the device node opens, and the sm the driver's classes select must equal the admitted one. The driver-version check
stays in the doctor only. Callers must not bypass the preflight or the operator's GPU authorization procedure.
The M16 receipt imported here is a historical summary whose referenced raw logs
are absent; it is not current-commit qualification (see README evidence limitation).

How to read an entry:

| Tier | Meaning |
| --- | --- |
| **STABLE** | Used by other code in the project, covered by chip results, and kept compatible under the rule in section 2. Has its own `####` section below. |
| **EXPERIMENTAL** | Works and is listed, but may change in any minor version, because the design is still moving or only one use of it exists. Listed in tables with the reason. |
| **INTERNAL** | Exists in a header for the project's own libraries or tests. Not part of the API; not in the manifest; may change or disappear any time. Every such name in a manifest header is listed, with a reason, in `docs/runtime/omega_runtime_api.internal` (section 10), so nobody builds on it by accident. |

Every C signature in this file is copied from the header it names. `tools/runtime_api_check.sh` re-checks every quoted
signature against the headers on every run.

## 2. Version and compatibility rule

The version is MAJOR.MINOR.PATCH (semantic versioning), currently **0.18.0**. It is a property of the list of entries and
their behaviour, not of the library build. Changes so far:
- 0.18.0: adds experimental seeded row softmax/layernorm/rmsnorm, ReLU/GELU/SiLU and implicit conv2d, a shared DNN storage-format contract, and native f16 inputs to any-shape attention. Native scalar f16 conversion is implemented in the seeded sm_121 encoder. Shape admission and kernel arithmetic are Omega; C execution entries are memory/session shells. Hardware qualification is NOT_RUN.
- 0.17.0: adds experimental seeded-Omega arbitrary-shape f32/bf16 attention, `omega_gpu_attn_any`, with a CPU-only code-generation entry. Geometry, GQA and causal tails are decided in `om/attn-any/`; hardware qualification is NOT_RUN. Existing tuned model-shape entries are unchanged.
- 0.16.0: adds experimental complete-access-set command-list recording and whole-program planning/submission. The seeded planner compares all outstanding dispatches, drains RAW/WAR/WAW hazards (including overlapping VA aliases), retains admission and completion ordering, and refuses programs that cannot fit one submission. Legacy recording/submission remains conservative.
- 0.15.0: adds the EXPERIMENTAL random-number and FFT entries of section 9.10 (`src/omega_gpu_mathlibs_api.h`,
  a new manifest header). The header already held the xoshiro256** generator and the complex FFT outside the manifest; it
  now also has `omega_gpu_philox_generate` (Philox4x32-10, cuRAND's stream), `omega_gpu_fft_r2c`, `omega_gpu_fft_c2r`,
  `omega_gpu_fft_c2c_nd` (2-D and 3-D) and `omega_gpu_fft_plan_walk`, and all of them enter the manifest together.
  `omega_gpu_mathlibs_test_set_mutant` goes on the INTERNAL list. The same lane adds `omega_gpu_philox_generate_dist`
  (cuRAND's uniform, uniform_double, uniform2_double and normal on Philox, any offset), `omega_gpu_fft_many` and
  `omega_gpu_fft_many_walk` (cufftPlanMany layouts, any length 2..4096 by mixed radix 2/3/4/5/7 or Bluestein) with
  `OmegaGpuFftMany`, `OmegaGpuFftManyLaunch` and their constants; `omega_gpu_fft_c2c`, `_r2c`, `_c2r` and `_c2c_nd`
  now accept any length 2..4096 where they refused lengths that are not powers of two, and the callback of
  `omega_gpu_fft_plan_walk` runs after the session lock is released. No existing entry changes otherwise.
- 0.14.0: adds experimental `omega_runtime_api_version` in `omega_runtime_api.h`, which returns the
  `OMEGA_RUNTIME_API_VERSION` the linked library was built at (section 9.9), and library admission at device open
  (section 1): `nvrm_open`, under every session open, now refuses an unknown or unqualified PCI id, or a GPU the driver
  reports with an sm other than the admitted one, with a sentence and without opening a `/dev/nvidia` node. A machine
  that opened before still opens unless its GPU is not a CUDA++ target. Changed behaviour of existing entries while MAJOR
  is 0, so MINOR.
- 0.13.0: adds experimental `omega_gpu_prefill_attn256_scoped` in `omega_gpu_attention_mirror.h`. Mode 0 preserves ordinary attention; mode 1 copies the complete original 27B K/V into one GPU-private reusable buffer before attention; mode 2 uses the provisional Omega-selected 1024-row threshold. Omega checks geometry, bounds, allocation capacity and completed PREP/copy/ATTEND ordering. Other modes and unsupported mirrored shapes are refused. `copy_ns` reports copy chip time separately from the unchanged attention info layout; total call time includes allocation and copying. Existing host/device/recorded entries and defaults are unchanged. See `om/attention-mirror/README.md` for qualification status.
- 0.12.0: adds experimental `omega_gpu_pf_qgemm_reading_option` with `(rows, index)`. The checked Omega policy returns the scoped rt, warps, wait and group values at indices 0..3, or UINT32_MAX for invalid input. This proposed adaptive profile uses 64 rows for requests at most 64 rows and 128 above that threshold. Callers obtain the checked height and use `omega_gpu_pf_qgemm_workspace_fits` to check existing row, padded-row and tile capacities before switching profiles. The query returns 1 only when all capacities cover the request; otherwise callers allocate a compatible workspace.
- 0.11.0: adds experimental `omega_gpu_pf_qgemm_scoped` per-call options and the checked `omega_gpu_pf_qgemm_scoped_bm` tile-height query. Explicit calls neither read nor modify process-global tuning knobs. Tile storage must use the queried height; actual projection dimensions are validated on every call.
- 0.10.0: adds the experimental `omega_gpu_pf_qgemm_set_group` selector. Existing entry signatures and the default execution order are unchanged.
- 0.9.0 (AmperePort, CUDApp#170): grows STABLE `OmegaGpuLaunch` with `target_sm` (a MAJOR-class change
  taken as MINOR under the major-version-zero rule); adds EXPERIMENTAL
  `omega_gpu_session_target_sm`. Zero-initialized legacy launches still mean sm_121, not auto-detect.
  An explicit sm_86 launch must match the opened device and use sm_86 code and argument layout.
  Adds discrete VRAM allocation/BAR-mapped visibility and a VRAM-based D1 reserve;
  no Ampere hardware qualification is claimed. See `docs/PORT-AMPERE.md`.
- 0.8.0 (CUDApp#139): adds the EXPERIMENTAL general matrix multiply of section 9.8 (`src/omega_gpu_gemm_ex.h`:
  `omega_gpu_gemm_ex`, `omega_gpu_gemm_ex_fp8_block_rows`, `OmegaGemmExArgs`, `OmegaGemmExInfo` and their constants),
  and two of its test hooks to the INTERNAL list. **Layout change:** `OmegaGpuFp8GemmInfo` (EXPERIMENTAL, section 9.7)
  grows from 80 to 88 bytes with the new field `general` at offset 80 (1 when the call took the general fallback for a
  shape the tuned kernel refuses); a caller built against the 0.7.0 layout must be rebuilt. `OmegaGpuFp8Info` gains the
  same field at offset 156, inside existing padding, so it stays 160 bytes. `omega_gpu_fp8_general_rows` goes on the
  INTERNAL list. The struct is EXPERIMENTAL, so the size change is MINOR under the rule below.
- 0.7.0 (omega #588, carried with the b5ab3fe sync): adds EXPERIMENTAL `omega_gpu_decode_graph_abort_token`, which
  releases the session lock and leaves the token without submitting what was recorded. A failed `begin_token` now
  releases the lock itself, and `omega_gpu_decode_graph_token` answers every begin failure (not only BAD_ARGS) without
  holding the lock (`tests/test_omega_gpu_decode_graph_abort.c`, 5 mutants in `make test-gpu-decode-graph`).
- 0.6.0 (AttnGeneral): adds EXPERIMENTAL `omega_gpu_attention_general`, `OmegaGpuAttnGeneralDesc`,
  `OMEGA_GPU_ATTN_GENERAL_MAX_HEAD_DIM` (256), `OMEGA_GPU_ATTN_GENERAL_MAX_Q_HEADS` (1024) and `OMEGA_GPU_ATTN_WINDOW_NONE`:
  one attention kernel for every head_dim 1..256, any query-head count with any kv-head count dividing it (MHA, GQA,
  MQA), causal, non-causal and sliding-window masks, strided f32 or bf16 KV, any kv_len. The existing attention entries
  keep their tuned kernels for the shapes they took and now run the general kernel for the shapes they refused
  (head_dim other than 64 / 128, non-power-of-two GQA, more than 64 query heads, contexts past 4096); the paged
  entries do the same without the context split. Five test hooks and the general mutant ids go on the INTERNAL list.
- 0.5.0: adds EXPERIMENTAL `omega_gpu_cmdlist_submit_pair`, `omega_gpu_cmdlist_wait_for`,
  `OmegaGpuCmdListOpts.first_barrier_none`, `omega_gpu_session_extra_queue` (streams, C4) and `omega_gpu_session_recover`,
  `omega_gpu_session_recoveries` (recovery after a latch, C5), carried from omega-mv e5b32567 by CUDApp#121; two
  module-to-session hooks go on the INTERNAL list (section 10). Also adds EXPERIMENTAL `omega_gpu_fp8_grouped_ctas_rule`
  and `omega_gpu_fp8_grouped_ctas_auto` (section 9.7) and INTERNAL `OMEGA_GPU_SM_COUNT`.
- 0.4.0: `OMEGA_GPU_HANDLE_SLOTS` 1024 -> 4096 and `OMEGA_GPU_CMDLIST_MAX_LAUNCHES` 2048 -> 4096 (STABLE constants), and
  so `OmegaGpuHandleTable` (EXPERIMENTAL) grows to 69648 bytes: the values the live server (omega-mv 6de999da, carried by
  CUDApp#120) was built with. A MAJOR-class change, taken as MINOR under the major-version-zero rule below. Also adds the
  carried decode-path calls of section 9.7 (FP8 knobs and quantiser, record-into-command-list forms, router row batching)
  as EXPERIMENTAL, and six more names to the INTERNAL list.
- 0.3.0: adds the EXPERIMENTAL decode-path entries of section 9.7 and their result codes; no STABLE entry changed.
- 0.2.0: adds `OMEGA_RUNTIME_API_VERSION_PATCH`, lists `OMEGA_RUNTIME_API_VERSION` in the manifest and the INTERNAL list
  (section 10); no entry changed.

**A PATCH bump (0.5.0 to 0.5.1) is for:**
- a fix that changes no manifest entry: a STABLE function that did not do what this file says now does (the fix makes it
  match), or a correction to this file's prose.

**A MINOR bump (0.5 to 0.6) is allowed to:**
- add a function, a constant, an enum value at the end of an existing enum, or a new struct;
- add an EXPERIMENTAL entry, change or remove an EXPERIMENTAL entry, or move an entry from EXPERIMENTAL to STABLE;
- add a new failure name for the same call to report (callers must treat an unknown failure code or stage text as a failure).

**From 1.0 on it needs a MAJOR bump (1.x to 2.0) to:**
- remove or rename a STABLE function, struct, field or constant;
- change a STABLE function's argument or return types, a STABLE struct's size, field order or field offsets, or the value
  of a STABLE constant or enum value;
- change documented behaviour of a STABLE entry: what the lock covers, what a latch does, which buffers get parked, what
  a return value means;
- move a STABLE entry down to EXPERIMENTAL or INTERNAL;
- raise the platform contract's major number (section 9.2).

**Major version zero** (semantic versioning 2.0.0, item 4: "anything MAY change at any time"): while MAJOR is 0, a change
from the MAJOR list above needs a MINOR bump (0.5 to 0.6) plus its line in the change list above and in the manifest;
MAJOR is reserved for the first break after 1.0. It still never goes unbumped: a STABLE change with no bump fails the
check. STABLE in 0.x means "written down and checked", not "frozen". The first 1.0 will be cut only when section 8 (what
v0 does not have) has been looked at again.

How it is enforced (all host checks, no GPU; CI job `runtime-api`, `.github/workflows/runtime-api.yml`):
- `make test-runtime-api` (`tools/runtime_api_check.sh`): a header change that moves any listed entry fails until the
  manifest line is edited on purpose (`tools/runtime_api_check.sh --bless` refreshes the numbers; the diff is reviewed). A
  function, macro or enumerator added to a manifest header fails until it is listed in the manifest or in the INTERNAL list
  with a reason. The version macros, the manifest's `abi` line and consts, and the `Omega Runtime API version:` line in this
  file must agree.
- `make test-runtime-api-semver` (`tools/runtime_api_semver.sh`, base `origin/main` by default; CI uses the pull request's
  base): compares the manifest with the base's entry by entry and fails when the change needs a bigger bump than it has,
  or when the version goes down. Constants count as STABLE unless their manifest line carries `# EXPERIMENTAL`. While the
  base's MAJOR is 0, a MAJOR-class change passes with a MINOR bump (the rule above); once the base is 1.x it needs MAJOR.
- What stays a review duty: a change of documented behaviour with no entry change (the script cannot see prose), and the
  limits listed in section 11.

The platform contract has its own number (`OMEGA_GPU_PLATFORM_ABI_MAJOR` 1, `OMEGA_GPU_PLATFORM_ABI_MINOR` 2, set in
`src/omega_gpu_platform.h`). A platform minor bump needs at least a runtime MINOR bump; a platform major bump needs a runtime MAJOR bump.

## 3. How the pieces fit (plain English)

**Device and platform.** Omega talks to the GPU through a small "platform" layer: a table of services (a clock, a lock,
device-visible memory, a submission queue, a way to poll for completion). Today one platform exists, the Linux reference
(`src/omega_gpu_platform_linux.c`), which uses NVIDIA's open kernel driver. The contract is written so a second backend
(WSL2 on Windows, or AIENOS with no Linux driver) can fill the same table. A program normally never touches the platform:
the first call that needs the GPU installs the Linux reference by itself. There is **one device, one channel per process**.
There is no call to ask the device its name, SM count or memory size yet (section 8).

**Memory.** You ask the session for a buffer and get an `OmegaGpuMem`: a handle, a CPU pointer, a GPU address and a size.
Every ordinary buffer is GPU-uncached, because the CPU writes inputs the chip reads and polls markers the chip writes.
You make CPU writes visible to the GPU with `omega_gpu_session_sync_to_gpu` and GPU writes visible to the CPU with
`omega_gpu_session_sync_to_cpu`. Code (the machine code of a kernel) lives in a separate kind of buffer
(`omega_gpu_session_alloc_code`): the CPU writes it once, the GPU only executes it. Handles carry a generation number, so a
handle from a closed device or an already freed buffer is refused instead of silently reused. A buffer you mean to reuse and
grow, such as staging space, can be an `OmegaGpuScratch`. For read-only weights there is a GPU-cached allocation
(`omega_gpu_session_alloc_cached`, EXPERIMENTAL). There is no async copy and no unified memory in v0.

**Launch.** One kernel launch is an `OmegaGpuLaunch`: where the code is, how many registers, the block and grid size, the
argument words, shared memory, and a timeout. `omega_gpu_session_launch` submits it and waits for the chip to finish (it
is synchronous). The wait is by polling a completion word, never by an interrupt.
For sm_86, set `OmegaGpuLaunch.target_sm = 86` and use the target-aware encoder in
`omega_ampere_codegen.h`. `target_sm = 0` preserves legacy sm_121 behavior; it is
not permission to run Blackwell code on Ampere. A target mismatch refuses before
submission. Existing Blackwell library kernels and command-list packets are not
silently retargeted; unsupported Ampere packets are refused.

**Command lists.** `omega_gpu_cmdlist_*` records many launches and submits them as one unit with one completion wait. By
default each launch is preceded by a barrier so dependent launches cannot overlap; a launch can be marked independent.
EXPERIMENTAL: streams, events and dependency-ordered execution are being built on top of it (omega-mv#580), so its options
and statistics will grow.

**Module load.** There is no module or fat-binary loader in v0, and no PTX or JIT: the program brings the machine code.
Loading a kernel is `omega_gpu_session_alloc_code` (section 5) plus a CPU copy of the code into `code.cpu`; the launch then
names `code.va`. The project's own kernels come from its encoder (`src/omega_blackwell_*`, INTERNAL), and
`examples/hello_gpu` shows the whole path. A loader that reads a packaged module is section 8 work.

**Streams and events.** EXPERIMENTAL since 0.5.0, and only on command lists: `omega_gpu_session_extra_queue` opens a
second queue (behind `AIEN_OMEGA_GPU_CHANNELS=2`), `omega_gpu_cmdlist_submit_pair` submits two lists on two queues before
waiting for either, `omega_gpu_cmdlist_wait_for` makes one list wait for a launch of the other, and
`OmegaGpuCmdListOpts.first_barrier_none` drops the leading barrier. There is no stream or event object, and
`omega_gpu_session_launch` still returns only when the chip has finished. Chip qualification is CUDApp#113; the `.om`
successor is tracked in CUDApp#115.

**Job engine.** `omega_gpu_execute` runs one already-built job (today only a 1-D vector job: two inputs, one output) through
named states and returns success or exactly one named failure. EXPERIMENTAL.

**Errors, the latch and parked buffers.**
- A failing call returns a non-zero value and names its stage in text: `omega_gpu_session_last_error()`. A device-open failure
  carries the driver's own text. Do not read a bare return code as the cause.
- If the chip's completion cannot be confirmed (a completion wait ran out of time, the completion poll was refused, or a launch was cancelled after submit),
  the work may still be running and still be reading or writing your buffers. The process then **latches**:
  `omega_gpu_session_is_blocked()` returns 1 for the rest of the process, every later launch is refused, close and reopen are
  refused, and nothing is returned to the platform.
- After the latch, every buffer freed through `omega_gpu_session_free` is **parked**: kept allocated, counted by
  `omega_gpu_session_parked`, never released. Handles stay valid for inspection (`omega_gpu_session_is_parked`).
- Today there is no way back from a latch except ending the process. Recovery is open work (omega-mv#518).

**Threads.** One process-wide lock (the platform LOCK service, lock id `OMEGA_GPU_SESSION_LOCK_ID`, which is 0). A call
sequence that touches the device (open, alloc, launch, readback) is done between `omega_gpu_session_lock()` and
`omega_gpu_session_unlock()`. Threads therefore take turns on the one channel. The Linux reference lock is a plain
(non-recursive) pthread mutex (`locks[id]`, `src/omega_gpu_platform_linux.c:29` and `:119`), so taking it twice from one thread
deadlocks. These take the lock themselves and must be called without it: `omega_gpu_session_close`, `omega_gpu_cmdlist_create`,
`omega_gpu_cmdlist_free`, and every library call of section 9.5 (matmul, elementwise, attention, serving).
`omega_gpu_session_cancel` may be called from any thread without the lock (header). `omega_gpu_session_is_blocked` reads the latch with an atomic load (`src/omega_gpu_session.c:157`), so it is safe without the lock too.
`omega_gpu_session_last_error` returns a pointer into one process-wide text buffer: read it under the lock, before the next call.

### 3.1 A minimal sequence (sketch)

```c example
omega_gpu_session_lock();
OmegaGpuSession *s = omega_gpu_session_open();         /* NULL: see omega_gpu_session_last_error() */
OmegaGpuMem in, code;
omega_gpu_session_alloc(bytes, &in);                   /* GPU-uncached data buffer */
omega_gpu_session_alloc_code(code_size, &code);        /* write the machine code to code.cpu */
omega_gpu_session_sync_to_gpu(&in);
OmegaGpuLaunch L = { .code_va = code.va, .timeout_ms = 1000, /* gpr_count, threads_*, grid_*, args, n_args ... */ };
uint64_t elapsed_ns = 0; uint32_t marker = 0;     /* launch adds to elapsed_ns: start it at 0 */
int rc = omega_gpu_session_launch(&L, &elapsed_ns, &marker);
omega_gpu_session_sync_to_cpu(&in);
omega_gpu_session_free(&in);
omega_gpu_session_unlock();
```

This is a shape sketch, not a worked example (the worked example is slice R2).

## 4. Session: device, lock and state

#### `omega_gpu_session_lock`
STABLE: used by every library API and chip-proven; the lock model is the documented one.
```c
void omega_gpu_session_lock(void);
```
Takes the process-wide session lock (platform LOCK service, id `OMEGA_GPU_SESSION_LOCK_ID`). Does not touch the device.
With no platform installed it does nothing, and the open that follows refuses.

#### `omega_gpu_session_unlock`
STABLE: pairs with the lock.
```c
void omega_gpu_session_unlock(void);
```

#### `omega_gpu_session_open`
STABLE: used by every library API and chip-proven. Treat the returned pointer as "the device is open"; the fields of
`OmegaGpuSession` are EXPERIMENTAL (the session's own launch buffers) and a caller needs none of them.
```c
OmegaGpuSession *omega_gpu_session_open(void);
```
Caller holds the lock. Opens the device on first use. Returns NULL when the process is latched, no platform is installed or the
open failed; the stage is in `omega_gpu_session_last_error()`.

#### `omega_gpu_session_target_sm`
EXPERIMENTAL: physical target selection for the prepared Ampere path; device qualification is NOT_RUN.
```c
uint32_t omega_gpu_session_target_sm(void);
```
Caller holds the lock. Returns the opened device's exact target (`86` or `121`),
or zero with no open device. It does not open a device.

#### `omega_gpu_session_is_open`
STABLE: simple state query.
```c
bool omega_gpu_session_is_open(void);
```

#### `omega_gpu_session_is_blocked`
STABLE: the latch query that every caller needs.
```c
int omega_gpu_session_is_blocked(void);
```
Returns 1 once an uncertain completion has latched this process, else 0. Any thread.

#### `omega_gpu_session_close`
STABLE: the only way to close the device; its refusal after a latch is part of the error model.
```c
void omega_gpu_session_close(void);
```
Closes the device and frees every session buffer. **Takes the lock itself.** Refused (a no-op) after an uncertain completion.
The next open reopens; every handle of the closed device is stale from then on.

#### `omega_gpu_session_open_count`
STABLE: diagnostic with a fixed meaning.
```c
uint32_t omega_gpu_session_open_count(void);
```
Device opens so far in this process (a persistent session shows 1).

#### `omega_gpu_session_set_spin_us`
STABLE: used by sovereign-core's binding; fixed meaning and limit.
```c
int omega_gpu_session_set_spin_us(uint32_t us);
```
Process-wide marker-poll spin window, used by every launch whose own `spin_us` is 0. 0 (the default) keeps the platform's
sleep-poll. Returns 0 on success, -1 above `OMEGA_GPU_SESSION_MAX_SPIN_US` (setting unchanged).

#### `omega_gpu_session_spin_us`
STABLE: reads the setting above.
```c
uint32_t omega_gpu_session_spin_us(void);
```

## 5. Session: memory

All allocation calls need the caller to hold the lock with the device open.

#### `OmegaGpuMem`
STABLE: the buffer type every API passes around.
A device buffer: a typed handle, its kind, the CPU view, the GPU virtual address and the size. A `cpu` of NULL means there is no CPU view.

| Field | Meaning (from `src/omega_gpu_session.h`) |
| --- | --- |
| `handle` | `OmegaGpuBufferHandle`: index and generation |
| `kind` | `OMEGA_GPU_HANDLE_BUF_DATA`, `_BUF_CODE` or `_BUF_QUEUE` |
| `cpu` | CPU view |
| `va` | GPU virtual address |
| `size` | bytes of both views |

#### `OmegaGpuBufferHandle`
STABLE: a field of `OmegaGpuMem`, so its layout is part of it.
`{ uint32_t index; uint32_t generation; }`. Generation 0 is never issued, so a zeroed handle means "none".
A handle is valid only while its slot generation matches; a closed handle is refused.

#### `OmegaGpuDeviceHandle`
STABLE: same shape and rule as the buffer handle; named in the platform contract.
`{ uint32_t index; uint32_t generation; }`.

#### `OmegaGpuQueueHandle`
STABLE: same shape and rule as the buffer handle; named in the platform contract.
`{ uint32_t index; uint32_t generation; }`.

#### `omega_gpu_session_alloc`
STABLE: the ordinary buffer call, used by every library.
```c
int omega_gpu_session_alloc(size_t bytes, OmegaGpuMem *out);
```
A GPU-uncached DATA buffer, CPU and GPU read/write, rounded up to a page. Returns 0 on success; non-zero with the stage named on
failure (the driver can refuse, for example with no memory; the refusal is counted in `OmegaGpuAllocStats.alloc_failures`).

#### `omega_gpu_session_alloc_code`
STABLE: the only way to place kernel code; chip-proven.
```c
int omega_gpu_session_alloc_code(size_t code_size, OmegaGpuMem *out);
```
A CODE buffer (CPU writes, GPU executes, GPU never writes) for `code_size` bytes plus the 2 KB instruction-prefetch tail
(`src/omega_gpu_code_alloc.h`), the tail zero-filled. The caller writes the code to `out->cpu`. A size of 0 still gets one page (`omega_gpu_code_alloc_bytes` rounds up to at least one page).

#### `omega_gpu_session_free`
STABLE: the release call; its latch behaviour is part of the error model.
```c
void omega_gpu_session_free(OmegaGpuMem *m);
```
Releases `m` to the platform and zeroes `*m`. **Latched: `m` is parked instead (never released).** A stale handle (from a
closed device) or a closed session is refused: nothing is released, the refusal is counted
(`OmegaGpuAllocStats.stale_refusals`) and `*m` is zeroed.

#### `omega_gpu_session_sync_to_gpu`
STABLE: the visibility rule for CPU writes.
```c
int omega_gpu_session_sync_to_gpu(const OmegaGpuMem *m);
```
Makes CPU writes to the whole buffer visible to the device (platform SYNC). Returns 0, also for a buffer with no CPU view;
-1 with the stage named when refused or when there is no device.

#### `omega_gpu_session_sync_to_cpu`
STABLE: the visibility rule for GPU writes.
```c
int omega_gpu_session_sync_to_cpu(const OmegaGpuMem *m);
```
Makes device writes to the whole buffer visible to the CPU. Same return values as `sync_to_gpu`.

#### `OmegaGpuScratch`
STABLE: the staging-buffer type the library APIs use.
A caller-owned staging buffer that grows to its high-water mark: `mem` (an `OmegaGpuMem`), `cap` (usable bytes), `high_water`
(largest request so far) and `reserved` (a serving reservation is active, see `omega_gpu_session_scratch_reserve`).
Zero-initialise it before first use. This is required: `omega_gpu_session_scratch` reads `reserved`, `cap`, `mem.cpu` and
`high_water` before it writes them (`src/omega_gpu_session.c:290-296`).

#### `omega_gpu_session_scratch`
STABLE: used by every library API.
```c
int omega_gpu_session_scratch(OmegaGpuScratch *s, size_t bytes);
```
Makes `s->mem` at least `bytes` long (reallocated when it grows; **contents are not kept**). Returns 0 or -1; with a
reservation active, a request past the reserved size returns `OMEGA_GPU_SESSION_SCRATCH_RESERVED` (-2) and allocates nothing.

#### `omega_gpu_session_scratch_free`
STABLE: the matching release.
```c
void omega_gpu_session_scratch_free(OmegaGpuScratch *s);
```

#### `OmegaGpuAllocStats`
STABLE: used by sovereign-core's binding to show that serving asks the driver for nothing.
Counters of driver allocations made through `omega_gpu_session_alloc` since the process started: `allocs`, `alloc_bytes`
(page-rounded), `alloc_failures`, `frees`, `stale_refusals`. Covers the session layer only.

#### `omega_gpu_session_alloc_stats`
STABLE: reads the counters above.
```c
void omega_gpu_session_alloc_stats(OmegaGpuAllocStats *out);
```

## 6. Session: launch

#### `OmegaGpuLaunch`
STABLE: the launch descriptor; chip-proven by every library.
One launch: a kernel already resident at `code_va`, argument words for constant bank 0 at 0x380 (buffer addresses, then
parameters), and the grid.

Argument layout and the checks `omega_gpu_session_launch` makes (`src/omega_gpu_session.c:433-446`):
- a 64-bit GPU address takes two words, the low word first, then the high word;
- `n_args` is 1 to `OMEGA_GPU_SESSION_MAX_ARGS`, `args` is not NULL, and `threads_x`, `grid_x` and `grid_y` are not 0 (a
  `threads_y` of 0 means 1); otherwise the launch is refused with "launch arguments" and nothing runs;
- `gpr_count` is rounded up to a multiple of 8, and to at least 64;
- `elapsed_ns` is added to, not set: start it at 0.
What the kernel itself may read in constant bank 0 besides these words, and where GB10 machine code comes from, is slice
R2 (`examples/hello_gpu`, README "Run your first kernel").

| Field | Meaning (from `src/omega_gpu_session.h`) |
| --- | --- |
| `code_va` | GPU address of the kernel code |
| `gpr_count` | registers the QMD (the launch descriptor the hardware reads) declares; multiple of 8, at least 64 in practice |
| `threads_x`, `threads_y` | block size |
| `grid_x`, `grid_y` | grid size |
| `num_elements` | QMD config num_elements (kept per API for identical words) |
| `args`, `n_args` | `n_args` words written at constant bank + 0x380; `n_args` at most `OMEGA_GPU_SESSION_MAX_ARGS` (32) |
| `timeout_ms` | one absolute submission-wide deadline covering queue submission, spin and both markers; completion at or after it is uncertain and **the process latches**. 0 is refused with "launch arguments"; an unrepresentable nanosecond deadline is refused with "launch watchdog deadline overflow" before submission |
| `shared_bytes` | shared memory per block, rounded up to 128 bytes; 0 = the default (1024). At most `OMEGA_BW_QMD_MAX_SHARED_BYTES` (32768: the SM-config target the QMD declares); one byte more is refused before any device work |
| `spin_us` | spin-poll window before the platform's sleep-poll; 0 = the process-wide setting |

#### `omega_gpu_session_launch`
STABLE: the single-kernel launch; chip-proven.
```c
int omega_gpu_session_launch(const OmegaGpuLaunch *L, uint64_t *elapsed_ns, uint32_t *marker_out);
```
Caller holds the lock, device open, not latched. Submits one launch and polls for both completion markers, so it returns when
the chip is done. 0 on success; -1 with the stage named on failure; `OMEGA_GPU_SESSION_CANCELLED` (-3) when a cancel request
was seen. A marker timeout latches the process. `elapsed_ns` (submit to second marker) is added to when the pointer is given;
`marker_out` receives the first marker word observed.

The deadline and acknowledged recovery decisions are implemented in
`tools/c5/policy.om` and `tools/c5/runtime.om`, with import-free generated
bindings. `CUDAPP_MIGRATION_PENDING_C_SESSION_TIMEOUT=1` selects the old
per-marker timing only as a differential oracle; the default is OFF. Host
fake-RM and deterministic channel-model coverage is described in
[`tools/c5/README.md`](../../tools/c5/README.md). This does not replace the
hardware receipt required to qualify the changed timeout path.

## 7. Session: errors, the latch, parked buffers

#### `omega_gpu_session_last_error`
STABLE: the error-model entry point.
```c
const char *omega_gpu_session_last_error(void);
```
The stage of the most recent failure, `""` if none. Shared by every API. Points into one process-wide buffer: read it under
the lock, before the next call.

#### `OmegaGpuParkStats`
STABLE: the parked-buffer report.
`buffers` (parked, listed plus untracked), `untracked` (parked past the listed `OMEGA_GPU_SESSION_PARK_MAX` = 512) and
`bytes`.

#### `omega_gpu_session_parked`
STABLE: how a caller sees what the latch kept.
```c
void omega_gpu_session_parked(OmegaGpuParkStats *out);
```
Buffers parked after an uncertain completion: kept allocated, never returned to the platform. The session's own launch
buffers (ring, constant bank, marker, QMD page) are parked at the latch and every later free parks too.

#### `omega_gpu_session_is_parked`
STABLE: per-handle form of the above.
```c
int omega_gpu_session_is_parked(OmegaGpuBufferHandle h);
```
1 if this handle is parked and listed.

## 8. Not in v0 (so nobody assumes it)

Each line is a gap named in `docs/runtime/ROADMAP.md` section 1 (on branch `runtime/roadmap`, not yet merged); none of it is in the manifest.
- A device query (name, SM count, memory): there is no API call for it.
- More than one device in a process (a second channel in the same group is `omega_gpu_session_extra_queue`, EXPERIMENTAL); sharing the GPU between processes (a second channel failed under load: omega-mv#523).
- Recovery after a latch beyond `omega_gpu_session_recover` (EXPERIMENTAL since 0.5.0: throw the device away and open a
  new one in the same process; a driver teardown fault still ends GPU use for the process).
- Async copy, stream and event objects, host callbacks (0.6.0 has only two-queue submission and cross-queue waits on command lists, EXPERIMENTAL).
- Cached read-write buffers; unified memory.
- Launches with more than the 32 argument words, or an asynchronous single launch.
- A second real backend (WSL2 `/dev/dxg` is on branch `rig/gb20x`; native AIENOS is planned), so the platform contract has only been exercised by one.

## 9. EXPERIMENTAL entries

Listed in the manifest, may change in any minor version. Signatures are in the manifest and the headers; the names are
checked there. The reason is why each is not STABLE yet.

### 9.1 Session extras (`src/omega_gpu_session.h`, `src/omega_gpu_mem_cached.h`)

| Entry | Reason it is EXPERIMENTAL |
| --- | --- |
| `omega_gpu_session_alloc_cached` | GPU-cached memory is correct only for write-once, never-polled, GPU-read-only weights; the API cannot enforce that (the caller must). |
| `omega_gpu_session_scratch_reserve`, `omega_gpu_session_scratch_unreserve` | Serving-daemon feature (no driver call after warm-up); one user. |
| `omega_gpu_session_cancel` | A cancel after submit cannot stop the chip: it latches the process. Semantics may tighten when recovery (#518) lands. |
| `omega_gpu_session_extra_queue` | A second GPFIFO channel in the same group, only with `AIEN_OMEGA_GPU_CHANNELS` > 1 at open; one backend, chip qualification in CUDApp#113. |
| `omega_gpu_session_recover`, `omega_gpu_session_recoveries` | Recovery after a latch (omega-mv#518, C5): closes and reopens the device in-process; every old handle and list goes stale. Chip qualification in CUDApp#119. |
| `omega_gpu_session_set_platform`, `omega_gpu_session_platform`, `omega_gpu_session_submit_prefix_words`, `omega_gpu_session_completion_mode`, `omega_gpu_session_now_ns` | Tied to the platform contract, which has one real backend. |
| `omega_gpu_host_alloc`, `omega_gpu_host_calloc`, `omega_gpu_host_free` | CPU-only memory from the platform HOST_MEM service; the freestanding build needs them, hosted code can use malloc. |
| `OmegaGpuSession` | The session's own launch buffers; a caller needs none of its fields. |

### 9.2 Platform contract (`src/omega_gpu_platform.h`, ABI 1.2)

Types: `OmegaGpuPlatform` (the service table), `OmegaGpuPlatformCaps`, `OmegaGpuBufferDesc`, `OmegaCpuView`, `OmegaGpuVa`,
`OmegaGpuBusAddr`, `OmegaGpuHandleTable`. Functions: `omega_gpu_platform_refusal_name`, `omega_gpu_platform_negotiate`,
`omega_gpu_platform_check`, `omega_gpu_platform_check_for`, `omega_gpu_platform_check_buffer`, `omega_gpu_handles_init`,
`omega_gpu_handles_open`, `omega_gpu_handles_check`, `omega_gpu_handles_close`. Enums and constants: the handle kinds,
rights (`OMEGA_GPU_RIGHT_*`), cache policy, services (`OMEGA_GPU_SVC_*`) and refusal names (`OMEGA_GPU_PLAT_*`).

Reason: the contract is written for three backends but only the Linux reference has run; the second backend (`rig/gb20x`) and
the native one will likely force changes. What it already promises (from the header): resources are typed handles with a
generation; every byte range carries a length and rights; anything the CPU polls is `OMEGA_GPU_RIGHT_HOST_POLLED` and must be
GPU-uncached; polling a word is never claimed as interrupt support; a missing service is refused, never emulated.

### 9.3 Command lists (`src/omega_gpu_cmdlist.h`)

Types: `OmegaGpuCmdListOpts`, `OmegaGpuCmdListStats`, the opaque `OmegaGpuCmdList`. Functions: `omega_gpu_cmdlist_create`
(takes the lock itself), `omega_gpu_cmdlist_free`, `omega_gpu_cmdlist_reset`, `omega_gpu_cmdlist_count`,
`omega_gpu_cmdlist_add`, `omega_gpu_cmdlist_mark_independent`, `omega_gpu_cmdlist_submit`, `omega_gpu_cmdlist_submit_pair`,
`omega_gpu_cmdlist_wait_for`, `omega_gpu_cmdlist_is_latched`, `omega_gpu_cmdlist_stamps`, `omega_gpu_cmdlist_barrier_name`.
Up to `OMEGA_GPU_CMDLIST_MAX_LAUNCHES` (4096) launches per list.

`OmegaGpuCmdListAccess` supplies a complete set of half-open GPU VA/byte ranges and
`OMEGA_GPU_CMDLIST_ACCESS_READ` / `_WRITE` bits (combine both for in-place updates).
Different virtual mappings of the same storage must be normalized to a common
canonical interval namespace before annotation; the planner cannot discover
physical aliases from distinct VAs. If the producer cannot establish that
normalization, leave the record's effects unknown rather than claim independence.
`omega_gpu_cmdlist_add_accesses` copies up to `OMEGA_GPU_CMDLIST_MAX_ACCESSES` ranges;
zero ranges means known empty. `omega_gpu_cmdlist_set_accesses` attaches or replaces
the complete set on an existing launch recorded by a kernel helper, without
changing its launch bytes; a rejected replacement leaves the previous set intact.
Legacy `add` records unknown effects.
`omega_gpu_cmdlist_plan_program` returns `OmegaGpuCmdListProgramPlan` counts without
submitting; `omega_gpu_cmdlist_submit_program` emits exactly one submission, or
refuses before queue commit when the ring plus platform prefix cannot hold it.
Empty lists produce no submission. Callers hold the session lock.
`waits` counts explicit WFI methods: admission, hazardous inter-launch drains,
and the optional final timestamp WFI; the completion release is separate.
The whole-program path rejects manual barrier/skip/first-barrier suppression and
cross-list waits. A caller must describe every buffer the kernel reads or writes;
an incomplete effect set is not safe. Unknown legacy effects force drains.
The planner compares every launch since the previous drain, not just its immediate
predecessor. The serialized legacy API remains available for controlled comparison.
See `om/cmdlist/README.md` for method-source pins, tests and the chip runbook.

Reason: omega-mv#580 is building pipelined and dependency-ordered execution on this module, so the options, statistics
and barrier modes will grow. The measured extra cost of each launch in a list is about 30 us with a wait-for-idle barrier
(`evidence/OWN30B-LAUNCH-COST-20261014T0120Z/r1/receipt.json`, verdict FAIL on 4 of 11 checks). A timeout latches the list
and then the session. The `OMEGA_GPU_CMDLIST_BARRIER_NONE` mode and `skip_launch` option are test-only.

### 9.4 Job engine (`src/omega_gpu_engine.h`)

Types: `OmegaGpuJob`, `OmegaGpuResult`, `OmegaGpuTimeouts`, `OmegaGpuBackend`, `OmegaGpuBackendDiag`. Functions:
`omega_gpu_execute`, `omega_gpu_engine_set_backend`, `omega_gpu_engine_is_blocked`, `omega_gpu_engine_set_clock`,
`omega_gpu_engine_failure_name`, `omega_gpu_engine_state_name`, `omega_gpu_engine_wait_name`, `omega_gpu_engine_step_name`. One failure name per run
(`OMEGA_GPU_ENGINE_*`), states `INITIAL` to `SUCCESS`, the failed step recorded, and an uncertain completion blocks every
later `omega_gpu_execute` in the process with `UNCERTAIN_COMPLETION_BLOCKED`.

Reason: accepts one job shape (`OMEGA_GPU_LAYOUT_VECTOR_1D`); #580 widens it; the chip receipt for the vector-add job is on an
unmerged branch (`hive/DEEP2-A3-gpu-engine-impl`). Note: the status paragraph at the top of `omega_gpu_engine.h` says only a
fake backend exists; `src/omega_blackwell_engine.c` is on main and `src/omega_blackwell_submit.c` installs it, so that
paragraph is out of date (whether it is chip-qualified on main is UNVERIFIED here).

### 9.5 Libraries (matmul, elementwise, attention, serving)

The entries carried over from aien-sovereign-core's manifest: `src/omega_gpu_matmul_api.h`, `omega_gpu_elementwise_api.h`,
`omega_gpu_attention_api.h`, `omega_gpu_serving.h` (the functions and structs after the `libraries` comment in the
manifest). Reason: shapes and formats follow one model (Qwen3-30B); the general any-shape GEMM is the separate entry of
section 9.8. Attention is the exception since 0.6.0: `omega_gpu_attention_general` (declared after the carried block) takes every head_dim 1..256 and any MHA / GQA / MQA head split, and the carried attention entries fall back to it for the shapes their tuned kernels refuse. They are the surface sovereign-core's Rust binding uses today. They are not described entry by entry in v0;
their headers are the description. One value differs from sovereign-core's pinned copy: `OMEGA_GPU_MATMUL_MAX_CTAS` is 1024 in
the header (changed by omega #473, commit 98cecadc), 64 in their manifest. Omega's manifest is canonical; R12 brings theirs level.


### 9.6 Library entries (manifest spelling)

Types are the manifest's Rust spelling (`*const`, `*mut`, `u32`, ...); the headers hold the C. Every one is EXPERIMENTAL except the three STABLE session entries noted in section 4 and 5.

| Entry | Signature |
| --- | --- |
| `OmegaGpuMatmulInfo` | struct, 160 bytes (fields in the manifest) |
| `OmegaGpuServingBounds` | struct, 44 bytes (fields in the manifest) |
| `OmegaGpuEwInfo` | struct, 120 bytes (fields in the manifest) |
| `OmegaGpuAttnInfo` | struct, 176 bytes (fields in the manifest) |
| `OmegaGpuKvLayout` | struct, 64 bytes (fields in the manifest) |
| `OmegaGpuAttnGeneralDesc` | struct, 64 bytes (fields in the manifest) |
| `omega_gpu_matmul_bf16` | `fn(u32, u32, u32, *const u16, *const u16, *mut f32, *mut OmegaGpuMatmulInfo) -> i32` |
| `omega_gpu_matmul_f32` | `fn(u32, u32, u32, *const f32, *const f32, *mut f32, *mut OmegaGpuMatmulInfo) -> i32` |
| `omega_gpu_matmul_rc_name` | `fn(i32) -> *const c_char` |
| `omega_gpu_matmul_cache_clear` | `fn() -> ()` |
| `omega_gpu_tensor_upload_bf16` | `fn(u32, u32, *const u16, *mut *mut OmegaGpuTensor) -> i32` |
| `omega_gpu_tensor_upload_f32` | `fn(u32, u32, *const f32, *mut *mut OmegaGpuTensor) -> i32` |
| `omega_gpu_tensor_free` | `fn(*mut OmegaGpuTensor) -> ()` |
| `omega_gpu_tensor_shape` | `fn(*const OmegaGpuTensor, *mut u32, *mut u32) -> ()` |
| `omega_gpu_matmul_resident_bf16` | `fn(u32, *const u16, *const OmegaGpuTensor, *mut f32, *mut OmegaGpuMatmulInfo) -> i32` |
| `omega_gpu_matmul_resident_f32` | `fn(u32, *const f32, *const OmegaGpuTensor, *mut f32, *mut OmegaGpuMatmulInfo) -> i32` |
| `omega_gpu_matmul_last_error` | `fn() -> *const c_char` |
| `omega_gpu_matmul_is_blocked` | `fn() -> i32` |
| `omega_gpu_device_close` | `fn() -> ()` |
| `omega_gpu_matmul_set_cta_budget` | `fn(u32) -> ()` |
| `omega_gpu_matmul_cta_budget` | `fn() -> u32` |
| `omega_gpu_reserve_serving` | `fn(*const OmegaGpuServingBounds) -> i32` |
| `omega_gpu_serving_release` | `fn() -> ()` |
| `omega_gpu_matmul_prepare` | `fn(u32, u32, u32) -> i32` |
| `omega_gpu_serving_seal` | `fn() -> ()` |
| `omega_gpu_rmsnorm_f32` | `fn(u32, u32, *const f32, *const f32, f32, *mut f32, *mut OmegaGpuEwInfo) -> i32` |
| `omega_gpu_rope_f32` | `fn(u32, u32, *const f32, *const f32, *const f32, *mut f32, *mut OmegaGpuEwInfo) -> i32` |
| `omega_gpu_swiglu_f32` | `fn(u32, *const f32, *const f32, *mut f32, *mut OmegaGpuEwInfo) -> i32` |
| `omega_gpu_elementwise_rc_name` | `fn(i32) -> *const c_char` |
| `omega_gpu_gqa_attention_f32` | `fn(*const f32, *const f32, *const f32, u32, u32, u32, u32, *mut f32, *mut OmegaGpuAttnInfo) -> i32` |
| `omega_gpu_gqa_attention_rows_f32` | `fn(*const f32, *const f32, *const f32, u32, u32, u32, u32, u32, *mut f32, *mut OmegaGpuAttnInfo) -> i32` |
| `omega_gpu_attention_pool_alloc` | `fn(usize, *mut *mut c_void) -> i32` |
| `omega_gpu_attention_pool_free` | `fn() -> ()` |
| `omega_gpu_paged_attention_bf16` | `fn(*const f32, *const c_void, *const OmegaGpuKvLayout, *const u32, u32, u32, u32, u32, u32, u32, *mut f32, *mut OmegaGpuAttnInfo) -> i32` |
| `omega_gpu_paged_attention_batch_bf16` | `fn(*const f32, *const c_void, *const OmegaGpuKvLayout, *const i32, *const i32, u32, u32, u32, u32, u32, u32, *mut f32, *mut OmegaGpuAttnInfo) -> i32` |
| `omega_gpu_attention_rc_name` | `fn(i32) -> *const c_char` |
| `omega_gpu_attention_last_error` | `fn() -> *const c_char` |
| `omega_gpu_attention_general` | `fn(*const OmegaGpuAttnGeneralDesc, *const f32, *const c_void, *const c_void, *mut f32, *mut OmegaGpuAttnInfo) -> i32` |

### 9.7 Decode path (ADR 0043 task P2.2)

The kernel APIs the serving decode and prefill paths call, listed so the check sees any change to them. All are
EXPERIMENTAL for the same three reasons: each serves one model (Qwen3-30B-A3B, the OWN-30B lane), each is proven on one
chip (sm_121), and each is tied to its kernel generator in `src/omega_blackwell_codegen_*.h`, which ADR 0043 moves behind
a backend. Signatures and struct layouts are in the manifest. They were added in 0.3.0, a MINOR bump (section 2).
Their result codes (`*_OK`, `*_BAD_ARGS`, `*_TOO_LARGE`, `*_CODEGEN_FAIL`, `*_CHIP_FAIL`, `*_NAN_WEIGHT`, `*_BAD_TRIT`) are
EXPERIMENTAL constants in the manifest; the other constants these headers define (shape limits, kernel slot ids, mode
values, poison patterns) are in the INTERNAL list.

| Header | Entries | Reason it is EXPERIMENTAL |
| --- | --- | --- |
| `src/omega_gpu_fp8_api.h` | `OmegaGpuFp8Info`, `omega_gpu_fp8_tensor_upload`, `omega_gpu_fp8_tensor_free`, `omega_gpu_fp8_tensor_shape`, `omega_gpu_fp8_gemv_f32`, `omega_gpu_fp8_gemv_bf16`, `omega_gpu_fp8_gemv_grouped_f32`, `omega_gpu_fp8_gemv_grouped_dev`, `omega_gpu_fp8_gemv_grouped_record`, `omega_gpu_fp8_tensor_entry`, `omega_gpu_fp8_prepare`, `omega_gpu_fp8_seal`, `omega_gpu_fp8_rc_name`, `omega_gpu_fp8_last_error`, `omega_gpu_fp8_cache_clear`, `omega_gpu_fp8_set_shape`, `omega_gpu_fp8_set_depth`, `omega_gpu_fp8_set_cta_budget`, `omega_gpu_fp8_set_weight_cache`, `omega_gpu_fp8_set_variant`, `omega_gpu_fp8_variant`, `omega_gpu_fp8_weight_cache`, `omega_gpu_fp8_set_gpr_pad`, `omega_gpu_fp8_shape_for`, `omega_gpu_fp8_set_bigk`, `omega_gpu_fp8_bigk`, `omega_gpu_fp8_set_coal`, `omega_gpu_fp8_coal`, `omega_gpu_fp8_set_deep`, `omega_gpu_fp8_deep`, `omega_gpu_fp8_set_sched`, `omega_gpu_fp8_sched`, `omega_gpu_fp8_set_wfirst`, `omega_gpu_fp8_wfirst`, `omega_gpu_fp8_quantize_bf16`, `omega_gpu_fp8_e4m3_rne`, `omega_gpu_fp8_grouped_ctas_rule`, `omega_gpu_fp8_grouped_ctas_auto` | FP8 e4m3 block-scaled GEMV for the OWN-30B lane (Qwen3-30B-A3B): decode GEMV and grouped experts; depth and weight-cache knobs are tuned on one chip. |
| `src/omega_gpu_fp8gemm_api.h` | `OmegaGpuFp8GemmInfo`, `omega_gpu_fp8gemm_f32`, `omega_gpu_fp8gemm_bf16`, `omega_gpu_fp8gemm_prepare`, `omega_gpu_fp8gemm_rc_name`, `omega_gpu_fp8gemm_last_error`, `omega_gpu_fp8gemm_cache_clear`, `omega_gpu_fp8gemm_shape_for`, `omega_gpu_fp8gemm_set_shape`, `omega_gpu_fp8gemm_set_cta_budget`, `omega_gpu_fp8gemm_prepare_shape`, `omega_gpu_fp8gemm_cta_budget_for`, `omega_gpu_fp8gemm_record` | FP8 e4m3 grouped GEMM for OWN-30B prefill; one model's shapes. |
| `src/omega_gpu_bf16_gemv_api.h` | `OmegaGpuBf16Info`, `omega_gpu_bf16_tensor_upload`, `omega_gpu_bf16_tensor_free`, `omega_gpu_bf16_tensor_shape`, `omega_gpu_bf16_gemv_f32`, `omega_gpu_bf16_gemv_bf16`, `omega_gpu_bf16_gemv_record`, `omega_gpu_bf16_prepare`, `omega_gpu_bf16_rc_name`, `omega_gpu_bf16_last_error`, `omega_gpu_bf16_cache_clear`, `omega_gpu_bf16_shape_for`, `omega_gpu_bf16_set_shape`, `omega_gpu_bf16_set_cta_budget`, `omega_gpu_bf16_gemv_mask_record`, `omega_gpu_bf16_gemv_rows_record`, `omega_gpu_bf16_gemv_xbatch_record`, `omega_gpu_bf16_prepare_mask`, `omega_gpu_bf16_prepare_xbatch` | bf16 streaming GEMV for the OWN-30B lm_head (vocab 151936 x hidden 2048); one shape. |
| `src/omega_gpu_decode_attn_api.h` | `OmegaGpuDecodeAttnInfo`, `OmegaGpuDecodeAttnCacheView`, `omega_gpu_decode_attn_cache_alloc`, `omega_gpu_decode_attn_cache_free`, `omega_gpu_decode_attn_set_norms`, `omega_gpu_decode_attn`, `omega_gpu_decode_attn_record`, `omega_gpu_decode_attn_counters_check`, `omega_gpu_decode_attn_cache_write`, `omega_gpu_decode_attn_cache_read`, `omega_gpu_decode_attn_cache_view`, `omega_gpu_decode_attn_last_error`, `omega_gpu_decode_attn_rc_name`, `omega_gpu_decode_attn_set_depth`, `omega_gpu_decode_attn_set_per_split` | One-launch decode attention for Qwen3-30B-A3B with its own KV cache; the cache view hands out device addresses. |
| `src/omega_gpu_router_api.h` | `OmegaGpuRouterInfo`, `OmegaGpuRouterDevOut`, `omega_gpu_router_alloc`, `omega_gpu_router_free`, `omega_gpu_router_set_layer`, `omega_gpu_router_set_directory`, `omega_gpu_router_set_directory_fp8`, `omega_gpu_router`, `omega_gpu_router_device_outputs`, `omega_gpu_router_record`, `omega_gpu_router_read_back`, `omega_gpu_router_last_error`, `omega_gpu_router_rc_name`, `omega_gpu_router_set_shape`, `omega_gpu_router_alloc_shape`, `omega_gpu_router_layout_bytes`, `omega_gpu_router_prepare_rows`, `omega_gpu_router_record_to`, `omega_gpu_router_record_rows` | One-launch MoE router for Qwen3-30B-A3B; the device-output path writes device addresses. |
| `src/omega_gpu_tern_api.h` | `OmegaGpuTernSegments`, `OmegaGpuTernInfo`, `omega_gpu_tern_tensor_upload`, `omega_gpu_tern_tensor_free`, `omega_gpu_tern_tensor_shape`, `omega_gpu_tern_tensor_bytes`, `omega_gpu_tern_gemv_i32`, `omega_gpu_tern_gemv_f32`, `omega_gpu_tern_gemv_record`, `omega_gpu_tern_rc_name`, `omega_gpu_tern_last_error`, `omega_gpu_tern_cache_clear`, `omega_gpu_tern_pack`, `omega_gpu_tern_padded_rows`, `omega_gpu_tern_segment_bounds`, `omega_gpu_tern_shape_for`, `omega_gpu_tern_set_layout`, `omega_gpu_tern_set_shape`, `omega_gpu_tern_set_cta_budget`, `omega_gpu_tern_set_weight_cache` | Ternary engine stage T0 (W2A8 ternary GEMV); the packing layout is still being designed. |
| `src/omega_gpu_tcgemm_api.h` | `OmegaGpuTcGemmInfo`, `omega_gpu_tcgemm_f32`, `omega_gpu_tcgemm_prepare`, `omega_gpu_tcgemm_rc_name`, `omega_gpu_tcgemm_last_error`, `omega_gpu_tcgemm_cache_clear`, `omega_gpu_tcgemm_set_shape` | OWN-30B T3 cut 1: FP8 grouped GEMM on the tensor cores; one model's shapes. |
| `src/omega_gpu_prefill_attn_api.h` | `OmegaGpuPrefillAttnInfo`, `OmegaGpuPrefillAttnMirrorView`, `omega_gpu_prefill_attn`, `omega_gpu_prefill_attn_release`, `omega_gpu_prefill_attn_dev`, `omega_gpu_prefill_attn_set_warps`, `omega_gpu_prefill_attn_set_fused`, `omega_gpu_prefill_attn_set_kv_mirror`, `omega_gpu_prefill_attn_kv_mirror_view`, `omega_gpu_prefill_attn_kv_mirror_invalidate`, `omega_gpu_prefill_attn_kv_mirror_forget`, `omega_gpu_prefill_attn_kv_mirror_available` | Causal prefill attention for Qwen3-30B-A3B, with an optional KV mirror; one model. |
| `src/omega_gpu_moe_api.h` | `omega_gpu_moe_tensor_upload_bf16`, `omega_gpu_moe_tensor_upload_f32`, `omega_gpu_moe_tensor_free`, `omega_gpu_moe_tensor_shape`, `omega_gpu_matmul_resident_window_f32`, `omega_gpu_matmul_resident_window_bf16`, `omega_gpu_matmul_resident_windows_f32`, `omega_gpu_moe_reserve`, `omega_gpu_moe_prepare`, `omega_gpu_moe_seal`, `omega_gpu_moe_unreserve`, `omega_gpu_moe_cache_clear`, `omega_gpu_moe_set_cta_budget`, `omega_gpu_moe_cta_budget`, `omega_gpu_moe_window_byte_offset` | Column windows over a resident bf16 expert tensor (OWN-30B); tied to the serving pool. |
| `src/omega_gpu_decode_graph.h` | `OmegaGpuDecodeGraphConfig`, `OmegaGpuDecodeGraphStats`, `OmegaGpuDecodeGraphVas`, `omega_gpu_decode_graph_create`, `omega_gpu_decode_graph_free`, `omega_gpu_decode_graph_set_layer`, `omega_gpu_decode_graph_set_final_norm`, `omega_gpu_decode_graph_token`, `omega_gpu_decode_graph_begin_token`, `omega_gpu_decode_graph_record_layer`, `omega_gpu_decode_graph_record_final`, `omega_gpu_decode_graph_end_token`, `omega_gpu_decode_graph_abort_token`, `omega_gpu_decode_graph_record_res`, `omega_gpu_decode_graph_record_swiglu`, `omega_gpu_decode_graph_list`, `omega_gpu_decode_graph_vas`, `omega_gpu_decode_graph_read`, `omega_gpu_decode_graph_set_stamps`, `omega_gpu_decode_graph_set_barrier`, `omega_gpu_decode_graph_barrier`, `omega_gpu_decode_graph_stamps`, `omega_gpu_decode_graph_kind_name`, `omega_gpu_decode_graph_set_flush_each`, `omega_gpu_decode_graph_set_prefetch`, `omega_gpu_decode_graph_prefetch` | One Qwen3-30B-A3B decode token as one command list; scheduling, barrier and prefetch modes are still measured. |
| `src/omega_gpu_prefill.h` | `OmegaPfWeights`, `omega_gpu_pf_rc_name`, `omega_gpu_pf_alloc_dev`, `omega_gpu_pf_alloc_host`, `omega_gpu_pf_free`, `omega_gpu_pf_upload`, `omega_gpu_pf_download`, `omega_gpu_pf_copy_va`, `omega_gpu_pf_stats`, `omega_gpu_pf_stats_reset`, `omega_gpu_pf_weights_upload`, `omega_gpu_pf_weights_free`, `omega_gpu_pf_weights_host_form`, `omega_gpu_pf_gemm_bm`, `omega_gpu_pf_gemm_tiles`, `omega_gpu_pf_gemm`, `omega_gpu_pf_gemm_set_shape`, `omega_gpu_pf_quant`, `omega_gpu_pf_qgemm_bm`, `omega_gpu_pf_qgemm_tiles`, `omega_gpu_pf_qgemm_tiles_va`, `omega_gpu_pf_qgemm`, `omega_gpu_pf_qgemm_set_shape`, `omega_gpu_pf_qgemm_set_group`, `omega_gpu_pf_quant_record`, `omega_gpu_pf_qgemm_record` | Device-resident one-pass prefill for OWN-30B; arguments carry raw device addresses. |
| `src/omega_gpu_pfhost.h` | `omega_gpu_pf_router_rows`, `omega_gpu_pf_router_packed_bytes`, `omega_gpu_pf_router_pack`, `omega_gpu_pf_router_logits`, `omega_gpu_pf_moe_combine`, `omega_gpu_pf_router_set_rows` | Whole-prompt prefill router logits and MoE combine for OWN-30B; arguments carry raw device addresses. |

### 9.8 General matrix multiply (`src/omega_gpu_gemm_ex.h`, added in 0.8.0)

One entry with the shape contract of `cblas_?gemm` (RowMajor): `C_b = alpha * s * op(A_b) op(B_b) + beta * C_b` for any
M, N, K >= 1 (up to `OMEGA_GPU_GEMM_EX_MAX_DIM`), op N or T for each input, any leading dimension, strided batches,
alpha/beta, A and B each f32, bf16, f16, e4m3 or e5m2, C f32, bf16 or f16, and per-tensor or block fp8 scales. The header
comment is the full contract (which input pairs run natively on HMMA/QMMA, which are widened exactly, which go through
bf16x3, how inf and NaN propagate, how block scales cut k). Result codes are the `OMEGA_GPU_GEMM_*` codes of
`src/omega_gpu_gemm.h`, which is not a manifest header.

It is EXPERIMENTAL for four reasons, each one a stated limit rather than a defect:
- **Host memory, host orchestration.** A, B and C are host pointers, and every call with alpha != 0 scans A and B on the
  host for inf and NaN (one read of each). The common case, the native route with one k part, alpha s 1, beta 0, C f32
  with ldc n, n <= 16384 and finite inputs, is one call of the C3 kernels (`omega_gpu_gemm_ld`, which copies its operands
  to the GPU) per 16384 rows of C, writing C in place. Every other call cuts C into host tiles of at most 4096 x 4096 and
  k into parts of at most 16384 (1808 on the bf16x3 route), runs every part as one C3 call, and sums the parts and
  applies alpha, beta and the scales on the host in double. There is no device-resident form and no
  record-into-command-list form, so it is not a serving path: the tuned per-model kernels of sections 9.5 and 9.7 stay
  the fast path for the shapes they take.
  Measured cost at the nine served 35B shapes, bf16 and e4m3 (GB10, predeclared A/B against `omega_gpu_gemm_ld`,
  `evidence/GEMM-GENERAL-AB-20261027/`): after the 0d2b14a host-overhead fix the median ratio is 0.94-1.24
  (ab-2), over the plan's 1.10 limit in five small cells: e4m3 64x1024x2048 1.24, e4m3 64x2048x512 1.20, bf16
  64x1024x2048 1.17, e4m3 16x12288x2048 1.10, e4m3 1x8192x2048 1.10 (about 0.1-0.35 ms per call; not yet attributed).
  Before the fix it was 2.17-3.25 (ab-1).
- **f32 is not IEEE fp32.** f32 inputs run as bf16x3 (three bf16 planes, nine plane products); accuracy is stated
  against a double reference in `tests/gpu_gemm_ex_test.c`, and parts of a value below 2^-133 are dropped.
  `OMEGA_GEMM_EX_F32_FFMA_REF` is the slow per-k FFMA comparator, not a fast path.
- **fp8 output is refused** (`OMEGA_GPU_GEMM_UNSUPPORTED`).
- **Omega with a migration-pending C oracle, one chip.** The plan, routes, k parts, operand conversion, k-part sums,
  host recompute, epilogue and fp8 dequantization are seeded Omega (`om/gemmex/*.om`, generated
  `src/cudapp_om_gemm_ex_*.c`, CUDApp#144); `src/omega_gpu_gemm_ex.c` is their memory shell. The old C stays behind
  `CUDAPP_MIGRATION_PENDING_GEMM_EX` (`mk/gemm-ex.mk`, default 0; 1 builds the C path) as the differential oracle
  (`make test-gpu-gemm-ex-diff`). An Omega unit refusing its window returns `OMEGA_GPU_GEMM_CODEGEN_FAIL`.
  Exercised on sm_121 (GB10) only, by the C path; the Omega path's chip tier is NOT_RUN.

The fp8 GEMV and grouped GEMM of section 9.7 call `omega_gpu_gemm_ex_fp8_block_rows` for shapes their tuned kernels
refuse, and report it in `OmegaGpuFp8Info.general` / `OmegaGpuFp8GemmInfo.general`; those rows carry the bf16x3
accuracy, not the tuned kernels' bit pattern. Shapes the tuned kernels take never reach the fallback.

| Entry | Signature (manifest spelling) |
| --- | --- |
| `OmegaGemmExArgs` | struct, 200 bytes (fields in the manifest) |
| `OmegaGemmExInfo` | struct, 24 bytes: `route`, `core_type`, `core_calls`, `host_fixed` |
| `omega_gpu_gemm_ex` | `fn(*const OmegaGemmExArgs) -> i32` |
| `omega_gpu_gemm_ex_fp8_block_rows` | `fn(*const u8, *const f32, f64, u32, u32, u32, *const f32, *const u16, u32, *mut f32, u64, *mut OmegaGemmExInfo) -> i32` |

The enumerators of `OmegaGemmOp`, `OmegaGemmDtype`, `OmegaGemmExCompute`, `OmegaGemmScale` and `OmegaGemmRoute` and the two
maxima are EXPERIMENTAL constants in the manifest. `omega_gpu_gemm_ex_set_backend` and `omega_gpu_gemm_ex_test_set_mutant`
are TEST ONLY (INTERNAL list).

### 9.9 Version query (`src/omega_runtime_api.h`, added in 0.14.0)

`omega_runtime_api_version()` returns `OMEGA_RUNTIME_API_VERSION`, `(MAJOR << 16) | MINOR`, of the library that was linked,
so a program can compare it with the macro it was compiled against. While MAJOR is 0 the library serves a program only
when MAJOR and MINOR are both equal (section 2); `examples/hello_gpu` prints both and stops with exit 2 when they differ.
EXPERIMENTAL because it has no chip coverage yet and one user.

| Entry | Signature (manifest spelling) |
| --- | --- |
| `omega_runtime_api_version` | `fn() -> u32` |

### 9.10 Random numbers and FFT (`src/omega_gpu_mathlibs_api.h`, added in 0.15.0)

The cuRAND-class and cuFFT-class entries. Every kernel, and the pass plan of the real and multi-dimensional transforms, is
generated by the seeded Omega unit `om/mathlibs/mathlibs_ir.om` (generated C `src/cudapp_om_mathlibs_ir.c`); the C file
`src/omega_gpu_mathlibs_api.c` stages buffers and launches. The header comment is the full contract. In short:
- `omega_gpu_rng_generate` (`OMEGA_GPU_RNG_U64`, `OMEGA_GPU_RNG_UNIFORM`, `OMEGA_GPU_RNG_NORMAL`): xoshiro256**, the
  aien-math stream. `omega_gpu_philox_generate`: Philox4x32-10, the `curand()` stream of `curandStatePhilox4_32_10_t`
  after `curand_init(seed, subseq0 + g, offset)`, raw 32-bit words only; `offset` and `per_stream` must be multiples of 4
  and output blocks of four words interleave across streams. Checked against the Random123 known answers and 1024 rows
  of a host cuRAND 10.4.0 oracle (`tests/fixtures/philox_curand_oracle.txt`).
- `omega_gpu_fft_c2c`: batched complex f32, n = 4..2048 (`OMEGA_GPU_FFT_MIN_N`, `OMEGA_GPU_FFT_MAX_N`), inverse not
  normalised. `omega_gpu_fft_r2c` and `omega_gpu_fft_c2r`: cuFFT's R2C/C2R layout (n / 2 + 1 complex outputs per row),
  n = 8..4096 (`OMEGA_GPU_FFT_REAL_MIN_N`, `OMEGA_GPU_FFT_REAL_MAX_N`); C2R is not normalised and needs Hermitian input
  (Im X[0] = Im X[n/2] = 0, not cleared by the kernel). `omega_gpu_fft_c2c_nd`: 1-D to 3-D row-major complex transforms,
  each dimension 4..2048, one batched 1-D pass per axis followed by a transpose. Batch times points is at most
  `OMEGA_GPU_FFT_MAX_ELEMENTS`.
- `omega_gpu_fft_plan_walk` lists a plan's launches (`OmegaGpuFftLaunch`, modes `OMEGA_GPU_FFT_C2C_ND`,
  `OMEGA_GPU_FFT_R2C`, `OMEGA_GPU_FFT_C2R`) and the in, out and scratch sizes. Host only; the plan and private copies of
  its twiddle tables are built under the session lock and the callback runs after the lock is released, so it may call
  this API.
- `omega_gpu_philox_generate_dist` (`OmegaGpuPhiloxDist`: `OMEGA_GPU_PHILOX_RAW`, `OMEGA_GPU_PHILOX_UNIFORM`,
  `OMEGA_GPU_PHILOX_UNIFORM_DOUBLE`, `OMEGA_GPU_PHILOX_UNIFORM2_DOUBLE`, `OMEGA_GPU_PHILOX_NORMAL`): cuRAND's
  `curand`, `curand_uniform`, `curand_uniform_double`, `curand_uniform2_double` and `curand_normal` drawn from the
  Philox stream above, any offset and any `per_stream`. The integer and uniform outputs are bit-identical to a host
  cuRAND 10.4.0 oracle (`tests/fixtures/philox_curand_dist_oracle.txt`, 4480 rows); the normal output is within
  2^-21 sqrt(-2 log u) of it (the chip's MUFU.RSQ; bound derived with Wolfram in `evidence/MATHLIBS-GAPS-2026-11-06/`).
- `omega_gpu_fft_many` is cufftPlanMany: `OmegaGpuFftMany` gives rank 1..3, the lengths n[] (any length 2..4096,
  `OMEGA_GPU_FFT_MANY_MAX_N`), `inembed`/`istride`/`idist`, `onembed`/`ostride`/`odist`, the batch and the type
  (`OMEGA_GPU_FFT_TYPE_C2C`, `OMEGA_GPU_FFT_TYPE_R2C`, `OMEGA_GPU_FFT_TYPE_C2R`). All-zero embeds select cuFFT's
  basic layout; in place (in == out) pads each real row of R2C and C2R to 2 (n / 2 + 1) floats, as cuFFT. Each axis
  runs Stockham passes of radix 4, 2, 3, 5 and 7 when its length factors over them, else Bluestein's chirp-z algorithm
  on a power-of-two length M >= 2n - 1. C2R is unnormalised, as cuFFT: C2R(R2C(x)) = prod(n) x, and it ignores Im of
  the DC and Nyquist bins. `omega_gpu_fft_many_walk` lists the launches (`OmegaGpuFftManyLaunch`) and the sizes of the
  `OMEGA_GPU_FFT_MANY_BUFS` buffers (`OMEGA_GPU_FFT_BUF_IN` .. `OMEGA_GPU_FFT_BUF_TABLE`); host only, its callback
  runs unlocked. `omega_gpu_fft_c2c`, `_r2c`, `_c2r` and `_c2c_nd` route lengths outside their power-of-two ranges
  (still 2..4096) through the same plans.
- `omega_gpu_rng_codegen`, `omega_gpu_fft_codegen` and `omega_gpu_philox_codegen` build a kernel on the host and fill
  `OmegaGpuMathInfo` without opening a device. `omega_gpu_fft_twiddles` returns the f32 table the C2C kernel reads.
  Result codes: `OMEGA_GPU_ML_OK`, `OMEGA_GPU_ML_BAD_ARGS`, `OMEGA_GPU_ML_TOO_LARGE`, `OMEGA_GPU_ML_CODEGEN_FAIL`,
  `OMEGA_GPU_ML_CHIP_FAIL`, `OMEGA_GPU_ML_UNWRITTEN`; `omega_gpu_mathlibs_last_error` names the failure and
  `omega_gpu_mathlibs_cache_clear` drops the cached kernels.
- Launch limits: `OMEGA_GPU_RNG_THREADS`, `OMEGA_GPU_RNG_MAX_STREAMS`, `OMEGA_GPU_RNG_MAX_OUTPUTS`,
  `OMEGA_GPU_FFT_MAX_CTAS`, `OMEGA_GPU_PHILOX_THREADS`, `OMEGA_GPU_PHILOX_MAX_STREAMS`, `OMEGA_GPU_FFT_MANY_THREADS`.

It is EXPERIMENTAL because:
- **Host memory.** Inputs and outputs are host pointers copied through uncached session buffers on every call; there is
  no device-resident or command-list form.
- **Chip evidence is partial.** The xoshiro256** generator and the complex FFT have one GB10 run
  (`evidence/MATHLIBS-RNG-FFT-2026-10-27/`). Philox and its distributions, R2C, C2R, the 2-D/3-D transforms and the
  many plans (Stockham radix 2/3/4/5/7, Bluestein, strided copies) are checked on the host and in the warp simulator
  (`make test-gpu-mathlibs`); their chip tier is NOT_RUN.
- **Missing pieces.** No `curand_normal_double` (it needs f64 arithmetic in the IR). Buffers are host-staged (see above).

| Entry | Signature (manifest spelling) |
| --- | --- |
| `OmegaGpuMathInfo` | struct, 96 bytes (fields in the manifest) |
| `OmegaGpuFftLaunch` | struct, 88 bytes (fields in the manifest) |
| `OmegaGpuFftMany` | struct, 68 bytes (fields in the manifest) |
| `OmegaGpuFftManyLaunch` | struct, 160 bytes (fields in the manifest) |
| `omega_gpu_philox_generate_dist` | `fn(OmegaGpuPhiloxDist, u64, u64, u32, u64, u32, *mut c_void, *mut OmegaGpuMathInfo) -> i32` |
| `omega_gpu_fft_many` | `fn(*const OmegaGpuFftMany, *const f32, *mut f32, *mut OmegaGpuMathInfo) -> i32` |
| `omega_gpu_fft_many_walk` | `fn(*const OmegaGpuFftMany, i32, *mut usize, fn(*mut c_void, *const OmegaGpuFftManyLaunch) -> i32, *mut c_void) -> i32` |
| `omega_gpu_philox_generate` | `fn(u64, u64, u32, u64, u32, *mut u32, *mut OmegaGpuMathInfo) -> i32` |
| `omega_gpu_fft_r2c` | `fn(*const f32, *mut f32, u32, u32, *mut OmegaGpuMathInfo) -> i32` |
| `omega_gpu_fft_c2r` | `fn(*const f32, *mut f32, u32, u32, *mut OmegaGpuMathInfo) -> i32` |
| `omega_gpu_fft_c2c_nd` | `fn(*const f32, *mut f32, u32, *const u32, u32, i32, *mut OmegaGpuMathInfo) -> i32` |
| `omega_gpu_fft_plan_walk` | `fn(OmegaGpuFftPlanMode, u32, *const u32, u32, i32, *mut usize, fn(*mut c_void, *const OmegaGpuFftLaunch) -> i32, *mut c_void) -> i32` |

The other signatures are in the manifest. `omega_gpu_mathlibs_test_set_mutant` is TEST ONLY (INTERNAL list).

## 10. INTERNAL (not part of the API)

The INTERNAL CPU verification helpers `omega_matmul_cpu_oracle_i32`,
`omega_matmul_cpu_oracle_f16` and `omega_matmul_cpu_oracle_bf16` in
`src/omega_blackwell_matmul.h` take positive row-major m/k/n dimensions. The output
must not overlap either input; read-only inputs may overlap. Hosted checked Omega
views return -1 before touching an aliased output. The hosted implementation uses
runtime extents; the `OMEGA_GPU_NO_POSIX` C exception is pending omega-bridge#72.
These verification helpers are not production GPU dispatch and are not manifest
entries. No public ABI signature or layout changes with this migration.

Not in the manifest; may change or vanish without notice. The complete list of such names in the manifest headers, each
with its reason, is `docs/runtime/omega_runtime_api.internal` (195 names in 0.9.0: test hooks, kernel builders, encoder
probes, serving-daemon internals and launch limits; the 25 serving internals and launch limits are marked "candidate
EXPERIMENTAL", the ones a caller is most likely to want promoted). `make test-runtime-api` fails on a name in a manifest header that is in neither file. Notable ones:
- `omega_gpu_session_set_error` (library code names its failure stage through it).
- `omega_gpu_session_on_close` (lets a library drop its resident kernels before the session closes; up to 8 hooks).
- `omega_gpu_engine_test_reset_block`, `omega_gpu_engine_test_force_block`, `omega_gpu_engine_test_set_clock`: TEST ONLY. A blocked process must stay blocked because the GPU may still be running.
- Take or return a kernel-generator type, which is backend (ADR 0043): `omega_gpu_fp8_depth_for`, `omega_gpu_fp8_kernel_fits`, `omega_gpu_fp8gemm_kernel_fits`, `omega_gpu_bf16_kernel_fits`, `omega_gpu_tern_kernel_fits`, `omega_gpu_tcgemm_spec_for`, `omega_gpu_prefill_attn_encode`.
- TEST ONLY (mutant and fault-injection setters in the decode-path headers): `omega_gpu_fp8_test_set_mutant`, `omega_gpu_fp8_test_set_no_fold`, `omega_gpu_fp8gemm_test_set_mutant`, `omega_gpu_bf16_test_set_mutant`, `omega_gpu_decode_attn_test_set_mutant`, `omega_gpu_router_test_set_mutant`, `omega_gpu_tern_test_set_mutant`, `omega_gpu_tcgemm_test_set_mutant`, `omega_gpu_prefill_attn_test_set_mutant`, `omega_gpu_prefill_attn_test_set_mirror_per_call`, `omega_gpu_prefill_attn_test_launch_ns`, `omega_gpu_moe_test_set_mutant`, `omega_gpu_moe_test_set_window_skew`, `omega_gpu_moe_test_a_row0_hits`, `omega_gpu_decode_graph_test_set_mutant`, `omega_gpu_decode_graph_test_set_barrier_none`, `omega_gpu_decode_graph_test_set_prefetch_red`, `omega_gpu_pf_gemm_test_set_mutant`, `omega_gpu_pf_quant_test_set_mutant`, `omega_gpu_pf_qgemm_test_set_mutant`, `omega_gpu_pf_router_test_set_mutant`, `omega_gpu_pf_combine_test_set_mutant`.
- Random numbers and FFT (section 9.10): `omega_gpu_mathlibs_test_set_mutant` (TEST ONLY: selects a mutant of the Omega unit).
- General GEMM (section 9.8): `omega_gpu_gemm_ex_set_backend` (TEST ONLY: swaps the C3 core for the simulator or a host model), `omega_gpu_gemm_ex_test_set_mutant` (TEST ONLY), and `omega_gpu_fp8_general_rows`, the fp8 GEMV and grouped GEMM bridge to `omega_gpu_gemm_ex_fp8_block_rows`.
- Host reference and packing helpers in the decode-path headers (not `omega_gpu_` entries): `omega_fp8_e4m3_ref`, `omega_fp8_e4m3_kernel_bits`, `omega_fp8_e4m3_is_nan`, `omega_fp8_pack_scan_nan`, `omega_fp8_pack_scales`, `omega_fp8_pack_x_f32`, `omega_fp8_pack_x_bf16`, `omega_fp8_gemv_oracle`, `omega_fp8gemm_permute_x`, `omega_dattn_bf16_rne`, `omega_dattn_bf16_f32`, `omega_dattn_rope_row`.
- Launch helpers whose caller already holds the session lock: `omega_gpu_pf_kernel_locked`, `omega_gpu_pf_launch_locked`, `omega_gpu_pf_open_locked`.
- Everything under `src/omega_gpu_*` and `src/omega_blackwell_*` in headers the manifest does not name (kernel builder, encoder, per-model APIs, launch helpers).

The private `omega_gpu_native_runtime.h` and `omega_gpu_q36_native.h` headers
bind generated Omega residency/selection policy to existing runtime handles.
They are not public API or a caller-controlled qualification interface. A
prepared lease pins code; recording takes a distinct pin; submission acquires
an inflight reference before queueing. A timeout is not completion: reset and
cache clearing must retain pending code, and freeing the host command-list
object cannot release its inflight code reference. A confirmed device teardown
advances the private session epoch; only then may stale cache ownership be
invalidated. Exact-native recording consumes the selected canonical artifact's
uploaded code address and validated launch declaration rather than generating
an incumbent kernel for the same shape.

Canonical manifests and the existing independent qualification/admission path
remain authoritative. Realization digests and live closure digests are binding
metadata, not alternative trust stores. No native candidate is activated by
these private interfaces, host fixture handles, manifest text or caller flags.
The public header inventory, version and manifest are intentionally unchanged.
The private `omega_native_artifact_snapshot.h` interface retains exactly the
manifest, text, cubin, QMD and constant-bank bytes checked by existing artifact
admission. Digest and semantic checks share those read-once buffers; reopening
a mutable path cannot substitute unchecked bytes. A snapshot transfers byte
custody only, not independent qualification or permission to activate code.

### 10.1 Private API-plan policy

The six RuntimeHost adapters use `om/components/api_{mm,at,ew,bf,da,rt}_plan.om`
for scalar validation, refusal selection, padding, scratch sizing, kernel-plan
selection and launch/chunk geometry. `src/omega_api_plan.h` is private glue,
not a public header. Each root takes a mode and a fixed 16-word edit view:
words 0..7 are inputs, words 8..15 are results. Router allocation also returns
whether the caller's output pointer must be cleared in word 7, preserving
the original order of argument refusal and that observable write. Policy
refusals 1/2/3 map to the existing negative BAD_ARGS/TOO_LARGE/CODEGEN_FAIL
codes. No public signature, manifest entry or version changes.
Private locals also avoid OM2 reserved words: the attention allocation-size
temporary is `partsz`, not the reserved `part`, so whole-unit OM2 composition
does not fail at that identifier.

The default is Omega. `CUDAPP_MIGRATION_PENDING_C_API_PLAN=1` selects the C
oracle in that private header, not a fallback after an Omega refusal.
Both implementations repair the old BF16 `k < 256` divide-by-zero in default
depth derivation, and widen decode's `pos + per_split` before division to
avoid a zero-split plan on unsigned overflow. Shell chunk counters are wide
enough to terminate at the upper u32 input boundary.

The C adapters retain pointer/object admission, ABI marshalling, buffer
ownership, cache/session state, code generation, data packing, copies and
acting on the returned plan. Host numerical reference kernels and the
adapter-local device-code generators are not converted by this policy
change. These units neither access a device nor replace independent artifact
qualification or launch admission.

`tests/runtime/api_plan_unit.c` is the executable definition of the bounded
differential domain. Every call compares the return code and **all 16 words**,
including unchanged inputs, unused output canaries and refusal-side writes.
The bounds are chosen from API limits and the threshold/divisibility classes
used by the policy, rather than from typical model shapes:

| Root / adapter | Exhaustive bounds and boundary products |
| --- | --- |
| `api_mm_plan` / matmul | Every M 0..8193, K 0..16385 and N 0..262145 on the independent admission axes; edge Cartesian products include simultaneous invalid fields. Every column-tile count 1..32768 crosses representative row-tile and CTA-budget classes; every row-tile count 1..512 crosses budgets 0..1024 and column counts 1..8. These cover the full admitted padded dimensions, all tile divisibility residues and the 1024-CTA default. All K values cover the packing quotient; all N values cover tail width. Every K-slice count 1..1024, rt/ct in {1,2,4}, unroll override 0..128 and mutant bit covers selection and halving; every row/chunk count 1..512 crosses rt in {1,2,4}. |
| `api_at_plan` / attention | Every query/KV-head pair 0..1025 crosses head-dimension threshold representatives and the general-path force bit; every head dimension 1..256 also crosses every legal query-head count. Head-map tests enumerate every legal divisor and every query head through 1024. All contexts 0..131073 cross both tuned dimensions; every u32 power of two and its neighbours covers exact-log refusal classes. Every legal query count crosses row/capacity edges and split logs 0..4. Causal/window tests exhaust positions and lengths 0..128, windows {-2,-1,0,1,2,INT32_MAX} and the causal bit. Layout, capacity, block, staging, reserve and overflow modes cross the u32 threshold-edge set. Scale tests cross every f32 sign/exponent with mantissas {0,1,all-ones}, both scale modes and every admitted head dimension, plus neighbours of the scale ceiling. |
| `api_ew_plan` / elementwise | Every dimension 0..16385 crosses row-count edges; every 1-D length 0..262145 covers the 131072-element chunk cap, 128-thread residues and exchange alignment. Rope's even/odd and 2048 ceiling classes and RMS's 128 divisibility and 16384 ceiling are exhaustive within their admitted dimensions. All reconvergence operation offsets 0..6 cross length edges. |
| `api_bf_plan` / BF16 GEMV | Every K 0..16385 crosses all combinations of default/nondefault tuning knobs; individual overrides also cross u32 edges. Every N 0..65536 crosses rows 1..4 and 16/32 warps, covering row divisibility and capped-grid classes. Every recorded row count 0..65536 crosses record, mask, tau, xbatch 0..5 and y-stride threshold classes. Register budgets exhaust warps 0..1024 and declarations 0..512. |
| `api_da_plan` / decode attention | Every position/capacity 0..131073 crosses layers 0..65. Every legal position 0..131071 crosses every 256-aligned split size through 131072; every larger representable u32 aligned split size is exercised at the maximum legal position (the split count is then always one). The explicit UINT32_MAX-adjacent split case catches the old addition overflow. Layer/head/span boundaries cross the edge set; theta tests cover signed zero, subnormal, finite positive/negative, infinity and NaN; every declaration 0..1024 crosses the register bound. |
| `api_rt_plan` / router | Every layer 0..65 crosses experts 0..257 and shared 0..2, including both supported expert counts and their neighbours. Every row count 0..65 crosses x-strides 8176..8224 and launch-buffer strides 16640..17152 for both launch-buffer sizes. Those stride intervals include both refusal thresholds and every modulo-16/modulo-256 class. Layer/rows/record/buffer-selection products and all tuning/stride edges exercise plan results; register budgets exhaust warps 0..1024 and declarations 0..512. |

Codegen admission additionally exhausts general/tuned attention selection,
block logs 0..33, GQA logs 0..8 and head dimensions 0..257, including refusal
precedence when several fields are invalid. Split and elementwise mutant
controls cross every accepted value, both rejection/clamping sides and signed
int extremes. The matmul CTA setter crosses every budget 0..1025 and edges.

The shared edge set includes zero, all policy thresholds and their neighbours,
power-of-two/divisibility transitions, and UINT32_MAX-1/UINT32_MAX. Bounds
cover all finite admitted shape axes; larger values select rejection,
saturation or the same quotient/bit-mask classes, with explicit overflow
edges. This is bounded exhaustive differential evidence, **not** a formal
equivalence proof over the Cartesian product of arbitrary u32/u64 words or
every floating-point mantissa. No such unbounded proof is claimed.

The short-view ABI negative control must refuse before writing its result.
`tests/runtime/api_plan_mutants.py` changes two unique source expressions in
each root, compiles each changed `.om` with the pinned compiler, and requires
the unchanged differential to fail. A compiler error does not count as a
killed mutant. `test-api-plan` is registered in `CARRIED_HOST_TESTS`;
source mutation is separate because it executes the AArch64 compiler.

The max-position adapter gate retains seven controls against five checked
generated-binding mutants. Its read-position, write-count and step-position
controls now target the same shared Omega span predicate, but must each emit
their own failed API expectation. The other controls cover both cap boundaries,
the separate write-position guard, and short RoPE allocation. A
`--revert-first` runner invocation deliberately restores the first mutation;
the unchanged binding must pass the API battery, and the mutation gate must
reject it with exit 1. Both the ordinary and reverted runs use the device-free
wrapper below. These portable binding controls do not replace the two pinned
`.om` source mutants per new unit.

Focused host-only reproduction, with the existing pinned compiler/bridge:

```sh
export OMEGA_OMC=/path/to/omega-mv/seed/out/omc
export OMEGA_BRIDGE=/path/to/omega-bridge
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 sh om/units.sh
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 test-api-plan test-api-plan-mutants
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 test-gpu-matmul-api test-gpu-elementwise \
  test-gpu-attention test-gpu-bf16-gemv test-gpu-decode-attn test-gpu-router
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 OUT_DIR=build/api-plan-c \
  CC='cc -DCUDAPP_MIGRATION_PENDING_C_API_PLAN=1' \
  test-gpu-matmul-api test-gpu-elementwise test-gpu-attention \
  test-gpu-bf16-gemv test-gpu-decode-attn test-gpu-router
```

The separate output directory prevents stale objects from hiding a switch
change. These targets exercise existing host and simulator cases only.
Full host integration remains a separate integrator gate; chip qualification
and deployment require their own authorization and receipts.

The f32 ceiling-neighbour literals in the differential come from
`WolframLanguageEvaluator:
n=Round[(3*10^38/2^127)*2^23];
Table[IntegerString[(127+127)*2^23+n-2^23+i,16],{i,-2,2}]`
→ `{"7f61b1e4","7f61b1e5","7f61b1e6","7f61b1e7","7f61b1e8"}`.
Other numerical policy literals are unchanged from the incumbent adapters.

## 11. Checking this file

`make test-runtime-api` runs `tools/runtime_api_check.sh` on the real headers and then `tools/runtime_api_negative.sh`. The
negative script breaks a copy of the headers (or this file) in fourteen ways and requires the check to fail each time: a
removed function, a missing MAJOR and a missing PATCH version macro, a changed function signature, a changed struct, a
changed constant, a version line in this file that disagrees with the macro, a quoted signature in this file that no header
has, a quoted signature that only appears inside a longer one (`int32_t` inside `uint32_t`), a STABLE entry with no section
here, an added undeclared function, an added undeclared macro, an added undeclared enumerator, and a stale INTERNAL entry.
It then breaks a copy of the manifest five ways and requires `tools/runtime_api_semver.sh` to fail each time (a removed
STABLE function with no bump on a 0.x base, a removed STABLE function with only a MINOR bump on a base already at 1.x, a
changed STABLE signature with no bump, an added function with no bump, a version that goes down) and to pass two correct
changes (a removed STABLE function with a MINOR bump on a 0.x base; an added function with a MINOR bump).
`make test-runtime-api-semver` compares the real manifest with the base branch's. None of it needs a GPU or a physics
checkout. It checks: every manifest function against its header with exact types; sizes, offsets and constants against a
compiled probe; the version macros against the manifest and this file; every public name in the manifest headers against
the manifest and the INTERNAL list; that every STABLE entry has a `####` section here and every EXPERIMENTAL one is named
here; that every C signature quoted in this file appears verbatim in a header; and the size of the version bump. It does
not check that the prose is true. A struct field added inside padding is not seen, nor a field that changes type but keeps
its size, nor a constant value quoted in prose, nor a new type (struct or typedef) that no function uses yet, nor a
function-like macro.

## 12. Any-shape attention (EXPERIMENTAL)

Header: `src/omega_gpu_attn_any_api.h`. `OmegaGpuAttnAnyShape` carries
`seq_q`, `seq_kv`, `head_dim`, `q_heads`, `kv_heads`, `causal`, `dtype` (u32)
and `scale` (f32), in that order. Contiguous `[head][sequence][component]`
Q/K/V use dtype `OMEGA_GPU_ATTN_ANY_F32` (0), `OMEGA_GPU_ATTN_ANY_BF16` (1)
or `OMEGA_GPU_ATTN_ANY_F16` (2);
output is f32. Head dimensions 1..`OMEGA_GPU_ATTN_ANY_MAX_HEAD_DIM` (256),
sequence lengths 1..65535 and head counts 1..65535 are admitted when KV heads
divide Q heads and each tensor has at most 1073741824 elements. No padding or
power-of-two restriction exists. Causal mode 1 means top-left alignment
(`key <= query`), including rectangular matrices; mode 0 is non-causal.
Scale is a finite caller-provided multiplier, including zero and negative values.
Inputs and intermediate dots must be finite; the call does not scan input values.
All other dtype values are refused, not widened or reinterpreted.

```c
int omega_gpu_attn_any(const OmegaGpuAttnAnyShape *shape, const void *q, const void *k, const void *v, float *out);
int omega_gpu_attn_any_codegen(const OmegaGpuAttnAnyShape *shape, BlackwellIRProgram *program, uint8_t *code, size_t capacity, OmegaBlackwellKernel *kernel);
```
`omega_gpu_attn_any_codegen` generates an admitted shape's IR and encoded kernel
without opening a device. The execution entry point `omega_gpu_attn_any` owns
allocations, copies and launches; both use the same seeded admission unit.


Both return `OMEGA_GPU_ATTN_ANY_OK` (0), `REFUSED` (1) for unsupported
geometry/format, `BAD_ARGS` (2) for null arguments, `CODEGEN` (3) for
generation/encoding/storage failure, or `CHIP` (4) for device execution failure.
These names all have prefix `OMEGA_GPU_ATTN_ANY_`. Refusals happen before
device open. Codegen is CPU-only and writes caller-owned storage; kernel code
aliases `code` and owns nothing. The execution entry allocates GPU memory,
launches one warp per query row per query head, then copies output back.
The session retains its normal uncertain-completion latch/parking policy.
Online softmax uses f32 dot/accumulator/running maximum and sum.
See `om/attn-any/README.md` for the tested numerical envelope and
`om/attn-any/CHIP.md` for the operator-gated chip procedure. No performance or
chip-qualification claim is made.


## 13. DNN layers (EXPERIMENTAL)

### 13.1 Scalar formats (`src/omega_gpu_dnn_types.h`)

`OMEGA_GPU_DNN_F32` (0), `OMEGA_GPU_DNN_BF16` (1) and
`OMEGA_GPU_DNN_F16` (2) identify distinct contiguous scalar input storage
formats. DNN kernels use f32 accumulation and f32 output. Neither 16-bit
format is an alias for the other. These constants are EXPERIMENTAL;
native conversion encoding is host/oracle exercised, not chip qualified.

### 13.2 Row softmax and normalization (`src/omega_gpu_dnn_rows_api.h`)

CPU-only builder: `omega_gpu_dnn_rows_codegen`. Execution shell:
`omega_gpu_dnn_rows`.

```c
int omega_gpu_dnn_rows_codegen(const OmegaGpuDnnRowsShape *shape, BlackwellIRProgram *program, uint8_t *code, size_t capacity, OmegaBlackwellKernel *kernel);
int omega_gpu_dnn_rows(const OmegaGpuDnnRowsShape *shape, const void *input, float *output);
```

`OmegaGpuDnnRowsShape` contains `uint32_t rows, length, dtype, op` and
`float epsilon`. Input and output are contiguous `[rows][length]`; output is
f32 for every storage type. Rows and length are independently 1..65535,
with at most 1073741824 elements. `OMEGA_GPU_DNN_SOFTMAX` (0) selects stable
row softmax, `OMEGA_GPU_DNN_LAYERNORM` (1) mean-centered normalization, and
`OMEGA_GPU_DNN_RMSNORM` (2) root-mean-square normalization. Norms have no
affine transform. Their epsilon must be positive finite **normal** f32;
softmax ignores epsilon.

Finite inputs and finite intermediate differences, sums, squares,
variance-plus-epsilon and output are caller preconditions. The codegen entry
is CPU-only. The execution entry requires an authorized GPU window; Omega
rejects unsupported geometry, storage, operation and epsilon before session
access. Status constants are `OMEGA_GPU_DNN_ROWS_OK` (0),
`OMEGA_GPU_DNN_ROWS_REFUSED` (1), `OMEGA_GPU_DNN_ROWS_BAD_ARGS` (2),
`OMEGA_GPU_DNN_ROWS_CODEGEN` (3), and `OMEGA_GPU_DNN_ROWS_CHIP` (4).
See `om/dnn-rows/README.md` for the independent double oracle, MODELED test
envelope and finite numerical test domain, and `om/dnn-rows/CHIP.md` for the
operator-gated NOT_RUN procedure. MUFU simulation is not a chip accuracy bound.

### 13.3 Elementwise activations (`src/omega_gpu_dnn_activations_api.h`)

CPU-only builder: `omega_gpu_dnn_activations_codegen`. Execution shell:
`omega_gpu_dnn_activations`.

```c
int omega_gpu_dnn_activations_codegen(const OmegaGpuDnnActivationsShape *shape, BlackwellIRProgram *program, uint8_t *code, size_t capacity, OmegaBlackwellKernel *kernel);
int omega_gpu_dnn_activations(const OmegaGpuDnnActivationsShape *shape, const void *input, float *output);
```

`OmegaGpuDnnActivationsShape` contains `uint64_t elements` and
`uint32_t operation, dtype`. Nonempty contiguous counts through `UINT32_MAX`
are admitted, including non-warp multiples. Input uses the shared storage
constants; arithmetic and output are f32. `OMEGA_GPU_DNN_RELU` (0),
`OMEGA_GPU_DNN_GELU_ERF` (1), `OMEGA_GPU_DNN_GELU_TANH` (2) and
`OMEGA_GPU_DNN_SILU` (3) select the operation. Inputs must be finite and caller
buffers large enough; input values are not scanned on the host.

Erf GELU uses the documented A&S 7.1.26 approximation; tanh GELU and SiLU use
stable negative-exponential forms. Signed zero is preserved. The codegen
entry is CPU-only; execution requires an authorized GPU window. Admission
precedes session access. Status constants are
`OMEGA_GPU_DNN_ACTIVATIONS_OK` (0), `OMEGA_GPU_DNN_ACTIVATIONS_BAD_ARGS` (1),
`OMEGA_GPU_DNN_ACTIVATIONS_REFUSED` (2),
`OMEGA_GPU_DNN_ACTIVATIONS_CODEGEN` (3), and
`OMEGA_GPU_DNN_ACTIVATIONS_CHIP` (4).
See `om/dnn-activations/README.md` for formulas, saturation, the Wolfram
queries/results and the independent reference envelope, and
`om/dnn-activations/CHIP.md` for the operator-gated NOT_RUN procedure.

### 13.4 Implicit conv2d (`src/omega_gpu_conv2d_api.h`)

CPU-only planner: `omega_gpu_conv2d_plan`; builder:
`omega_gpu_conv2d_codegen`. Execution shell: `omega_gpu_conv2d`.

```c
int omega_gpu_conv2d_plan(const OmegaGpuConv2dShape *shape, uint64_t launch, uint64_t plan[32]);
int omega_gpu_conv2d_codegen(const OmegaGpuConv2dShape *shape, BlackwellIRProgram *program, uint8_t *code, size_t capacity, OmegaBlackwellKernel *kernel);
int omega_gpu_conv2d(const OmegaGpuConv2dShape *shape, const void *image, const void *filter, float *output);
```

`OmegaGpuConv2dShape` has eighteen `uint32_t` fields in declaration order:
`batch, in_channels, height, width, out_channels, kernel_h, kernel_w,
stride_h, stride_w, pad_top, pad_bottom, pad_left, pad_right, dilation_h,
dilation_w, groups, layout, dtype`. Image/output layout is contiguous
NCHW (`layout=0`) or NHWC (`layout=1`); filters are always OIHW
`[out_channels][in_channels/groups][kernel_h][kernel_w]`.
This is cross-correlation with independent top/bottom/left/right zero
padding, positive stride/dilation and groups, including depthwise cases.
There is no fused bias or activation. Input/filter storage uses the shared
DNN constants; accumulation/output are f32.

Positive dimensions, strides, dilations and groups are at most 65535;
padding is 0..65535. Channels must divide by groups, output must be nonempty,
and every tensor element count and launch count must be below 1073741824.
The CPU-only planner returns output extents, byte counts, grid and launch
offsets in `plan[32]`; caller storage must cover those byte counts. The
codegen entry is CPU-only. The execution entry is a memory/session shell
requiring an authorized GPU window; admission occurs first.

Convolution supplies virtual image/filter addresses to the same seeded
scalar dot-product reduction as default dense f32 GEMM. No global im2col or
production host arithmetic kernel is used. Existing tensor GEMM interfaces
remain unchanged; the old C scalar generator is a default-OFF differential
oracle. Status constants are `OMEGA_GPU_CONV2D_OK` (0),
`OMEGA_GPU_CONV2D_BAD_ARGS` (1), `OMEGA_GPU_CONV2D_REFUSED` (2),
`OMEGA_GPU_CONV2D_CODEGEN` (3), and `OMEGA_GPU_CONV2D_CHIP` (4).
See `om/conv2d/README.md` for planner output slots, independent reference
tolerances and test scope, and `om/conv2d/CHIP.md` for the operator-gated
NOT_RUN procedure. Performance and chip qualification remain NOT_RUN.
