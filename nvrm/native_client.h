/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* C ABI glue only. File/ioctl/mapping operations are compiled Omega. */
#ifndef CUDAPP_NVRM_NATIVE_CLIENT_H
#define CUDAPP_NVRM_NATIVE_CLIENT_H
/* Only the explicitly selected test boundary may use injected libc macros.
 * Merely defining ioctl/mmap must never select the migration-pending C path. */
#if CUDAPP_MIGRATION_PENDING_C_NVRM || (defined(CUDAPP_NVRM_TEST_TRANSPORT) && CUDAPP_NVRM_TEST_TRANSPORT)
#define NVRM_C_TRANSPORT 1
#else
#define NVRM_C_TRANSPORT 0
#include "cudapp_nvrm_native.h"
static int nvrm_native_ok(uint32_t status, uint64_t value) {
    if (status) { errno = EPROTO; return -1; }
    if ((int64_t)value < 0) { errno = (int)-(int64_t)value; return -1; }
    return 0;
}
static int nvrm_mapping_context(Nvrm *rm) {
    if (!rm->omega_context && omega_native_context_create(NVRM_MAX_LIVE + 2, &rm->omega_context)) {
        errno = ENOMEM; return -1;
    }
    return 0;
}
/* Owned mapping handle and byte offset for [p, p+n) inside exactly one live,
 * unquarantined CPU mapping of this client. 0 on one match; -1 with EINVAL
 * (bad args/no context) or EFAULT (no or ambiguous owner). Shared by the
 * submit stores and the platform marker poll; never a second owner. */
static inline int nvrm_live_token(const Nvrm *rm, const void *p, size_t n, uint64_t *token, uint64_t *offset) {
    if (!rm || !p || !n || !token || !offset || !rm->omega_context) { errno = EINVAL; return -1; }
    uintptr_t a = (uintptr_t)p;
    if (a > UINTPTR_MAX - n) { errno = EINVAL; return -1; }
    uint32_t hits = 0;
    for (uint32_t i = 0; i < rm->live_count; i++) {
        const NvrmLiveAlloc *l = &rm->live[i];
        uintptr_t b = (uintptr_t)l->cpu;
        if (!l->cpu_live || l->quarantined || !l->omega_mapping || !b || a < b || l->size < n ||
            a - b > l->size - n) continue;
        *token = l->omega_mapping; *offset = a - b; hits++;
    }
    if (hits != 1) { errno = EFAULT; return -1; }
    return 0;
}
#endif

static int nvrm_file_open(const char *path) {
#if NVRM_C_TRANSPORT
    return open(path, O_RDWR | O_CLOEXEC);
#else
    uint64_t result = 0;
    uint32_t status = omega2_nvrm_open_file(&result, (const unsigned char *)path, strlen(path));
    return nvrm_native_ok(status, result) ? -1 : (int)result;
#endif
}
static int nvrm_file_close(int fd) {
#if NVRM_C_TRANSPORT
    return close(fd);
#else
    uint64_t result = 0;
    uint32_t status = omega2_nvrm_close_file(&result, (uint64_t)(int64_t)fd);
    return nvrm_native_ok(status, result);
#endif
}
static int nvrm_rm_ioctl(int fd, unsigned nr, void *arg, size_t size) {
#if NVRM_C_TRANSPORT
    return ioctl(fd, NV_IOWR(nr, size), arg);
#else
    void *params = NULL; size_t length = 0;
    if (nr == NV_ESC_RM_ALLOC) {
        NVOS21_PARAMETERS *h = arg; params = (void *)(uintptr_t)h->pAllocParms; length = h->paramsSize;
    } else if (nr == NV_ESC_RM_CONTROL) {
        NVOS54_PARAMETERS *h = arg; params = (void *)(uintptr_t)h->params; length = h->paramsSize;
    }
    uint64_t result = 0;
    uint32_t status = omega2_nvrm_ioctl(&result, (uint64_t)(int64_t)fd, nr,
                                       arg, size, params, length);
    return nvrm_native_ok(status, result);
#endif
}
static int nvrm_uvm_ioctl(int fd, unsigned long cmd, void *arg) {
#if NVRM_C_TRANSPORT
    return ioctl(fd, cmd, arg);
#else
    size_t size;
    switch (cmd) {
    case UVM_INITIALIZE: size = sizeof(UVM_INITIALIZE_PARAMS); break;
    case UVM_MM_INITIALIZE: size = sizeof(UVM_MM_INITIALIZE_PARAMS); break;
    case UVM_REGISTER_GPU: size = sizeof(UVM_REGISTER_GPU_PARAMS); break;
    case UVM_UNREGISTER_GPU: size = sizeof(UVM_UNREGISTER_GPU_PARAMS); break;
    case UVM_REGISTER_GPU_VASPACE: size = sizeof(UVM_REGISTER_GPU_VASPACE_PARAMS); break;
    case UVM_UNREGISTER_GPU_VASPACE: size = sizeof(UVM_UNREGISTER_GPU_VASPACE_PARAMS); break;
    case UVM_REGISTER_CHANNEL: size = sizeof(UVM_REGISTER_CHANNEL_PARAMS); break;
    case UVM_UNREGISTER_CHANNEL: size = sizeof(UVM_UNREGISTER_CHANNEL_PARAMS); break;
    case UVM_MAP_EXTERNAL_ALLOCATION: size = sizeof(UVM_MAP_EXTERNAL_ALLOCATION_PARAMS); break;
    case UVM_FREE: size = sizeof(UVM_FREE_PARAMS); break;
    case UVM_CREATE_EXTERNAL_RANGE: size = sizeof(UVM_CREATE_EXTERNAL_RANGE_PARAMS); break;
    default: errno = EINVAL; return -1;
    }
    uint64_t result = 0;
    uint32_t status = omega2_nvrm_uvm(&result, (uint64_t)(int64_t)fd, cmd, arg, size);
    return nvrm_native_ok(status, result);
#endif
}
static void *nvrm_cpu_reserve(Nvrm *rm, uint64_t address, uint64_t size) {
#if NVRM_C_TRANSPORT
    (void)rm;
    return mmap((void *)(uintptr_t)address, size, PROT_NONE,
                MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
#else
    if (nvrm_mapping_context(rm)) return MAP_FAILED;
    if (rm->omega_pending_mapping) { errno = EBUSY; return MAP_FAILED; }
    uint64_t token = 0;
    uint32_t status = omega2_ctx_nvrm_reserve(rm->omega_context, &token, address, size);
    if (nvrm_native_ok(status, token)) return MAP_FAILED;
    rm->omega_pending_mapping = token;
    rm->omega_pending_address = address; rm->omega_pending_size = size;
    return (void *)(uintptr_t)address;
#endif
}
static void *nvrm_cpu_map(Nvrm *rm, int fd, uint64_t size, void *target) {
#if NVRM_C_TRANSPORT
    (void)rm;
    return mmap(target, size, PROT_READ | PROT_WRITE, MAP_SHARED | (target ? MAP_FIXED : 0), fd, 0);
#else
    if (nvrm_mapping_context(rm)) return MAP_FAILED;
    uint64_t result = 0, token;
    uint32_t status;
    if (target) {
        if (!rm->omega_pending_mapping || rm->omega_pending_address != (uintptr_t)target ||
            rm->omega_pending_size != size) { errno = EINVAL; return MAP_FAILED; }
        status = omega2_ctx_nvrm_bind(rm->omega_context, &result,
                                      rm->omega_pending_mapping, (uint64_t)(int64_t)fd);
        if (nvrm_native_ok(status, result)) {
            /* The checked mapbind contract unmaps and retires on a Linux
             * failure. A trap does not establish that ownership was retired. */
            if (!status) {
                rm->omega_pending_mapping = 0;
                rm->omega_pending_address = rm->omega_pending_size = 0;
            }
            return MAP_FAILED;
        }
        return target;
    }
    if (rm->omega_usermode_mapping) { errno = EBUSY; return MAP_FAILED; }
    status = omega2_ctx_nvrm_map_file(rm->omega_context, &result, (uint64_t)(int64_t)fd, size);
    if (nvrm_native_ok(status, result)) return MAP_FAILED;
    token = result;
    rm->omega_usermode_mapping = token;
    status = omega2_ctx_nvrm_map_address(rm->omega_context, &result, token);
    if (nvrm_native_ok(status, result)) {
        int saved_errno = errno;
        uint64_t ignored = 0;
        uint32_t cleanup = omega2_ctx_nvrm_drop(rm->omega_context, &ignored, token);
        if (!cleanup && ignored == 0) rm->omega_usermode_mapping = 0;
        else rm->faulted = 1; /* retain ownership for close's retry */
        errno = saved_errno;
        return MAP_FAILED;
    }
    return (void *)(uintptr_t)result;
#endif
}
static int nvrm_cpu_drop(Nvrm *rm, void *address, size_t size) {
#if NVRM_C_TRANSPORT
    (void)rm;
    return munmap(address, size);
#else
    uint64_t *owned = NULL;
    NvrmLiveAlloc *live = NULL;
    if (rm->omega_pending_mapping && rm->omega_pending_address == (uintptr_t)address &&
        rm->omega_pending_size == size) owned = &rm->omega_pending_mapping;
    else if (rm->omega_usermode_mapping && rm->usermode_cpu == address && size == 0x10000)
        owned = &rm->omega_usermode_mapping;
    else for (uint32_t i = 0; i < rm->live_count; i++) {
        if (rm->live[i].cpu == address && rm->live[i].size == size && rm->live[i].omega_mapping) {
            live = &rm->live[i]; owned = &live->omega_mapping; break;
        }
    }
    if (!owned || !rm->omega_context) { errno = EINVAL; return -1; }
    uint64_t result = 0;
    uint32_t status = omega2_ctx_nvrm_drop(rm->omega_context, &result, *owned);
    if (nvrm_native_ok(status, result)) return -1;
    *owned = 0;
    if (live) live->cpu_live = 0;
    if (owned == &rm->omega_pending_mapping)
        rm->omega_pending_address = rm->omega_pending_size = 0;
    return 0;
#endif
}
static void nvrm_mapping_transfer(Nvrm *rm, NvrmLiveAlloc *live) {
#if NVRM_C_TRANSPORT
    (void)rm; (void)live;
#else
    live->omega_mapping = rm->omega_pending_mapping;
    live->cpu_live = live->omega_mapping != 0;
    if (live->omega_mapping && (rm->omega_pending_address != live->va || rm->omega_pending_size != live->size))
        rm->faulted = 1;
    rm->omega_pending_mapping = rm->omega_pending_address = rm->omega_pending_size = 0;
#endif
}
static int nvrm_mapping_finish(Nvrm *rm) {
#if NVRM_C_TRANSPORT
    (void)rm;
#else
    if (rm->omega_pending_mapping && nvrm_cpu_drop(rm, (void *)(uintptr_t)rm->omega_pending_address,
                                                 rm->omega_pending_size)) return -1;
    if (rm->omega_usermode_mapping && !rm->usermode_cpu) {
        uint64_t result = 0;
        uint32_t status = omega2_ctx_nvrm_drop(rm->omega_context, &result, rm->omega_usermode_mapping);
        if (nvrm_native_ok(status, result)) return -1;
        rm->omega_usermode_mapping = 0;
    }
    if (rm->omega_context) {
        if (omega_native_context_destroy(rm->omega_context)) { errno = EBUSY; return -1; }
        rm->omega_context = NULL;
    }
#endif
    return 0;
}
#endif
