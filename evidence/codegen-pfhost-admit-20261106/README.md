# PFHOST admission migration — 2026-11-06

Base: `feat/omega-codegen-pfhost` at `1a56f317002e`.
IMPLEMENTED: `omega_rl_spec_ok`, `omega_rl_shared_bytes`, `omega_mc_spec_ok`
now call strong seeded `omega_pf_admit`. Old C exists only with default-OFF
`CUDAPP_MIGRATION_PENDING_C_CODEGEN_PFHOST_ADMIT=1`. Instruction generation
is unchanged. Public headers/API/manifest intentionally unchanged.

Files: `om/codegen/pf_admit.om`, unit registry and generated binding/header/ident,
`src/omega_blackwell_codegen_pfhost.c`, `mk/codegen-pfhost.mk`, three
`tests/codegen_pfhost_admit_*` drivers, ROADMAP note and this receipt.

## Commands and observed results

Every runtime command below ran with:

```
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10
```

* `make -s -j3 test-codegen-pfhost-admit`: rc 0,
  `PFHOST-ADMIT-DIFF PASS cases=656275`, then the intentional red returns 1
  at case 1 and `PFHOST-ADMIT-RED PASS`.
* `env OMEGA_OMC=~/work/cudastack/omega-mv/seed/out/omc OMEGA_BRIDGE=~/work/cudastack/omega-bridge make -s -j3 test-codegen-pfhost build/gpu_pfhost_test`:
  rc 0, `PASS pfhost exhaustive IR, allocation and encoded-byte differential rl=19150 mc=384 matching_refusals=50`;
  record-flip negative caught; final `PASS pfhost Omega source mutants killed`.
* `make -s -j3 test-codegen-pfhost-admit-mutants` with the same tool environment:
  rc 0, `PFHOST-ADMIT MUTANTS PASS (7 killed)`;
  `PFHOST-ADMIT STRONG-LINK PASS`.
* `make -s -j3 test-codegen-pfhost-admit-cubin`: rc 0,
  `PFHOST-ADMIT-CUBIN-IDENTITY PASS cases=11 matching_refusals=1`.
  Both accepted containers are compared byte-for-byte. The one matching
  allocator refusal produces no container, and is not counted as identity.
* `build/gpu_pfhost_test --host-only`: rc 0, final `GPU_PFHOST: PASS`.
* `build/gpu_pfhost_test --sim`: rc 0, final `GPU_PFHOST: PASS`.

The first new cubin driver incorrectly required every spec-admitted kernel to
survive register allocation. The existing family differential separately
records matching allocation refusals. The corrected driver requires equal
return codes, exactly eleven accepted cubins and one matching refusal.

The spec partition covers every accepted K/E/tile/mutant tuple and H/top-k
choice, plus source-derived transition values v-1/v/v+1, zero, UINT32_MAX,
and negative/out-of-range signed mutant values on each scalar axis.

| Omega mutation | Verdict |
| --- | --- |
| mutant upper bound 2 → 1 | KILLED |
| combine H ceiling 4096 → 2048 | KILLED |
| combine top-k floor 4 → 8 | KILLED |
| router K ceiling 16384 → 8192 | KILLED |
| router E ceiling 1024 → 128 | KILLED |
| tile divisor 32 → 16 | KILLED |
| shared multiplier 512 → 256 | KILLED |

Compilation/link failures are errors, not kills. Strong-link control renames
the only implementation and requires an unresolved `omega_pf_admit`.
The compiler/converter pins in `om/units.sh` are unchanged. Seeded omc is
`omega-mv e5b32567`, SHA-256
`2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.

## Tiers and remaining gates

Host/spec partition, family encoded identity, full-container identity and
simulator: PASS. Chip/live RM/VFIO/hardware: NOT_RUN. MERGED/DEPLOYED: no.
Full `make test` belongs to the integrator and was not run here.

## Chip runbook — NOT_RUN

Only after the operator grants an exclusive GPU window, run the existing
`gpu_pfhost_test` hardware mode with the current window procedure. Bind the
receipt to this source revision; compare router and combine output and all
existing negative selectors. A host result is not chip qualification. No
service, GPU lock, module or live device was touched for this work.
