/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_chip_enum.h"
#include <stdio.h>
#include <string.h>
uint64_t oracle_enum(uint64_t,uint64_t,uint64_t,uint64_t,uint64_t);
static unsigned cases;
static int plant;
static void check(uint64_t m,uint64_t a,uint64_t b,uint64_t c,uint64_t d) {
    uint64_t want=oracle_enum(m,a,b,c,d), got=omega_chip_enum_decide(m,a,b,c,d);
    if (plant && !cases) got ^= 1;
    if (got != want) { fprintf(stderr,"enum mismatch mode=%lu a=%lu b=%lu\n",m,a,b); exit(1); }
    ++cases;
}
int main(int argc,char **argv) {
    if (argc > 2 && !strcmp(argv[1],"--scan")) {
        OmegaGpuPciDev rows[1024];
        for (int cap=-1;cap<=3;cap++) {
            memset(rows,0xa5,sizeof rows);
            int n=omega_gpu_chip_scan(argv[2],rows,cap);
            printf("cap=%d n=%d\n",cap,n);
            for(int i=0;i<n && i<cap;i++) printf("%s %x %x %x\n",rows[i].bdf,rows[i].vendor,rows[i].device,rows[i].class_code);
            if(cap<=0 && rows[0].vendor != 0xa5a5) return 3;
        }
        printf("null=%d\n",omega_gpu_chip_scan(argv[2],NULL,1024));
        int n=omega_gpu_chip_scan(argv[2],rows,1024);
        for(int i=0;i<n;i++) printf("%s %x %x %x\n",rows[i].bdf,rows[i].vendor,rows[i].device,rows[i].class_code);
        return 0;
    }
    plant=argc>1;
    const uint64_t masks[]={0xffff,0xff,0x1f,7};
    for(uint64_t v=0;v<=0xffff;v++) {
        check(0,v,0x2e12,0x10de,0x2e12);
        check(0,0x10de,v,0x10de,0x2e12);
        check(1,v,0,0,0);
        if (!!omega_gpu_chip_find(v,0x2e12) != (v==0x10de)) return 4;
        if (!!omega_gpu_chip_find(0x10de,v) != (v==0x2e12)) return 5;
        for(unsigned m=0;m<4;m++) check(3,v,masks[m],0,0);
    }
    const uint64_t edges[]={0,1,6,7,8,30,31,32,254,255,256,65534,65535,65536,UINT32_MAX-1,UINT32_MAX,UINT64_MAX};
    for(unsigned i=0;i<sizeof edges/sizeof edges[0];i++) {
        uint64_t a=edges[i];
        check(1,a,0,0,0); check(2,a,0,0,0);
        for(unsigned m=0;m<4;m++) check(3,a,masks[m],0,0);
        for(unsigned j=0;j<sizeof edges/sizeof edges[0];j++) {
            uint64_t b=edges[j];
            check(4,a,b,0,0);check(4,a,b,1,0);
            char why[512],bdf[64];uint32_t sm=123;
            snprintf(bdf,sizeof bdf,"%04x:%02x:%02x.%x",(unsigned)a&0xffffu,(unsigned)b&0xffu,(unsigned)a&0x1fu,(unsigned)b&7u);
            if(omega_gpu_chip_admit_card(a,b,a,b,0,0,&sm,why,sizeof why)!=-1 || sm || !strstr(why,bdf)) return 6;
        }
    }
    for(uint64_t cls=0;cls<256;cls++) {
        check(2,cls<<16,0,0,0);check(2,(cls<<16)|1,0,0,0);check(2,(cls<<16)|0xffff,0,0,0);
        check(2,(UINT64_C(1)<<32)|(cls<<16),0,0,0);
        check(5,cls,0,0,0);
    }
    uint32_t sm=0;char why[512]={0};
    if(omega_gpu_chip_admit_card(0,0,0,0,0x10de,0x2e12,&sm,why,sizeof why) || sm!=121) return 7;
    printf("CHIP ENUM DIFFERENTIAL PASS cases=%u\n",cases);return 0;
}
