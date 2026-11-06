/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* External verification instrument for the existing 5120-wide final RMSNorm. */
#include "omega_blackwell_codegen_dec.h"
#include "omega_gpu_session.h"
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../standalone-oracles/oracle.h"

#define H 5120u
static void demand(int rc) {
    if (rc) { fprintf(stderr, "GPU: %s\n", omega_gpu_session_last_error()); exit(4); }
}
static void pointer(uint32_t *a, unsigned off, uint64_t p) {
    a[(off-0x380)/4]=(uint32_t)p; a[(off-0x380)/4+1]=(uint32_t)(p>>32);
}
static void scalar(uint32_t *a, unsigned off, float x) { memcpy(a+(off-0x380)/4,&x,4); }
static uint32_t rng=710;
static float random_value(void) {
    rng=(uint32_t)so_call(22,rng,0,0,0);
    return so_float(so_call(23,rng,0,0,0));
}
int main(int argc,char **argv) {
    if(argc!=4 || strcmp(argv[1],"--chip")) return 65;
    char *end=NULL; float envelope=strtof(argv[3],&end);
    if(!end || *end || !so_all(so_call(24,so_bits(envelope),0,0,0),so_call(37,so_bits(envelope),so_bits(0.0f),0,0),1)) return 65;
    float weights[H]; FILE *f=fopen(argv[2],"rb"); assert(f);
    assert(fread(weights,4,H,f)==H && fgetc(f)==EOF); fclose(f);
    float wm=0; for(unsigned j=0;j<H;j++) {assert(so_call(24,so_bits(weights[j]),0,0,0));wm=so_float(so_call(25,so_bits(wm),so_bits(weights[j]),0,0));}
    OmegaDecSpec spec={0};spec.kind=OMEGA_DEC_RES;spec.mode=OMEGA_DEC_RES_NORM;spec.width=H;
    static BlackwellIRProgram program;
    assert(!omega_blackwell_codegen_dec_prog(&spec,&program));
    size_t size=omega_bw_program_code_bytes(&program); assert(size);
    OmegaGpuMem code={0},data={0};OmegaBlackwellKernel kernel;
    omega_gpu_session_lock(); demand(omega_gpu_session_open()?0:-1);
    demand(omega_gpu_session_alloc_code(size,&code));
    assert(!omega_blackwell_kernel_encode_into(&program,code.cpu,size,&kernel));
    demand(omega_gpu_session_sync_to_gpu(&code));
    demand(omega_gpu_session_alloc(so_bytes(H,12,0),&data));
    float *x=data.cpu,*w=x+H,*y=w+H;
    uint32_t args[OMEGA_DEC_RES_WORDS]={0};
    pointer(args,OMEGA_DEC_RES_X,data.va);pointer(args,OMEGA_DEC_RES_NW,so_call(5,data.va,so_bytes(H,4,0),0,0));
    pointer(args,OMEGA_DEC_RES_Y,so_call(5,data.va,so_bytes(H,8,0),0,0));
    scalar(args,OMEGA_DEC_RES_EPS,1e-6f);scalar(args,OMEGA_DEC_RES_SCALE,1.0f);
    OmegaGpuLaunch launch={0};launch.code_va=code.va;
    launch.gpr_count=omega_bw_gpr_declared(kernel.gpr_count);
    if(launch.gpr_count<64)launch.gpr_count=64;
    launch.threads_x=256;launch.threads_y=1;launch.grid_x=launch.grid_y=1;
    launch.num_elements=H;launch.args=args;launch.n_args=OMEGA_DEC_RES_WORDS;launch.timeout_ms=20000;
    int exponents[]={-149,-126,-80,-20,-10,0,20,60,120};
    unsigned checks=0,failed=0;long double largest=0;
    for(unsigned wi=0;wi<2;wi++) for(unsigned e=0;e<sizeof exponents/sizeof *exponents;e++)
    for(unsigned pattern=0;pattern<5;pattern++) {
        for(unsigned j=0;j<H;j++) {
            float z=so_call(34,pattern,3,0,0)?so_float(so_call(27,j,pattern,0,0)):
                so_eq(pattern,3)?random_value():ldexpf(random_value(),(int)(int64_t)so_call(40,j,0,0,0));
            x[j]=ldexpf(z,exponents[e]);w[j]=wi?so_float(so_call(39,j,so_bits(wm),0,0)):weights[j];y[j]=NAN;
        }
        demand(omega_gpu_session_sync_to_gpu(&data));
        demand(omega_gpu_session_launch(&launch,NULL,NULL));
        demand(omega_gpu_session_sync_to_cpu(&data));
        /* Independent binary128 oracle: seeded Omega has no f128 type.
         * Keep square, sqrtl and reference comparison at their original precision. */
        long double square=0;
        for(unsigned j=0;j<H;j++) {failed=so_bad(failed,(int)so_call(24,so_bits(y[j]),0,0,0));square+=(long double)y[j]*y[j];}
        long double norm=sqrtl(square);if(norm>largest)largest=norm;
        int oracle_ok=norm<=envelope;
        failed=so_bad(failed,oracle_ok);
        if(!oracle_ok) printf("FAIL weight=%u exponent=%d pattern=%u norm=%.12Lg bound=%.12g\n",wi,exponents[e],pattern,norm,envelope);
        checks=(unsigned)so_call(5,checks,1,0,0);
    }
    omega_gpu_session_free(&data);omega_gpu_session_free(&code);omega_gpu_session_unlock();
    printf("RMSNorm envelope: %u cases, %u failures, largest norm %.12Lg, bound %.12g: %s\n",checks,failed,largest,envelope,failed?"FAIL":"PASS");
    return failed?1:0;
}
