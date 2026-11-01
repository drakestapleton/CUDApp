/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* External verification instrument. Never linked into the runtime. */
#include "bw_warp_sim_k.h"
#include "components/native-frame.h"
#include "omega_gpu_session.h"
#include <float.h>
#include <assert.h>
static uint32_t bits(float f) { uint32_t u; memcpy(&u,&f,4); return u; }
static int chip;
static OmegaGpuMem code;
static void demand(int rc) {if(rc) {fprintf(stderr,"GPU: %s\n",omega_gpu_session_last_error());exit(4);}}
static void execute(float **v,uint32_t n,float hn,unsigned order) {
    if(chip) {
        OmegaGpuMem mem[4]; uint32_t args[10];
        omega_gpu_session_lock();
        for(unsigned j=0;j<4;j++) {
            demand(omega_gpu_session_alloc((n+32)*4,&mem[j]));
            memcpy(mem[j].cpu,v[j],(n+32)*4);demand(omega_gpu_session_sync_to_gpu(&mem[j]));
            args[j*2]=(uint32_t)mem[j].va;args[j*2+1]=(uint32_t)(mem[j].va>>32);
        }
        args[8]=bits(hn);args[9]=n;
        OmegaGpuLaunch l={0};l.code_va=code.va;l.gpr_count=64;l.threads_x=256;l.threads_y=1;
        l.grid_x=(n+255)/256;l.grid_y=1;l.num_elements=n;l.args=args;l.n_args=10;l.timeout_ms=20000;
        demand(omega_gpu_session_launch(&l,NULL,NULL));
        for(unsigned j=0;j<4;j++) {
            demand(omega_gpu_session_sync_to_cpu(&mem[j]));memcpy(v[j],mem[j].cpu,(n+32)*4);
            omega_gpu_session_free(&mem[j]);
        }
        omega_gpu_session_unlock();return;
    }
    static WsSim s; memset(&s,0,sizeof s);s.p=&native_program;s.threads=256;s.order=order;
    s.nranges=4;
    for(unsigned j=0;j<4;j++) {
        uintptr_t p=(uintptr_t)v[j];s.cbank[(896+j*8)/4]=(uint32_t)p;s.cbank[(896+j*8)/4+1]=(uint32_t)(p>>32);
        s.ranges[j].lo=p;s.ranges[j].hi=p+n*4;
    }
    s.cbank[928/4]=bits(hn);s.cbank[932/4]=n;
    int rc=ws_run(&s,(n+255)/256,1);
    if(rc) {fprintf(stderr,"sim n%u order%u: %s\n",n,order,s.msg);exit(2);}
}
static unsigned run(uint32_t n, float hn, unsigned order) {
    float *v[4];
    for(unsigned j=0;j<4;j++) {v[j]=calloc(n+32,4);assert(v[j]);}
    for(uint32_t i=0;i<n;i++) {
        v[0][i]=ldexpf((float)((int)(i%67)-33), (int)(i%173)-90);
        v[1][i]=ldexpf((float)(i%17), (int)(i%143)-80);
    }
    if(n>4) {v[0][0]=FLT_MAX; v[1][0]=FLT_MAX; v[0][1]=-FLT_MAX; v[1][1]=FLT_MAX;
        v[0][2]=0;v[1][2]=0;v[0][3]=FLT_TRUE_MIN;v[1][3]=FLT_TRUE_MIN;}
    for(unsigned j=2;j<4;j++) for(uint32_t i=0;i<n+32;i++) v[j][i]=12345.0f;
    execute(v,n,hn,order);
    unsigned bad=0;
    for(uint32_t i=0;i<n;i++) {
        long double radius=(long double)v[1][i]*hn;
        long double lo=(long double)v[0][i]-radius,hi=(long double)v[0][i]+radius;
        long double slack=radius*0x1p-16L+fabsl(v[0][i])*0x1p-16L+0x1p-96L;
        /* A conservatively inflated radius may itself overflow even when one
         * endpoint cancels. The contract then explicitly retains this row. */
        int tight=radius+slack>FLT_MAX ||
                  ((lo-slack < -FLT_MAX || (long double)v[2][i]>=lo-slack)
                && (hi+slack > FLT_MAX || (long double)v[3][i]<=hi+slack));
        if(!((long double)v[2][i]<=lo && (long double)v[3][i]>=hi && tight)) {
            if(!bad) fprintf(stderr,"bad row%u s%g c%g hn%g lo%g hi%g\n",i,v[0][i],v[1][i],hn,v[2][i],v[3][i]);
            bad++;
        }
    }
    for(unsigned j=2;j<4;j++)for(uint32_t i=n;i<n+32;i++)assert(v[j][i]==12345.0f);
    for(unsigned j=0;j<4;j++)free(v[j]);
    return bad;
}
static void read_floats(const char *dir,const char *name,float *v,size_t n) {
    char path[4096];assert(snprintf(path,sizeof path,"%s/%s",dir,name)<(int)sizeof path);
    FILE *f=fopen(path,"rb");assert(f);assert(fread(v,4,n,f)==n);assert(fgetc(f)==EOF);fclose(f);
}
static void replay(const char *screens,const char *captured) {
    const uint32_t n=248320;unsigned positions[]={1,16,32,64,96,97,98,127};
    float *v[4],*full=calloc(n,4),h[5120];assert(full);
    for(unsigned j=0;j<4;j++){v[j]=calloc(n+32,4);assert(v[j]);}
    read_floats(screens,"coef.f32",v[1],n);
    for(unsigned p=0;p<sizeof positions/sizeof *positions;p++) {
        char name[64];snprintf(name,sizeof name,"%u-screen.f32",positions[p]);read_floats(screens,name,v[0],n);
        snprintf(name,sizeof name,"%u-hidden.f32",positions[p]);read_floats(captured,name,h,5120);
        snprintf(name,sizeof name,"%u-logits.f32",positions[p]);read_floats(captured,name,full,n);
        long double sum=0;
        for(unsigned j=0;j<5120;j++)sum=nextafterl(sum+(long double)h[j]*h[j],INFINITY);
        float hn=nextafterf((float)nextafterl(sqrtl(sum),INFINITY),INFINITY);assert(isfinite(hn));
        for(unsigned j=2;j<4;j++)for(uint32_t i=0;i<n+32;i++)v[j][i]=12345.0f;
        execute(v,n,hn,0);
        float tau=-INFINITY;unsigned winner=0,candidates=0,violations=0;
        for(uint32_t i=0;i<n;i++) {
            assert(isfinite(full[i])&&isfinite(v[0][i])&&isfinite(v[1][i])&&v[1][i]>=0);
            if(full[i]>full[winner])winner=i;
            if(v[2][i]>tau)tau=v[2][i];
            violations+=!(v[2][i]<=full[i]&&v[3][i]>=full[i]);
        }
        for(uint32_t i=0;i<n;i++)candidates+=v[3][i]>=tau;
        for(unsigned j=2;j<4;j++)for(uint32_t i=n;i<n+32;i++)assert(v[j][i]==12345.0f);
        printf("replay position=%u candidates=%u violations=%u winner_retained=%d\n",positions[p],candidates,violations,v[3][winner]>=tau);
        assert(!violations&&v[3][winner]>=tau);
    }
    for(unsigned j=0;j<4;j++)free(v[j]);
    free(full);
}
int main(int argc,char **argv) {
    if((argc!=2&&argc!=3&&argc!=5)||!frame_load(argv[1],0))return 65;
    chip=argc>=3&&!strcmp(argv[2],"--chip");
    if(argc>=3&&!chip)return 65;
    if(chip) {
        unsigned char bytes[16384]; FILE *f=fopen(argv[1],"rb");assert(f);
        size_t off=48+16*(native_program.regalloc.num_vregs+native_program.regalloc.num_uvregs)+64*native_program.count;
        assert(!fseek(f,(long)off,SEEK_SET));size_t nb=fread(bytes,1,sizeof bytes,f);assert(!ferror(f));fclose(f);
        omega_gpu_session_lock();demand(omega_gpu_session_open()?0:-1);
        demand(omega_gpu_session_alloc_code(nb,&code));
        memcpy(code.cpu,bytes,nb);demand(omega_gpu_session_sync_to_gpu(&code));omega_gpu_session_unlock();
    }
    uint32_t counts[]={1,31,32,33,255,256,257,1027};
    float norms[]={FLT_TRUE_MIN,0x1p-60f,1,123.25f,0x1p60f,FLT_MAX};
    for(unsigned a=0;a<sizeof counts/sizeof *counts;a++)
    for(unsigned b=0;b<sizeof norms/sizeof *norms;b++)
    for(unsigned o=0;o<2;o++) if(run(counts[a],norms[b],o))return 1;
    puts("PASS: bounds enclose long-double reference with bounded slack; tails untouched; both orders/repetitions");
    if(argc==5)replay(argv[3],argv[4]);
    if(chip) {
        omega_gpu_session_lock();omega_gpu_session_free(&code);omega_gpu_session_unlock();omega_gpu_session_close();
        puts("PASS: generated machine code executed on GPU");return 0;
    }
    if(!frame_wrong_result())return 66;
    if(!run(1027,123.25f,0))return 3;
    puts("PASS: arithmetic mutant caught");
    assert(frame_load(argv[1],0));
    int changed=0;
    for(size_t i=0;i<native_program.count;i++)if(native_program.insns[i].op==BW_IR_FMUL) {
        BlackwellIRInsn *x=&native_program.insns[i];
        x->op=BW_IR_MOV_IMM;x->imm=bits(INFINITY);
        x->src1_vreg=x->src2_vreg=x->src3_vreg=-1;changed=1;break;
    }
    if(!changed||!run(1027,123.25f,0))return 3;
    puts("PASS: all-infinite radius mutant caught by bounded slack");return 0;
}
