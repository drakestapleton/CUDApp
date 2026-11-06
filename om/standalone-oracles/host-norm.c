/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Verification only: execute the unchanged emitted norm IR in WsSim. No GPU,
 * driver, ioctl, device file or physical address participates in this adapter. */
#include "omega_gpu_session.h"
#include "omega_blackwell_codegen_dec.h"
#include "bw_warp_sim_k.h"
#include <stdlib.h>
#include <string.h>
static const BlackwellIRProgram *norm_program;
static OmegaGpuMem data;
int __real_omega_blackwell_codegen_dec_prog(const OmegaDecSpec *,BlackwellIRProgram *);
int __wrap_omega_blackwell_codegen_dec_prog(const OmegaDecSpec *s,BlackwellIRProgram *p) {
    int rc=__real_omega_blackwell_codegen_dec_prog(s,p);norm_program=p;return rc;
}
void __wrap_omega_gpu_session_lock(void) {}
void __wrap_omega_gpu_session_unlock(void) {}
OmegaGpuSession *__wrap_omega_gpu_session_open(void) {return (OmegaGpuSession *)(uintptr_t)1;}
static int allocate(size_t n,OmegaGpuMem *m) {memset(m,0,sizeof *m);m->cpu=calloc(1,n);m->size=n;m->va=(uintptr_t)m->cpu;return m->cpu?0:-1;}
int __wrap_omega_gpu_session_alloc_code(size_t n,OmegaGpuMem *m) {return allocate(n,m);}
int __wrap_omega_gpu_session_alloc(size_t n,OmegaGpuMem *m) {int rc=allocate(n,m);data=*m;return rc;}
int __wrap_omega_gpu_session_sync_to_gpu(const OmegaGpuMem *m) {(void)m;return 0;}
int __wrap_omega_gpu_session_sync_to_cpu(const OmegaGpuMem *m) {(void)m;return 0;}
void __wrap_omega_gpu_session_free(OmegaGpuMem *m) {free(m->cpu);memset(m,0,sizeof *m);}
int __wrap_omega_gpu_session_launch(const OmegaGpuLaunch *l,uint64_t *ns,uint32_t *marker) {
    (void)ns;(void)marker;static WsSim sim;static unsigned char shared[65536];
    memset(&sim,0,sizeof sim);sim.p=norm_program;sim.threads=l->threads_x;sim.smem=shared;sim.smem_bytes=sizeof shared;
    sim.nranges=1;sim.ranges[0].lo=data.va;sim.ranges[0].hi=data.va+data.size;
    memcpy(sim.cbank+0x380/4,l->args,l->n_args*sizeof(uint32_t));
    int rc=ws_run(&sim,l->grid_x,l->grid_y);
    if(rc) {fprintf(stderr,"host norm simulation: %s\n",sim.msg);return rc;}
    if(getenv("SO_HOST_RED")) {
        unsigned i=(OMEGA_DEC_RES_Y-0x380)/4;
        uintptr_t p=(uintptr_t)l->args[i]|((uint64_t)l->args[i+1]<<32);
        *(float *)p=INFINITY;
    }
    return 0;
}
