# Linux platform residual migration

Base: CUDApp#297 at `313b1249` (policy predecessor #273). `plat_rest.om` is the default for the remaining marker, alias mapping, handle admission, time, queue-slot, submission-size and counter decisions. `CUDAPP_MIGRATION_PENDING_C_PLATFORM_REST=1` opts into the retained scalar C oracle; default is 0. Existing policy/publication/wait oracles remain opt-in.

## Function classification

This inventory accounts for every definition in `src/omega_gpu_platform_linux.c`. Imported declarations are not implementations.

| Functions | Class | Successor / retained C shell reason |
|---|---|---|
| plat_decide, plat_l2_words | Ported #273 | plat_policy.om computes policy and literal method words; C transports packets. |
| plat_rest, plat_scalar | Computation moved | plat_rest.om; C marshals fixed packets. Rest oracle header is absent from the default build. |
| plat_fd_is_memfd | Computation moved + OS shell | plat_rest mode 1 parses marker bytes; snprintf/readlink stay C to inspect the caller's descriptor namespace. |
| mmap | Computation moved + OS shell | plat_rest mode 12 chooses capture from syscall success, signed descriptor, capture flag and mmap flags; syscall/dup/close, native pointer checks and TLS ownership remain OS shell. |
| plat_wait_ready, plat_drop_token | OS shell | Context creation/destruction and descriptor ownership; no numeric algorithm. |
| plat_bind_fd | Computation moved + OS shell | plat_rest mode 0 computes rounded pages, admissible alias and next cursor. Existing bounded eight-attempt loop sequences OS mappings and releases failed handles; it does not introduce retry policy. |
| ref_lock, ref_unlock, refuse | OS shell | pthread locking, native table initialization, diagnostic formatting and driver error strings. |
| dev_ok, queue_ok | Ported existing platform service | Calls omega_gpu_handles_check; no duplicate handle algorithm. |
| buf_ok | Computation moved | plat_rest mode 2 owns index/generation/live/kind verdict; a C array bound protects gathering foreign table entries, not a competing policy verdict. |
| range_ok | Ported #273 | plat_policy mode 3; C supplies the buffer size. |
| ref_now_ns, ref_now_ms, ref_wait_ms | Computation moved + OS shell | plat_rest modes 3/4/9 perform ns/ms conversion and timespec splitting. clock_gettime/nanosleep stay OS shell. |
| ref_lock_id, ref_unlock_id | Computation moved + OS shell | plat_rest mode 5 checks the lock index; pthread calls and mapping their OS success/failure stay C. |
| omega_gpu_platform_linux_channels_env, xq_slot | Ported #273 | plat_policy modes 2/12; getenv/strlen, typed table gathering and representation conversion remain host shell. |
| ref_device_open | Computation moved + OS shell | plat_rest mode 7 owns ordinal/already-open admission; NVRM/channel lifecycle, output pointers and error unwinding remain OS shell. |
| ref_device_close | OS shell | Walk live native resources, free/close handles, release mappings, clear foreign structs and report driver teardown status. |
| ref_diag_text | OS shell | Returns a native C string pointer. |
| mem_reserve_check | Ported #273 + OS shell | plat_policy modes 5/7 parse minimum and round pages; omega_ampere_policy checks reserve. getenv/VRAM query and diagnostic formatting remain C. Shared-memory reserve delegates to its existing service. |
| ref_dma_alloc | Ported #273 + OS shell | plat_policy mode 4 selects allocation class/cache policy. Driver allocation dispatch, TLS descriptor capture, binding selected mappings, typed handle/pointer outputs and error cleanup remain host OS shell. |
| ref_dma_free | OS shell | Checks native ring pointer identity to avoid freeing an active resource, then closes mapping, driver allocation and handle. No replacement numeric implementation. |
| plat_publish, plat_fence | Ported #297 | Omega OM2 ring_store/cpufence; retained C implementation only under the predecessor's opt-in oracle. |
| ref_sync, ref_sync_to_gpu, ref_sync_to_cpu | Ported #273/#297 + OS shell | Range and cached pending policy in plat_policy mode 13; fence in Omega. C calls services, owns locking and updates native lifetime flags. |
| ref_poll_word | Ported #297 + computation moved / OS shell | Omega wait budget/mapget polling; plat_rest mode 13 ensures at least one poll. Native pointer lookup, fallback service call, sleep sequencing, observed volatile load and error translation remain OS shell. |
| ref_queue_open | Computation moved + OS shell | plat_rest mode 6 chooses the first unused extra slot; existing platform handles service validates/allocates queue. C connects foreign ring and channel resources. |
| ref_queue_close, ref_queue_retire | OS shell | Close/retire driver resources, reset native lifetime bookkeeping, release ring associations. |
| ref_queue_submit | Computation moved + ported #273/#297 / OS shell | plat_rest modes 8/10/11 compute prefix, byte sizes, combined words and wrapping inflight counter; plat_policy modes 8/9 enforce busy/capacity. Omega publishes/fences. malloc/free, typed pointer offsets, selected driver calls and success/failure sequencing remain C. |
| ref_host_alloc, ref_host_free | OS shell | malloc/free of foreign caller memory. |
| omega_gpu_platform_linux | OS shell | Returns native service vtable and copies capability struct. |
| omega_gpu_platform_linux_target_sm | Ported #273 | plat_policy mode 14; native locking/field gathering remain C. |
| omega_gpu_platform_linux_vram_info | OS shell | Queries live NVRM object only while native resource is open; no VRAM computation. |

## Defects repaired

* Real Linux memfd symlink targets begin `/memfd:`. The old bare `memfd:` parser rejected them, silently losing alias capture. The seeded parser accepts the existing bare spelling and the real Linux spelling. A live anonymous memfd test proves the old predicate rejects the actual readlink target while the new unit accepts it; no GPU is accessed.
* The old alias arithmetic allowed a rounded allocation larger than the alias address space, underflowing its upper-bound subtraction. The new unit rejects impossible page spans and checks the reset cursor too. Ordinary admissible plans remain byte-exact to the source-derived oracle; these invalid plans are explicit corrected partitions.
* #297's merge `748953c` retained the #281 unit registrations but dropped their files. This branch restores the exact source and generated blobs from ancestor `e2086965ba8ec60d7e385b98abd7565caa13293d`; it does not alter NVRM policy. It also removes the inherited trailing comma that made the platform .crumb invalid JSON.
* The link-unit generator now namespaces x86 assembly labels as well as declarations and `.size` expressions. Labels following an escaped newline have no regex word boundary; leaving them unrenamed made x86 assemblers reject the `.size` operands. The unmodified generated binding is retained as the failing assembly control in the CI-280 evidence.

## Host acceptance

```
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 make -j3 test-platform-linux-rest test-platform-linux-policy test-platform-linux-effects test-platform-linux-wait test-platform-linux-fake-device test-gpu-session-platform
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 sh om/units.sh --check
```

The timeout regression wraps `omega2_ctx_plat_wait_ge` at link time. It requires actual Omega polling with a nonzero bound handle and one poll per session slice, and requires zero fallback `m16_native_wait_sync` calls. This replaces the obsolete assertion that only passed while the broken memfd parser prevented capture.

### Remaining native-layout boundary

C gathers fields from mixed native channel/allocation structs and performs typed pointer publication, ownership resets and driver-resource walks. These are foreign ABI/lifetime operations, not duplicate scalar policy. The same concrete bounded-record bridge extension described in `tests/runtime/COSTMODEL_REST.md` on the sibling costmodel branch would move field gathering/scattering: byte-backed record views with checked field widths/offsets/stride, overflow and bounds refusals, bit-preserving access and mixed-layout red tests. Primitive fixed views alone do not express these mixed native record fields. No language/pin/public-API change or alternate runtime is introduced here.

New scalar differential partitions cover every mode, independent fields, each boundary and neighbors, 0, UINT32_MAX and UINT64_MAX. Explicit red output comparison must fail. Eight compiled source mutants cover rounding, cursor, marker slash/letter, stale generation, nanosecond scale, queue bound and sleep remainder. Existing policy/effects/wait/session targets remain authoritative for the integrated fake-device path.

WolframLanguageEvaluator: `ToCharacterCode["/memfd:"]` -> `{47,109,101,109,102,100,58}`; `2^63` -> `9223372036854775808` (signed descriptor threshold). No compiler/bridge pin or public API changes.


## Shared reserve parsing (ham/session-cached, host only)

`om/session/reserve.om` now owns C-locale unsigned-decimal parsing of
`CUDAPP_MEM_RESERVE_MIB`, the `MemAvailable:` conversion, and saturating MiB-to-byte
conversion. The existing engine unit still owns reserve admission. The shell
borrows input bytes without copying them. `CUDAPP_MIGRATION_PENDING_C_MEMRESV`
retains the original parser and arithmetic as the independently compiled oracle.
A binding failure never selects that oracle: the reserve saturates or allocation
is refused. File selection, O_NOFOLLOW/O_NONBLOCK/regular-file checks and locking
are unchanged. Missing or malformed meminfo preserves the existing unknown-memory
policy. This does not change any cache policy or make a C shell deletable.

The new Omega cases exercise sign/whitespace/overflow/end-pointer semantics and
long input, plus the unchanged handle generation, closed-handle, pin-underflow,
pinned-eviction and capacity refusals. Compile `tests/components/proc.om` followed
by `tests/session_memory_gate.om` with the pinned omc, then run the resulting
conductor with that compiler path as its one argument, from the repository root.
It composes the real parser, engine and existing slot unit and requires seven
source mutants to exit at a failed assertion, not crash. The existing
`make test-om-eng-diff test-om-eng-red test-om-eng-mutants` compares complete C
and Omega runtime transcripts; its reserve string partitions include malformed
lines and overflow values. `test-gpu-session-platform` and `test-gpu-serving-alloc`
remain the integrated host gates. No chip receipt follows from these tests.

## Chip/live-RM/VFIO runbook — NOT_RUN

Requires a fresh explicit exclusive window and exact operator approval. First integrate the stacked prerequisites and run the host commands above with the pinned compiler. In the authorized window, use the existing Linux session and stale-L2 chip runbooks: open the selected device, allocate a real shared buffer, record its captured alias token, publish a submission, observe the marker through Omega mapget, exercise timeout and stale-handle refusals, then release buffer/queue/device in order. Compare observed alias offsets and marker values to the submitted request and negative controls. Preserve raw logs and record device/driver/pin versions. Do not change gates, drivers, ports or ownership locks; do not infer permission from an empty lock. Host memfd and fake-device success are not chip qualification.
Compile-only preparation is `taskset -c 10-14 nice -n 10 make -j3 build/gpu_l2_stale_test`. After approval, the operator's approved chip launcher must execute `build/gpu_l2_stale_test --mode rewrite --cached 0 --reps 3`, then `--mode rewrite --cached 1 --reps 3`, and `--mode realloc --cached 1 --reps 3`. This CLI is defined by `tests/gpu_l2_stale_test.c`; rc 0 means FRESH, rc 1 means stale/API failure, rc 2 means usage error. Preserve full SNAP output and confirm the initial A read before interpreting B. The device-hidden host bwrap is not a chip launcher: do not remove it or expose devices without a separately approved device-scoped execution recipe. These commands have not been run in this lane.
