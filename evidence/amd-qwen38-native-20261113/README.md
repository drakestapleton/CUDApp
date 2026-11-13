# Native Omega Qwen3.8-27B on AMD, 2026-11-13

The Omega executable runs the real Qwen3.8-27B-FP8 checkpoint on the AMD
MI300X notebook through its own KFD/AQL path. Its process uses no HIP, ROCr,
Torch or Python. Those libraries appear only in the independent external
verification instruments preserved under `oracle/`.

This is a command-line executor for trusted model plans, not an HTTP serving
release. This receipt qualifies execution and records the comparison. Publication,
review and landing authority belong to the separate release receipt.

## What ran

- Model: `Qwen/Qwen3.8-27B-FP8`, revision
  `017b9c7af6b5689d5dd426a76e0bc077eb5ca20a`.
- Native executable SHA-256:
  `dd626fcdf6c405e988499f728eb791a5c12f1ca9fec97d9ca7dc9ead2a19facf`.
- Compiler: Omega source `64d6db3bb8a0c6b4a4f5334b75191abf77c66ab9`,
  seeded executable SHA-256
  `26540e6de62402d5784b30b24d7c6faae488137c1f8a4b3e784fd0b5d5528991`.
- The card package identifies launcher commit `069fe4e8`, expanded to
  `069fe4e84a62d51b2456c65a26ce1720b02892fd`. A fresh checkout of
  `a5f95dd1cf671ce23b7c2ad940268063850e0854` rebuilt the identical executable
  and every one of the 18 card-tested code objects. That head also contains the
  kernel scheduling regression gate. `host/qualification.json` records its gates.
- The coding fixture generated all 24 reference IDs exactly; its altered-weight
  control was caught. Both ran 47 model steps on this executable.
- The longer fixture used the archived Spark lighthouse request: 30 prompt IDs,
  256 generated IDs and capacity 131072. Each request ran 285 model steps,
  with 1126 dispatches per step. All five accepted requests matched the
  independent AMD HIP graph's 256 IDs. The altered-weight control was caught.

The 131072 capacity establishes allocation and dispatch at that configured
capacity. It does not establish long-prompt correctness or retrieval quality.

## Timing and memory

The five accepted native decode medians were 26.828, 26.794, 26.744, 26.745 and
26.751 ms. The warmup, 26.872 ms, is retained and excluded. `calculate.py` reads
these measurements and produces `calculations.json`.

| Measurement | Native AMD, November 13 | Archived Spark, November 1 |
|---|---:|---:|
| Median decode step / server-reported graph time | 26.751 ms | 125.4 ms |
| Prefill | 782.266 ms | 507 ms |
| Process/engine ready | 19.3847 s | 48.0411 s |
| Process start to first generated ID / streaming first visible content | 20.1669 s | 0.4900 s |
| New process total / already-running server request | 27.0980 s | 32.7968 s |
| Sampled peak device VRAM | 37,555,696 KiB | Not separately reported |
| Sampled peak GTT | 183,860 KiB | Not separately reported |
| Sampled peak process RSS | 3,312 KiB | 38.55 GB |

The decode-time ratio is 125.4 / 26.751 = **4.6877**. The reciprocal of the
native median step is 37.3818 tokens/s; this is a derived decode statistic, not
measured HTTP throughput. AMD prefill is slower in this implementation.

The timing boundaries matter:

- Native step time includes packet publication and completion waiting on the
  host monotonic clock. Prefill is token by token. Decode excludes prompt steps.
- Every native trial starts a new process. The weights are already downloaded,
  and the filesystem cache is warm. Allocation, loading and template construction
  are excluded from per-step time and included in process timings.
- The Spark figures are an archived server qualification, not a new paired run.
  Its first visible content and request timings describe an already-running HTTP
  server. They are different boundaries from native process timings. The startup
  and request rows above are descriptive, not controlled speedup claims.
- AMD memory was sampled every 100 ms from the native process and DRM fdinfo.
  Spark uses unified memory; its process RSS and AMD's separate VRAM/GTT/RSS
  counters are different accounting. No memory-saving ratio is established.
- Diagnostic processes exited before the five native trials. This harness does
  not establish exclusive access to the provider's entire physical host.

The baseline is the canonical Core Radicle repository
`rad:z44GqrPVAPjZkhoD2MWVjqy18ighN`,
`docs/qwen38/qualification-20261101/README.md`, last changed at
`5c95594a923de32a833e85512d677dba0b213110`.
`spark-baseline/qualification.json` pins the source blob and source content hash.
The exact request, response and input-u32 bytes are preserved beside it.

## Numerical comparison

The coding fixture has exact 24/24 reference agreement. The longer free-running
AMD trajectory first differs from the archived Spark trajectory at generated
index 109, using zero-based indexing. Both native and HIP graph produce the
same complete 256-ID trajectory on AMD.

Teacher-forcing the entire Spark trajectory into the AMD graph isolates two
choices where AMD's highest score differs from Spark's saved choice:

| Generated index | Spark choice rank on AMD, zero based | AMD top-1 minus Spark logit |
|---|---:|---:|
| 109 | 1 | 0.1898 |
| 138 | 1 | 0.0101 |

The independent AMD library implementation first differs from Spark at index
29, where its top two choices are only 0.125 logit apart. At the same prefix,
the AMD Omega graph agrees with Spark, with a 0.1057-logit margin.

These observations are consistent with close choices changing under different
numerical implementations. They do not establish bitwise cross-engine agreement,
model-quality equivalence, or an error bound for every layer. Position-wise
agreement after a free-running divergence is not treated as a quality metric,
because the subsequent contexts differ. The unchanged Spark IDs remain in the
fixture; the native benchmark separately requires exact independent AMD graph
agreement and reports Spark divergence. No golden output was silently replaced.

## Reproduction

Build the launcher with the compiler pinned in
`backend/amd/kfd/launch/COMPILER`, following that directory's README. The
launcher gate checks its compiler digest and rebuilds the code objects. The
host gates and their exact build head are recorded under `host/`.

The scripts in `oracle/` are portable copies of external measuring instruments.
Only the repository license header was prepended; `oracle/ORIGINAL_SCRIPTS.json`
records the original hashes and the exact prefix, so the original bytes can be
verified by removing that prefix. No measuring algorithm was changed. They are
not imported by, linked into, or required at runtime by the native executable.
They require Torch and safetensors to record/check a model plan on the CPU.
`driver-real2.py` pins the checkpoint files and contains the independent HIP
loader/reference. Its `prepare()` function downloads and verifies those files.
Use a separate process for each GPU diagnostic.

From this receipt directory, with the checkpoint files in `$WEIGHTS`:

```sh
mkdir out
python3 oracle/emit-plan.py oracle/driver-real2.py "$WEIGHTS" out/real2.plan --capacity=131072 --fixture-json=oracle/lighthouse256.json
python3 oracle/check-plan.py out/real2.plan "$WEIGHTS"
python3 oracle/emit-plan.py oracle/driver-real2.py "$WEIGHTS" out/real2-controls.plan --controls --capacity=131072 --fixture-json=oracle/lighthouse256.json
python3 oracle/check-plan.py out/real2-controls.plan "$WEIGHTS"
```

The emitter writes the code objects and an argument list in each `.plan.cmd`.
The launcher arguments are the plan, its code objects in sorted manifest order,
and the checkpoint files in sidecar order. Normal lighthouse plan SHA-256:
`2a292dadc236f224005ed4e2d8ac2ba8b33dc71d79dabcc8497d177561f9c012`.
Control plan SHA-256:
`40701b9a81ef55c3dc92620df801d595cd1a10360ae047e14e76de37e279228e`.
The 70 MB of generated plans and the checkpoint weights are not stored here.

`oracle/lighthouse-oracle.py` documents the graph, forced-token and library
commands. `oracle/benchmark-qualified.py` preserves the exact native timing
instrument used for the five trials, including its source-manifest verification,
285-step/output checks, timeout, control and separate AMD/Spark comparisons.
`native/candidate.json` is the original card package metadata, not a landing
receipt. `native/runner.stdout` and per-request stdout retain the actual token
IDs and timings. `coding24/` retains the final executable's original fixture
check and its control. `numerical/` contains all three numerical diagnostics.

Run `python3 verify.py` to verify artifact hashes, raw native steps and IDs,
independent graph agreement, the coding fixture and the calculation output.

Measurement data is CC0-1.0. Source and documentation retain the repository
license, Apache-2.0 WITH LLVM-exception.
