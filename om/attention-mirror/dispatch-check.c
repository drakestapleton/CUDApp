/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Host-only integration instrument: execute the actual adapter with fake device transport. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/omega_gpu_prefill_attn256_api.c"
#include "../standalone-oracles/oracle.h"

static int depth, n, fail, allocs, copies;
static unsigned events[16];
static OmegaGpuDecodeAttn256CacheView cv;
static OmegaGpuSession fake_session;
static unsigned current_m;
static void event(unsigned x) { assert(so_all(so_eq(depth,1),so_call(34,n,16,0,0),1)); events[n] = x; n=(int)so_call(5,n,1,0,0); }
void __wrap_omega_gpu_session_lock(void) { assert(so_eq(depth,0)); depth=(int)so_call(5,depth,1,0,0); }
void __wrap_omega_gpu_session_unlock(void) { depth=(int)so_call(35,depth,1,0,0); assert(so_eq(depth,0)); }
OmegaGpuSession *__wrap_omega_gpu_session_open(void) { assert(so_eq(depth,1)); return &fake_session; }
int __wrap_omega_gpu_session_on_close(void (*f)(void)) { (void)f; return so_eq(fail,9) ? -1 : 0; }
int __wrap_omega_gpu_decode_attn256_cache_view(const OmegaGpuDecodeAttn256Cache *c, uint32_t layer, OmegaGpuDecodeAttn256CacheView *v) {
    assert(so_all((uintptr_t)c,so_eq(layer,0),1)); *v = cv; return 0;
}
int __wrap_omega_gpu_session_sync_to_gpu(const OmegaGpuMem *m) { assert(so_all(so_eq(depth,1),(uintptr_t)m->cpu,1)); return 0; }
int __wrap_omega_gpu_session_sync_to_cpu(const OmegaGpuMem *m) { assert(so_all(so_eq(depth,1),(uintptr_t)m->cpu,1)); return 0; }
void __wrap_omega_gpu_session_free(OmegaGpuMem *m) { assert(so_eq(depth,1)); free(m->cpu); memset(m, 0, sizeof *m); }
int __wrap_omega_gpu_pf_alloc_dev_locked(size_t bytes, OmegaGpuMem *m) {
    event(1); allocs=(int)so_call(5,allocs,1,0,0);
    assert(so_eq(bytes,so_call(9,cv.k_va,cv.v_va,0,0)));
    if (so_eq(fail,1)) return -1;
    memset(m, 0, sizeof *m); m->va = 0x100123450ull; m->size = bytes; return 0;
}
int __wrap_omega_gpu_pf_copy_va_locked(uint64_t dst, uint64_t src, size_t bytes, uint64_t *ns) {
    event(3); copies=(int)so_call(5,copies,1,0,0);
    assert(so_call(33,n,1,0,0) && so_eq(events[n-2],2));
    assert(so_all(so_eq(dst,g.mirror.va),so_eq(src,cv.k_va),so_eq(bytes,so_call(9,cv.k_va,cv.v_va,0,0))));
    if (ns) *ns = 17;
    return so_eq(fail,3) ? -1 : 0;
}
static uint64_t ptr(const uint32_t *a, unsigned i) { return so_call(8,a[i],a[i+1],0,0); }
int __wrap_omega_gpu_session_launch(const OmegaGpuLaunch *l, uint64_t *ns, uint32_t *marker) {
    (void)marker;
    unsigned action = (unsigned)so_call(41,so_eq(l->n_args,OMEGA_DATTN256_ARGS_WORDS),4,2,0);
    event(action);
    if (ns) *ns = action;
    if (so_eq(action,2)) { assert(so_all(so_eq(ptr(l->args,2),cv.k_va),so_eq(ptr(l->args,4),cv.v_va),1)); }
    else {
        assert(so_eq(l->n_args,OMEGA_PATTN256_ARGS_WORDS));
        const int copied = so_call(33,n,1,0,0) && so_eq(events[n-2],3);
        assert(so_eq(ptr(l->args,2),so_call(41,copied,cv.k_va,g.mirror.va,0)));
        assert(so_eq(ptr(l->args,4),so_call(41,copied,cv.v_va,so_call(5,g.mirror.va,so_call(35,cv.v_va,cv.k_va,0,0),0,0),0)));
        for (size_t i = 0; i < so_bytes(current_m,cv.out_words,0); i++) ((float *)g.out.cpu)[i] = 1.0f;
    }
    return so_eq(fail,action) ? -1 : 0;
}
static void run(unsigned mode, unsigned m, const unsigned *want, size_t count, int failure) {
    float *q = calloc(so_bytes(m,cv.qkv_words,0),4), *out = calloc(so_bytes(m,cv.out_words,0),4);
    assert(q && out); n = 0; fail = failure; current_m = m; uint64_t copy_ns = 99;
    OmegaGpuPrefillAttn256Info info;
    int rc = omega_gpu_prefill_attn256_scoped((void *)1,0,0,m,q,1e-6f,1.0f,out,&info,mode,&copy_ns);
    assert(so_eq(depth,0)); assert(so_call(36,(uint64_t)(int64_t)rc,(uint64_t)(int64_t)failure,0,0)); assert(so_eq(n,count));
    for (size_t i = 0; i < count; i++) assert(so_eq(events[i],want[i]));
    if (!rc) { assert(so_all(so_eq(so_bits(out[0]),so_bits(1.0f)),so_eq(so_bits(out[so_call(35,so_bytes(m,cv.out_words,0),1,0,0)]),so_bits(1.0f)),1)); assert(so_eq(copy_ns,so_call(10,mode,m,0,0))); }
    free(q); free(out);
}
int main(void) {
    cv.max_pos=8192;cv.q_heads=24;cv.kv_heads=4;cv.q_per_cta=2;cv.qkv_words=8192;cv.out_words=6144;
    cv.k_va=0x200345670ull;cv.v_va=so_call(5,cv.k_va,so_call(5,so_bytes(cv.max_pos,4*OMEGA_DATTN256_ROW_BYTES,0),OMEGA_DATTN256_PLANE_SLACK,0,0),0,0);
    uint32_t sh[3]={cv.q_heads,cv.kv_heads,cv.q_per_cta};
    for(unsigned k=0;k<KK_N;k++){spec_of(k,sh,&g.slot[k]);g.slot[k].valid=true;g.slot[k].kernel.gpr_count=8;g.slot[k].code.va=so_call(5,0x300000000ull,so_bytes(k,4096,0),0,0);}
    /* Fixed 1024-row q' fixture: 512 token pairs, 24 heads, 1024 bytes/head. */
    g.have_qf=true;g.qf_size=so_bytes(512,so_bytes(24,1024,0),0);g.qf.va=0x400000000ull;
    g.have_bufs=true;g.qkv_size=so_bytes(1024,so_bytes(8192,4,0),0);g.out_size=so_bytes(1024,so_bytes(6144,4,0),0);g.qkv.cpu=calloc(1,g.qkv_size);g.out.cpu=calloc(1,g.out_size);assert(g.qkv.cpu&&g.out.cpu);g.qkv.va=0x500000000ull;g.out.va=0x600000000ull;
    const unsigned small[]={2,4},first[]={1,2,3,4},reuse[]={2,3,4},prep_fail[]={2},copy_fail[]={2,3},alloc_fail[]={1};
    run(2,30,small,2,0);assert(so_all(so_eq(allocs,0),so_eq(copies,0),1));
    run(2,1024,first,4,0);assert(so_all(so_eq(allocs,1),so_eq(copies,1),1));
    run(1,30,reuse,3,0);assert(so_all(so_eq(allocs,1),so_eq(copies,2),1));
    run(0,30,small,2,0);run(2,30,small,2,0);
    run(1,30,prep_fail,1,2);run(1,30,copy_fail,2,3);run(1,30,reuse,3,4);
    __wrap_omega_gpu_session_lock();__wrap_omega_gpu_session_free(&g.mirror);__wrap_omega_gpu_session_unlock();
    run(1,30,alloc_fail,1,1);
    g.hooked=false;run(1,30,small,0,9);fail=0;
    float q[1]={0},out[1]={0};n=0;
    assert(so_eq((uint64_t)(int64_t)omega_gpu_prefill_attn256_scoped((void*)1,0,0,30,q,1e-6f,1,out,NULL,3,NULL),(uint64_t)(int64_t)OMEGA_GPU_DATTN_BAD_ARGS));assert(so_eq(n,0));
    cv.q_heads=23;assert(so_eq((uint64_t)(int64_t)omega_gpu_prefill_attn256_scoped((void*)1,0,0,30,q,1e-6f,1,out,NULL,1,NULL),(uint64_t)(int64_t)OMEGA_GPU_DATTN_BAD_ARGS));assert(so_eq(n,0));
    cv.q_heads=24;cv.v_va=so_call(35,cv.v_va,16,0,0);assert(so_eq((uint64_t)(int64_t)omega_gpu_prefill_attn256_scoped((void*)1,0,0,30,q,1e-6f,1,out,NULL,1,NULL),(uint64_t)(int64_t)OMEGA_GPU_DATTN_BAD_ARGS));assert(so_eq(n,0));
    omega_gpu_prefill_attn256_release();assert(so_eq(g.mirror.size,0));puts("PASS actual attention adapter: scoped order, reuse, bypass, isolation, geometry and operation failures");return 0;
}
