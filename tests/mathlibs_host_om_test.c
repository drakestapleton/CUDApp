/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* The default-OFF C branch is compiled in this TU under distinct public names.
 * The production archive must still resolve every default Omega entry strongly. */
#include "../src/omega_gpu_mathlibs_host.h"
#define CUDAPP_MIGRATION_PENDING_C_MATHLIBS_HOST 1
#define omega_gpu_mathlibs_cache_clear oracle_cache_clear
#define omega_gpu_mathlibs_test_set_mutant oracle_set_mutant
#define omega_gpu_mathlibs_last_error oracle_last_error
#define omega_gpu_rng_codegen oracle_rng_codegen
#define omega_gpu_rng_generate oracle_rng_generate
#define omega_gpu_fft_codegen oracle_fft_codegen
#define omega_gpu_fft_twiddles oracle_fft_twiddles
#define omega_gpu_fft_c2c oracle_fft_c2c
#define omega_gpu_philox_codegen oracle_philox_codegen
#define omega_gpu_philox_generate_dist oracle_philox_dist
#define omega_gpu_philox_generate oracle_philox_generate
#define omega_gpu_fft_plan_walk oracle_walk
#define omega_gpu_fft_r2c oracle_r2c
#define omega_gpu_fft_c2r oracle_c2r
#define omega_gpu_fft_c2c_nd oracle_nd
#define omega_gpu_fft_many_walk oracle_many_walk
#define omega_gpu_fft_many oracle_many
#include "../src/omega_gpu_mathlibs_api.c"
#undef omega_gpu_fft_plan_walk
#undef omega_gpu_fft_many_walk
#undef omega_gpu_fft_c2c
#undef omega_gpu_fft_r2c
#undef omega_gpu_fft_c2r
#undef omega_gpu_fft_c2c_nd
#undef omega_gpu_fft_many
#undef omega_gpu_rng_generate
#undef omega_gpu_philox_generate_dist
extern __typeof__(oracle_rng_generate) omega_gpu_rng_generate;
extern __typeof__(oracle_philox_dist) omega_gpu_philox_generate_dist;
extern __typeof__(oracle_walk) omega_gpu_fft_plan_walk;
extern __typeof__(oracle_many_walk) omega_gpu_fft_many_walk;
extern __typeof__(oracle_fft_c2c) omega_gpu_fft_c2c;
extern __typeof__(oracle_r2c) omega_gpu_fft_r2c;
extern __typeof__(oracle_c2r) omega_gpu_fft_c2r;
extern __typeof__(oracle_nd) omega_gpu_fft_c2c_nd;
extern __typeof__(oracle_many) omega_gpu_fft_many;
#include <stdio.h>

static unsigned cases;
#define CHECK(c) do { if (!(c)) { fprintf(stderr,"FAIL line %d case %u: %s\n",__LINE__,cases,#c); exit(1); } } while (0)
typedef struct { OmegaGpuFftManyLaunch *v; size_t n,cap; unsigned char *table; size_t bytes; } MC;
static int mc(void *ctx, const OmegaGpuFftManyLaunch *p) {
    MC *c=ctx;
    if (!c->table && c->bytes) { c->table=malloc(c->bytes); CHECK(c->table); memcpy(c->table,p->table,c->bytes); }
    if (c->n==c->cap) { c->cap=c->cap?2*c->cap:64; c->v=realloc(c->v,c->cap*sizeof *c->v); CHECK(c->v); }
    c->v[c->n]=*p; c->v[c->n++].table=NULL;
    return 0;
}
static void plan(const OmegaGpuFftMany *p,int ip) {
    cases++;
    size_t a[8]={0},b[8]={0};
    int ra=oracle_many_walk(p,ip,a,NULL,NULL),rb=omega_gpu_fft_many_walk(p,ip,b,NULL,NULL);
    if (ra!=rb) fprintf(stderr,"plan rank %u n %u,%u,%u batch %u type %u ip %d rc %d/%d\n",p->rank,p->n[0],p->n[1],p->n[2],p->batch,p->type,ip,ra,rb);
    CHECK(ra==rb);
    if (ra) return;
    CHECK(!memcmp(a,b,sizeof a));
    MC x={.bytes=a[7]},y={.bytes=b[7]};
    CHECK(!oracle_many_walk(p,ip,a,mc,&x)); CHECK(!omega_gpu_fft_many_walk(p,ip,b,mc,&y));
    CHECK(x.n==y.n); CHECK(x.bytes==y.bytes); CHECK(!memcmp(x.table,y.table,x.bytes));
    for (size_t i=0;i<x.n;i++) {
        if (memcmp(&x.v[i],&y.v[i],sizeof *x.v)) {
            fprintf(stderr,"launch %zu rank %u n %u,%u,%u type %u: kind %u/%u a %u/%u b %u/%u nptr %u/%u nw %u/%u elems %zu/%zu\n",i,p->rank,p->n[0],p->n[1],p->n[2],p->type,x.v[i].kind,y.v[i].kind,x.v[i].a,y.v[i].a,x.v[i].b,y.v[i].b,x.v[i].nptr,y.v[i].nptr,x.v[i].nwords,y.v[i].nwords,x.v[i].elements,y.v[i].elements);
            for(int k=0;k<18;k++) if(x.v[i].words[k]!=y.v[i].words[k]) fprintf(stderr,"word%d %u/%u\n",k,x.v[i].words[k],y.v[i].words[k]);
            for(int k=0;k<3;k++) if(x.v[i].buf[k]!=y.v[i].buf[k]||x.v[i].off[k]!=y.v[i].off[k]) fprintf(stderr,"buf%d %u/%u off %llu/%llu\n",k,x.v[i].buf[k],y.v[i].buf[k],(unsigned long long)x.v[i].off[k],(unsigned long long)y.v[i].off[k]);
            CHECK(0);
        }
    }
    free(x.v);free(y.v);free(x.table);free(y.table);
}
typedef struct { OmegaGpuFftLaunch *v; size_t n,cap; } LC;
static int lc(void *ctx,const OmegaGpuFftLaunch *p) {
    LC *c=ctx;
    if(c->n==c->cap){ c->cap=c->cap?2*c->cap:16;c->v=realloc(c->v,c->cap*sizeof *c->v);CHECK(c->v); }
    c->v[c->n]=*p;c->v[c->n++].tw=NULL;
    if(p->tw_n) { float *t=malloc(8*p->tw_n);CHECK(t);memcpy(t,p->tw,8*p->tw_n);c->v[c->n-1].tw=t; }
    return 0;
}
static void walk(unsigned mode,unsigned rank,const uint32_t *n,unsigned batch,int inv) {
    cases++; size_t a[3]={0},b[3]={0};LC x={0},y={0};
    int ra=oracle_walk(mode,rank,n,batch,inv,a,lc,&x),rb=omega_gpu_fft_plan_walk(mode,rank,n,batch,inv,b,lc,&y);
    CHECK(ra==rb);
    if(!ra){CHECK(!memcmp(a,b,sizeof a));CHECK(x.n==y.n);for(size_t i=0;i<x.n;i++){
        const float *xt=x.v[i].tw,*yt=y.v[i].tw; x.v[i].tw=y.v[i].tw=NULL;
        CHECK(!memcmp(&x.v[i],&y.v[i],sizeof *x.v));
        if(x.v[i].tw_n) CHECK(!memcmp(xt,yt,8*x.v[i].tw_n));
        free((void*)xt);free((void*)yt);
    }}
    free(x.v);free(y.v);
}
static void scalar(void) {
    const uint32_t ns[]={0,1,127,128,129,131071,131072,131073,UINT32_MAX};
    const uint32_t per[]={0,1,2,3,4,5,2047,2048,2049,UINT32_MAX};
    const uint64_t seeds[]={0,1,UINT64_MAX,UINT64_MAX-11400714819323198485ull};
    for(unsigned k=0;k<4;k++) for(size_t i=0;i<sizeof ns/sizeof *ns;i++) for(size_t j=0;j<sizeof per/sizeof *per;j++) {
        cases++; CHECK(rng_args_ok(k,ns[i],per[j])==-(int)omega_mh_rng(k,ns[i],per[j],2));
        CHECK(omega_mh_rngval(k,ns[i],per[j],0)==(uint64_t)ns[i]*per[j]);
        CHECK(omega_mh_rngval(k,ns[i],per[j],1)==(k==0?8:4));
        CHECK(omega_mh_rngval(k,ns[i],per[j],2)==((uint64_t)ns[i]+127)/128);
    }
    for(unsigned d=0;d<=4;d++) for(size_t i=0;i<sizeof ns/sizeof *ns;i++) for(size_t j=0;j<sizeof per/sizeof *per;j++) {
        cases++; unsigned g=d==3?2:4,esz=d==2||d==3?2:1,r=per[j]%g;
        CHECK(omega_mh_phil(d,ns[i],per[j],UINT64_MAX,0)==(uint64_t)ns[i]*per[j]*esz);
        CHECK(omega_mh_phil(d,ns[i],per[j],UINT64_MAX,1)==3);
        CHECK(omega_mh_phil(d,ns[i],per[j],UINT64_MAX,2)==UINT64_MAX/4);
        CHECK(omega_mh_phil(d,ns[i],per[j],UINT64_MAX,3)==per[j]/g+(r!=0));
        CHECK(omega_mh_phil(d,ns[i],per[j],UINT64_MAX,4)==(r?r-g:0u));
    }
    for(size_t i=0;i<sizeof seeds/sizeof *seeds;i++)for(size_t j=0;j<sizeof seeds/sizeof *seeds;j++)for(unsigned k=0;k<4;k++) {
        cases++; CHECK(omega_mh_seed(seeds[i],seeds[j],k)==seeds[i]+seeds[j]+(k+1)*ML_GOLDEN);
    }
    const uint32_t lens[]={0,1,2,3,4,5,7,8,9,2047,2048,2049,4095,4096,4097,UINT32_MAX};
    for(size_t i=0;i<sizeof lens/sizeof *lens;i++){cases++;CHECK(lg_in(lens[i],4,2048)==(int)(uint32_t)omega_mh_lg(lens[i],4,2048));}
    CHECK(omega_mh_time(123,999999999)==123999999999ull);
    CHECK(omega_mh_poison(ML_POISON64,0,1,0)==1);CHECK(!omega_mh_poison(ML_POISON64-1,0,1,0));
    CHECK(omega_mh_poison(ML_POISON32,ML_POISON32_ALT,0,1)==1);CHECK(!omega_mh_poison(ML_POISON32,0,0,1));
    for(uint64_t gap=31;gap<=33;gap++){cases++;CHECK(omega_mh_overlap(4096,32,4096+gap,32)==(gap<32));CHECK(omega_mh_overlap(4096+gap,32,4096,32)==(gap<32));}
    CHECK(!omega_mh_overlap(0,8,UINT64_MAX-3,8));
    uint64_t l[8]={7,99,31,7,1},e[4]={2,3,4,1};
    for(uint64_t i=0;i<48;i++){cases++;CHECK(omega_mh_index(l,e,3,i)==7+(i/24)*99+((i/12)%2)*31+((i/4)%3)*7+i%4);}
    CHECK(omega_mh_fill(0,7)==128);CHECK(omega_mh_fill(128,7)>255);
    uint64_t out[8]; CHECK(omega_mh_fft(4096,4128,4,1,0,out)==0);CHECK(omega_mh_fft(4096,4127,4,1,0,out)==1);
    CHECK(omega_mh_fft(4096,8192,4,UINT32_MAX,0,out)==2);CHECK(omega_mh_fft(4096,8192,3,1,0,out)==4);
}
static int stop_many(void *ctx, const OmegaGpuFftManyLaunch *launch) { (void)ctx; (void)launch; return -91; }
static int stop_walk(void *ctx, const OmegaGpuFftLaunch *launch) { (void)ctx; (void)launch; return -92; }
static void admission(void) {
    float a[8194]={0};
    uint32_t dims[3]={4,4,4};
    const uint32_t badns[]={0,131073,UINT32_MAX};
    for(size_t i=0;i<sizeof badns/sizeof *badns;i++) {
        CHECK(oracle_rng_generate(0,1,2,badns[i],0,1,a,NULL)==omega_gpu_rng_generate(0,1,2,badns[i],0,1,a,NULL));
        CHECK(oracle_philox_dist(0,1,2,badns[i],0,1,a,NULL)==omega_gpu_philox_generate_dist(0,1,2,badns[i],0,1,a,NULL));
    }
    CHECK(oracle_rng_generate(3,0,0,1,0,1,a,NULL)==omega_gpu_rng_generate(3,0,0,1,0,1,a,NULL));
    CHECK(oracle_rng_generate(0,0,0,1,0,0,a,NULL)==omega_gpu_rng_generate(0,0,0,1,0,0,a,NULL));
    CHECK(oracle_rng_generate(0,0,0,1,0,1,NULL,NULL)==omega_gpu_rng_generate(0,0,0,1,0,1,NULL,NULL));
    CHECK(oracle_philox_dist(5,0,0,1,0,1,a,NULL)==omega_gpu_philox_generate_dist(5,0,0,1,0,1,a,NULL));
    CHECK(oracle_philox_dist(0,0,0,1,0,0,a,NULL)==omega_gpu_philox_generate_dist(0,0,0,1,0,0,a,NULL));
    CHECK(oracle_philox_dist(0,0,0,1,0,1,NULL,NULL)==omega_gpu_philox_generate_dist(0,0,0,1,0,1,NULL,NULL));
    const uint32_t lens[]={0,1,2,3,4,8,4096,4097,UINT32_MAX};
    for(size_t i=0;i<sizeof lens/sizeof *lens;i++) {
        cases++;
        CHECK(oracle_fft_c2c(a,a+1,lens[i],1,0,NULL)==omega_gpu_fft_c2c(a,a+1,lens[i],1,0,NULL));
        CHECK(oracle_r2c(a,a+1,lens[i],1,NULL)==omega_gpu_fft_r2c(a,a+1,lens[i],1,NULL));
        CHECK(oracle_c2r(a,a+1,lens[i],1,NULL)==omega_gpu_fft_c2r(a,a+1,lens[i],1,NULL));
    }
    CHECK(many_basic(0,0,dims,1,0,a,a+1,NULL)==OMEGA_GPU_ML_BAD_ARGS);
    CHECK(many_basic(0,1,dims,1,0,a,a+1,NULL)==OMEGA_GPU_ML_BAD_ARGS);
    CHECK(oracle_nd(a,a+1,3,dims,1,0,NULL)==omega_gpu_fft_c2c_nd(a,a+1,3,dims,1,0,NULL));
    const uint32_t bits[]={ML_POISON32,0,0x80000000u,1,0x80000001u,0x7f800000u,0xff800000u,0x7fc00000u,ML_POISON32_ALT};
    ManyScan scan={.dev=bits,.words=1};
    uint64_t count=0;
    for(uint32_t i=0;i<sizeof bits/sizeof *bits;i++) { CHECK(!many_scan(&scan,i)); count=omega_mh_add(count,omega_mh_poison(bits[i],0,0,0)); }
    CHECK(scan.unwritten==count && count==1);
}
int main(int argc,char **argv) {
    scalar();
    admission();
    const uint32_t batches[]={0,1,1023,1024,1025};
    for(uint32_t n=4;n<=2048;n*=2) for(size_t b=0;b<sizeof batches/sizeof *batches;b++) for(int inv=0;inv<2;inv++) {
        uint32_t dims[3]={n,4,2};walk(0,1,dims,batches[b],inv);walk(0,2,dims,batches[b],inv);
        OmegaGpuFftMany p={.rank=1,.n={n,0,0},.batch=batches[b],.inverse=inv};plan(&p,0);
        if(b==1){p.rank=2;p.n[1]=4;plan(&p,1);p.rank=3;p.n[2]=2;plan(&p,0);}
    }
    const uint32_t shapes[]={2,3,5,6,7,8,11,16,30,64,127,1024,2047,2048,2049,4095,4096};
    for(size_t i=0;i<sizeof shapes/sizeof *shapes;i++) for(unsigned type=0;type<3;type++)for(unsigned rank=1;rank<=3;rank++) {
        OmegaGpuFftMany p={.rank=rank,.n={shapes[i],3,2},.batch=2,.type=type};
        plan(&p,0);plan(&p,1);
        p.inembed[0]=shapes[i]+1;p.inembed[1]=4;p.inembed[2]=3;
        p.onembed[0]=shapes[i]+1;p.onembed[1]=4;p.onembed[2]=3;
        p.istride=2;p.ostride=3;p.idist=65536;p.odist=65537;plan(&p,0);
        p.odist=0;plan(&p,0);p.odist=1;plan(&p,0);p.ostride=0;plan(&p,0);
    }
    const uint32_t bad[]={0,1,3,2049,4097,UINT32_MAX};
    for(size_t i=0;i<sizeof bad/sizeof *bad;i++){uint32_t d[3]={bad[i],4,8};walk(0,1,d,1,0);walk(1,1,d,1,0);walk(2,1,d,1,0);OmegaGpuFftMany p={.rank=1,.n={bad[i]},.batch=1};plan(&p,0);}
    for(uint32_t n=8;n<=4096;n*=2){uint32_t d[3]={n,8,8};walk(1,1,d,1,0);walk(2,1,d,1025,0);walk(0,3,d,1,0);}
    OmegaGpuFftMany p={.rank=3,.n={4096,4096,4096},.batch=UINT32_MAX};plan(&p,0);
    p.rank=0;plan(&p,0);p.rank=4;plan(&p,0);p.rank=1;p.n[0]=8;p.batch=1;p.type=3;plan(&p,0);p.type=0;p.inverse=-1;plan(&p,0);
    p.inverse=0;p.inembed[0]=8;p.onembed[0]=0;plan(&p,0);
    const uint32_t edges[]={0,1,2,3,4,5,15,16,17,4095,4096,4097,UINT32_MAX};
    for(size_t k=0;k<sizeof edges/sizeof *edges;k++) {
        OmegaGpuFftMany q={.rank=2,.n={4,4},.batch=2,.inembed={4,4},.onembed={4,4},
                          .istride=1,.ostride=1,.idist=16,.odist=16};
        q.istride=edges[k];plan(&q,0);q.istride=1;
        q.ostride=edges[k];plan(&q,0);q.ostride=1;
        q.idist=edges[k];plan(&q,0);q.idist=16;
        q.odist=edges[k];plan(&q,0);q.odist=16;
        q.inembed[1]=edges[k];plan(&q,0);q.inembed[1]=4;
        q.onembed[1]=edges[k];plan(&q,0);
        uint32_t dims[3]={4,4,4};walk(0,edges[k],dims,1,0);
    }
    for(uint32_t k=268435451;k<=268435453;k++) {
        OmegaGpuFftMany q={.rank=1,.n={4},.batch=2,.inembed={4},.onembed={4},
                          .istride=1,.ostride=1,.idist=k,.odist=k};
        plan(&q,0);
    }
    uint32_t vd[3]={4,4,4};walk(0,1,vd,UINT32_MAX,0);walk(0,1,vd,1,-1);walk(1,2,vd,1,0);
    walk(3,1,vd,1,0);walk(UINT32_MAX,1,vd,1,0);walk(0,UINT32_MAX,vd,1,0);walk(1,1,vd,1,1);
    for(uint32_t axis=0;axis<3;axis++) for(size_t k=0;k<sizeof edges/sizeof *edges;k++) {
        uint32_t dims[3]={4,4,4};dims[axis]=edges[k];walk(0,3,dims,1,0);
        OmegaGpuFftMany q={.rank=3,.n={4,4,4},.batch=1};q.n[axis]=edges[k];plan(&q,0);
    }
    for(uint32_t b=33554431;b<=33554433;b++) {
        OmegaGpuFftMany q={.rank=1,.n={4},.batch=b};plan(&q,0);
    }
    OmegaGpuFftMany invalid={.rank=1,.n={4},.batch=1};
    plan(&invalid,-1);plan(&invalid,2);invalid.batch=UINT32_MAX;plan(&invalid,0);
    invalid.batch=1;invalid.type=UINT32_MAX;plan(&invalid,0);invalid.type=0;invalid.rank=UINT32_MAX;plan(&invalid,0);
    for(uint32_t b=8388607;b<=8388609;b++) {
        OmegaGpuFftMany q={.rank=1,.n={11},.batch=b};plan(&q,0);
    }
    size_t sizes[OMEGA_GPU_FFT_MANY_BUFS]={0};
    invalid.rank=1;
    CHECK(oracle_many_walk(NULL,0,sizes,NULL,NULL)==omega_gpu_fft_many_walk(NULL,0,sizes,NULL,NULL));
    CHECK(oracle_many_walk(&invalid,0,NULL,NULL,NULL)==omega_gpu_fft_many_walk(&invalid,0,NULL,NULL,NULL));
    CHECK(oracle_many_walk(&invalid,0,sizes,stop_many,NULL)==-91);
    CHECK(omega_gpu_fft_many_walk(&invalid,0,sizes,stop_many,NULL)==-91);
    CHECK(oracle_walk(0,1,vd,1,0,sizes,stop_walk,NULL)==-92);
    CHECK(omega_gpu_fft_plan_walk(0,1,vd,1,0,sizes,stop_walk,NULL)==-92);
    CHECK(oracle_walk(0,1,NULL,1,0,sizes,NULL,NULL)==omega_gpu_fft_plan_walk(0,1,NULL,1,0,sizes,NULL,NULL));
    CHECK(oracle_walk(0,1,vd,1,0,NULL,NULL,NULL)==omega_gpu_fft_plan_walk(0,1,vd,1,0,NULL,NULL,NULL));
    uint32_t oversized[3]={2048,2048,2048};walk(0,3,oversized,1,0);
    const uint32_t blue[3][2]={{13,11},{11,31},{31,11}};
    for(size_t i=0;i<3;i++) for(unsigned type=0;type<3;type++) for(int inv=0;inv<2;inv++) {
        OmegaGpuFftMany q={.rank=2,.n={blue[i][0],blue[i][1]},.batch=2,.type=type,.inverse=inv};plan(&q,0);
    }
    if(argc>1 && !strcmp(argv[1],"--red")){ uint64_t l[8]={0,1,1},e[4]={4,1,1,1};CHECK(omega_mh_index(l,e,1,1)==2); }
    printf("MATHLIBS-HOST-OM PASS cases=%u\n",cases);return 0;
}
