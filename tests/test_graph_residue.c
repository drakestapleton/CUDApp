/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* The test reaches hello's real adapters without entering its device CLI. */
#define main hello_cli_main
#include "../examples/hello_gpu/hello_gpu.c"
#undef main
#include "omega_gpu_graph_residue.h"
#include <assert.h>
static uint64_t digest = 1469598103934665603ULL;
static void word(uint64_t x) { digest = (digest ^ x) * 1099511628211ULL; }
static uint32_t pslot(uint32_t i) { uint32_t p=32,j=i-128; return i<128 ? (i<p?i:64+i-p) : (j<p?p+j:64+128+j-p); }
static void tables(void) {
    uint32_t dst[24576];
    omega_graph_table(0,0,0,dst,24576);
    for (uint32_t n=0;n<24576;n++) {
        uint32_t j=n%512, ref=n/512*512+(j/4%32)*16+(j/128)*4+j%4;
        assert(dst[n]==ref); word(dst[n]);
    }
    for (uint32_t q=1;q<=24;q++) for (uint32_t kv=1;kv<=q;kv++) if(q%kv==0) {
        uint32_t count=(q+kv)*512;
        omega_graph_table(1,q,kv,dst,count);
        for(uint32_t n=0;n<count;n++) {
            uint32_t qn=q*256,kn=kv*256,ref;
            if(n<qn) ref=n/256*512+pslot(n%256);
            else if(n<qn+kn) ref=q*512+(n-qn)/256*256+pslot(n%256);
            else if(n<qn+2*kn) ref=q*512+kn+n-qn-kn;
            else ref=(n-qn-2*kn)/256*512+256+(n-qn-2*kn)%256;
            assert(dst[n]==ref); word(dst[n]);
        }
    }
    uint64_t r=7;
    assert(omega_grtable(&r,0,0,0,NULL,0)==OMEGA_OK&&r==1);
    r=7;assert(omega_grtable(&r,0,0,0,dst,24577)==OMEGA_BAD_ARG&&r==7);
    assert(omega_grtable(&r,0,0,0,NULL,1)==OMEGA_BAD_ARG&&r==7);
    uint64_t alias[2]={7,0};
    assert(omega_grtable(alias,0,0,0,alias,1)==OMEGA_BAD_ARG&&alias[0]==7);
}
int main(void) {
    for(uint32_t l=0;l<=65;l++) for(uint32_t h=0;h<=9;h++) for(uint32_t v=0;v<=4097;v+=257) {
        word(omega_graph_config(l,64,v,64,h,8,0,1));
        word(omega_graph_config(l,64,v,64,h,8,UINT64_MAX,1));
        word(omega_graph_config(l,64,v,64,h,8,UINT64_MAX,0));
    }
    uint64_t layer_counts[2]={0};
    for(uint32_t l=0;l<64;l++) {int a;uint32_t k; omega_graph_layer(0xa55aa55aa55aa55aULL,l,layer_counts,&a,&k);word(a);word(k);}
    printf("metadata digest %016llx\n",(unsigned long long)digest);
    for(uint32_t hd=0;hd<=6144;hd+=1024) for(uint32_t in=0;in<=17664;in+=256)
    for(uint32_t q=0;q<=25;q++) for(uint32_t kv=0;kv<=5;kv++) for(uint32_t gh=0;gh<=49;gh+=16) {
        uint64_t c[16]={hd,in,q,kv,gh},d[16]={0}; int ok=omega_graph_q38(c,d);word(ok);
        if(ok) for(unsigned j=0;j<11;j++)word(d[j]);
    }
    { const uint32_t edges[]={0,1,255,256,257,1023,1024,1025,4095,4096,4097,5119,5120,5121,17407,17408,17409,UINT32_MAX};
      for(unsigned j=0;j<sizeof edges/sizeof edges[0];j++) {
        word(omega_graph_config(1,64,edges[j],64,1,8,0,1));
        for(unsigned f=0;f<5;f++) { uint64_t c[16]={0},d[16]={0};c[f]=edges[j];
          int ok=omega_graph_q38(c,d);word(ok);if(ok)for(unsigned x=0;x<11;x++)word(d[x]);}
      }
      word(omega_graph_config(UINT32_MAX,64,1,64,1,8,0,1));
      word(omega_graph_config(1,64,1,64,UINT32_MAX,8,0,1));
      word(omega_graph_shape(1,2,UINT32_MAX,1,2,7));
      int from=0;assert(!omega_graph_timeline(0,UINT32_MAX,4097,&from)); }
    for(unsigned j=0;j<8;j++) { uint64_t c[16]={j},d[16]; uint64_t r=omega_graph_meta(3,c,d);assert(r==(j<=4&&j!=1?j:0));word(r);}
    for(unsigned a=0;a<4;a++) for(unsigned b=0;b<4;b++) word(omega_graph_shape(a,b,7,1,2,7));
    tables();
    printf("shape/table digest %016llx\n",(unsigned long long)digest);
    for(uint32_t used=0;used<=4098;used+=683) for(uint32_t count=0;count<=4098;count+=683) {
        int from=0;word(omega_graph_timeline(used,count,4097,&from));word(from);
    }
    {int from=0;assert(omega_graph_timeline(4096,2,4097,&from)&&from==1);
     assert(!omega_graph_timeline(4097,2,4097,&from)&&from==1);
     from=7;assert(!omega_graph_timeline(4098,1,4097,&from)&&from==7);}
    printf("timeline digest %016llx\n",(unsigned long long)digest);
    for(unsigned peak=0;peak<=260;peak++) for(unsigned limit=0;limit<=260;limit++)
        for(int ok=0;ok<2;ok++) word(omega_graph_budget(peak,limit,BW_PHYS_GPR_MAX,BW_PHYS_GPR_HW_LIMIT,ok));
    float *a=malloc(N_TOTAL*4u),*b=malloc(N_TOTAL*4u);uint32_t *c=malloc(N_TOTAL*4u);assert(a&&b&&c);
    inputs(a,b,c); assert(check(a,b,c)==1);
    for(unsigned i=0;i<N_TOTAL;i++) {float want=a[i]+b[i];memcpy(c+i,&want,4);}
    assert(check(a,b,c)==0); c[N_TOTAL-1]=POISON;assert(check(a,b,c)==1);c[0]^=1;assert(check(a,b,c)==1);
    for(unsigned i=0;i<N_TOTAL;i++) {uint32_t x,y;memcpy(&x,a+i,4);memcpy(&y,b+i,4);word(x);word(y);word(c[i]);}
    free(a);free(b);free(c);
    printf("hello check digest %016llx\n",(unsigned long long)digest);
    for(unsigned bug=0;bug<3;bug++) for(uint32_t base=0;base<N_TOTAL;base+=CHUNK) {
        uint64_t d[16]={0}; chunk_plan(base,bug,d);for(unsigned j=0;j<6;j++)word(d[j]);
    }
    printf("hello chunk digest %016llx\n",(unsigned long long)digest);
    print_work(N_TOTAL*8u,N_TOTAL*4u,N_TOTAL,128.0); target_sm=86;
    print_work(N_TOTAL*8u,N_TOTAL*4u,N_TOTAL,128.0); target_sm=121;
    BlackwellIRProgram p;assert(build_vecadd(&p)==0);for(size_t i=0;i<p.count;i++)word(p.insns[i].control);
#ifdef CUDAPP_GRAPH_DIFF_RED
    digest ^= 1;
#endif
    printf("graph-residue digest %016llx\n",(unsigned long long)digest);return 0;
}
