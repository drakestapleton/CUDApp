# Omega running Qwen3.8-27B on an AMD Instinct MI300X

Technical brief for the UXL Open Source Working Group
Drake Stapleton, drake@aienos.com, 13 November 2026

Source for the AMD side: Omega's GPU layer at commit `dc536882c06a692991be97dc65cd2d037d82b129` and the Omega compiler repository `omega-mv` at commit `2ee0a70d1830581b706f288e722f3329d51f3583`. The GPU layer still lives in a repository named `CUDApp`, from when it only targeted NVIDIA. I plan to move it into the Omega repository. If the clone URLs in section 6 move, the commit hashes stay the reference. The Spark engine binary (sha256 `df9360112fdc815539f3e289fccfd8070845129ef4ccb466bb90c729ec71d0e5`) was built, according to my build records, from my Core repository https://www.aienos.com/rad/z44GqrPVAPjZkhoD2MWVjqy18ighN.git at commit `46134247`. The binary does not report its own version. The 1 November record is `docs/qwen38/qualification-20261101/README.md` there at `5c95594a`. Code is Apache-2.0 WITH LLVM-exception. Measurement data is CC0-1.0.

The 10:12 UTC run used the launcher built at `CUDApp` commit `ab58f171` with the compiler at `omega-mv` commit `64d6db3b`. The commits above are later and rebuild the same bytes (same sha256), which Level 2 checks. The front page of the `CUDApp` repository is still about NVIDIA. For the AMD work, start at `backend/amd/kfd/launch/README.md`.

## Summary

On 13 November 2026 a program written in Omega, my own language, ran the Qwen3.8-27B-FP8 model on an AMD Instinct MI300X with no AMD user-space runtime in its process: no HIP, no ROCr, no PyTorch. It talks to the Linux AMD compute interface (`/dev/kfd`) directly and runs 18 GPU kernels whose code objects my own Omega tooling wrote.

Median decode time was 26.7 ms per token. I measured it twice that day in two separate sessions (26.726 and 26.721 ms). The second time I followed only the public instructions in section 6. The same request on my DGX Spark takes 125.4 ms per token on a quiet machine (1 November). So the AMD run is 4.7 times faster per decode step (5.1 against a busier Spark the same evening).

That ratio is not a measure of the card. The Spark is a desktop machine and the MI300X is a data-centre card with 19 times its memory bandwidth. At one request at a time my AMD path uses under a fifth of that bandwidth, while my Spark engine uses about three quarters of its own (section 4). What the run shows is that a launcher and kernels written in one small language can drive an AMD card with nothing from the vendor above the kernel driver. Section 4 also puts my path beside other software on the same card in the same session.

Section 6 has the commands, at three levels of effort. Level 1 takes five minutes and needs no GPU. Level 2 needs no GPU either. The small example, a vector add on the card, is the first step of Level 3.

## 1. The machine

| | |
|---|---|
| Card | AMD Instinct MI300X, target gfx942. The machine reports it as "AMD Instinct MI300X VF", model 0x74b5, with 191.7 GiB of VRAM. The Linux KFD topology reports `gfx_target_version 90402`, vendor 4098, device 29877, 1216 SIMDs. |
| Whose machine | AMD's. It is the hosted One-Click notebook "Qwen/Qwen3.8-27B on AMD Instinct MI300X" at notebooks.amd.com/oneclick, opened from my AMD developer sign-in. Not a rented box and not a card I own. |
| Sessions | `jupyter-user-261113101013-76e03a28` (10:12 UTC, the published run) and `jupyter-user-261113185025-46d1b9c5` (18:50 UTC, the repeat and the same-session comparisons) |
| System | Linux 6.12.48+deb13-amd64, x86-64, glibc 2.39, ROCm 7.2.2. Root inside the notebook container, with `/dev/kfd` and the card's render node visible. |
| Not established | That I had the physical host to myself. The container's `/dev/dri` lists 64 cards. The KFD topology has ten nodes. My container can read the properties of one GPU node only, and my launcher uses that one and skips the rest (reading them returns "not permitted"). I did not try to open any other device. |

The comparison machine is my own NVIDIA DGX Spark (GB10).

## 2. What ran

- **`omega-launch`**, a static x86-64 Linux executable (sha256 `08a504911ec6e87a9746632a14d22245155597e3a8ed384c93c0e5260bb691e1`). The Omega compiler built it from the Omega source in `backend/amd/kfd/launch/`. It opens `/dev/kfd`, allocates and maps GPU memory, loads the kernels, writes its own AQL dispatch packets, rings the doorbell and waits for completion.
- **18 gfx942 kernels**, as AMD HSA code objects written by my Omega code-object writer, not by LLVM or the ROCm compiler: block-scaled FP8 matrix-vector, BF16 matrix-vector, attention (prepare, partial, merge, gate), the gated delta-net layers, RMSNorm, the SiLU MLP step, residual add, embedding and argmax.
- **A plan file** that lists the 1,126 kernel dispatches of one token step: which kernel, in what order, with which arguments and weight slices. A Python script records it on the CPU from the checkpoint. The launcher replays it once per token.

At run time the process contains only Omega code. The model step itself is laid out by that Python script, so the Omega part today is the launcher and the kernels, not the model description. Python, PyTorch and C are otherwise test and measurement tools outside the process: they check the plan on the CPU, and they replay the same step through AMD's HIP loader to compare token IDs against.

This is a command-line program. It is not an HTTP server, and I have not built the serving layer on AMD.

On the Spark the same request goes to a different program: my own engine (Rust and C, its own GB10 code path) running as an HTTP server. It is not the launcher above. The same engine binary served both Spark measurements.

## 3. The model

`Qwen/Qwen3.8-27B-FP8` from Hugging Face, revision `017b9c7af6b5689d5dd426a76e0bc077eb5ca20a`. Text path only, no vision tensors loaded. The run downloads about 30 GB and checks the sha256 of all 68 files against a pinned list before using them. The Spark runs used the same checkpoint and revision, also hash-checked (the 1 November record lists 81 files checked there).

## 4. The numbers

Same request on every Omega row and on the Spark: the prompt "Write a detailed essay about the history of lighthouses, from ancient times to today.", thinking off, greedy decoding, 30 prompt tokens in, 256 tokens out. The AMD runs were fed the exact prompt token IDs recorded from the Spark.

### Omega on the MI300X against my Spark

| | AMD MI300X, 13 Nov 10:12 UTC (published run) | AMD MI300X, 13 Nov 19:07 UTC (repeat) | DGX Spark, 13 Nov 19:16 UTC | DGX Spark, 1 Nov |
|---|---|---|---|---|
| Timed runs, plus 1 warmup left out | 5 | 5 | 5 | 5 |
| **Decode time per token, median** | **26.726 ms** | **26.721 ms** | **135.6 ms** | **125.4 ms** |
| The same as tokens per second (1000 / the row above) | 37.42 | 37.42 | 7.37 | 7.97 |
| Prefill, 30 tokens, median | 779 ms | 780 ms | 607 ms | 507 ms |

The run medians behind those rows, in ms: published AMD run 26.731, 26.642, 26.675, 26.726, 26.760. AMD repeat 26.743, 26.803, 26.706, 26.671, 26.721. Spark on 13 November 135.7, 135.5, 135.6, 135.2, 135.9.

Ratio of decode times, Spark over AMD: 4.69 against the Spark's quiet 1 November run, 5.07 with both sides measured the same evening. I quote the smaller one. On 13 November the Spark was shared with other CPU work: its one-minute load was 10.5 to 16.3 during the warmup and timed requests, where the public 1 November record holds one sample, 3.7. Two small GPU processes were also resident. It ran 8 percent slower than its own earlier number. The first 24-token requests in the same server log, about a minute earlier, ran at 125 to 127 ms per token, the 1 November speed, and the last two before the timed runs were already at 134 to 135. So I read the slowdown as contention and not a slower build. I have not tested that.

The Spark server's own end-to-end figures are a little lower than 1000 / graph time: 7.93 tokens per second on 1 November and 7.30 to 7.34 on 13 November.

AMD prefill is 1.3 to 1.5 times slower than the Spark. My AMD path feeds the prompt one token at a time.

What is not the same on the two sides:

- **Clock.** On AMD the launcher times each step by wall clock, from the first packet write to the completion signal. On the Spark it is the server's reported GPU graph time per token, which leaves out a little host time: its wall time per token was 136.8 to 137.5 ms on 13 November. So this choice of clock favours the Spark. Both leave out loading and text decoding.
- **Startup and whole-request time.** Each AMD run was a new process: ready after 19.2 s, first token at 19.95 s, whole 256-token run 26.9 s including the load. The Spark was a warm server: first visible text at 0.49 to 0.61 s, whole request 32.8 s on 1 November and 35.5 to 35.7 s on 13 November. I claim no ratio from these.
- **Memory.** AMD peak VRAM was 37,556,044 KiB (35.8 GiB) in the published run. The Spark has unified memory and its process used 38.55 GB after loading. Different accounting, no ratio.

How far each side is from its hardware, as an estimate: one decode step reads at most about 26.9 GB of weights (less the 2.5 GB input embedding table, which is read one row per token). At 26.721 ms that is at most about 1.0 TB/s on a card whose peak memory bandwidth is 5.3 TB/s (AMD's figure): 17 to 19 percent. At 125.4 ms it is at most about 215 GB/s on a machine whose bandwidth is 273 GB/s (NVIDIA's figure): 71 to 79 percent. My Spark engine is close to its limit. My AMD path is not.

### The same card, the same session, four ways

All four ran in session `jupyter-user-261113185025-46d1b9c5` between 19:07 and 19:18 UTC. The first three ran the same token IDs. The stock pipeline call ran the same prompt text through its own template and settings.

| Path | Decode time per token, median |
|---|---|
| Omega launcher and Omega kernels, direct to `/dev/kfd` | 26.721 ms |
| The same Omega kernels, loaded and replayed by AMD's HIP runtime | 24.741 ms |
| PyTorch 2.10.0 on ROCm 7.2.2 with transformers 5.12.1, the same FP8 checkpoint, plain greedy loop | 131.011 ms |
| The notebook's stock pipeline call inside my timing script (`uxl-stock.py`), the unquantised BF16 checkpoint | 70.628 ms |

How to read it:

- My own launch path is 2.0 ms per token slower than AMD's runtime driving the same kernels. The HIP figure is synchronized wall time for one replayed step and leaves out the host reading the token back, so part of the 2.0 ms may be the clock and not the launch path. Most of the remaining time is in the kernels, which are far from the card's limit, as the estimate above shows.
- Neither PyTorch row is a speed baseline. For the record, they were 4.9 and 2.6 times slower per token than my path. I did not run vLLM or SGLang on the card, so this says nothing about the best AMD can do.
- The PyTorch row on the FP8 checkpoint is a correctness reference. It ran eagerly, transformers reported that its fast path was not available, and its FP8 kernels came from a community kernel package on the Hugging Face Hub, an older version, which fell back from DeepGEMM to Triton.
- The stock row is not like for like: BF16 weights, a chat template that makes 70 prompt tokens instead of 30, thinking on, and the model's default generation settings. transformers printed the same missing fast path there.

## 5. How I know the output is right

- **Short coding prompt.** All 24 generated token IDs equal the 24 IDs of my own CPU reference forward (Rust, run on the Spark's CPU on 18 October), which the Spark engine also matches. A copy with deliberately wrong weights produces different IDs and is caught.
- **Lighthouse request.** All 256 token IDs are identical across the published run, the repeat and a fresh replay of the same step through AMD's HIP loader in the repeat session. That replay checks my launcher, not the model arithmetic, because it runs the same kernels in the same order. A plan with altered weights diverges at the first token, in both sessions.
- **The Spark against itself.** The 13 November Spark run returned the same 256 IDs as the 1 November run.
- **AMD against the Spark.** The first 109 generated tokens are identical. At index 109 (counting from zero) the two machines pick different tokens, where the top two candidates on the AMD side are 0.19 logit apart. Feeding the Spark's tokens through the AMD kernels finds one more such position, index 138, with a 0.01 margin. PyTorch on the same checkpoint leaves the Spark's tokens earlier, at index 29, which is also a near tie: its top two there are 0.125 apart, and my AMD path agrees with the Spark at that position by only 0.106. I read all of that as rounding differences between implementations. I have not proven it, so I claim neither bit-identical output across machines nor equal model quality.

## 6. Repeat it

Every level below has been run as printed. Level 1 ran on a Mac, on arm64 Linux and on the AMD machine. Level 2 ran from fresh public clones on arm64 Linux. Level 3 is the 19:07 UTC repeat in section 4, done from a public clone on a fresh AMD notebook.

I work with AI coding agents. They write much of the code and tooling, and they ran these measurements under my direction. The commands and logs in the archive are what ran. One long trace log is trimmed, with a note beside it.

### Level 1: check my numbers (any machine with git and Python 3, five minutes)

```sh
git clone https://www.aienos.com/rad/z4RxD8vXigmWg61nM8miQ8DVPAXEP.git CUDApp
cd CUDApp
git checkout dc536882c06a692991be97dc65cd2d037d82b129
python3 -I evidence/amd-qwen38-native-refusal-20261113/verify.py
```

Expected output:

```
EVIDENCE PASS 65 hashed files; raw steps/IDs; independent graph; reference coding24; calculations
```

It checks the published files against their hashes and recomputes the medians and the ratio from the raw per-step timings of the published run. The raw output of each run is in `evidence/amd-qwen38-native-refusal-20261113/native/sample1.stdout` to `sample5.stdout`. The write-up beside it is `README.md` in the same folder, also at
https://interplane.io/drakestapleton/CUDApp/blob/dc536882c06a692991be97dc65cd2d037d82b129/evidence/amd-qwen38-native-refusal-20261113/README.md

### Level 2: rebuild the launcher and kernels from source (Linux on arm64, no GPU)

```sh
cd ..    # back to the folder that holds CUDApp, which stays at dc536882
git clone https://www.aienos.com/rad/z6do9WDBhuSnvopDkgFdFr1yWA62.git omega-mv
git -C omega-mv checkout 2ee0a70d1830581b706f288e722f3329d51f3583
sha256sum omega-mv/seed/out/omc
#   26540e6de62402d5784b30b24d7c6faae488137c1f8a4b3e784fd0b5d5528991

cd CUDApp
python3 -I tests/amd-kfd-launch/check.py "$(realpath ../omega-mv/seed/out/omc)"
#   second-to-last line: AMD-KFD-LAUNCH CPU PASS {...}
#   last line: NOT_RUN every AMD device row ... (expected: this level has no GPU)
sha256sum build/amd-kfd-launch/gate/launch-x86
#   08a504911ec6e87a9746632a14d22245155597e3a8ed384c93c0e5260bb691e1
```

The last hash is the executable that ran on the card. The same check rebuilds all 18 kernels and fails unless each is byte-identical to the one the card ran. It runs 274 checks in under a minute. The run is good only if `check.py` exits 0 and prints the `AMD-KFD-LAUNCH CPU PASS` line. The hash line alone is not enough.

It needs `python3`, `cc` with the C library and Linux kernel headers (it includes `linux/kfd_ioctl.h`), and `qemu-x86_64`, which it uses to run the x86-64 launcher on the arm64 host. On Ubuntu 24.04 that is `sudo apt-get install gcc libc6-dev linux-libc-dev qemu-user`. The check itself needs no root and no GPU. The one setup I tested is Ubuntu 24.04 on arm64 with 4 KiB pages, Python 3.12.3, gcc 13.3.0, Linux headers 6.8.0 and qemu-user 8.2.2. Other setups are untested.

The Omega compiler runs on arm64 Linux only today, which is why this level needs that host. The launcher it produces is cross-compiled for x86-64. To rebuild the compiler from its hand-written seed instead of trusting the checked-in binary, run `sh seed/omega-from-seed.sh` in `omega-mv`. It takes about a second, ends with `RC=0`, and leaves `seed/out/omc` with the same hash.

### Level 3: run it on an MI300X

You need x86-64 Linux with an MI300X, root, `/dev/kfd` (KFD 1.14 or later), about 30 GB of disk for the weights, about 36 GiB of free VRAM, git, and Python with PyTorch and safetensors for the plan script. The comparison commands at the end also need ROCm's HIP library (`libamdhip64`). AMD's One-Click MI300X notebook has all of that. My sessions there ended after 30 to 40 minutes and the download took 12 of them, so start it first.

The small example first. It adds two vectors on the card and checks every output bit. Copy `launch-x86` and `vadd.hsaco` from the Level 2 output folder (`build/amd-kfd-launch/gate/`). They are the only two files that do not come from the public clone. If you have no arm64 Linux machine, the same two files are in the evidence archive under `level2-rebuild/`. Check them against these hashes: `launch-x86` is the sha256 printed in Level 2, and `vadd.hsaco` is `10eb12e019ff2117ddc0190650899d4254589375c62e5ebb970b1bbb7e67e55f`.

```sh
chmod +x launch-x86

./launch-x86 vadd vadd.hsaco vadd 10000000000
#   OMEGA vadd=PASS count=256
```

Then the model, from the `CUDApp` folder at `dc536882`:

```sh
cd evidence/amd-qwen38-native-20261113
export AMDSERVE_WEIGHTS=/tmp/qwen3.8-27b-fp8

# download the checkpoint and check all 68 file hashes
python3 -c "import runpy; runpy.run_path('oracle/driver-real2.py', run_name='prepare-import')['prepare']()"

# write the plan and the kernels, then check the plan against the checkpoint on the CPU
mkdir out    # in a fresh clone; remove an old out folder first
python3 oracle/emit-plan.py oracle/driver-real2.py "$AMDSERVE_WEIGHTS" out/real2.plan \
    --capacity=131072 --fixture-json=oracle/lighthouse256.json
python3 oracle/check-plan.py out/real2.plan "$AMDSERVE_WEIGHTS"
sha256sum out/real2.plan
#   2a292dadc236f224005ed4e2d8ac2ba8b33dc71d79dabcc8497d177561f9c012

# run it (the plan's command file expects the launcher to be called omega-launch)
cp /path/to/launch-x86 out/omega-launch
cd out && PATH="$PWD:$PATH" sh real2.plan.cmd > run1.txt
```

`run1.txt` holds one `OMEGA_STEP i tok=T us=U` line per step and ends with `OMEGA_PLAN ids=[...]`. The first 30 steps are the prompt. The median `us` over the 255 steps after that is the decode number in section 4, and each run takes about 27 seconds. The 256 IDs should equal the `ids` list in `evidence/amd-qwen38-native-refusal-20261113/numerical/graph.json`.

At this level the 18 kernels are written out by `emit-plan.py` from bytes carried inside `oracle/driver-real2.py`. Level 2 is what ties those bytes to the Omega source: its check rebuilds each kernel and compares the hash.

Two of the same-session comparisons from section 4 are one command each, run one level up from `out`. The PyTorch one also needs transformers and the `kernels` package, and it fetches an FP8 kernel package from the Hugging Face Hub at run time. The script for the stock notebook row is `amd-repeat-20261113T1907Z/uxl-stock.py` in the evidence archive. It expects the clone at `/workspace/uxl/CUDApp`. Edit its lines 4 and 8 otherwise.

```sh
cd ..
python3 oracle/lighthouse-oracle.py oracle/driver-real2.py "$AMDSERVE_WEIGHTS" \
    oracle/lighthouse256.json graph out/graph.json --capacity=131072       # my kernels through AMD's HIP runtime
python3 oracle/lighthouse-oracle.py oracle/driver-real2.py "$AMDSERVE_WEIGHTS" \
    oracle/lighthouse256.json reference out/reference.json --capacity=131072   # PyTorch on ROCm, same checkpoint
```

Each prints a line with `decode_ms_median`.

## 7. What this does not show

- **Serving.** It is a command-line run of one request at a time on one card. No HTTP server, no batching, no multi-GPU.
- **A comparison with a tuned serving engine on the card.** The two PyTorch rows in section 4 use the PyTorch and transformers the notebook ships. The FP8 row adds a community kernel package from the Hugging Face Hub, in fallback mode. Neither is vLLM or SGLang. For scale: AMD's hosted Token Lab API (SGLang, hardware not shown) returned this model at 37.8 tokens per second on 12 November. My log of that is not in the archive.
- **A quiet Spark on the same day.** The same-day Spark run shared the machine with other work. The quiet number is from 1 November.
- **A repeatable Spark side.** Repeating the Spark numbers needs a GB10 and my engine build. This brief does not give steps for that.
- **Long prompts.** The test prompt is 30 tokens. Prefill is slower than the Spark's. I have not measured decode speed deep into the context.
- **Intel hardware.** Nothing in this brief ran on an Intel GPU.
- **Other AMD cards.** Only gfx942 has run.
- **Safety against untrusted input.** The launcher runs my own kernels and plans. It is not a sandbox.
- **Independence from the kernel driver.** Everything in user space is mine. The path still uses AMD's Linux kernel driver (amdgpu and KFD) and the card's firmware.
- **Public logs for the 19:07 UTC repeat.** The published run is in the repository and Level 1 checks it. The raw logs of the repeat, the same-session comparisons, the Level 2 rebuild and the 13 November Spark run are not in the repository yet. They are in the archive sent with this brief, `uxl-brief-evidence-20261113.tar.gz`. Its top-level `SHA256SUMS` lists every file.

## 8. What would help

- Someone in the working group running Level 1 or Level 2 and telling me what breaks.
- Advice on what a fair comparison against a tuned ROCm serving stack on this card should look like, before I publish one.
