# NVRM client: Omega boundary

The library uses `nvrm/nvrm.c` through `m16/m16_native.c`. It talks directly
to the pinned NVIDIA 580.173.02 open kernel ABI; it does not load libcuda.
The proprietary kernel/module replacement is not part of this client change.

## Source map

| Responsibility | Source |
| --- | --- |
| Public lifecycle, class selection, VA ownership, quarantine and channel policy | `nvrm/nvrm.c`, `nvrm/nvrm.h` |
| Library adapter and completion waits | `m16/m16_native.c`, `m16/m16_gpu_wait.c` |
| NVOS21, NVOS54, NVOS00, NVOS33+fd, NVOS46, NVOS47, register-fd bytes and reply decoding | `native/nvrm/wire.om`, `wire_pack.om` |
| NVOS32 heap-information request bytes | `native/nvrm/heap_wire.om` |
| All UVM requests used by this client, including the complete external-allocation attribute array | `native/nvrm/uvm_wire.om`, `uvm_map_wire.om` |
| Checked file, RM, UVM and owned mapping operations | `native/nvrm/transport.om`, generated `src/cudapp_om_nvrm_transport.c` |
| Earlier standalone program-mode context, with its own smaller resource tables | `native/nvrm/context.om` |

The standalone `context.om` is not substituted for the library client: its
singleton tables do not implement the library's independent contexts or its
4096 live allocations per context.

`CUDAPP_MIGRATION_PENDING_C_NVRM=0` is the default. Request serialization and
the actual open/close, RM/UVM ioctl, reserve/bind/map/drop operations are Omega.
The checked native bridge calls the compiled transport, not a C syscall shim.
Setting the switch to 1 retains the original C transport and C request
construction as the differential oracle. The existing fake-driver fixture
selects `CUDAPP_NVRM_TEST_TRANSPORT=1` explicitly to interpose its C syscall
boundary while retaining Omega request construction. Defining `ioctl` or
`mmap` macros alone never selects C; a separate link control checks that their
forbidden symbol names are not referenced. Production has no fallback.
High-level lifecycle policy and vendor class-specific parameter values still
originate in C; this is not a claim that every line of the library is Omega.

Each client owns a native mapping context with 4098 slots: 4096 live
allocations, the usermode page and one transient reservation. Only an owned
reservation can be replaced by a fixed mapping. A Linux bind failure retires
the reservation; a trap does not prove retirement. Failed cleanup retains its
token and quarantines its VA. Callers must serialize each complete
reserve/bind/transfer/cleanup transaction, not only individual bridge calls.

The hosted transport uses omega-bridge `689777c13b351b7450de1a2641cb4af9c97209ce`
(draft #68, stacked on context draft #67 and device-word draft #66; the original
function-mode device-word dependency was #29). It requires compiler drafts
omega-mv #1043 and #1046. These dependencies need Drake's approval and merge;
the existing host-x86 gate must be green before this client can merge.
No live RM qualification is claimed by host serialization, real CPU mappings
or fake-driver tests.

Regenerate the hosted binding with `sh native/nvrm/units.sh`, separately from
the portable wire units. It requires `OMEGA_NVRM_BRIDGE` at the revision and
source/tool digests recorded in `src/cudapp_om_nvrm_transport.ident`,
`OMEGA_NATIVE_OMC` at `734f0a39319fbfa930bcc62cb30dbf06d47d78e126372b74f5baf478e6126b61`,
`OMEGA_NATIVE_OMC2` at `d1a3047153e14676c4baf9248fa2b88bb1b68c37cfeb71d76f30f5230c7d2765`
(omega-mv `ce35b351`, pending #1043), and `OMEGA_NATIVE_X86_OMC` at
`cf4fcbbb4752c5dd0652b426c0f2d08f82e380e8fc6f5114acf51a4c077bb579`
(omega-mv `6f60fec9`, pending #1046). Old `68a5d7fa` context binaries are
superseded: they do not quarantine a failed bind whose cleanup also fails.
The script invokes `tools/native --host` independently for each target and
commits architecture-guarded payloads behind the same C ABI; instruction
streams are never mixed in one generation. Set `OMEGA_NVRM_CC_AARCH64` and
`OMEGA_NVRM_CC_X86` to the respective C compilers when the defaults do not fit.

## Host gates

With `OMEGA_OMC` and `OMEGA_BRIDGE` set to the repository's pinned toolchain:

```sh
taskset -c 10-14 nice -n 10 sh om/units.sh
taskset -c 10-14 nice -n 10 make -j3 test-nvrm-wire test-nvrm-native test-nvrm-differential test-nvrm-boundary test-ampere-runtime build/test_nvrm_va_slots
taskset -c 10-14 nice -n 10 ./build/test_nvrm_va_slots
```

The wire gate compares whole buffers, including padding, against independent
vendored C structures with varying full-width scalar values and UUIDs. It
checks fake replies with nonzero RM statuses, invalid operations, byte mutants,
and a deliberately wrong serializer that must be rejected. UVM serialization
covers INITIALIZE, MM_INITIALIZE, GPU/VA-space/channel register and unregister,
external-range create, external-allocation map and free.
The Ampere fake driver exercises allocation, control, mapping and cleanup
without opening a GPU. These are **HOST_ONLY**, not chip receipts.
`test-nvrm-differential` also compiles that lifecycle fixture with the retained
C oracle explicitly enabled and compares its complete output with the Omega
default; both binaries must satisfy the same internal assertions.

The native fixture executes the compiled Omega syscall path. In a child only,
a seccomp filter traps ioctl calls to deliberately unopened fd 12345. A SIGSYS
oracle compares every request number and payload byte to the independent C
vendor layout, injects nonzero status replies, and verifies RM parameter
copyback. A wrong-request mutant must fail. No driver node is opened.
Separate native tests exercise ordinary files, errno results, exact-VA
reservation, failed-bind retirement, cleanup, all 4098 slots, independent
contexts, stale/foreign handles, refusal to bind an existing data mapping, and
the explicit envelope errors below. These are **HOST_ONLY**, not RM proofs.
The cleanup fixture denies the first real `munmap` after a failed fixed bind.
The caller must retain the quarantined owner, reject rebinding and destruction,
and succeed when cleanup is retried. Only its child process installs the
syscall filter; the second cleanup is forwarded to the real Linux syscall.

## Native admission envelope and extension proposal

There is no silent C fallback. The transport returns `-EFBIG` (Linux errno 27)
before invoking mapping words when an extent exceeds 64 GiB or a fixed span
would leave the existing below-1-TiB range. The C ABI returns `-1` with
`errno=EFBIG`. Oversized extents, high addresses and an end-crossing range are
negative fixtures; zero extents are `EINVAL`. Positive fixtures reserve and
release exactly 64 GiB of `PROT_NONE` VA and the final admissible page below
1 TiB, without allocating backing memory. The compiler gate is unchanged.

Source-backed workload bounds, not live measurements:

| Workload | Bytes / bound | Source |
| --- | ---: | --- |
| Largest actual FP8-95a723d0 model tensor, BF16 embed/lm_head `[248320,2048]` | 1017118720 | Local pinned model `outside.safetensors` headers |
| Larger in-tree head shape `[249856,2048]`, BF16 | 1023410176 | `tests/gpu_bf16_gemv_test.c:98` |
| All 64196 model tensor payloads, including vision/MTP | 37454789472 | Byte-offset bookkeeping over the pinned safetensors headers |
| Q36 K/V allocation per layer at the admitted maximum 131072 positions | 268451968 | `src/omega_gpu_decode_attn256_api.c:92-96`, shape 2 KV heads, row 512 bytes, slack from `omega_blackwell_codegen_attn256.h:61` |
| Actual 40-layer model's 10 full-attention K/V allocations | 2684519680 | Pinned model `config.json` layer types |
| Conservative 64-layer API maximum K/V payload | 17180925952 | `src/omega_gpu_decode_attn256_api.h:51-52`; separate allocations, not one extent |
| Explicitly simultaneous in-tree RM contexts | 2 | `m16/m16_concurrent.c:131,139`; the serving session is a singleton (`src/omega_gpu_session.c:363-365`) |

The largest head-shape allocation leaves 67696066560 bytes below the 64-GiB
extent cap. Two 4-GiB context strides from the initial 64-GiB base leave
1022202216448 bytes below 1 TiB before allocation-driven slot growth.
As a deliberately conservative modeled bound, all model payloads plus the
64-layer K/V maximum plus 4096 worst-case page-alignment gaps consume
54652488544 bytes per context. Even two such clients plus two slot strides
above the initial base leave 912897239360 bytes below 1 TiB. This is not a
complete measured serving allocation trace or an unlimited-context guarantee.
Existing arbitrary external callers can exceed the envelope and are refused.

WolframLanguageEvaluator queries (results in bytes except context counts):

```text
{248320*2048*2,249856*2048*2,256*1024*2048,
 2*(2*131072*512+(4*4*512+64)),2*(4*131072*512+(4*4*512+64)),
 64*2*(2*131072*512+(4*4*512+64)),2^36-248320*2048*2,
 2^36-249856*2048*2,2^36-2*(2*131072*512+(4*4*512+64)),
 2^40-(16*2^32+2*2^32),2^40/2^32,Floor[(2^40-16*2^32)/2^32]}
-> {1017118720,1023410176,536870912,268451968,536887424,17180925952,
    67702358016,67696066560,68451024768,1022202216448,256,240}
{10*2*(2*131072*512+(4*4*512+64)),
 37454789472+64*2*(2*131072*512+(4*4*512+64))+4096*4095,
 2^40-(16*2^32+2*(37454789472+64*2*(2*131072*512+(4*4*512+64))+4096*4095)+2*2^32),
 2^36-37454789472,2^40-(16*2^32+2*2^32)}
-> {2684519680,54652488544,912897239360,31264687264,1022202216448}
```

Concrete compiler extension proposal, **PROPOSED, not implemented**: admit a
configurable fixed-VA ceiling up to the architecture's canonical 47-bit user
range, or raise the current fixed-span cap after an explicit safety review.
Retain checked nonwrapping address-plus-length arithmetic, page alignment,
`MAP_FIXED_NOREPLACE` plus returned-address verification, the 64-GiB
per-extent bound unless separately justified, and ownership-checked whole-
reservation-only `MAP_FIXED`. Test both sides of every ceiling, occupied and
adjacent spans, stale/foreign tokens, failed-bind retirement and unchanged
neighbor mappings. Do not conflate a larger admitted virtual range with
physical-memory availability, device addressability or hardware qualification.

## Live RM runbook — NOT_RUN

Only Drake's current exclusive-window authorization permits these commands.
Do not acquire or modify the GPU lock, stop a service or open `/dev/nvidia*`
outside that procedure. First resolve omega-bridge#29 and record the exact
compiler/bridge identities used to regenerate and link the native transport.

In the authorized window, use `AGENTS.md` and the existing lifecycle and M16
qualification procedures, with a separate new output directory; never replace
historical `evidence/` receipts:

```sh
taskset -c 10-14 nice -n 10 env CUDAPP_MIGRATION_PENDING_C_NVRM=0 bash tests/run_nvrm_lifecycle_gates.sh
taskset -c 10-14 nice -n 10 env EVIDENCE_DIR=/path/to/new-window/omega RECEIPT_FILE=/path/to/new-window/omega-receipt.json CUDAPP_MIGRATION_PENDING_C_NVRM=0 bash tests/run_m16_requalification.sh
```
These scripts must be run according to their documented GPU-window and
receipt-durability preconditions. Run the same lifecycle workload with a
separately built `CUDAPP_MIGRATION_PENDING_C_NVRM=1` oracle, recording the build
flags, compiler identity, RM statuses and allocation/free accounting. Compare
request bytes before calling the kernel; pointer fields must name the same
parameter span, not a fabricated fixed address. Include injected UVM-free and
RM-free failures, stale/duplicate frees, independent concurrent contexts and
mapping-capacity exhaustion. A negative control must fail rather than be
reported as a clean teardown. Preserve uncertain cleanup state and quarantine
its VA. Report the live tier **NOT_RUN** until a new authorized receipt exists.
