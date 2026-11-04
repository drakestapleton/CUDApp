/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Host-only instrument: execute the actual transport with fake device operations. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/omega_gpu_gdn_api.c"

typedef struct { unsigned n; OmegaGpuLaunch launch[3]; uint32_t args[3][32]; } FakeList;
static OmegaGpuSession session;
static OmegaGpuGdn *owner;
static int depth, failure, creates, frees, adds, submits, reads, ordinary;
static size_t offsets[PF_N];
void __wrap_omega_gpu_session_lock(void) { assert(depth++ == 0); }
void __wrap_omega_gpu_session_unlock(void) { assert(--depth == 0); }
OmegaGpuSession *__wrap_omega_gpu_session_open(void) { assert(depth == 1); return &session; }
int __wrap_omega_gpu_session_on_close(void (*f)(void)) { (void)f; return 0; }
int __wrap_omega_gpu_session_scratch(OmegaGpuScratch *s, size_t bytes) {
    assert(depth == 1 && s == &owner->pf && s->cap >= bytes);
    return failure == 2 ? -1 : 0;
}
void __wrap_omega_gpu_session_scratch_free(OmegaGpuScratch *s) { assert(depth == 1); free(s->mem.cpu); memset(s,0,sizeof *s); }
int __wrap_omega_gpu_session_sync_to_gpu(const OmegaGpuMem *m) { assert(depth == 1 && m->cpu); return failure == 3 ? -1 : 0; }
int __wrap_omega_gpu_session_sync_to_cpu(const OmegaGpuMem *m) { assert(depth == 1 && m->cpu); reads++; return failure == 13 ? -1 : 0; }
int __wrap_omega_gpu_cmdlist_create(uint32_t cap, OmegaGpuCmdList **out) {
    assert(depth == 0 && cap == 3); creates++;
    if (failure == 1) return -1;
    FakeList *fl = calloc(1, sizeof *fl); assert(fl); *out = (void *)fl;
    /* Model another caller installing after this caller released the lock. */
    if (failure == 18) { FakeList *other = calloc(1, sizeof *other); assert(other); owner->pf_batch = (void *)other; creates++; }
    return 0;
}
void __wrap_omega_gpu_cmdlist_free(OmegaGpuCmdList *cl) { assert(depth == 0); if (cl) { frees++; free(cl); } }
void __wrap_omega_gpu_cmdlist_reset(OmegaGpuCmdList *cl) { assert(depth == 1 && cl); ((FakeList *)cl)->n=0; }
uint32_t __wrap_omega_gpu_cmdlist_count(const OmegaGpuCmdList *cl) {
    assert(depth == 1 && cl); unsigned n=((const FakeList *)cl)->n;
    return failure==16 && n==2 ? 1 : n;
}
int __wrap_omega_gpu_cmdlist_add(OmegaGpuCmdList *cl, const OmegaGpuLaunch *l) {
    FakeList *f=(void *)cl;assert(depth == 1 && f->n<3);adds++;
    unsigned i=f->n;
    const unsigned words[3]={OMEGA_GDN_PREP_ARGS_WORDS,OMEGA_GDN_SCAN_ARGS_WORDS,OMEGA_GDN_POST_ARGS_WORDS};
    assert(l->n_args==words[i] && l->n_args<=32);
    assert(l->code_va==g_k.slot[KK_PREP+i].code.va);
    if (failure==(int)i+4 || failure==15) return -1;
    f->launch[i]=*l;memcpy(f->args[i],l->args,l->n_args*4);f->launch[i].args=f->args[i];f->n++;
    return (int)i;
}
static void complete_output(void) {
    float *out=(void *)((unsigned char *)owner->pf.mem.cpu+offsets[PF_OUT]);
    for(size_t i=0;i<3*owner->sh.vdim;i++) if (failure!=14 || i!=7) out[i]=1.25f;
}
int __wrap_omega_gpu_cmdlist_submit(OmegaGpuCmdList *cl,const OmegaGpuCmdListOpts *o,OmegaGpuCmdListStats *st) {
    FakeList *f=(void *)cl;assert(depth == 1 && f->n==3);submits++;
    assert(o->stamps==0 || o->stamps==1);assert(!o->skip_launch && !o->first_barrier_none && !o->mode && !o->barrier);
    complete_output();st->launches=failure==10?2:3;st->sems_done=failure==8?2:3;st->first_missing=failure==9?1:-1;
    return failure==7?-1:0;
}
int __wrap_omega_gpu_cmdlist_stamps(const OmegaGpuCmdList *cl,uint64_t *ts,uint32_t cap) {
    assert(depth == 1 && cl && cap==4);if(failure==11)return -1;
    ts[0]=100;ts[1]=110;ts[2]=failure==12?109:130;ts[3]=160;return 4;
}
int __wrap_omega_gpu_session_launch(const OmegaGpuLaunch *l,uint64_t *ns,uint32_t *marker) {
    (void)marker;assert(depth == 1 && l);ordinary++;
    if(l->n_args==OMEGA_GDN_POST_ARGS_WORDS)complete_output();
    if(ns)*ns=7;
    return 0;
}
static OmegaGpuGdn *make_owner(void) {
    OmegaGpuGdn *g=calloc(1,sizeof *g);assert(g);g->layers=1;assert(shape_of(0,&g->sh)==0);
    g->par.va=4096;g->tail.va=8192;g->state.va=12288;
    size_t bytes=pf_layout(&g->sh,3,offsets);g->pf.mem.cpu=calloc(1,bytes);assert(g->pf.mem.cpu);
    g->pf.mem.va=65536;g->pf.mem.size=bytes;g->pf.cap=bytes;return g;
}
static void run(unsigned mode,int fail,int timed) {
    failure=fail;adds=submits=reads=ordinary=0;
    float *q=calloc(3*owner->sh.conv,4),*z=calloc(3*owner->sh.vdim,4),*ab=calloc(6*owner->sh.vh,4),*out=malloc(3*owner->sh.vdim*4);
    assert(q&&z&&ab&&out);for(size_t i=0;i<3*owner->sh.vdim;i++)out[i]=23.5f;
    OmegaGpuGdnPfInfo info;memset(&info,0,sizeof info);
    int rc=omega_gpu_gdn_prefill_scoped(owner,0,3,q,z,ab,1e-6f,1e-6f,1,1,out,timed?&info:NULL,mode);
    const int good=fail==0 || fail==18;
    assert(depth==0 && (rc==0)==good);
    for(size_t i=0;i<3*owner->sh.vdim;i++)assert(out[i]==(good?1.25f:23.5f));
    if(good) {
        assert(reads==1);
        if(mode) {assert(adds==3&&submits==1&&!ordinary);if(timed)assert(info.elapsed_ns[0]==10&&info.elapsed_ns[1]==20&&info.elapsed_ns[2]==30);}
        else assert(!adds&&!submits&&ordinary==3);
    }
    if(fail>=4&&fail<=6)assert(adds==fail-3&&!submits&&!reads);
    if(fail>=7&&fail<=12)assert(!reads);
    if(fail==16)assert(adds==2&&!submits&&!reads);
    free(q);free(z);free(ab);free(out);
}
int main(void) {
    for(unsigned k=0;k<KK_N;k++) { g_k.slot[k].valid=true;g_k.slot[k].kernel.gpr_count=8;g_k.slot[k].code.va=131072+k*4096; }
    owner=make_owner();run(0,0,1);assert(!creates);
    run(1,0,1);assert(creates==1);run(1,0,0);assert(creates==1);
    for(int f=2;f<=16;f++)run(1,f,1);
    float x=0;int before=creates;
    assert(omega_gpu_gdn_prefill_scoped(owner,0,3,&x,&x,&x,1,1,1,1,&x,NULL,2)==OMEGA_GPU_GDN_BAD_ARGS);
    assert(omega_gpu_gdn_prefill_scoped(owner,0,2,&x,&x,&x,1,1,1,1,&x,NULL,1)==OMEGA_GPU_GDN_BAD_ARGS);
    assert(omega_gpu_gdn_prefill_scoped(owner,1,3,&x,&x,&x,1,1,1,1,&x,NULL,1)==OMEGA_GPU_GDN_BAD_ARGS);
    assert(creates==before);omega_gpu_gdn_free(owner);assert(frees==1);
    owner=make_owner();run(1,1,1);assert(!owner->pf_batch);run(1,18,1);assert(frees==2);omega_gpu_gdn_free(owner);assert(frees==3);
    puts("PASS actual GDN batch transport: bypass, order, reuse, no-timing, operation/completion/stamp/readback failures, poison, stale list, concurrent installation, cleanup");
    return 0;
}
