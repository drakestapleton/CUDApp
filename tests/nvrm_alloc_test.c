/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Host verification oracle: compile twice, selecting the retained original C.
 * The runner compares every operation's result and complete active range state. */
#include "nvrm.c"
#include <assert.h>

static uint32_t rng = 1;
static int red;
static uint32_t random32(void) { rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5; return rng; }
static void emit(Nvrm *rm, uint64_t result) {
    if (red) { result ^= 1; red = 0; }
    fwrite(&result, sizeof result, 1, stdout);
    fwrite(&rm->va_next, sizeof rm->va_next, 1, stdout);
    fwrite(&rm->free_count, sizeof rm->free_count, 1, stdout);
    fwrite(&rm->faulted, sizeof rm->faulted, 1, stdout);
    fwrite(rm->err, sizeof rm->err, 1, stdout);
    fwrite(rm->free_list, sizeof rm->free_list[0], rm->free_count, stdout);
}
static void reset(Nvrm *rm) { memset(rm, 0, sizeof *rm); rm->va_next = 4096; }
static void take(Nvrm *rm, uint64_t size, uint64_t align) { emit(rm, va_take(rm, size, align)); }
static void release(Nvrm *rm, uint64_t va, uint64_t size) { emit(rm, (uint64_t)va_release(rm, va, size)); }
static void allocator(void) {
    Nvrm rm;
    reset(&rm);
    release(&rm, 4097, 8192); take(&rm, 100, 4096); /* both pad and tail */
    take(&rm, 1, 1); take(&rm, 4094, 1); /* exact removal */
    reset(&rm); release(&rm, 100, 10); release(&rm, 120, 10); release(&rm, 110, 10);
    release(&rm, 105, 1); release(&rm, 1, 0); release(&rm, UINT64_MAX, 1);
    const uint64_t sizes[] = {0,1,4095,4096,4097,UINT32_MAX-1,UINT32_MAX,UINT64_C(4294967296),UINT64_MAX-1,UINT64_MAX};
    const uint64_t aligns[] = {0,1,2,3,4095,4096,4097,UINT32_MAX,UINT64_MAX};
    for (size_t i=0;i<sizeof sizes/sizeof *sizes;i++) for(size_t j=0;j<sizeof aligns/sizeof *aligns;j++) {
        reset(&rm); take(&rm,sizes[i],aligns[j]);
        reset(&rm); release(&rm,4097,8192); take(&rm,sizes[i],aligns[j]);
        reset(&rm); rm.va_next=UINT64_MAX; take(&rm,sizes[i],aligns[j]);
    }
    reset(&rm); rm.free_count=NVRM_MAX_FREE-1;
    for(uint32_t i=0;i<rm.free_count;i++) rm.free_list[i]=(NvrmVaRange){4097+(uint64_t)i*16384,8192};
    release(&rm,1,1); release(&rm,3,1);
    reset(&rm); rm.free_count=NVRM_MAX_FREE;
    for(uint32_t i=0;i<NVRM_MAX_FREE;i++) rm.free_list[i]=(NvrmVaRange){4097+(uint64_t)i*16384,8192};
    release(&rm,1,1); take(&rm,1,4096); take(&rm,8192,1);
    reset(&rm); rm.free_count=1; rm.free_list[0]=(NvrmVaRange){UINT64_MAX,2}; take(&rm,1,1);
    for(uint32_t seed=1;seed<=16;seed++) {
        reset(&rm); rng=seed; uint64_t va[128]={0},sz[128]={0};
        for(uint32_t step=0;step<2048;step++) {
            uint32_t slot=random32()%128;
            if(va[slot]) { release(&rm,va[slot],sz[slot]); va[slot]=0; }
            else {
                sz[slot]=1+random32()%16384;
                uint64_t align=UINT64_C(1)<<(random32()%14);
                va[slot]=va_take(&rm,sz[slot],align); emit(&rm,va[slot]);
            }
        }
    }
}
static void methods(void) {
    const uint32_t edges[]={0,1,2,3,4,5,6,7,8,9,8190,8191,8192,8193,32767,32768,32769,65535,65536,65537,UINT32_MAX};
    for(size_t i=0;i<sizeof edges/sizeof *edges;i++) for(size_t j=0;j<sizeof edges/sizeof *edges;j++) for(size_t k=0;k<sizeof edges/sizeof *edges;k++) {
        uint32_t v=nvrm_mthd(edges[i],edges[j],edges[k]);
        uint32_t c=(2u<<28)|(edges[k]<<16)|(edges[i]<<13)|(edges[j]>>2);
        assert(v==c); fwrite(&v,sizeof v,1,stdout);
    }
    Nvrm rm; reset(&rm); rm.discrete=1;
    const uint32_t counts[]={0,1,2,3,4,5,6,8190,8191};
    const uint32_t meth[]={0,36,40,44,88,92,96,536,540,544,688,692,696,700,704,708};
    uint32_t w[8200]={0}; NvrmMem pb={.cpu=w};
    for(uint32_t sub=0;sub<=8;sub++) for(size_t j=0;j<sizeof counts/sizeof *counts;j++) for(size_t k=0;k<sizeof meth/sizeof *meth;k++) {
        w[0]=nvrm_mthd(sub,meth[k],counts[j]);
        w[1]=(k&1)?OMEGA_AMPERE_COMPUTE_B:4113;
        for(uint32_t n=0;n<=counts[j]+2;n++) {
            if(n>2 && n+1<counts[j]) { n=counts[j]-1; }
            memset(rm.err,0,sizeof rm.err); emit(&rm,(uint64_t)validate_ampere_push(&rm,&pb,0,n));
        }
    }
    uint32_t valid[]={nvrm_mthd(1,0,1),OMEGA_AMPERE_COMPUTE_B,nvrm_mthd(1,692,1),0,nvrm_mthd(1,540,1),4113,nvrm_mthd(1,704,1),3,nvrm_mthd(0,92,5),0,0,0,0,0,nvrm_mthd(0,40,4),0,0,0,0};
    pb.cpu=valid; memset(rm.err,0,sizeof rm.err); emit(&rm,(uint64_t)validate_ampere_push(&rm,&pb,0,sizeof valid/sizeof *valid));
    /* Cross the implicit million-step guard with minimum admitted packets. */
    uint32_t *large = calloc(2097152, sizeof *large);
    assert(large);
    for (uint32_t i=0;i<2097152;i+=2) large[i]=nvrm_mthd(1,692,1);
    pb.cpu=large;
    for (uint32_t n=2097149;n<=2097151;n++) {
        memset(rm.err,0,sizeof rm.err);
        emit(&rm,(uint64_t)validate_ampere_push(&rm,&pb,0,n));
    }
    free(large);
    pb.cpu=NULL; memset(rm.err,0,sizeof rm.err); emit(&rm,(uint64_t)validate_ampere_push(&rm,&pb,0,1));
    rm.discrete=0; memset(rm.err,0,sizeof rm.err); emit(&rm,(uint64_t)validate_ampere_push(&rm,&pb,0,1));
}
static void slots(void) {
    const uint64_t edge[]={0,1,UINT64_C(0xfffffffefffffffe),UINT64_C(0xfffffffeffffffff),UINT64_C(0xffffffff00000000),UINT64_C(0xffffffff00000001),UINT64_MAX};
    for(size_t i=0;i<sizeof edge/sizeof *edge;i++) {
        atomic_store(&g_va_counter,edge[i]); g_va_recycled_count=0;
        uint64_t v=acquire_va_slot(),c=atomic_load(&g_va_counter);
        fwrite(&v,sizeof v,1,stdout); fwrite(&c,sizeof c,1,stdout);
    }
    atomic_store(&g_va_counter,UINT64_C(0x1000000000)); g_va_recycled_count=0;
    for(unsigned i=0;i<1026;i++) recycle_va_slot(i);
    for(unsigned i=0;i<1026;i++) {
        uint64_t v=acquire_va_slot(); fwrite(&v,sizeof v,1,stdout);
        fwrite(&g_va_recycled_count,sizeof g_va_recycled_count,1,stdout);
    }
}
int main(int argc,char **argv) {
    if(argc>1 && strcmp(argv[1],"--red")==0) red=1;
    fwrite(&rng,sizeof rng,1,stdout);
    allocator(); methods(); slots();
    return ferror(stdout)?1:0;
}
