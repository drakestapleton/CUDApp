/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Explicit --sim returns before ANY session call. --chip requires an operator
 * authorized exclusive window; it is never part of CARRIED_HOST_TESTS. */
#define fadd omega_em_fadd
#define ffma omega_em_ffma
#include "omega_bw_em.h"
#undef fadd
#undef ffma
#include "omega_gpu_session.h"
#include "bw_warp_sim.h"
#include <float.h>
#include <stdlib.h>

static uint64_t dbits(double x){uint64_t u;memcpy(&u,&x,8);return u;}
static int program(BlackwellIRProgram *p,unsigned mode,int negative) {
    omega_bw_ir_init(p);Em em={.p=p};Em *e=&em;
    int u=UV64(e),out=V64(e),a=V64(e),b=V64(e),c=V64(e),d=V64(e),tid=V(e),pred=V(e);
    if(e->err)return -1;
    ldcu64(e,u,0x358);ldc64(e,out,0x380);ldc64(e,a,0x388);ldc64(e,b,0x390);ldc64(e,c,0x398);
    s2r(e,tid,BW_SR_TID_X);imadw(e,out,tid,16,out);
    BlackwellIRInsn in=I0(negative?BW_IR_DMUL:BW_IR_DFMA);in.dst_vreg=d;in.src1_vreg=a;in.src2_vreg=b;in.src3_vreg=c;emit(e,in,K_VAR);
    stg(e,out,0,d,u);in=I0(BW_IR_STG_E);in.src1_vreg=out;in.src2_vreg=d;in.src2_subreg=1;in.ureg=u;in.imm=4;emit(e,in,K_STORE);
    in=I0(BW_IR_DSETP);in.src1_vreg=a;in.src2_vreg=b;in.imm=0x03f00000u|(mode<<12);emit(e,in,K_PRED);
    movi(e,pred,0);in=I0(BW_IR_MOV_IMM);in.dst_vreg=pred;in.imm=1;in.predicate_p0=true;emit(e,in,K_FIXED);
    stg(e,out,8,pred,u);movi(e,pred,0x12345678);stg(e,out,12,pred,u);tail(e);
    return e->err?-1:omega_bw_regalloc_solve(p);
}
static int compare(const uint32_t *out,double a,double b,double c,unsigned mode) {
    double want=fma(a,b,c);unsigned rel=(isnan(a)||isnan(b))?8u:a<b?1u:a>b?4u:2u;
    for(unsigned i=0;i<32;i++) {
        uint64_t raw=(uint64_t)out[i*4]|((uint64_t)out[i*4+1]<<32);double got;memcpy(&got,&raw,8);
        if((isnan(want)?!isnan(got):raw!=dbits(want))||out[i*4+2]!=!!(mode&rel)||out[i*4+3]!=0x12345678){
            fprintf(stderr,"f64 probe lane=%u mode=%u got=%a want=%a predicate=%u/%u canary=%08x\n",
                    i,mode,got,want,out[i*4+2],!!(mode&rel),out[i*4+3]);return 1;
        }
    }
    return 0;
}
static void args(uint32_t *w,uint64_t addr,double a,double b,double c) {
    uint64_t q[]={addr,dbits(a),dbits(b),dbits(c)};memcpy(w,q,sizeof q);
}
static int execute(BlackwellIRProgram *p,int chip,double a,double b,double c,unsigned mode) {
    uint32_t out[32*4];memset(out,0xee,sizeof out);
    if(!chip) {
        WsSim s={.p=p,.threads=32};args(&s.cbank[0x380/4],(uintptr_t)out,a,b,c);
        s.nranges=1;s.ranges[0].lo=(uintptr_t)out;s.ranges[0].hi=(uintptr_t)out+sizeof out;
        if(ws_run(&s,1,1)){fprintf(stderr,"f64 probe sim: %s\n",s.msg);return 1;}
        return compare(out,a,b,c,mode);
    }
    OmegaGpuMem code={0},mem={0};OmegaBlackwellKernel k;int failed=1;
    size_t need=omega_bw_program_code_bytes(p);
    if(omega_gpu_session_alloc_code(need,&code)||omega_blackwell_kernel_encode_into(p,code.cpu,need,&k)||
       omega_gpu_session_sync_to_gpu(&code)||omega_gpu_session_alloc(sizeof out,&mem))goto done;
    memcpy(mem.cpu,out,sizeof out);uint32_t words[8];args(words,mem.va,a,b,c);
    OmegaGpuLaunch launch={.code_va=code.va,.gpr_count=omega_bw_gpr_declared(p->regalloc.peak_gpr_usage),.threads_x=32,.threads_y=1,
                          .grid_x=1,.grid_y=1,.num_elements=32,.args=words,.n_args=8,.timeout_ms=20000};
    if(omega_gpu_session_sync_to_gpu(&mem)||omega_gpu_session_launch(&launch,NULL,NULL)||omega_gpu_session_sync_to_cpu(&mem))goto done;
    failed=compare(mem.cpu,a,b,c,mode);
done:
    omega_gpu_session_free(&mem);omega_gpu_session_free(&code);return failed;
}
int main(int argc,char **argv) {
    int chip=argc==2&&!strcmp(argv[1],"--chip");
    if(argc!=2||(!chip&&strcmp(argv[1],"--sim"))){fputs("usage: codegen_f64_probe --sim | --chip (authorized exclusive window only)\n",stderr);return 2;}
    BlackwellIRProgram *p=malloc(sizeof *p);if(!p)return 2;
    /* Finite fused cancellation, rounding tie, subnormals, signed zero, infinity,
     * unordered NaN. Every comparison mask is exercised in each case. */
    const double cases[][3]={{0x1.0000000000001p0,0x1.ffffffffffffep-1,-1.0},{1.0,0x1p-53,1.0},
        {DBL_TRUE_MIN,1.0,DBL_TRUE_MIN},{-0.0,1.0,-0.0},{INFINITY,0.0,1.0},{NAN,1.0,0.0},{-1.0,2.0,3.0}};
    int failed=0;
    /* The negative control ALWAYS runs in host simulation, before session open.
     * DMUL in place of DFMA must fail the host fused-result check. */
    if(program(p,6,1)||!execute(p,0,cases[0][0],cases[0][1],cases[0][2],6)){free(p);puts("codegen-f64 probe: FAIL (negative control)");return 1;}
    if(chip){omega_gpu_session_lock();if(!omega_gpu_session_open()){fprintf(stderr,"f64 session: %s\n",omega_gpu_session_last_error());omega_gpu_session_unlock();free(p);return 1;}}
    for(unsigned i=0;i<sizeof cases/sizeof *cases&&!failed;i++)for(unsigned m=0;m<16&&!failed;m++)
        failed=program(p,m,0)||execute(p,chip,cases[i][0],cases[i][1],cases[i][2],m);
    if(chip)omega_gpu_session_unlock();
    free(p);
    printf("codegen-f64 probe %s: %s (DFMA, all DSETP modes, DMUL negative control)\n",chip?"chip":"sim",failed?"FAIL":"PASS");return failed;
}
