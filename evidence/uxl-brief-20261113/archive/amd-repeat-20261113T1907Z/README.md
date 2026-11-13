> Note added when this folder was packaged for the brief, 13 November 2026: the text below uses the folder names from my working tree. In this archive `../card-run/analyze.py` is `../tools/analyze.py`, `card-evidence` is this folder, `../card-evidence-results.json` is `analyze-results.json` here, and `../level2/` is `../level2-rebuild/`. `launch-x86` and `vadd.hsaco` were copied in from `../level2-rebuild/` so that `sha256sum -c SHA256SUMS` passes here. "AMD's stock notebook cell" means the notebook's stock pipeline call inside my timing script `uxl-stock.py`.

# MI300X repeat from the public instructions, 2026-11-13 (uxl-brief finishing test)

State: EXERCISED on hardware once, by the brief's own printed commands. Not landed in any repository. Measuring tools are Python and shell; they are not part of Omega.

## Where and when
- AMD One-Click notebook "Qwen/Qwen3.8-27B on AMD Instinct MI300X", Drake's AMD sign-in, session `jupyter-user-261113185025-46d1b9c5`, launched 2026-11-13T18:50:25Z. Jupyter Notebook 7, root, cwd `/workspace`.
- Card as reported there: `AMD Instinct MI300X VF`, model `0x74b5`, gfx942, KFD node 6, gpu_id 36258, VRAM total 205,822,885,888 bytes (`uxl-card.txt`). Linux 6.12.48+deb13-amd64, ROCm 7.2.2, torch 2.10.0+rocm7.2.2.git40d237bf, transformers 5.12.1, safetensors 0.7.0 (`uxl-env.log`). The host shows 64 DRM cards; other tenants' KFD nodes refuse reads. Exclusive use of the physical host is not established.
- Driven from the Mac through Drake's Chrome with the `computer` accessibility helpers: text pasted into a notebook cell, files brought back with the notebook server's `/files/...?download=1` URL. `uxl-evidence.tar.gz` sha256 on the card and on the Mac: `94940b02854c2c9c07a41fe27d3a7107940f284d3f099b6ba6533b42104a7ac7`.

## Inputs
- Public clone made on the card: `git clone https://www.aienos.com/rad/z4RxD8vXigmWg61nM8miQ8DVPAXEP.git CUDApp`, checkout `dc536882c06a692991be97dc65cd2d037d82b129` (`pre.log`, `uxl-cudapp-head.txt`).
- The only files not from that clone: `launch-x86` (sha256 `08a504911ec6e87a9746632a14d22245155597e3a8ed384c93c0e5260bb691e1`) and `vadd.hsaco` (sha256 `10eb12e019ff2117ddc0190650899d4254589375c62e5ebb970b1bbb7e67e55f`), pasted as xz+base64 with a sha256 check and re-hashed on the card. The same bytes come out of the Level 2 rebuild from public source (`../level2/`).
- Checkpoint: `Qwen/Qwen3.8-27B-FP8` revision `017b9c7af6b5689d5dd426a76e0bc077eb5ca20a`, downloaded on the card 18:54:25Z to 19:06:37Z, `CHECKPOINT SHA256 verified 68 files` (`pre.log`).

## What ran (scripts are in this folder: `pre.sh`, `uxl-env.sh`, `uxl-l3.sh`, `uxl-stock.sh`, `uxl-stock.py`, `uxl-fin.sh`)
1. Level 1: `python3 -I evidence/amd-qwen38-native-refusal-20261113/verify.py` printed `EVIDENCE PASS 65 hashed files; ...`, exit 0 (`pre.log`).
2. Small example: `./launch-x86 vadd vadd.hsaco vadd 10000000000` printed `OMEGA vadd=PASS count=256`, exit 0 (`uxl-env.log`).
3. Plans: emit-plan and check-plan exit 0 for both plans; `real2.plan` sha256 `2a292dad...9c012` and `real2-controls.plan` sha256 `40701b9a...9228e`, equal to the published values (`uxl-l3.log`, `uxl-plan-and-code.sha256`).
4. Native Omega runs: `PATH="$PWD:$PATH" sh real2.plan.cmd`, one warmup and five timed runs, each a new process, each exit 0 with 285 steps (`uxl-native-*.txt`).
5. Control: the controls plan (altered weights) once (`uxl-native-control.txt`).
6. Same-session references, same request (`oracle/lighthouse256.json`, 30 prompt ids, 256 generated):
   - `lighthouse-oracle.py ... graph`: the same Omega kernels loaded through AMD's HIP runtime with graph replay (`uxl-graph-fresh.json`).
   - `lighthouse-oracle.py ... reference`: PyTorch and transformers on ROCm with the same FP8 checkpoint, plain greedy loop with a key-value cache (`uxl-reference-fresh.json`).
   - `uxl-stock.py`: the notebook's stock configuration, `pipeline("image-text-to-text", model="Qwen/Qwen3.8-27B", device_map="cuda")`, BF16, lighthouse prompt text, 256 new tokens, one warmup and five timed requests, per-token time between StoppingCriteria calls with `torch.cuda.synchronize()` (`uxl-stock.json`).

## Results (MEASURED; printed by `../card-run/analyze.py card-evidence /tmp/uxl-repro/CUDApp`, which ended `ANALYZE PASS`; output in `../card-evidence-results.json`)
| Path, same card, same session | Decode ms per token (median) | Notes |
|---|---|---|
| Omega launcher and Omega kernels, direct to `/dev/kfd` | 26.721 (five run medians 26.743, 26.803, 26.706, 26.671, 26.721) | prefill median 779.599 ms; process wall about 26.9 s; all 256 ids equal the published AMD ids and the fresh HIP run; first difference from the Spark ids at index 109 |
| Same Omega kernels through AMD's HIP runtime, graph replay | 24.741 | ids equal the native runs |
| PyTorch + transformers on ROCm, same FP8 checkpoint | 131.011 | ids differ from the Spark ids from index 29 |
| Stock notebook pipeline, BF16 checkpoint | 70.628 (five run medians 70.498, 70.628, 68.604, 71.670, 71.829) | whole request median 18.537 s; load 44.5 s; peak allocated 54,928,256,000 bytes |
| Control plan (altered weights) | 26.670 | ids differ from index 0: caught |

Definitions: prefill is the sum of the first 30 step times; decode is the median of the remaining 255 step times; both as in `benchmark-qualified.py` lines 66 to 69 of the published receipt.

## Limits
- One session, one card, one request shape. Not a serving test.
- The stock pipeline row is not like for like: BF16 weights instead of FP8; its chat template produced 70 prompt tokens against 30; the pipeline ignored `do_sample` and `min_new_tokens` (its warnings are in `uxl-stock.log`), so decoding used the model's default generation settings, and the output starts in thinking mode; transformers reported "The fast path is not available ... Falling back to torch implementation" because flash-linear-attention and causal-conv1d are not installed in the image.
- The FP8 transformers row is a plain PyTorch loop, not a tuned serving engine. No vLLM or SGLang run was made on this card.
- Peak VRAM of the native runs was not sampled in this session (the published receipt has it).
- Launch exit codes are in `uxl-l3.log` (`runN rc=0`), not in the stdout captures.
