# Evidence for "Omega running Qwen3.8-27B on an AMD Instinct MI300X" (brief of 13 November 2026)

Raw logs of the runs made on 13 November 2026 that are not in the public repository yet. The published run is in `CUDApp` at commit `dc536882c06a692991be97dc65cd2d037d82b129` under `evidence/amd-qwen38-native-refusal-20261113/`.

| Folder | What it holds |
|---|---|
| `amd-repeat-20261113T1907Z/` | The repeat on a fresh AMD One-Click MI300X notebook from a public clone: Level 1, the small example, both plans, one warmup and five timed native runs, the altered-weights control, and the same-session comparisons (my kernels through AMD's HIP runtime, PyTorch on ROCm with the same checkpoint, the notebook's stock pipeline call inside my timing script). Start with its `README.md`. |
| `level2-rebuild/` | The rebuild of the launcher and the 18 kernels from fresh public clones on arm64 Linux, with the gate log, the three output files and their hashes. |
| `spark-20261113T1913Z/` | The same request on my DGX Spark the same evening. |
| `tools/analyze.py` | Python 3, standard library only. `python3 tools/analyze.py amd-repeat-20261113T1907Z /path/to/CUDApp` recomputes every AMD number in the brief from the raw files and prints `ANALYZE PASS`. |

The top-level `SHA256SUMS` lists every file. Check with `sha256sum -c SHA256SUMS`. Each of the three evidence folders also has its own.

These are working logs. Some carry notes written for my own use: local folder paths, job-scheduling lines, and how the runs were driven. I work with AI coding agents, and they ran these measurements under my direction. The README in `level2-rebuild/` was written against an earlier draft of the brief; a note at its top says what changed. Measurement data is CC0-1.0.
