/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* omega_gpu_prefill.c: device-resident prefill buffers, kernel cache and launch helper (OWN-30B T3b). See the header. */
#include "omega_gpu_prefill.h"
#include "omega_blackwell_codegen_pf.h"
#include "omega_blackwell_codegen_tc.h"
#include "omega_blackwell_codegen_qmma.h"
#include "omega_gpu_platform.h"
#include "omega_gpu_fmt.h"
#include "omega_gpu_cmdlist.h"
#include "omega_gpu_graph_om.h"
#include "omega_gpu_graph_residue.h"
#include <string.h>

#define PF_SLOTS 64
#define PF_STAGE_BYTES (64ull << 20)
#define PF_TIMEOUT_MS 20000ull
#define PF_SPIN_US 200000u /* spin on the marker: prefill launches are back to back, a sleep-poll costs ~100 us each */

typedef struct { bool valid; OmegaPfKey key; OmegaPfKernel k; OmegaGpuMem code; } Slot;
static struct {
    bool hooked;
    Slot slot[PF_SLOTS];
    OmegaGpuScratch stage;
    uint64_t chip_ns, launches;
} g;

const char *omega_gpu_pf_rc_name(int rc) {
    switch (rc) {
    case OMEGA_GPU_PF_OK: return "OK";
    case OMEGA_GPU_PF_BAD_ARGS: return "BAD_ARGS";
    case OMEGA_GPU_PF_TOO_LARGE: return "TOO_LARGE";
    case OMEGA_GPU_PF_CODEGEN_FAIL: return "CODEGEN_FAIL";
    case OMEGA_GPU_PF_CHIP_FAIL: return "CHIP_FAIL";
    default: return "UNKNOWN";
    }
}

static void on_close(void) {
    for (unsigned i = 0; i < PF_SLOTS; i++) if (g.slot[i].valid) { omega_gpu_session_free(&g.slot[i].code); memset(&g.slot[i], 0, sizeof g.slot[i]); }
    omega_gpu_session_scratch_free(&g.stage);
}
int omega_gpu_pf_open_locked(void) {
    if (!g.hooked) { if (omega_gpu_session_on_close(on_close) != 0) { omega_gpu_session_set_error("prefill: no session hook slot left"); return OMEGA_GPU_PF_CHIP_FAIL; } g.hooked = true; }
    return omega_gpu_session_open() ? OMEGA_GPU_PF_OK : OMEGA_GPU_PF_CHIP_FAIL;
}

void omega_gpu_pf_stats(uint64_t *chip_ns, uint64_t *launches) { if (chip_ns) *chip_ns = g.chip_ns; if (launches) *launches = g.launches; }
void omega_gpu_pf_stats_reset(void) { g.chip_ns = 0; g.launches = 0; }

int omega_gpu_pf_alloc_dev_locked(size_t bytes, OmegaGpuMem *out) {
    if (!out || bytes == 0) return OMEGA_GPU_PF_BAD_ARGS;
    memset(out, 0, sizeof *out);
    int rc = omega_gpu_pf_open_locked();
    if (rc == OMEGA_GPU_PF_OK) {
        OmegaGpuSession *s = omega_gpu_session_open();
        const OmegaGpuPlatform *p = omega_gpu_session_platform();
#if CUDAPP_MIGRATION_PENDING_GRAPH_RESIDUE
        const size_t want = (bytes + 0xfffu) & ~(size_t)0xfffu;
#else
        const uint64_t rcfg[16] = {bytes}; uint64_t rout[16];
        const size_t want = omega_graph_meta(4, rcfg, rout);
#endif
        OmegaGpuBufferDesc d = { .kind = OMEGA_GPU_HANDLE_BUF_DATA, .rights = OMEGA_GPU_RIGHT_GPU_READ | OMEGA_GPU_RIGHT_GPU_WRITE,
                                 .gpu_cache = OMEGA_GPU_CACHE_GPU_CACHED, .align = 0x1000u, .bytes = want };
        OmegaCpuView cpu; OmegaGpuVa va; OmegaGpuBusAddr bus;
        int prc = (s && p) ? p->dma_alloc(p->ctx, s->dev, &d, &out->handle, &cpu, &va, &bus) : OMEGA_GPU_PLAT_REFUSED_ARGS;
        if (prc != OMEGA_GPU_PLAT_OK || !cpu.ptr || cpu.len < want) {
            if (prc == OMEGA_GPU_PLAT_OK) (void)p->dma_free(p->ctx, out->handle);
            char e[96]; omega_gpu_fmt(e, sizeof e, "prefill: cached alloc of %zu bytes refused (%d)", want, prc);
            omega_gpu_session_set_error(e); memset(out, 0, sizeof *out); rc = OMEGA_GPU_PF_CHIP_FAIL;
        } else { out->kind = OMEGA_GPU_HANDLE_BUF_DATA; out->cpu = cpu.ptr; out->va = va.va; out->size = cpu.len; }
    }
    return rc;
}
int omega_gpu_pf_alloc_dev(size_t bytes, OmegaGpuMem *out) {
    omega_gpu_session_lock();
    int rc = omega_gpu_pf_alloc_dev_locked(bytes, out);
    omega_gpu_session_unlock();
    return rc;
}
int omega_gpu_pf_alloc_host(size_t bytes, OmegaGpuMem *out) {
    if (!out || bytes == 0) return OMEGA_GPU_PF_BAD_ARGS;
    omega_gpu_session_lock();
    int rc = omega_gpu_pf_open_locked();
    if (rc == OMEGA_GPU_PF_OK && omega_gpu_session_alloc(bytes, out) != 0) rc = OMEGA_GPU_PF_CHIP_FAIL;
    omega_gpu_session_unlock();
    return rc;
}
void omega_gpu_pf_free(OmegaGpuMem *m) { omega_gpu_session_lock(); omega_gpu_session_free(m); omega_gpu_session_unlock(); }

int omega_gpu_pf_kernel_locked(const OmegaPfKey *key, OmegaPfGen gen, OmegaPfKernel *out) {
    for (unsigned i = 0; i < PF_SLOTS; i++) if (g.slot[i].valid && !memcmp(&g.slot[i].key, key, sizeof *key)) { *out = g.slot[i].k; return OMEGA_GPU_PF_OK; }
    Slot *s = NULL;
    for (unsigned i = 0; i < PF_SLOTS && !s; i++) if (!g.slot[i].valid) s = &g.slot[i];
    if (!s) { omega_gpu_session_set_error("prefill kernel cache full"); return OMEGA_GPU_PF_TOO_LARGE; }
    BlackwellIRProgram *prog = omega_gpu_host_calloc(1, sizeof *prog);
    if (!prog) { omega_gpu_session_set_error("prefill codegen: host memory"); return OMEGA_GPU_PF_CODEGEN_FAIL; }
    int cg = gen(key, prog);
    size_t need = cg == 0 ? omega_bw_program_code_bytes(prog) : 0;
    if (cg != 0 || need == 0) { omega_gpu_host_free(prog); omega_gpu_session_set_error("prefill codegen refused the shape"); return OMEGA_GPU_PF_CODEGEN_FAIL; }
    const uint32_t peak = prog->regalloc.peak_gpr_usage, decl = omega_bw_gpr_declared(peak);
    /* Preserve the old 128-register declaration unless codegen explicitly opted
     * into more. The allocator's hardware limit leaves declaration headroom;
     * launch/record below independently enforce registers times threads.
     * Blackwell Tuning Guide, physical page 13: 255/thread, 64K per SM. */
    if (!omega_graph_budget(peak, prog->regalloc.gpr_limit, BW_PHYS_GPR_MAX, BW_PHYS_GPR_HW_LIMIT,
                            omega_bw_gpr_declared_ok(decl, peak))) {
        omega_gpu_host_free(prog); omega_gpu_session_set_error("prefill kernel: register budget exceeded"); return OMEGA_GPU_PF_TOO_LARGE;
    }
    OmegaGpuMem code;
    if (omega_gpu_session_alloc_code(need, &code) != 0) { omega_gpu_host_free(prog); return OMEGA_GPU_PF_CHIP_FAIL; }
    OmegaBlackwellKernel kernel;
    cg = omega_blackwell_kernel_encode_into(prog, code.cpu, need, &kernel);
    if (cg == 0 && omega_qmma_patch_code(prog, code.cpu, need) < 0) cg = -1;
    omega_gpu_host_free(prog);
    if (cg != 0 || kernel.code_size != need) { omega_gpu_session_free(&code); omega_gpu_session_set_error("prefill encode failed"); return OMEGA_GPU_PF_CODEGEN_FAIL; }
    if (omega_gpu_session_sync_to_gpu(&code) != 0) { omega_gpu_session_free(&code); return OMEGA_GPU_PF_CHIP_FAIL; }
    memset(s, 0, sizeof *s);
    s->valid = true; s->key = *key; s->code = code;
    s->k.code_va = code.va; s->k.gpr_peak = kernel.gpr_count; s->k.gpr_declared = omega_bw_gpr_declared(kernel.gpr_count);
    *out = s->k;
    return OMEGA_GPU_PF_OK;
}

#ifndef CUDAPP_MIGRATION_PENDING_C_GRAPHS
static int pf_launch_plan(const OmegaPfKernel *k, uint32_t threads, uint32_t gx, uint32_t gy,
                          uint32_t shared, const uint32_t *args, uint32_t nargs, OmegaGpuLaunch *launch,
                          const char *budget, const char *registers) {
    if (!k) return OMEGA_GPU_PF_BAD_ARGS;
    const uint64_t q[16]={k->code_va,k->gpr_declared,threads,gx,gy,shared,nargs,k->gpr_peak};
    uint64_t d[16],result=0;
    if(omega_mg_lauch(&result,q,16,d,16)!=OMEGA_OK)return OMEGA_GPU_PF_CHIP_FAIL;
    if(result==0)return OMEGA_GPU_PF_BAD_ARGS;
    if(result==2){omega_gpu_session_set_error(budget);return OMEGA_GPU_PF_TOO_LARGE;}
    if(result==3){omega_gpu_session_set_error(registers);return OMEGA_GPU_PF_CHIP_FAIL;}
    *launch=(OmegaGpuLaunch){.code_va=d[0],.gpr_count=d[1],.threads_x=d[2],.threads_y=1,
        .grid_x=d[3],.grid_y=d[4],.shared_bytes=d[5],.n_args=d[6],.num_elements=d[8],
        .timeout_ms=d[9],.spin_us=d[10],.args=args};
    return OMEGA_GPU_PF_OK;
}
#endif
int omega_gpu_pf_launch_locked(const OmegaPfKernel *k, uint32_t threads, uint32_t grid_x, uint32_t grid_y, uint32_t shared_bytes,
                               const uint32_t *args, uint32_t n_args, uint64_t *ns) {
#ifndef CUDAPP_MIGRATION_PENDING_C_GRAPHS
    OmegaGpuLaunch L;
    int rc=pf_launch_plan(k,threads,grid_x,grid_y,shared_bytes,args,n_args,&L,
        "prefill launch: registers x threads exceed the file","declared registers below peak + 2: refusing to launch");
    if(rc!=OMEGA_GPU_PF_OK)return rc;
#else
    if (!k || !threads || !grid_x || !grid_y) return OMEGA_GPU_PF_BAD_ARGS;
    if ((uint64_t)threads * k->gpr_declared > 65536u) { omega_gpu_session_set_error("prefill launch: registers x threads exceed the file"); return OMEGA_GPU_PF_TOO_LARGE; }
    if (!omega_bw_gpr_declared_ok(k->gpr_declared, k->gpr_peak)) { omega_gpu_session_set_error("declared registers below peak + 2: refusing to launch"); return OMEGA_GPU_PF_CHIP_FAIL; }
    OmegaGpuLaunch L = { .code_va = k->code_va, .gpr_count = k->gpr_declared, .threads_x = threads, .threads_y = 1, .grid_x = grid_x, .grid_y = grid_y,
                         .num_elements = (size_t)threads * grid_x * grid_y, .args = args, .n_args = n_args, .timeout_ms = PF_TIMEOUT_MS,
                         .shared_bytes = shared_bytes, .spin_us = PF_SPIN_US };
#endif
    uint64_t t = 0;
    if (omega_gpu_session_launch(&L, &t, NULL) != 0) return OMEGA_GPU_PF_CHIP_FAIL;
    g.chip_ns += t; g.launches++;
    if (ns) *ns += t;
    return OMEGA_GPU_PF_OK;
}

int omega_gpu_pf_record_locked(OmegaGpuCmdList *cl, const OmegaPfKernel *k, uint32_t threads, uint32_t grid_x, uint32_t grid_y,
                               uint32_t shared_bytes, const uint32_t *args, uint32_t n_args) {
#ifndef CUDAPP_MIGRATION_PENDING_C_GRAPHS
    if(!cl)return OMEGA_GPU_PF_BAD_ARGS;
    OmegaGpuLaunch L;
    int rc=pf_launch_plan(k,threads,grid_x,grid_y,shared_bytes,args,n_args,&L,
        "prefill record: registers x threads exceed the file","declared registers below peak + 2: refusing to record");
    if(rc!=OMEGA_GPU_PF_OK)return rc;
#else
    if (!cl || !k || !threads || !grid_x || !grid_y) return OMEGA_GPU_PF_BAD_ARGS;
    if ((uint64_t)threads * k->gpr_declared > 65536u) { omega_gpu_session_set_error("prefill record: registers x threads exceed the file"); return OMEGA_GPU_PF_TOO_LARGE; }
    if (!omega_bw_gpr_declared_ok(k->gpr_declared, k->gpr_peak)) { omega_gpu_session_set_error("declared registers below peak + 2: refusing to record"); return OMEGA_GPU_PF_CHIP_FAIL; }
    OmegaGpuLaunch L = { .code_va = k->code_va, .gpr_count = k->gpr_declared, .threads_x = threads, .threads_y = 1, .grid_x = grid_x, .grid_y = grid_y,
                         .num_elements = (size_t)threads * grid_x * grid_y, .args = args, .n_args = n_args, .timeout_ms = PF_TIMEOUT_MS,
                         .shared_bytes = shared_bytes, .spin_us = PF_SPIN_US };
#endif
    return omega_gpu_cmdlist_add(cl, &L) < 0 ? OMEGA_GPU_PF_CHIP_FAIL : OMEGA_GPU_PF_OK;
}

static int gen_copy(const OmegaPfKey *key, void *prog) { (void)key; return omega_blackwell_codegen_pf_copy_prog(prog); }
#define KIND_COPY 1u
static int copy_locked(uint64_t dst, uint64_t src, size_t bytes) {
#ifndef CUDAPP_MIGRATION_PENDING_C_GRAPHS
    uint64_t q[16]={dst,src,bytes,0,1ull<<30,OMEGA_PF_COPY_THREADS},plan[16],result=0;
    if(omega_mg_copy(&result,q,16,plan,16)!=OMEGA_OK||result==0)return OMEGA_GPU_PF_BAD_ARGS;
    if(result==2)return OMEGA_GPU_PF_OK;
#else
    if ((bytes & 15u) || (dst & 15u) || (src & 15u)) return OMEGA_GPU_PF_BAD_ARGS;
    if (bytes == 0) return OMEGA_GPU_PF_OK;
#endif
    OmegaPfKey key; memset(&key, 0, sizeof key); key.kind = KIND_COPY;
    OmegaPfKernel k; int rc = omega_gpu_pf_kernel_locked(&key, gen_copy, &k);
    for (size_t done = 0; rc == OMEGA_GPU_PF_OK && done < bytes;) {
#ifndef CUDAPP_MIGRATION_PENDING_C_GRAPHS
        q[3]=done;
        if(omega_mg_copy(&result,q,16,plan,16)!=OMEGA_OK||result!=1)return OMEGA_GPU_PF_CHIP_FAIL;
        const size_t part=plan[2];
        const uint32_t units=plan[3],grid=plan[4];
        const uint64_t d=plan[0],s=plan[1];
#else
        size_t part = bytes - done; if (part > (1ull << 30)) part = 1ull << 30;
        const uint32_t units = (uint32_t)(part / 16u), grid = (units + OMEGA_PF_COPY_THREADS - 1u) / OMEGA_PF_COPY_THREADS;
        const uint64_t d = dst + done, s = src + done;
#endif
        uint32_t args[5] = { (uint32_t)d, (uint32_t)(d >> 32), (uint32_t)s, (uint32_t)(s >> 32), units };
        rc = omega_gpu_pf_launch_locked(&k, OMEGA_PF_COPY_THREADS, grid, 1, 0, args, 5, NULL);
        done += part;
    }
    return rc;
}
int omega_gpu_pf_copy_va_locked(uint64_t dst_va, uint64_t src_va, size_t bytes, uint64_t *chip_ns) {
    const uint64_t before = g.chip_ns;
    int rc = omega_gpu_pf_open_locked();
    if (rc == OMEGA_GPU_PF_OK) rc = copy_locked(dst_va, src_va, bytes);
    if (chip_ns) *chip_ns = g.chip_ns - before;
    return rc;
}
int omega_gpu_pf_copy_va(uint64_t dst_va, uint64_t src_va, size_t bytes) {
    omega_gpu_session_lock();
    int rc = omega_gpu_pf_copy_va_locked(dst_va, src_va, bytes, NULL);
    omega_gpu_session_unlock();
    return rc;
}

static int xfer(const OmegaGpuMem *dev, size_t off, void *host, size_t bytes, int up) {
#ifndef CUDAPP_MIGRATION_PENDING_C_GRAPHS
    if(!dev||!host)return OMEGA_GPU_PF_BAD_ARGS;
    uint64_t q[16]={dev->va,off,bytes,dev->size,0,0,up!=0},plan[16],result=0;
    if(omega_mg_xfer(&result,q,16,plan,16)!=OMEGA_OK||result==0)return OMEGA_GPU_PF_BAD_ARGS;
#else
    if (!dev || !host || (off & 15u) || (bytes & 15u) || off + bytes > dev->size) return OMEGA_GPU_PF_BAD_ARGS;
#endif
    omega_gpu_session_lock();
    int rc = omega_gpu_pf_open_locked();
    if (rc == OMEGA_GPU_PF_OK && omega_gpu_session_scratch(&g.stage, PF_STAGE_BYTES) != 0) rc = OMEGA_GPU_PF_CHIP_FAIL;
    for (size_t done = 0; rc == OMEGA_GPU_PF_OK && done < bytes;) {
#ifndef CUDAPP_MIGRATION_PENDING_C_GRAPHS
        q[4]=done;q[5]=g.stage.mem.va;
        if(omega_mg_xfer(&result,q,16,plan,16)!=OMEGA_OK||result!=1){rc=OMEGA_GPU_PF_CHIP_FAIL;break;}
        const size_t part=plan[2];
        const uint64_t dst=plan[0],src=plan[1];
#else
        size_t part = bytes - done; if (part > PF_STAGE_BYTES) part = PF_STAGE_BYTES;
        const uint64_t dst=up?dev->va+off+done:g.stage.mem.va,src=up?g.stage.mem.va:dev->va+off+done;
#endif
        if (up) {
            memcpy(g.stage.mem.cpu, (const uint8_t *)host + done, part);
            if (omega_gpu_session_sync_to_gpu(&g.stage.mem) != 0) rc = OMEGA_GPU_PF_CHIP_FAIL;
            else rc = copy_locked(dst, src, part);
        } else {
            rc = copy_locked(dst, src, part);
            if (rc == OMEGA_GPU_PF_OK && omega_gpu_session_sync_to_cpu(&g.stage.mem) != 0) rc = OMEGA_GPU_PF_CHIP_FAIL;
            if (rc == OMEGA_GPU_PF_OK) memcpy((uint8_t *)host + done, g.stage.mem.cpu, part);
        }
        done += part;
    }
    omega_gpu_session_unlock();
    return rc;
}
int omega_gpu_pf_upload(const OmegaGpuMem *dst, size_t off, const void *src, size_t bytes) { return xfer(dst, off, (void *)src, bytes, 1); }
int omega_gpu_pf_download(const OmegaGpuMem *src, size_t off, void *dst, size_t bytes) { return xfer(src, off, dst, bytes, 0); }
