# gfx942 E4M3FNUZ GEMV host receipt

IMPLEMENTED and EXERCISED on Linux AArch64. Hardware QUALIFIED, MERGED and DEPLOYED: NOT_RUN.
The local commit containing this receipt is the candidate; base.commit and inputs.sha256 identify its source inputs. Independent review and integration belong to Main. Nothing was pushed or installed.

## Commands and observed exits

Working directory: `/home/drakestapleton/work/hive/amdserve/Gfx942Exec/fp8gemv`.
Branch: `amdserve/gfx942-fp8gemv`.

Successful execution gate (exit0; exec.log):

```sh
flock /home/drakestapleton/handoffs/hive/queen/pool-14.lock nice -n 10 sh -c 'AMD_EXEC_OUT=/home/drakestapleton/work/hive/amdserve/Gfx942Exec/fp8-exec-out-3 sh tests/amd-exec/run.sh /home/drakestapleton/work/hive/amdserve/AmdGfx942/pin/omc > ../fp8-exec-3.log 2>&1'
```

Successful host encoding/descriptor gate (exit0; host.log):

```sh
flock /home/drakestapleton/handoffs/hive/queen/pool-14.lock nice -n 10 sh -c 'AMD_OUT=/home/drakestapleton/work/hive/amdserve/Gfx942Exec/fp8-host-out-4 sh tests/amd/run.sh /home/drakestapleton/work/hive/amdserve/AmdGfx942/pin/omc > ../fp8-host-4.log 2>&1; rc=$?; echo host_exit=$rc; cat ../fp8-host-4.log; exit "$rc"'
```

The execution command above ran inside the combined attempt3 wrapper: host gate first with AMD_OUT ending `fp8-host-out-3`, then execution gate; both exits and logs were printed. That wrapper exited1 because the host assembly metadata failed, while execution independently exited0. The only later source change was the oracle assembly YAML string annotation, which the host gate then exercised; production binaries and execution tests did not change and were not rerun.

The gate itself performs two compilations of both producer and executor and compares their bytes. Every kernel is emitted twice and compared. Existing vadd/fsum16/corrected-SGEMM whole-object hashes are checked against their previous values. Artifacts remain in the absolute output directories above, with hashes in artifacts.sha256. All256 conversion reference rows and llvm-mc encodings are copied here.

## Failed attempts and compiler errors

All failures are preserved; no gate was weakened.

- Attempt1: host exit1, exec exit3. Pinned omc `hsaco.om:504 error3` (unknown name): one `get forms f` reference remained after replacing the fixed arrays. Removed that obsolete duplicate check; `inst` already checks the new registry's declared-word count.
- Attempt2: host exit1, exec exit11. Pinned omc `hsaco.om:228 error11` (ambiguous bare number): the signed-file-offset bound expression needed an explicit `ulong` on its constant. Added the type; the bound is the real signed64 file-offset boundary, not a format-count ceiling.
- Attempt3: host exit1, exec exit0. LLVM18 rejected FP8 oracle assembly metadata at `.end_amdgpu_metadata`: unquoted YAML `y` parsed as a boolean. Annotated `.name: !str y`, like the existing SGEMM `N` convention. Attempt4 host exit0.

Those were all compiler/assembler errors encountered in this FP8 slice. Error explanations came from omega explain_error (newer tool pin); all actual compilation used the assigned e5b32567 compiler digest in authorities.sha256. Failed source-stage compile logs are retained under compiler/; attempt logs identify their outcomes.

## Decode/refusal design

1. Decode raw instruction words extracted through the emitted ELF section headers; validate all code before opening execution memory.
2. Extend only SDWA CVT_F32_FP8 BYTE0, unsigned VOP3 MUL_LO/HI, and GLOBAL_LOAD_UBYTE; unsupported encodings refuse66.
3. Construct exact E4M3FNUZ-to-FP32 bits with integer operations; all256 bytes checked against an independent Fraction format oracle; canonical host NaN is0x7fc00000.
4. Accumulate ascending-k exact FMA, then multiply the row scale once, with nearest-even rounding and denormals retained; EXEC and flags are checked by manual-derived single-step cases.
5. Keep malformed-code65, memory-range67, I/O74 and preflight pin69 refusals; grow the producer's format registry by declarations rather than raising a capacity cap.

## Hardware boundary

Hardware NOT_RUN. AMD Instinct MI300 CDNA3 ISA, physical PDF p60: “The register SH_MEM_CONFIG, bit[8] must be set to 1 to produce the correct results for BF8 and FP8 operations.” With that bit set this kernel assumes E4M3FNUZ bytes, bias8, unsigned zero0x00, NaN0x80. No register read/write or device dispatch was attempted. Notebook execution must establish this precondition through its authorized hardware path. NaN payload propagation and timing are not hardware-qualified.

Prior separate fixes retained in this branch: SCC/manual semantics correction `ada44155fd13b96fadc79648c4c34ba7afc79713`; KFD refusal-note/gate corrections `f20b880607781a4070a12a07b30720deead796f1`. Their original receipts remain intact.
