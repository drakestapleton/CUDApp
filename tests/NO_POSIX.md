<!-- SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception -->
# NO_POSIX inventory and native cutover

Inventory anchor: CUDApp `eceb8c542fa6` (PR #330, this lane's specified base).
Line numbers below refer to that revision, not to moving main. The supplied
41-site origin/main inventory and this stacked base are not identical inventories.
Comments and build definitions are included so every textual occurrence is classified.

Classes: **guard** means an include, unavailable hosted operation or its syscall
stub; **shell** means ABI, callback selection, memory ownership or build glue;
**compute** means computation previously selected in C specifically by NO_POSIX.
This is an inventory of macro sites, not a claim that every C function in the
surrounding files has migrated. Other lanes own those surrounding functions.

| Base file | Base lines | Class | Disposition |
|---|---|---|---|
| `src/omega_blackwell_codegen.c` | 7, 11 | guard | Hosted includes and diagnostic stub. No syscall in freestanding. |
| same | 274, 280, 284 | compute | C encoder exception removed. `bw_enc_sm121.om`, composed with `om2`, called through `tools/native --freestanding`. C remains default-OFF oracle. |
| same | 1257 | guard | Hosted dump uses file IO; freestanding dump is inert. |
| same | 1550, 1843, 2036, 2206, 2396 | shell | Allocating convenience entry points excluded. Allocation-free program/encode interfaces remain; unrelated generator migrations belong to their lanes. |
| same | 2770 | guard | Printing hosted self-test excluded. |
| `src/omega_blackwell_codegen.h` | 251 | shell | Documents caller-owned code-buffer ABI. |
| `src/omega_blackwell_codegen_bf16.c` | 9 | guard | Hosted allocator include. |
| same | 304 | shell | Allocating convenience entry excluded. |
| `src/omega_blackwell_codegen_gemm.c` | 7 | guard | Hosted allocator include. |
| `src/omega_blackwell_codegen_moe.c` | 12 | guard | Hosted allocator include. |
| same | 490 | shell | Allocating convenience entry excluded. |
| `src/omega_blackwell_codegen_moe2.c` | 329 | shell | Allocating convenience entry and allocator include excluded. |
| `src/omega_blackwell_matmul.c` | 4 | guard | Hosted allocator include. |
| same | 8 | shell | Import declarations only. |
| same | 98 | compute | Already calls seeded `native_f16` through the checked import-free scalar binding; retained, not duplicated. |
| same | 115 | compute | C integer bf16 fallback replaced by native seeded `no_bf16.om`; old integer oracle only selected by default-OFF `CUDAPP_MIGRATION_PENDING_C_NO_POSIX`. |
| same | 155, 193 | shell | Hosted allocation/free conveniences excluded. |
| `src/omega_cubin.c` | 10, 42, 46 | compute | C ELF-packing exception removed. Native `cubin_elf.om` now handles freestanding selection; C remains default-OFF oracle. Allocation, pointers and diagnostic formatting remain shell operations. |
| `src/omega_gpu_engine.c` | 25 | guard | No errno or POSIX clock imports. |
| same | 155 | shell | Invoke installed clock callback, or zero; no platform clock invented. |
| same | 352 | shell | Require clock capability before device work; existing ABI refusal, not numeric computation. |
| `src/omega_gpu_engine.h` | 243 | shell | Clock callback contract documentation. |
| `src/omega_gpu_session.c` | 8, 24 | guard | No Linux/pthread dependency. |
| same | 96, 108 | shell | No hosted platform default installation. Explicit platform required. |
| same | 376 | shell | No Linux platform identity query for a custom platform. |
| same | 587 | guard | Excludes hosted Ampere launch path; freestanding keeps its existing refusal. Command-list work remains owned by SessionCmdlistOm. |
| `src/omega_gpu_session.h` | 49 | shell | Explicit-platform ABI documentation. |
| `src/omega_gpu_wait.c` | 6 | guard | No POSIX clock; platform callback required. |
| `src/omega_gpu_wait.h` | 81 | shell | Callback ABI documentation. |
| `mk/omega-gpu-engine.mk` | 16, 119, 125, 154, 181, 210, 225, 297 | shell | Build definitions, comments and host-test link rules. Now links native caller-stack objects. |

## Build and custody

`om/units.sh --native-no-posix` dispatches to `om/no-posix.sh`, the native-unit
registry. It composes the existing encoder and cubin roots plus
`om/numeric/no_bf16.om` by prefixing exactly `om2\n`.
No handwritten C implementation or converter is introduced. Native generated
machine-code bindings and custody inputs are build products under
`build/no_posix/native`, not a second editable implementation. The existing
portable generated units remain committed and checked by `om/units.sh --check`.

Use the stage-5 combined omega-bridge `741c1ebb` (#76), as directed by Main
through Integrator4. The registry pins both `tools/native` and
`src/native.om` by SHA-256. `OMEGA_OMC` remains seeded `e5b32567`, digest
`2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
The bridge's *generator bootstrap* separately requires its pinned compiler digest
`734f0a39319fbfa930bcc62cb30dbf06d47d78e126372b74f5baf478e6126b61`,
as `OMEGA_NATIVE_OMC`; this does not change the compiler of the computation
units. `OMEGA_NATIVE_OMC2` is forced to `OMEGA_OMC` by the registry.

The whole GPU partial link retains only its pre-existing mem/str allowlist.
The new native binding objects and the static bare-ELF differential have no
libc imports. `omega_cubin.c` itself is still an allocating diagnostic ABI
shell, not part of the allocator-free GPU partial link. Defining NO_POSIX on
that shell chooses native computation; it does not make malloc or snprintf
available in firmware. The bare test directly calls its same native unit.

## Focused gates

Run every command in the device-free wrapper:

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 \
  make -j3 gpu-engine-freestanding test-gpu-engine-freestanding test-codegen-freestanding
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 \
  make -j3 test-no-posix-native
```

Set `OMEGA_BRIDGE` to the native bridge checkout for these targets. For the
unchanged portable `om/units.sh --check`, use the bridge checkout matching
`om/omega-bridge.lock` instead. See the dated evidence README for observed
results, exact environment, negative controls and chip runbook.

Protected regions (`frag_pack_*`, `result_sentinels`, session command lists,
and #327's codegen-base migration) were not edited. No runtime API changes.

## Encoder/core codegen completion on ham/encoder-om-primary

The preceding inventory is historical at its named anchor. The core codegen
NO_POSIX exceptions for allocator, five matmul IR builders, scalar matmul
policies, fragment packing, result sentinels, vecadd and binding decisions now
use their existing seeded units through native caller-stack bindings. The
encoder itself already used that route at integration base `cb3395aa`.
Explicit C switches still provide independent differential oracles; there is
no C retry after an Omega refusal. No instruction forms or allocation rules
were added. C1b refuses P0 guards on the uniform LDCU and LDCU.64 forms in both
the seeded encoder and C oracle.

`make test-om-encoder-primary` exercises the production freestanding encoder
on 1024 samples per operand form (eight forms for every opcode 0..78). Accepted/refused counts are
written to `build/om-encoder/primary-counts.txt`; a flipped response byte must
be rejected. Run with the same pinned `OMEGA_OMC` and `OMEGA_BRIDGE` as above.
