/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Independent retained emitter oracle; never opens a device. The production
 * source is included so its static legacy generator remains test-only. */
#define CUDAPP_MIGRATION_PENDING_C_API_KGEN 1
#define OMEGA_API_PLAN_ORACLE 1
#include "../src/omega_gpu_elementwise_api.c"
#include "omega_cubin.h"

static int cubin_same(const uint8_t *a, size_t na, const uint8_t *b, size_t nb) {
    static const OmegaCubinParam params[] = {
        {0x380,8,1},{0x388,8,1},{0x390,8,1},
        {0x398,4,0},{0x39c,4,0},{0x3a0,4,0},{0x3a4,4,0},{0x3a8,4,0}
    };
    uint8_t *ca=NULL,*cb=NULL; size_t nca=0,ncb=0; char err[128];
    int ra=omega_cubin_pack("api_kgen",a,na,params,8,0x3ac,64,4096,&ca,&nca,err,sizeof err);
    int rb=omega_cubin_pack("api_kgen",b,nb,params,8,0x3ac,64,4096,&cb,&ncb,err,sizeof err);
    int same=ra==0 && rb==0 && nca==ncb && memcmp(ca,cb,nca)==0;
    omega_cubin_free(ca);omega_cubin_free(cb);return same;
}
static int shape_partition(void) {
    /* om/components/api_ew_plan.om: zero admission; even/128 divisibility;
     * 1024-row and 131072-element clamps; 2048/16384 dimension caps;
     * reconvergence lower/upper bound and the mutant 0..3 enum. */
    static const uint64_t ns[]={0,1,2,3,4,127,128,129,1023,1024,1025,131071,131072,131073};
    static const uint64_t ds[]={0,1,2,3,127,128,129,2047,2048,2049,16383,16384,16385};
    unsigned admitted=0,refused=0;
    for(unsigned mode=0;mode<=6;mode++)for(size_t n=0;n<sizeof ns/sizeof *ns;n++)
      for(size_t d=0;d<sizeof ds/sizeof *ds;d++)for(uint64_t lo=0;lo<=4;lo++) {
        uint64_t c[16]={ns[n],ds[d],lo,3},om[16];
        memcpy(om,c,sizeof c);
        uint64_t cr=ap_c(AP_EW,mode,c);
        int orc=ap_plan(AP_EW,mode,om);
        if((cr==0)!=(orc==0) || (cr==0 && memcmp(c,om,sizeof c)!=0)) {
            fprintf(stderr,"FAIL shape mode=%u n=%llu dim=%llu lo=%llu\n",mode,
              (unsigned long long)ns[n],(unsigned long long)ds[d],(unsigned long long)lo);
            return 1;
        }
        if(cr==0)admitted++;else refused++;
      }
    printf("PASS API EW source-derived boundary partition: admitted=%u refused=%u\n",admitted,refused);
    return 0;
}
int main(void) {
    BlackwellIRProgram *c=calloc(1,sizeof *c),*om=calloc(1,sizeof *om);
    uint8_t *text=malloc(BW_MAX_IR_INSNS*16u);
    unsigned accepted=0,refused=0;
    if(!c||!om||!text)return 2;
    if(shape_partition()!=0)return 1;
    /* Shape is deliberately absent: EW IR depends only on op, mutant and
     * test_path. n/dim/epsilon/sieve limits reside in launch constant memory.
     * Exhaust the complete generator spec, not a sampling of runtime shapes. */
    for(int op=0;op<=OMEGA_GPU_EW_RC_REFUSE_JOIN_ONLY+1;op++)
      for(int mut=0;mut<=3;mut++)for(int path=0;path<=1;path++) {
        OmegaBlackwellKernel k={0};uint64_t err=0;size_t ntext=0;
        memset(c,0,sizeof *c);memset(om,0,sizeof *om);
        int cr=build_kernel((OmegaGpuEwOp)op,mut,&k,c,path);
        int orc=omega_gpu_om_ir_replay(omega_ew_ir,op,mut,path,0,om,&err);
        int ok=orc==0 && om->regalloc.peak_gpr_usage<=EW_GPR_BUDGET;
        if((cr==0)!=ok){fprintf(stderr,"FAIL disposition op=%d mut=%d path=%d c=%d om=%d err=%llu peak=%u\n",op,mut,path,cr,orc,(unsigned long long)err,om->regalloc.peak_gpr_usage);return 1;}
        if(cr!=0){refused++;continue;}
#ifdef CUDAPP_API_KGEN_RED
        om->insns[0].imm^=1u;
#endif
        if(c->count!=om->count || memcmp(c->insns,om->insns,c->count*sizeof c->insns[0])!=0){
          for(size_t i=0;i<c->count && i<om->count;i++)if(memcmp(&c->insns[i],&om->insns[i],sizeof c->insns[i])) {
            fprintf(stderr,"FAIL IR op=%d mut=%d path=%d at=%zu c_op=%d om_op=%d c_ctl=%x om_ctl=%x c_imm=%x om_imm=%x\n",op,mut,path,i,c->insns[i].op,om->insns[i].op,c->insns[i].control,om->insns[i].control,c->insns[i].imm,om->insns[i].imm);break;
          }return 1;
        }
        if(memcmp(&c->regalloc,&om->regalloc,sizeof c->regalloc)!=0){fprintf(stderr,"FAIL allocation op=%d mut=%d path=%d\n",op,mut,path);return 1;}
        if(omega_bw_encode_program(om,text,BW_MAX_IR_INSNS*16u,&ntext)!=0 || ntext!=k.code_size || memcmp(text,k.code,ntext)!=0){fprintf(stderr,"FAIL text op=%d mut=%d path=%d\n",op,mut,path);return 1;}
        if(!cubin_same(k.code,k.code_size,text,ntext)){fprintf(stderr,"FAIL cubin op=%d mut=%d path=%d\n",op,mut,path);return 1;}
        accepted++;free(k.code);
      }
    printf("PASS API EW complete spec IR, allocation, text and cubin: accepted=%u refused=%u\n",accepted,refused);
    free(text);free(om);free(c);return 0;
}
