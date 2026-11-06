/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Include the default-OFF host-policy C oracle, never the GX_C numerical path. */
#ifndef CUDAPP_MIGRATION_PENDING_C_GEMM_EX_HOST_POLICY
#define CUDAPP_MIGRATION_PENDING_C_GEMM_EX_HOST_POLICY 1
#endif
#include "../src/omega_gpu_gemm_ex.c"
#include <stdio.h>
#include <stdlib.h>

static unsigned long cases;
static void ck(uint64_t op, uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t want) {
    uint64_t got = 0;
    if (getenv("GEMM_POLICY_RED")) want ^= 1;
    if (omega_gemm_ex_policy(&got, op, a, b, c, d) || got != want) {
        fprintf(stderr, "FAIL policy op=%llu a=%llu b=%llu c=%llu d=%llu got=%llu want=%llu\n",
                (unsigned long long)op, (unsigned long long)a, (unsigned long long)b, (unsigned long long)c,
                (unsigned long long)d, (unsigned long long)got, (unsigned long long)want);
        exit(1);
    }
    cases++;
}
int main(void) {
    static const uint64_t v[] = {0,1,2,3,4,5,6,7,8,15,16,17,31,32,33,4095,4096,4097,16383,16384,16385,
                                32767,32768,32769,65535,65536,65537,UINT32_MAX-1,UINT32_MAX,UINT64_MAX};
    Gx g = {0};
    for (size_t i=0;i<sizeof v/sizeof *v;i++) for (size_t j=0;j<sizeof v/sizeof *v;j++) {
        ck(0,v[i],v[j],0,0,gx_min(v[i],v[j]));
        for (uint64_t tr=0;tr<2;tr++) {
            uint64_t st;
            g.x[0]=tr; g.p[5]=tr; g.p[2]=v[j]; g.p[3]=v[j];
            ck(1,tr,v[j],v[i],v[j],gx_arow(&g,v[i],v[j],&st)); ck(2,tr,v[j],0,0,st);
            ck(1,tr,v[j],v[j],v[i],gx_bcol(&g,v[i],v[j],&st)); ck(7,tr,v[j],0,0,st);
            ck(1,tr,v[j],v[i],v[j],gx_brow(&g,v[i],v[j],&st)); ck(2,tr,v[j],0,0,st);
        }
    }
    for (uint64_t op=0;op<10;op++) for (size_t i=1;i<sizeof v/sizeof *v;i++)
        for (uint64_t es=1;es<=8;es*=2) for (uint64_t ce=1;ce<=8;ce*=2) {
            /* Valid caller widths are 1,2,4,8 and nonzero bounded row lengths. Huge rows take the one-row path. */
            if (v[i] == UINT64_MAX) continue;
            ck(3,op,v[i],es,ce,gx_pack(op,v[i],es,ce));
        }
    for (uint64_t rn=1;rn<=3;rn++) for (uint64_t n=1;n<=33;n++) for (uint64_t ce=1;ce<=8;ce*=2)
        for (uint64_t delta=0;delta<3;delta++) {
            uint64_t ld=n*ce+delta;
            ck(4,rn,ld,n,ce,rn==1 || ld==n*ce);
            ck(5,rn,n,0,0,rn*n);
            for (uint64_t r=0;r<rn;r++) ck(6,r,n,ce,0,r*n*ce);
            memset(g.ga,0,sizeof g.ga); memset(g.gb,0xcc,sizeof g.gb);
            /* op 2 converts to bf16; compare the copy shell against independent row calls. */
            if (ce==2) {
                uint8_t ref[1024]; memset(ref,0xcc,sizeof ref);
                for(uint64_t r=0;r<rn;r++) if(gx_row(&g,2,OMEGA_GEMM_DT_BF16,n,g.ga,ref+r*ld)) return 2;
                if(gx_conv(&g,2,OMEGA_GEMM_DT_BF16,n,rn,g.ga,g.gb,ld,ce) || memcmp(ref,g.gb,(rn-1)*ld+n*ce)) return 3;
            }
        }
    /* Typed-view refusal is propagated before the padded memcpy shell can run. */
    if (gx_conv(&g,2,OMEGA_GEMM_DT_BF16,32769,2,g.ga,g.gb,65539,2) != GX_FAULT) return 4;
    printf("GEMM EX HOST POLICY DIFFERENTIAL PASS cases=%lu\n",cases);
    return 0;
}
