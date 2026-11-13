# gfx942 full-attention preparation: host receipt

IMPLEMENTED and EXERCISED on Linux AArch64. Hardware QUALIFIED, MERGED and DEPLOYED: NOT_RUN. Parent Main owns independent review and the MI300X bundle. No push, GPU, live service or installation operation occurred.

Intent: take packed projected Q/gate, projected K/V, model norm weights and token position; produce normalized/rotated Q, append normalized/rotated K and V to the BF16 cache in one GPU launch, and apply the reference sigmoid gate to the attention core's output in a separate tiny launch.

The pinned model and reference, exact operation order, layouts, launch dimensions and host RoPE producer interface are in PROVENANCE section17. Core attention remains section16. The previous core receipt remains immutable; this receipt supersedes its merge object because of the scheduling repair below. Part is unchanged. Standalone model RMSNorm, recurrent gated-delta and final norm/LM-head are separate assignments, not claimed complete here.

## Evidence

`commands.txt` records acceptance invocations and CPU-pool wrapper. Both final gate exits are0 (`final-exits.txt`); complete logs and their final15 lines are filed. All prior counts remain in the final executor line. Source/base identities are generated in `source-sha256.txt` and `base.txt`; `SHA256SUMS` covers this receipt. Both gates check the assigned seeded compiler digest before running a compiler. Program and object double-build identity is in the logs. All compiler diagnostics from the final builds are filed.

`objects/` holds prep, sigmoid gate, corrected merge, unchanged part, their raw instruction bytes, LLVM encodings/descriptor reports, numerical control objects, scheduling control objects, and the Linux-AArch64 seeded host RoPE executable. All new instruction bytes, metadata and descriptor fields match LLVM18; workgroup Y is enabled. No LLVM or Python component is a production runtime dependency.

`intervals/` holds deterministic compressed per-output bounds/widths for preparation and the corrected core. The independent Fraction/Decimal oracle checks all Q outputs and cache updates; history, overdispatch and invalid-capacity guards remain unchanged. The fixture's K conversion has no ambiguous BF16 result; V conversion is exact across ties, denormals, signed zeros, infinities and NaNs. Native RSQ/EXP/RCP are bounded representatives, not a promise of identical device approximation bits. Logs report widest output intervals. The host embedding producer is checked against independent high-precision trigonometry over the model range and full u32 endpoint.

## Scheduling repair

CDNA3 physical PDF pp28–29 explicitly requires software delay between a VALU transcendental and a consuming non-trans VALU. The original merge RCP and new gate EXP had immediate consumers; each now has `s_nop 0`. The part's EXP instructions and prep's RSQ instructions already have independent intervening instructions. S_NOP has manual-derived single-step tests and a flag-corruption control.

`scheduling.txt` records the static gate over LLVM-disassembled bytes from every emitted object, including all seven previously shipped arithmetic/GEMV objects. Those older objects have no Table12 transcendental instructions and are unchanged. Removing the added NOP from each affected object produces an actual mutant code object rejected by the checker. This gate covers this particular dependency rule only; it is not hardware timing simulation or certification of every possible hazard.

## Failed attempts and repairs

No Omega compiler errors occurred in this preparation slice. Earlier attention-core compiler errors remain documented in `../AMD-GFX942-ATTENTION/README.md` and its diagnostics.

* Initial LLVM parity failed for prep and gate because literal FP32 one used an extra DWORD while LLVM selected inline selector242. The tables now use the documented selector; full byte/branch parity passes. Gate allocation was reduced to its required rounded VGPR allocation, with complete descriptor parity.
* Initial prep execution refused64: its test caller supplied an obsolete extra kernarg-zero CLI argument. Removed that argument; the established executor CLI is text, memory, work-item grid X, group grid Y.
* The corrected call then refused66 because floating inline selectors were outside the executor subset. Implemented the manual Table23 FP32 selectors240..248, including its explicit reciprocal-two-pi bits; scalar/vector single-step cases and a restored-refusal control cover the extension.
* Adding S_NOP made an old unreachable-outside-subset control valid. That control now uses the still-unsupported S_CBRANCH_VCCZ, and both reachable and unreachable refusals are checked. The original failing log remains under `diagnostics/`; no refusal assertion was weakened.

Broken local model-config symlinks remain recorded in the previous receipt's `broken-config-symlinks.txt`. Their external owner/source issue was not changed by this slice.
