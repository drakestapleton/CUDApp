# NVML C ABI emitter extension

Status: PROPOSED, not an existing Omega syntax claim. Owner of integration: Main. Consumer: aien-dev/CUDApp#365, branch feat/omega-nvml-replacement, baseline 4bf58894d1985d41240736145de7b5fe03e377ee. Compiler: omega-mv e5b32567, omc SHA256 2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f. CUDApp bridge lock: bc11b30b1faa9af8b875ae6fec24e5f49cf03ac4. Current newer bridge documentation at d8174edd still refuses globals/text/host words in public native ABI with exit 110. This extension does not authorize weakening those existing modes: add an explicit library-emission mode.

## Current boundary and work available

`compat/nvml/cudapp_nvml.c` is 167 physical lines, not 167 lines of pure ABI marshalling. Its handle mapping (43–47), requested-action/call protocol (49–100), wrappers (102–148), and string cache (150–167) remain in the production path. `tests/nvml/host.om` is 942 physical lines: its lines 225–444 port the action/call protocol, wrappers and cache; lines 144–223 port the fake transport; the remaining code contains test state, helpers and the same 476 checks. This is an exercised model port, not a deployed .so. Do not describe the production C protocol as already removed.

Observed CPU behavior: pure Omega host has the same 476 passing checks and the same failure/check counts for all 22 source mutants as the prior C caller. The new seeded conductor additionally retains generated-source equality, CUDA-header ABI comparison, thin-shell guard and oracle negative control. Live RM, concurrent ABI calls and the generated .so remain NOT_RUN.

The deliverable of this extension is removal of the production C file, not generated boilerplate around that handwritten file. Lift the exercised protocol into `native/nvml/api.om`; bind its transport operations to the existing `native/nvrm/transport.om` functions in production and `rmfake.om` in the device-free gate. Keep the same NVML state machine, codes, copy decisions and fake faults. The bridge must lower the resulting program-mode globals and calls; it must not translate a C protocol into a second handwritten implementation.

## Declaration format

Use ordinary seeded `.om` declarations plus machine-readable `note cabi` metadata. Every file remains accepted by the pinned compiler because these are notes; the new bridge mode must validate and consume them. Unknown or incomplete `cabi` notes must be refused, not silently ignored in library mode. The following grammar is the proposed build contract, not already implemented:

```text
note cabi library libnvidia-ml.so.1 header nvml.h
note cabi lock NvLock process mutex
note cabi status nvmlReturn_t int32 valid 0..30,999 trap 999
note cabi enum nvmlTemperatureSensors_t uint32
note cabi handle nvmlDevice_t nvmlDevice_st pool NvDev count 8 invalid 18446744073709551615 lock NvLock
note cabi record nvmlMemory_t size 24 align 8 total:u64:0 free:u64:8 used:u64:16
note cabi record nvmlUtilization_t size 8 align 4 gpu:u32:0 memory:u32:4
note cabi record nvmlProcessInfo_t size 24 align 8 pid:u32:0 usedGpuMemory:u64:8 gpuInstanceId:u32:16 computeInstanceId:u32:20
note cabi state NvState process lock NvLock
note cabi state NvBytes process lock NvLock
note cabi state NvTexts process lock NvLock
note cabi state NvReady process lock NvLock
note cabi cstr NvTexts slots 32 width 64 fallback "Unknown Error" lock NvLock
note cabi export nvmlInit_v2 nv_init status nvmlReturn_t lock NvLock
note cabi export nvmlDeviceGetCount_v2 nv_count status nvmlReturn_t lock NvLock
note cabi arg nvmlDeviceGetCount_v2 deviceCount nullable out uint32 commit 0
note cabi export nvmlDeviceGetName nv_name status nvmlReturn_t lock NvLock
note cabi arg nvmlDeviceGetName device handle nvmlDevice_t
note cabi arg nvmlDeviceGetName name nullable out char length length commit 0
note cabi arg nvmlDeviceGetName length uint32
note cabi export nvmlErrorString nv_error cstr NvTexts lock NvLock
note cabi arg nvmlErrorString result signed int32
```

Types refer to existing Omega `uint`, `ulong`, `uchar`, `global`, `text` and bounded array parameters. No raw pointer enters Omega. Public names are metadata and need not satisfy the seeded eight-character local-name limit. The generator owns header and ELF export-list emission; headers are generated, not hand-edited. All remaining NVML entry points in `compat/nvml/nvml.h` use these same constructs: status, nullable scalar in/out, fixed record out, counted record-array out, handle in/out and cstr return. Add `nullable inout uint32` for `infoCount` and `nullable out nvmlProcessInfo_t count infoCount commit 1` for `infos`. Read an inout value only when non-null. Preserve input/output on errors unless Omega's commit bit explicitly allows a write (notably the count on INSUFFICIENT_SIZE).

### Explicit lowering contract

The bridge derives a bounded call frame from declarations. The implementation entry points use normal seeded signatures:

```text
global NvState ulong 128
global NvBytes uchar 83296
global NvTexts uchar 2048
global NvReady uchar 32

note Frame for nv_init: [status, commit_bits], no output bytes.
fn nv_init frame edit ulong 2 to ulong
note BODY is the existing exercised protocol port, not a new C body.
end

note Frame for nv_count: [status, commit_bits, count]. present is 0 or 1.
fn nv_count present ulong frame edit ulong 3 to ulong
end

note Frame for nv_name: [device_index, present, length, status, commit_bits, bytes_to_copy].
note bytes receives at most 6144 bytes. The state machine decides required length/error codes.
fn nv_name frame edit ulong 6 bytes edit uchar 6144 to ulong
end

note Frame for nv_error: [signed-code-bits, slot, string_length].
note Writes/checks NvTexts/NvReady in Omega and returns success 0; slot 32 means fallback.
fn nv_error frame edit ulong 3 to ulong
end
```

These signature sketches deliberately omit bodies: the bridge owner must connect the exercised implementations, not commit these empty sketches. Return value 0 means the Omega protocol completed; a nonzero return or a compiler trap becomes the declared NVML trap status without process abort. The emitter's generated checked call uses the existing `omega_*` status convention underneath. Frames are private, initialized, and never exposed to the foreign caller. The generator must document and enforce field indices above so metadata and Omega implementation cannot disagree silently.

Nullable out pointers lower to a presence flag and scratch memory. They are never eagerly dereferenced and never converted to integer pointers inside Omega. Only declared commit bits permit publication. Required span/capacity checks and successful scalar narrowing precede *all* publication, preventing partially modified caller outputs on adapter failure. A `NULL` buffer with a nonzero length is passed as `present=0`, not dereferenced or given fake capacity.

Opaque handles are addresses of immutable library-issued table entries. The generated adapter compares pointer identity with table addresses without dereferencing foreign pointers. Unknown/NULL handles lower to UINT64_MAX; Omega decides the NVML return status. Handle outputs validate the returned index before publishing a table address. No arbitrary pointer arithmetic or dereference of an untrusted handle.

Process globals are zero-initialized once per loaded library and persist across calls. A generated process mutex serializes *all* entries sharing the state and string cache. Library unload follows normal dlclose caller obligations. Error strings live in process-global static storage and cannot be freed or reused; returned pointer identity stays stable across repeated calls, init/shutdown and other error-string requests. No per-call allocation is required. `pthread_mutex_t` is an explicit temporary host dependency, with exact libc/toolchain versions recorded by the consumer; the production replacement is an Omega process-lock host primitive. This proposal must not silently adopt it as Omega implementation.

## Required generated C goldens

The following are expected output shapes, not handwritten source files to ship. Boilerplate helpers are generated once by the bridge. `nv_ok_status` checks the declared enum domain. `nv_handle_index` performs safe table-identity mapping. `omega_nv_*` denotes bridge-generated checked implementations of the seeded Omega entry points, including process-global storage. No call to the former handwritten `nvml_run` or `nvml_act` is permitted.

```c
/* GENERATED by omega-bridge <full source revision>; source SHA256 <digest>. */
nvmlReturn_t nvmlDeviceGetCount_v2(unsigned int *deviceCount) {
    uint64_t frame[3] = {999, 0, 0}, done = 1;
    nvmlReturn_t result = NVML_ERROR_UNKNOWN;
    if (pthread_mutex_lock(&NvLock) != 0) return result;
    uint32_t st = omega_nv_count(&done, deviceCount != NULL, frame, 3);
    if (st == OMEGA_OK && done == 0 && nv_ok_status(frame[0]) &&
        (frame[1] & ~UINT64_C(1)) == 0 &&
        (!(frame[1] & 1) || (deviceCount != NULL && frame[2] <= UINT32_MAX))) {
        if (frame[1] & 1) *deviceCount = (unsigned int)frame[2];
        result = (nvmlReturn_t)frame[0];
    }
    pthread_mutex_unlock(&NvLock);
    return result;
}

nvmlReturn_t nvmlDeviceGetName(nvmlDevice_t device, char *name, unsigned int length) {
    uint64_t frame[6] = {nv_handle_index(device), name != NULL, length, 999, 0, 0};
    uint8_t bytes[6144] = {0};
    uint64_t done = 1;
    nvmlReturn_t result = NVML_ERROR_UNKNOWN;
    if (pthread_mutex_lock(&NvLock) != 0) return result;
    uint32_t st = omega_nv_name(&done, frame, 6, bytes, sizeof bytes);
    if (st == OMEGA_OK && done == 0 && nv_ok_status(frame[3]) &&
        (frame[4] & ~UINT64_C(1)) == 0 &&
        (!(frame[4] & 1) || (frame[5] <= length && frame[5] <= sizeof bytes &&
                            (frame[5] == 0 || name != NULL)))) {
        if ((frame[4] & 1) && frame[5]) memcpy(name, bytes, (size_t)frame[5]);
        result = (nvmlReturn_t)frame[3];
    }
    pthread_mutex_unlock(&NvLock);
    return result;
}

const char *nvmlErrorString(nvmlReturn_t code) {
    uint64_t frame[3] = {(uint64_t)(int64_t)code, 32, 0}, done = 1;
    const char *result = "Unknown Error";
    if (pthread_mutex_lock(&NvLock) != 0) return result;
    uint32_t st = omega_nv_error(&done, frame, 3);
    if (st == OMEGA_OK && done == 0 && frame[1] < 32 && frame[2] < 64 &&
        NvTexts[frame[1] * 64 + frame[2]] == 0)
        result = (const char *)&NvTexts[frame[1] * 64];
    pthread_mutex_unlock(&NvLock);
    return result;
}
```

This is the minimal readable golden, not a required inefficient code shape: do not zero the entire byte scratch if the checked Omega writer can establish the written range before publication. Preserve the semantics and bounds; use compiler lifetime/initialization analysis to avoid avoidable copies/initialization. `NvTexts` is generated static state and the checked Omega function must address that same object, not a duplicate array. All generated C exports have default visibility; all internal Omega/helper symbols are hidden. Emit a version script enumerating precisely the 11 symbols currently declared in the NVML header, and link with `-Wl,-soname,libnvidia-ml.so.1`. Reject duplicate/unknown symbols and any source drift against the manifest. The test checks the actual symbol list rather than trusting the link command.

## Smallest acceptance and negative controls

1. Add a seeded `.om` bridge fixture defining the declarations and three complete functions for count/name/error-string, with no NVML RM access. Compile it with the pinned seeded compiler; emit C/header/export map twice and require byte equality and source/generator provenance. Build an isolated shared library with the exact soname and only declared exports. Verify metadata/exports using pinned readelf/nm as read-only oracles.
2. Use an Omega test driver to generate/build and invoke a generated foreign caller for the fixture. It must check null out-pointers, untouched error outputs, length-minus-one/exact/oversized buffers with sentinels, forged handles without dereference, signed negative enum inputs, repeated stable error-string pointers and process-state persistence. Emit the foreign caller through omega-bridge too; do not introduce a handwritten C driver. Until the callback/caller emitter exists, the existing CUDApp C caller remains an explicitly labelled ORACLE only.
3. Integrate the full API `.om` protocol against the existing RM fake. The generated .so/header must pass the original C-ABI ORACLE's 476 checks, the 55 CUDA-header ABI facts, compile against NVIDIA prototypes, and reject all 22 identical source mutants with identical failure/check counts. The same pure-Omega model must retain those counts. Run in a /dev-isolated CPU-only sandbox; no GPU is needed.
4. Mandatory generated-wrapper negative control: remove the `deviceCount != NULL` publication guard in a *generated-output copy* and force `commit_bits=1` with a null output. The subprocess must fail; a separate mutant suppressing the count publication with a non-null pointer must fail the observable count test without crashing. A surviving mutant makes the gate fail. Neither compiler failure nor exec failure counts as a caught behavioral mutant.
5. Declaration-refusal negative controls: undeclared global, duplicate export, unknown lock, invalid record offset/size, unsupported raw-pointer result, out-span lacking a length, and cstr offset/length outside the declared storage. Each must refuse before writing a usable .so; never silently generate a partial library.
6. Concurrency is a separate ABI gate: overlapping calls must preserve state and immutable string pointers; label NOT_RUN until a real generated concurrent caller is available. The previous 476 checks are not concurrency evidence. Live hardware is separately QUALIFIED only under an authorized GPU window.

## Extension completion contract

The bridge owner returns the exact source revision, new emitter invocation and generated-artifact hashes; CUDApp repins it, removes `compat/nvml/cudapp_nvml.c`, removes the handwritten header in favor of the generated header, and rewires all library callers/build paths. Only then is the pure-Omega HOLD cleared. A compile-only implementation, three-entry fixture alone, stale generated code, or leaving nvml_run/nvml_act handwritten does not complete #365.
