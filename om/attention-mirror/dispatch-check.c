/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Host-only integration instrument: execute the actual adapter with fake device transport. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/omega_gpu_prefill_attn256_api.c"

static int depth, n, fail, allocs, copies;
static unsigned events[16];
static OmegaGpuDecodeAttn256CacheView cv;
static OmegaGpuSession fake_session;
static unsigned current_m;
static void event(unsigned x) { assert(depth == 1 && n < 16); events[n++] = x; }
void __wrap_omega_gpu_session_lock(void) { assert(depth++ == 0); }
void __wrap_omega_gpu_session_unlock(void) { assert(--depth == 0); }
OmegaGpuSession *__wrap_omega_gpu_session_open(void) { assert(depth == 1); return &fake_session; }
int __wrap_omega_gpu_session_on_close(void (*f)(void)) { (void)f; return fail == 9 ? -1 : 0; }
int __wrap_omega_gpu_decode_attn256_cache_view(const OmegaGpuDecodeAttn256Cache *c, uint32_t layer, OmegaGpuDecodeAttn256CacheView *v) {
    assert(c && layer == 0); *v = cv; return 0;
}
int __wrap_omega_gpu_session_sync_to_gpu(const OmegaGpuMem *m) { assert(depth == 1 && m->cpu); return 0; }
int __wrap_omega_gpu_session_sync_to_cpu(const OmegaGpuMem *m) { assert(depth == 1 && m->cpu); return 0; }
void __wrap_omega_gpu_session_free(OmegaGpuMem *m) { assert(depth == 1); free(m->cpu); memset(m, 0, sizeof *m); }
int __wrap_omega_gpu_pf_alloc_dev_locked(size_t bytes, OmegaGpuMem *m) {
    event(1); allocs++;
    assert(bytes == 2*(cv.v_va-cv.k_va));
    if (fail == 1) return -1;
    memset(m, 0, sizeof *m); m->va = 0x100123450ull; m->size = bytes; return 0;
}
int __wrap_omega_gpu_pf_copy_va_locked(uint64_t dst, uint64_t src, size_t bytes, uint64_t *ns) {
    event(3); copies++;
    assert(n >= 2 && events[n-2] == 2);
    assert(dst == g.mirror.va && src == cv.k_va && bytes == 2*(cv.v_va-cv.k_va));
    if (ns) *ns = 17;
    return fail == 3 ? -1 : 0;
}
static uint64_t ptr(const uint32_t *a, unsigned i) { return a[i] | (uint64_t)a[i+1]<<32; }
int __wrap_omega_gpu_session_launch(const OmegaGpuLaunch *l, uint64_t *ns, uint32_t *marker) {
    (void)marker;
    unsigned action = l->n_args == OMEGA_DATTN256_ARGS_WORDS ? 2 : 4;
    event(action);
    if (ns) *ns = action;
    if (action == 2) { assert(ptr(l->args,2) == cv.k_va && ptr(l->args,4) == cv.v_va); }
    else {
        assert(l->n_args == OMEGA_PATTN256_ARGS_WORDS);
        const int copied = n >= 2 && events[n-2] == 3;
        assert(ptr(l->args,2) == (copied ? g.mirror.va : cv.k_va));
        assert(ptr(l->args,4) == (copied ? g.mirror.va + cv.v_va-cv.k_va : cv.v_va));
        for (size_t i = 0; i < (size_t)current_m*cv.out_words; i++) ((float *)g.out.cpu)[i] = 1.0f;
    }
    return fail == (int)action ? -1 : 0;
}
static void run(unsigned mode, unsigned m, const unsigned *want, size_t count, int failure) {
    float *q = calloc((size_t)m*cv.qkv_words,4), *out = calloc((size_t)m*cv.out_words,4);
    assert(q && out); n = 0; fail = failure; current_m = m; uint64_t copy_ns = 99;
    OmegaGpuPrefillAttn256Info info;
    int rc = omega_gpu_prefill_attn256_scoped((void *)1,0,0,m,q,1e-6f,1.0f,out,&info,mode,&copy_ns);
    assert(depth == 0); assert((rc != 0) == (failure != 0)); assert((size_t)n == count);
    for (size_t i = 0; i < count; i++) assert(events[i] == want[i]);
    if (!rc) { assert(out[0] == 1.0f && out[(size_t)m*cv.out_words-1] == 1.0f); assert(copy_ns == ((mode==1 || (mode==2 && m>=1024)) ? 17u : 0u)); }
    free(q); free(out);
}
int main(void) {
    cv.max_pos=8192;cv.q_heads=24;cv.kv_heads=4;cv.q_per_cta=2;cv.qkv_words=8192;cv.out_words=6144;
    cv.k_va=0x200345670ull;cv.v_va=cv.k_va+(uint64_t)cv.max_pos*4*OMEGA_DATTN256_ROW_BYTES+OMEGA_DATTN256_PLANE_SLACK;
    uint32_t sh[3]={cv.q_heads,cv.kv_heads,cv.q_per_cta};
    for(unsigned k=0;k<KK_N;k++){spec_of(k,sh,&g.slot[k]);g.slot[k].valid=true;g.slot[k].kernel.gpr_count=8;g.slot[k].code.va=0x300000000ull+k*4096;}
    g.have_qf=true;g.qf_size=qf_bytes(1024,24);g.qf.va=0x400000000ull;
    g.have_bufs=true;g.qkv_size=1024*8192*4;g.out_size=1024*6144*4;g.qkv.cpu=calloc(1,g.qkv_size);g.out.cpu=calloc(1,g.out_size);assert(g.qkv.cpu&&g.out.cpu);g.qkv.va=0x500000000ull;g.out.va=0x600000000ull;
    const unsigned small[]={2,4},first[]={1,2,3,4},reuse[]={2,3,4},prep_fail[]={2},copy_fail[]={2,3},alloc_fail[]={1};
    run(2,30,small,2,0);assert(!allocs&&!copies);
    run(2,1024,first,4,0);assert(allocs==1&&copies==1);
    run(1,30,reuse,3,0);assert(allocs==1&&copies==2);
    run(0,30,small,2,0);run(2,30,small,2,0);
    run(1,30,prep_fail,1,2);run(1,30,copy_fail,2,3);run(1,30,reuse,3,4);
    __wrap_omega_gpu_session_lock();__wrap_omega_gpu_session_free(&g.mirror);__wrap_omega_gpu_session_unlock();
    run(1,30,alloc_fail,1,1);
    g.hooked=false;run(1,30,small,0,9);fail=0;
    float q[1]={0},out[1]={0};n=0;
    assert(omega_gpu_prefill_attn256_scoped((void*)1,0,0,30,q,1e-6f,1,out,NULL,3,NULL)==OMEGA_GPU_DATTN_BAD_ARGS);assert(!n);
    cv.q_heads=23;assert(omega_gpu_prefill_attn256_scoped((void*)1,0,0,30,q,1e-6f,1,out,NULL,1,NULL)==OMEGA_GPU_DATTN_BAD_ARGS);assert(!n);
    cv.q_heads=24;cv.v_va-=16;assert(omega_gpu_prefill_attn256_scoped((void*)1,0,0,30,q,1e-6f,1,out,NULL,1,NULL)==OMEGA_GPU_DATTN_BAD_ARGS);assert(!n);
    omega_gpu_prefill_attn256_release();assert(!g.mirror.size);puts("PASS actual attention adapter: scoped order, reuse, bypass, isolation, geometry and operation failures");return 0;
}
