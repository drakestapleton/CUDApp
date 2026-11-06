/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#define CUDAPP_MIGRATION_PENDING_C_GEMM_HOST_PLAN 1
#include "../src/omega_gpu_gemm.c"
#include <stdio.h>
static unsigned long cases;
static void ck(uint64_t got, uint64_t want, const char *what) {
    if (getenv("GEMM_POLICY_RED") && strcmp(what,"value status")) want ^= 1;
    if (got != want) { fprintf(stderr,"FAIL %s case=%lu got=%lu want=%lu\n",what,cases,got,want); exit(1); }
    cases++;
}
static void load(uint64_t x[32], const OmegaGemmCfg *c) {
    memset(x,0,32*sizeof *x); x[0]=(unsigned)c->type; x[1]=c->rt;x[2]=c->ct;x[3]=c->wm;x[4]=c->wn;
    x[5]=c->stall;x[6]=(unsigned)c->mutant;x[7]=c->band;
}
static uint64_t val(uint64_t op,uint64_t a,uint64_t b,uint64_t c,uint64_t d) {
    uint64_t r; ck(omega_gemm_value(&r,op,a,b,c,d),OMEGA_OK,"value status");return r;
}
struct Calls { uint64_t y[64]; unsigned n, fail; };
static int collect(void *v,OmegaGemmKind k,const uint32_t *a,uint32_t gx,uint32_t gy,uint32_t th) {
    struct Calls *c=v; c->n++; c->y[8+2*k]=gx;c->y[9+2*k]=gy;c->y[14+k]=th;
    for(unsigned w=0;w<10;w++) c->y[24+10*k+w]=a[w];
    return c->n==c->fail;
}
int main(void) {
    const uint32_t edges[]={0,1,2,3,4,5,7,8,9,14,15,16,17,31,32,33,63,64,65,127,128,129,255,256,257,16383,16384,16385,UINT32_MAX-1,UINT32_MAX};
    const uint64_t lds[]={0,1,2,3,15,16,17,31,32,33,65535,65536,65537,UINT32_MAX-1,UINT32_MAX,UINT64_MAX};
    OmegaGemmCfg c; uint64_t x[32],y[64],r;
    for(unsigned t=0;t<5;t++) {
        omega_gpu_gemm_default_cfg((OmegaGpuGemmType)t,&c);
        ck(val(0,t,0,0,0),esz(c.type),"esz");ck(val(1,t,0,0,0),kstep(c.type),"kstep");
        ck(val(2,t,0,0,0),type_ok(t),"type");
        for(unsigned field=0;field<8;field++) for(unsigned z=0;z<sizeof edges/sizeof *edges;z++) {
            OmegaGemmCfg bad=c; uint32_t *fields=(uint32_t *)&bad; fields[field]=edges[z]; load(x,&bad);
            ck(omega_gemm_cfg(&r,x,32),OMEGA_OK,"cfg status");ck(r,omega_gpu_gemm_cfg_ok(&bad),"cfg");
            ck(val(3,bad.rt,bad.ct,0,0),main_regs(&bad),"registers");
        }
        for(unsigned rt=1;rt<=4;rt*=2) for(unsigned ct=1;ct<=4;ct*=2)
        for(unsigned wm=1;wm<=4;wm*=2) for(unsigned wn=1;wn<=4;wn*=2)
        for(unsigned band=0;band<=32;band=band?band*2:1) {
            c.rt=rt;c.ct=ct;c.wm=wm;c.wn=wn;c.band=band;
            for(unsigned q=0;q<sizeof edges/sizeof *edges;q++) {
                uint32_t M=edges[q],N=edges[(q+7)%(sizeof edges/sizeof *edges)],K=edges[(q+13)%(sizeof edges/sizeof *edges)];
                OmegaGemmPlan p; load(x,&c);x[8]=M;x[9]=N;x[10]=K;
                ck(omega_gemm_plan(&r,x,32,y,64),OMEGA_OK,"plan status");
                int rc=omega_gpu_gemm_plan(&c,M,N,K,&p);ck(r,(uint64_t)-rc,"plan rc");
                if(!rc) {
                    uint64_t want[]={p.Mp,p.Np,p.Kp,p.KS2,p.a_bytes,p.b_bytes,p.ap_bytes,p.bp_bytes,
                        p.grid[0][0],p.grid[0][1],p.grid[1][0],p.grid[1][1],p.grid[2][0],p.grid[2][1],p.threads[0],p.threads[1],p.threads[2]};
                    for(unsigned j=0;j<17;j++) ck(y[j],want[j],"plan field");
                }
            }
        }
    }
    for(unsigned l=0;l<=4;l++) for(unsigned i=0;i<sizeof edges/sizeof *edges;i++)
    for(unsigned j=0;j<sizeof lds/sizeof *lds;j++) {
        uint32_t M=edges[i],N=edges[(i+3)%(sizeof edges/sizeof *edges)],K=edges[(i+5)%(sizeof edges/sizeof *edges)];
        uint64_t sa=0,sb=0;memset(x,0,sizeof x);x[8]=M;x[9]=N;x[10]=K;x[11]=l;x[12]=lds[j];x[13]=lds[(j+2)%(sizeof lds/sizeof *lds)];
        ck(omega_gemm_span(&r,x,32,y,64),OMEGA_OK,"span status");
        int rc=omega_gpu_gemm_span(l,M,N,K,x[12],x[13],&sa,&sb);ck(r,(uint64_t)-rc,"span rc");
        if(!rc) {ck(y[17],sa,"span A");ck(y[18],sb,"span B");}
    }
    /* Exact valid/invalid 32-bit span boundaries with nonzero padded leading dimensions. */
    for(unsigned l=0;l<4;l++) for(unsigned m=1;m<=2;m++) for(unsigned k=1;k<=2;k++)
    for(unsigned j=12;j<sizeof lds/sizeof *lds;j++) {
        uint64_t sa=0,sb=0;memset(x,0,sizeof x);x[8]=m;x[9]=1;x[10]=k;x[11]=l;x[12]=lds[j];x[13]=lds[j];
        ck(omega_gemm_span(&r,x,32,y,64),OMEGA_OK,"span boundary status");
        int rc=omega_gpu_gemm_span(l,m,1,k,x[12],x[13],&sa,&sb);ck(r,(uint64_t)-rc,"span boundary rc");
        if(!rc) {ck(y[17],sa,"span boundary A");ck(y[18],sb,"span boundary B");}
    }
    for(unsigned t=0;t<4;t++) for(unsigned l=0;l<5;l++) for(unsigned i=0;i<sizeof edges/sizeof *edges;i++)
    for(unsigned variant=0;variant<4;variant++) for(unsigned fail=0;fail<4;fail++) {
        omega_gpu_gemm_default_cfg((OmegaGpuGemmType)t,&c);load(x,&c);
        uint32_t M=edges[i],N=17,K=33;x[8]=M;x[9]=N;x[10]=K;x[11]=l;
        x[12]=variant==0||variant==3?0:variant==1?65536:UINT32_MAX;
        x[13]=variant==3?65536:x[12];
        x[14]=0x1122334455667788ull;x[15]=0x2233445566778899ull;x[16]=0x33445566778899aaull;x[17]=0x445566778899aabbull;x[18]=0x5566778899aabbccull;
        ck(omega_gemm_host_drive(&r,x,32,y,64),OMEGA_OK,"drive status");
        struct Calls calls={.fail=fail};
        int rc=omega_gpu_gemm_drive_ld(&c,l,M,N,K,x[14],x[12],x[15],x[13],x[16],x[17],x[18],collect,&calls);
        if(rc==OMEGA_GPU_GEMM_CHIP_FAIL) rc=0;
        ck(r,(uint64_t)-rc,"drive rc");
        if(!r) for(unsigned k=(unsigned)y[54];k<3;k++) {
            if(fail && k>=(unsigned)y[54]+fail) break;
            ck(y[8+2*k],calls.y[8+2*k],"drive gx");ck(y[9+2*k],calls.y[9+2*k],"drive gy");ck(y[14+k],calls.y[14+k],"drive threads");
            for(unsigned w=0;w<10;w++) ck(y[24+10*k+w],calls.y[24+10*k+w],"drive args");
        }
    }
    for(unsigned i=0;i<sizeof lds/sizeof *lds;i++) {
        ck(val(4,lds[i],0,0,0),up16(lds[i]),"alignment");
        for(unsigned j=0;j<sizeof lds/sizeof *lds;j++) for(unsigned ptr=0;ptr<2;ptr++)
            ck(val(5,ptr,lds[i],lds[j],0),ptr && lds[i]>=lds[j],"reuse");
    }
    const char *envs[]={NULL,"","0","00","1","01"};
    for(unsigned i=0;i<sizeof envs/sizeof *envs;i++) {const char *v=envs[i];
        ck(val(6,v!=NULL,v?(unsigned char)v[0]:0,v&&v[0]?(unsigned char)v[1]:0,0),!(v&&v[0]=='0'&&v[1]==0),"cache");}
    /* C-only pointer guards remain in the marshalling shell, outside the fixed-view ABI. */
    OmegaGemmPlan p;
    omega_gpu_gemm_default_cfg(OMEGA_GPU_GEMM_BF16,&c);
    ck(omega_gpu_gemm_cfg_ok(NULL),0,"null cfg");
    ck(omega_gpu_gemm_plan(NULL,1,1,1,&p),OMEGA_GPU_GEMM_BAD_ARGS,"null plan cfg");
    ck(omega_gpu_gemm_plan(&c,1,1,1,NULL),OMEGA_GPU_GEMM_BAD_ARGS,"null output plan");
    ck(omega_gpu_gemm_span(0,1,1,1,0,0,NULL,NULL),OMEGA_GPU_GEMM_OK,"optional spans");
    ck(omega_gpu_gemm_drive_ld(&c,0,1,1,1,0,0,0,0,0,0,0,NULL,NULL),OMEGA_GPU_GEMM_BAD_ARGS,"null callback");
    printf("GEMM HOST PLAN DIFFERENTIAL PASS cases=%lu\n",cases);return 0;
}
