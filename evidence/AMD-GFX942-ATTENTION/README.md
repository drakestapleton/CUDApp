# gfx942 27B attention core: host receipt

IMPLEMENTED and EXERCISED on Linux AArch64. Hardware QUALIFIED, MERGED and DEPLOYED: NOT_RUN. No push, live-service operation or GPU use occurred. Parent Main owns independent review and the MI300X bundle.

Intent: implement the full-attention core for Omega serving the27B model on MI300X, with enough independently scheduled work to avoid the24-wave serial-head design. This receipt does not claim a hardware speedup or end-to-end serving.

The source base is generated in `base.txt`; changed-source digests are in `source-sha256.txt`. `SHA256SUMS` covers this receipt and its filed artifacts. The compiler pin is the assigned Omega e5b32567 binary; both acceptance gates verify its digest before compilation. Exact commands are in `commands.txt`; observed exit codes in `final-exits.txt`. Complete logs, last15 lines, controls and compiler logs are filed beside this receipt. No earlier receipt is modified.

## Result and interface

Both acceptance gates returned0. Their final lines enumerate all old/new kernels and case counts; the executor retains the old counts/shapes and now includes the attention shapes and manual instruction semantics. The two builds of each program/object are byte-identical. LLVM18.1.3 checks all new instruction bytes, complete metadata notes, descriptor sizes/resources (excluding only the resolved relocation field), and workgroup-id-Y enablement.

Objects and numerical controls are under `objects/`. The shared48-byte ABI, launch dimensions, scratch layout and exact instruction order are in PROVENANCE section16. T means token count, not last-token position. Part uses ceil(T/64) by24 workgroups; merge uses1 by24; each group has64 lanes. Q is already normalized/rotated and cache append is already complete. Output is before the model's output gate. The score scale is exactly1/16.

The oracle checks every partial/output with a propagated per-instruction enclosure of the manual's EXP/RCP1ULP guarantee, rather than a fixed output tolerance. Each output's bounds and width are filed in deterministic gzip JSON under `intervals/`; the execution log prints the widest interval for each case. The gate catches omitted max subtraction, wrong GQA grouping, and reading V from K. Its single-step controls separately catch changed LSHR, SUB, MAX, EXP and RCP semantics. The representative native-binary64 transcendental implementation is not claimed to reproduce the device's answer bits.

## Decode and refusal design (five lines)

1. ELF extraction uses section headers to locate raw .text; only instruction words reach the executor.
2. Fixed wave64 SGPR/VGPR state and flat byte memory execute the subset; s2/s3 carry workgroup X/Y.
3. Ordinary FP32 arithmetic is integer RN-even with denormals; EXP/RCP use independently bounded Omega binary64 representatives with manual FTZ.
4. Unknown encodings/modifiers refuse66, malformed text65, memory bounds67, I/O74 and CLI64; wrong compiler digest69 precedes execution.
5. No kernel-name shortcut, LLVM implementation dependency, guessed instruction, timing simulation, or hardware claim is present.

## Failed attempts and repairs

* Omega `omc: line 151: error 8`: the overlong decimal ln2 literal overflowed the pinned compiler's literal parser. Replaced it with the binary64-precision decimal `0.6931471805599453f64`; the subsequent compiler log is empty and the independent Decimal probe passed. No other Omega compiler error occurred in this attention slice.
* LLVM oracle: `accum_offset exceeds total VGPR allocation` at `.end_amdhsa_kernel`. Declared48 next-free VGPRs to match the already rounded48-register allocation and accumulator offset; no kernel algorithm or test was weakened.
* First full host gate found two descriptor-resource mismatches: the guessed SGPR allocation word0x00af00c5 disagreed with LLVM's0x00af0105. Corrected both tables and recorded35 SGPRs including reserved registers; hardware allocation rounds up. Full descriptor parity then passed.
* First full executor gate failed at T65 because the test caller passed a workgroup count where the established executor CLI expects work-item grid X. The kernel text was correct; corrected the caller to64 times the group count and retained overdispatch in both dimensions. T65/129 then passed. The original failed gate logs are retained under `diagnostics/`.

## Model provenance and hand-maintained artifact finding

The pinned public config was recovered from Qwen3.8-27B-FP8 revision017b9c7af6b5689d5dd426a76e0bc077eb5ca20a. Its51350 bytes and sha25674227dd615bf1ea975aa676bdf355a0379858c12f394b5365cd9dfa5fc2c70bc match `/home/drakestapleton/handoffs/Q27/evidence/manifest-27b.sha256sumfmt`. The exact bytes are filed in `tests/amd-exec/model/`.

Eight broken local config symlinks are recorded verbatim in `broken-config-symlinks.txt`. Their missing target is `/home/drakestapleton/models/qwen3.8-27b-fp8-017b9c7a/config.json`. Source of truth: the immutable model revision and historical digest. Who maintains those old installer symlinks today: not established. Staleness frequency: one observation of eight broken links in this slice, not a measured history. No other lane's installer tree or links were changed.

The Q/K norm, RoPE, cache append, output gate, standalone RMSNorm, recurrent gated-delta step, and final norm/LM-head slices remain separate assignments. They are not represented as complete by this core receipt.
