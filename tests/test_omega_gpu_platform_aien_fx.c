/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* test_omega_gpu_platform_aien_fx: the aien-gpu platform's REAL effects (no OmegaGpuAienSys replacement) against a
 * fake device file. Host program, opens no device and needs no module.
 *
 * The fake device is a memfd named "aien-fx"; CUDAPP_AIEN_GPU_DEV=/proc/self/fd/N makes the platform open it as it
 * would open /dev/aien-gpu, so open(2), mmap(2) of objects and of the doorbell page, munmap(2) and close(2) are the
 * kernel's. Only the ioctls are emulated: a seccomp filter turns every ioctl of type 'A' into SIGSYS, and the handler
 * answers AGPU_INFO / MEM_ALLOC / MEM_FREE / CHAN_CREATE / CHAN_DESTROY like aien-gpu.ko would (objects at bump
 * offsets inside the memfd, the usermode page at offset 0), records the request bytes in and out, and returns.
 *
 * Built twice (mk/omega-gpu-engine.mk test-gpu-platform-aien-effects): against native/aiengpu/effects.om through the
 * generated bridge library (CUDAPP_MIGRATION_PENDING_C_PLATFORM_AIEN_EFFECTS=0) and against the C system calls
 * (=1). The trace written to descriptor 3 when AIEN_FX_TRACE_FD=3 (every ioctl's bytes in and out, then every nonzero 32-byte line of the
 * fake device: pushbuffer, ring, USERD GPPut, doorbell word) must be byte-identical between the two builds.
 * It also checks: every object freed, the channel destroyed, and no mapping of the device left in /proc/self/maps
 * after device_close; in the Omega build, an operator request (AGPU_BOOT) is refused by the unit with -22 and never
 * reaches the kernel.
 */
#ifndef CUDAPP_MIGRATION_PENDING_C_PLATFORM_AIEN_EFFECTS
#define CUDAPP_MIGRATION_PENDING_C_PLATFORM_AIEN_EFFECTS 0 /* as src/omega_gpu_platform_aien.c on AArch64 */
#endif
#include "omega_gpu_platform_aien.h"
#include "aien_gpu_uapi.h"
#if !CUDAPP_MIGRATION_PENDING_C_PLATFORM_AIEN_EFFECTS
#include "cudapp_aiengpu_native.h"
#endif

#include <errno.h>
#include <fcntl.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <ucontext.h>
#include <unistd.h>

#if !defined(__aarch64__)
#error "the aien-gpu effects fixture reads the AArch64 SIGSYS register frame"
#endif

#define FX_BYTES (8u << 20)
#define FX_DOORBELL_OFF 0ull
#define FX_FIRST_OBJ 0x10000ull
#define FX_TOKEN 0x00010007u
#define FX_OBJS 16

static int ok = 1;
#define EXPECT(c) do { if (!(c)) { ok = 0; printf("  expect failed at line %d: %s\n", __LINE__, #c); } } while (0)

typedef struct { uint64_t off, size; uint32_t flags; int live; } FxObj;
static struct {
    int memfd;
    ino_t ino;
    uint64_t bump;
    FxObj obj[FX_OBJS];
    uint32_t nobj, traps, chan_live, chan_creates, chan_destroys, frees;
} fx;

/* ---- trace: built in a static buffer by the signal handler (no stdio there), written out at the end ---- */
static char tbuf[1u << 20];
static size_t tlen;
static void t_str(const char *s) { while (*s && tlen < sizeof tbuf) tbuf[tlen++] = *s++; }
static void t_hex(const void *p, size_t n) {
    static const char hx[] = "0123456789abcdef";
    const uint8_t *b = p;
    for (size_t i = 0; i < n && tlen + 2 < sizeof tbuf; i++) { tbuf[tlen++] = hx[b[i] >> 4]; tbuf[tlen++] = hx[b[i] & 15u]; }
}
static void t_u64(uint64_t v) {
    uint8_t be[8];
    for (int i = 0; i < 8; i++) be[i] = (uint8_t)(v >> (56 - 8 * i));
    t_hex(be, 8);
}

static FxObj *obj_of(uint32_t h) { return (h >= 1 && h <= fx.nobj && fx.obj[h - 1].live) ? &fx.obj[h - 1] : NULL; }

/* aien-gpu.ko's answer to one request; 0 or -errno as the syscall would return it. */
static long fx_ioctl(unsigned long req, void *arg) {
    if (req == AGPU_INFO) {
        struct agpu_info *i = arg;
        memset(i, 0, sizeof *i);
        i->abi = AGPU_ABI_VERSION; i->state = AGPU_STATE_RM_READY;
        return 0;
    }
    if (req == AGPU_MEM_ALLOC) {
        struct agpu_mem_alloc *a = arg;
        if (a->size == 0 || (a->size & 4095u) || (a->va & 4095u) || fx.nobj >= FX_OBJS ||
            fx.bump + a->size > FX_BYTES) { a->st.verdict = AGPU_RM_E_ARG; return 0; }
        FxObj *o = &fx.obj[fx.nobj++];
        *o = (FxObj){ .off = fx.bump, .size = a->size, .flags = a->flags, .live = 1 };
        fx.bump = (fx.bump + a->size + 0xffffu) & ~0xffffull;
        a->handle = fx.nobj; a->mmap_off = o->off; a->st.verdict = AGPU_RM_OK;
        return 0;
    }
    if (req == AGPU_MEM_FREE) {
        struct agpu_mem_free *f = arg;
        FxObj *o = obj_of(f->handle);
        if (!o) { f->st.verdict = AGPU_RM_E_ARG; return 0; }
        o->live = 0; fx.frees++; f->st.verdict = AGPU_RM_OK;
        return 0;
    }
    if (req == AGPU_CHAN_CREATE) {
        struct agpu_chan_create *c = arg;
        if (fx.chan_live || !obj_of(c->ring_handle) || !obj_of(c->userd_handle) || !obj_of(c->err_handle)) {
            c->st.verdict = AGPU_RM_E_ARG; return 0;
        }
        c->chid = 7; c->token = FX_TOKEN; c->doorbell_off = FX_DOORBELL_OFF; c->channel = 77; c->st.verdict = AGPU_RM_OK;
        fx.chan_live = 1; fx.chan_creates++;
        return 0;
    }
    if (req == AGPU_CHAN_DESTROY) {
        struct agpu_chan_destroy *d = arg;
        if (!fx.chan_live || d->channel != 77) { d->st.verdict = AGPU_RM_E_ARG; return 0; }
        fx.chan_live = 0; fx.chan_destroys++; d->st.verdict = AGPU_RM_OK;
        return 0;
    }
    return -ENOTTY;
}

static void on_sigsys(int sig, siginfo_t *si, void *uc_) {
    (void)sig;
    ucontext_t *uc = uc_;
    if (si->si_syscall != __NR_ioctl) { uc->uc_mcontext.regs[0] = (uint64_t)-ENOSYS; return; }
    const int fd = (int)uc->uc_mcontext.regs[0];
    const unsigned long req = (unsigned long)(uint32_t)uc->uc_mcontext.regs[1];
    void *arg = (void *)(uintptr_t)uc->uc_mcontext.regs[2];
    struct stat st;
    fx.traps++;
    long rc;
    if (fstat(fd, &st) != 0 || st.st_ino != fx.ino) rc = -EBADF;
    else {
        t_str("ioctl "); t_u64(req); t_str(" in "); t_hex(arg, _IOC_SIZE(req));
        rc = fx_ioctl(req, arg);
        t_str(" rc "); t_u64((uint64_t)rc); t_str(" out "); t_hex(arg, _IOC_SIZE(req)); t_str("\n");
    }
    uc->uc_mcontext.regs[0] = (uint64_t)rc;
}

static void trap_aien_ioctls(void) {
    static uint8_t altstack[1u << 16];
    stack_t ss = { .ss_sp = altstack, .ss_size = sizeof altstack };
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_sigaction = on_sigsys;
    sa.sa_flags = SA_SIGINFO | SA_ONSTACK;
    if (sigaltstack(&ss, NULL) != 0 || sigaction(SIGSYS, &sa, NULL) != 0) { perror("sigaction"); exit(2); }
    struct sock_filter f[] = {
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS, offsetof(struct seccomp_data, nr)),
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_ioctl, 0, 4),
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS, offsetof(struct seccomp_data, args[1])), /* low word, little endian */
        BPF_STMT(BPF_ALU | BPF_AND | BPF_K, 0xff00u),
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, (uint32_t)'A' << 8, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_TRAP),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
    };
    struct sock_fprog prog = { .len = (unsigned short)(sizeof f / sizeof f[0]), .filter = f };
    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) != 0 || prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog) != 0) {
        perror("seccomp"); exit(2);
    }
}

/* Every nonzero 32-byte line of the fake device, by file offset. */
static void trace_device(void) {
    uint8_t line[32];
    static const uint8_t zero[32];
    for (uint64_t off = 0; off < fx.bump; off += sizeof line) {
        if (pread(fx.memfd, line, sizeof line, (off_t)off) != (ssize_t)sizeof line) { ok = 0; return; }
        if (memcmp(line, zero, sizeof line) == 0) continue;
        t_str("dev "); t_u64(off); t_str(" "); t_hex(line, sizeof line); t_str("\n");
    }
}

static int device_still_mapped(void) {
    FILE *m = fopen("/proc/self/maps", "r");
    char l[512];
    int n = 0;
    if (!m) return -1;
    while (fgets(l, sizeof l, m)) n += strstr(l, "memfd:aien-fx") != NULL;
    fclose(m);
    return n;
}

static OmegaGpuBufferDesc desc(uint32_t kind, uint32_t rights, uint32_t cache, uint64_t bytes) {
    OmegaGpuBufferDesc d = { .kind = kind, .rights = rights, .gpu_cache = cache, .align = 0, .bytes = bytes };
    return d;
}
#define RW (OMEGA_GPU_RIGHT_CPU_READ | OMEGA_GPU_RIGHT_CPU_WRITE | OMEGA_GPU_RIGHT_GPU_READ | OMEGA_GPU_RIGHT_GPU_WRITE)

static int run(void) {
    fx.memfd = memfd_create("aien-fx", MFD_CLOEXEC);
    struct stat st;
    if (fx.memfd < 0 || ftruncate(fx.memfd, FX_BYTES) != 0 || fstat(fx.memfd, &st) != 0) { perror("memfd"); return 2; }
    fx.ino = st.st_ino;
    fx.bump = FX_FIRST_OBJ; /* [0, 64 KiB): the usermode (doorbell) page */
    char path[64];
    snprintf(path, sizeof path, "/proc/self/fd/%d", fx.memfd);
    setenv("CUDAPP_AIEN_GPU_DEV", path, 1);
    setenv("CUDAPP_MEM_RESERVE_MIB", "0", 1);
    trap_aien_ioctls();

    const OmegaGpuPlatform *p = omega_gpu_platform_aien(NULL);
    OmegaGpuDeviceHandle dev;
    OmegaGpuBufferHandle ring, marker, cached;
    OmegaCpuView ring_cpu, marker_cpu, cached_cpu;
    OmegaGpuVa ring_va, marker_va, cached_va;
    OmegaGpuQueueHandle q;
    OmegaGpuBufferDesc dr = desc(OMEGA_GPU_HANDLE_BUF_QUEUE, OMEGA_GPU_RIGHT_CPU_WRITE | OMEGA_GPU_RIGHT_GPU_READ,
                                 OMEGA_GPU_CACHE_GPU_UNCACHED, 0x10000);
    OmegaGpuBufferDesc dm = desc(OMEGA_GPU_HANDLE_BUF_DATA, RW | OMEGA_GPU_RIGHT_HOST_POLLED, OMEGA_GPU_CACHE_GPU_UNCACHED, 4096);
    OmegaGpuBufferDesc dc = desc(OMEGA_GPU_HANDLE_BUF_DATA, OMEGA_GPU_RIGHT_CPU_WRITE | OMEGA_GPU_RIGHT_GPU_READ,
                                 OMEGA_GPU_CACHE_GPU_CACHED, 4096);
    int r = p->device_open(p->ctx, 0, &dev);
    if (r != OMEGA_GPU_PLAT_OK) { printf("  device_open %d: %s\n", r, p->diag_text(p->ctx)); return 1; }
    EXPECT(p->dma_alloc(p->ctx, dev, &dr, &ring, &ring_cpu, &ring_va, NULL) == OMEGA_GPU_PLAT_OK);
    EXPECT(p->dma_alloc(p->ctx, dev, &dm, &marker, &marker_cpu, &marker_va, NULL) == OMEGA_GPU_PLAT_OK);
    EXPECT(p->dma_alloc(p->ctx, dev, &dc, &cached, &cached_cpu, &cached_va, NULL) == OMEGA_GPU_PLAT_OK);
    r = p->queue_open(p->ctx, dev, ring, &q);
    if (r != OMEGA_GPU_PLAT_OK) { printf("  queue_open %d: %s\n", r, p->diag_text(p->ctx)); return 1; }
    /* the CPU view is the device file: a store through it is in the memfd */
    if (marker_cpu.ptr) *(volatile uint32_t *)marker_cpu.ptr = 0x5eedf00du;
    const uint32_t w[8] = { 0x2005005cu, (uint32_t)marker_va.va, (uint32_t)(marker_va.va >> 32), 1u, 0u, 0x00100001u,
                            0x20010040u, 0x0000a5a5u };
    EXPECT(p->queue_submit(p->ctx, q, w, 6) == OMEGA_GPU_PLAT_OK);
    t_str("--- after submit 1\n"); trace_device();
    EXPECT(p->sync_cpu_to_gpu(p->ctx, cached, 0, 4096) == OMEGA_GPU_PLAT_OK); /* GPU-cached: the next submit has the prefix */
    EXPECT(p->queue_submit(p->ctx, q, w, 8) == OMEGA_GPU_PLAT_OK);
    t_str("--- after submit 2 (L2 prefix)\n"); trace_device();
    EXPECT(p->queue_submit(p->ctx, q, w + 6, 2) == OMEGA_GPU_PLAT_OK);
    t_str("--- after submit 3\n"); trace_device();
    EXPECT(p->queue_retire(p->ctx, q) == OMEGA_GPU_PLAT_OK);
    EXPECT(p->dma_free(p->ctx, cached) == OMEGA_GPU_PLAT_OK);
    EXPECT(p->queue_close(p->ctx, q) == OMEGA_GPU_PLAT_OK);
    EXPECT(p->dma_free(p->ctx, marker) == OMEGA_GPU_PLAT_OK);
    EXPECT(p->dma_free(p->ctx, ring) == OMEGA_GPU_PLAT_OK);
    EXPECT(p->device_close(p->ctx, dev) == OMEGA_GPU_PLAT_OK);
    uint32_t live = 0;
    for (uint32_t i = 0; i < fx.nobj; i++) live += (uint32_t)fx.obj[i].live;
    EXPECT(fx.nobj == 5 && live == 0 && fx.frees == 5); /* 3 buffers, the GPFIFO buffer, the notifier */
    EXPECT(fx.chan_creates == 1 && fx.chan_destroys == 1 && fx.chan_live == 0);
    EXPECT(device_still_mapped() == 0);
#if !CUDAPP_MIGRATION_PENDING_C_PLATFORM_AIEN_EFFECTS
    {
        /* An operator request is not in the unit's allowlist: refused with -22, no ioctl reaches the kernel. */
        uint8_t blk[_IOC_SIZE(AGPU_BOOT)];
        uint64_t res = 0;
        memset(blk, 0, sizeof blk);
        const int dfd = open(path, O_RDWR | O_CLOEXEC);
        const uint32_t traps = fx.traps;
        EXPECT(dfd >= 0 && omega2_aiengpu_ioctl(&res, (uint64_t)dfd, AGPU_BOOT, blk, sizeof blk) == 0 &&
               (int64_t)res == -EINVAL && fx.traps == traps);
        if (dfd >= 0) close(dfd);
    }
#endif
    t_str("--- closed\n"); trace_device();
    const char *tp = getenv("AIEN_FX_TRACE_FD");
    if (tp) {
        if (strcmp(tp, "3") != 0 || write(3, tbuf, tlen) != (ssize_t)tlen) { perror("trace"); return 2; }
    }
    return ok ? 0 : 1;
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    /* the seccomp filter cannot be removed, so the case runs in a child */
    const pid_t pid = fork();
    if (pid == 0) _exit(run());
    int status = 0;
    if (pid < 0 || waitpid(pid, &status, 0) != pid) { perror("fork"); return 2; }
    const int rc = WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status);
    printf("%s aien-gpu effects fixture (%s): rc %d\n", rc == 0 ? "PASS" : "FAIL",
           CUDAPP_MIGRATION_PENDING_C_PLATFORM_AIEN_EFFECTS ? "C system calls" : "native/aiengpu/effects.om", rc);
    return rc;
}
