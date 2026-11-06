# API artifact, decode scalars and matmul oracle residual migration

IMPLEMENTED and EXERCISED on the host. Chip qualification NOT_RUN. Not merged or deployed.

Base: `feat/api-views-attn-om`, `c1fec738fb3dfe4fd48bf9115a19ca9d4d3d6367` (#332).
Unit registration is the first commit (`3683822d`). Public function signatures, artifact admission/trust policy, and inherited `av_ropegen` are unchanged.

## Source and dependency pins

First-party computation: `om/api/attn_{art,scalar,oracle}.om` with one exported wrapper root per generated unit. Generated bridge C is committed, not a second hand-written implementation.

- Seeded compiler: omega-mv `e5b32567`, `omc` SHA-256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
- Hosted converter: omega-bridge `79e8c3613a6f6f9e8eaaa9c013fc25bcb39e0538`, inherited `toc.om` SHA-256 `d84fd6ebd258170a023ed5235880cd3e1ca8664703a2542542a7f8d103120538` (`/tmp/bridge-73-apiviews`). This is the existing #332 dependency, not an added bridge pin. Native generation uses `OMEGA_SEED=~/work/cudastack/omega-bridge/build/bootstrap/omc`.
- C compiler/linker, Python runner, objcopy and libm are host verification infrastructure. FILE writes and float bit copies remain host effects/byte transport. The seeded units replace the computation; no new production language/runtime dependency is adopted.

C artifact computations remain only behind default-OFF `CUDAPP_MIGRATION_PENDING_C_NATIVE_ARTIFACT_REST`. C double matmul oracle remains only behind default-OFF `CUDAPP_MIGRATION_PENDING_C_MATMUL_ORACLE_GUARD`. The decode path has no C computation fallback. All Omega references are strong.

The bridge does not accept f64 arrays, so the double accumulator is transported as `ulong` bits with typed f64 arithmetic inside Omega. Accumulation order, finite/nonfinite comparisons, max-error float conversion and mismatch threshold are unchanged.

## Commands actually run

All tests below ran with the device-hiding prefix:

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10
```

| Command after prefix | rc | Observed final result |
| --- | --- | --- |
| `env OMEGA_OMC=~/work/cudastack/omega-mv/seed/out/omc OMEGA_BRIDGE=/tmp/bridge-73-apiviews python3 tests/api_rest_check.py --mutants --out build/api-rest-check` | 0 | `API-REST-CHECK PASS` |
| `make -j3 test-native-artifact-manifest test-native-artifact test-gpu-matmul-api test-gpu-decode-attn test-gpu-decode-attn256` | 0 | `gpu_decode_attn_test: 12 checks, 0 failed: PASS` |

`commands.json` records the final differential run, individual commands, return codes and output; expected failing links/mutants are negative controls, not unexplained failures. `family-gates.log` records the focused family invocation: manifest 70 checks; artifact export 47 checks; artifact shape exports PASS; matmul API 178 checks and r4 receipt 7 checks; decode host 43 checks and simulator 12 checks; decode256 host 61 checks and simulator 14 checks; max-position 5 cases and 7 killed controls. All reported zero failures. The matmul API gate provides a host-only tier, not a simulator mode; this receipt does not claim a nonexistent matmul API simulator tier.

Paths in committed logs are normalized from `/home/drakestapleton` to `~`. No binary, cubin, PTX or disassembly is committed.

## Differential partitions and red controls

- Decode: every sign and exponent class, every high mantissa payload chunk, and low-payload values `0,1,0x7ffe,0x7fff,0x8000,0x8001,0xfffe,0xffff`. The actual `omega_dattn_bf16_rne` entry and its Omega root are both checked against the original raw unsigned C rounding expression. This deliberately preserves NaN/inf carry/wrap behavior, rather than replacing it with canonical NaN handling. Permutation covers every slot for every valid even rotary dimension 2..256, scalar 0/UINT32_MAX and boundary invalid-domain transport values, and f32/BF16 vector bit preservation. Inherited RoPE generation is untouched.
- Artifact: all byte values for JSON escaping; strict semver parse/comparison boundaries and UINT32 limits; ordered canonical manifest, every truncation and each-byte NUL/CR/LF/equals changes; synthetic ELF every truncation and each-byte mutations, name-length boundary 120..129, little-endian widths 2/4/8. Altered expected text is rejected. No trust/gate branch has been added or weakened.
- Matmul: original C oracle and Omega oracle, BF16 zero/sign/subnormal/normal/finite-extreme/infinity/NaN classes, K=0..32 and multi-row/column matrices; exact output bits for max absolute/relative errors and exact mismatch/parity flags. Correct 1x1 result passes; wrong output is rejected.
- Missing `omega_ds_round`, `omega_na_cubin` or `omega_mm_dot` causes link failure. No weak fallback exists.

## Seeded source mutants

Every mutant is made from the .om source, recompiled through the pinned seeded compiler/converter, linked instead of just that generated root, and rejected by the same differential. `mutants.json` records exact substitutions and return codes.

| Unit | Mutant | Verdict |
| --- | --- | --- |
| ds_round | invert finite/nonfinite path | KILLED |
| ds_slot | pair stride divide by 3 instead of 2 | KILLED |
| ds_vecf | force rotary dimension 256 | KILLED |
| na_cubin | accept mismatched text | KILLED |
| na_layout | wrong manifest capacity | KILLED |
| na_ver | reverse version ordering | KILLED |
| na_json | omit DEL escaping | KILLED |
| mm_dot | sum instead of product | KILLED |
| mm_err | loosen relative error threshold | KILLED |

## Constants provenance

`WolframLanguageEvaluator: {FromDigits["464c457f",16],FromDigits["7fff",16],ToCharacterCode[".text."],ToCharacterCode["abcdef"]} -> {1179403647,32767,{46,116,101,120,116,46},{97,98,99,100,101,102}}`.
These are encodings of existing source constants, not performance measurements.

## Tier verdicts and chip runbook

Host differential: PASS. Strong-link/content red controls: PASS. Nine .om mutants: KILLED. Existing family host gates: PASS. Decode and decode256 simulator gates: PASS. Chip/live-RM/VFIO/HW: NOT_RUN because no GPU window is authorized. Full repository integration gate belongs to the integrator and was not run here.

For a future explicitly authorized exclusive window, the operator must follow the current repository hardware procedure and verify the current device/service ownership before any chip call. Rebuild at the candidate revision, rerun the host gates above, then run the documented chip modes of `build/gpu_decode_attn_test`, `build/gpu_decode_attn256_test` and `build/gpu_matmul_api_test`, capturing chip parity/negative-control receipts in a new evidence directory. Artifact export remains host-only. Do not treat this runbook as authorization, stop a live service, or reuse a historical lock/window. No hardware result is asserted by this PR.
