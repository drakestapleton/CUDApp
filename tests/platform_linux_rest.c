/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include "omega_gpu_platform_linux_rest_oracle.h"
#ifndef PLAT_REST_IMPL
#define PLAT_REST_IMPL "cudapp_om_plat_rest.c"
#endif
#include PLAT_REST_IMPL
static int one(uint64_t mode, const uint64_t m[16], int red) {
    uint64_t q[16]={0}, expected[16]={0}, result;
    uint64_t reference=plat_rest_c(mode,m,expected);
    if(omega_plat_rest(&result,mode,m,16,q,16) || result!=reference+(unsigned)red ||
       memcmp(q,expected,sizeof q)) {
        fprintf(stderr,"FAIL platform rest mode=%llu m0=%llu m1=%llu\n",
                (unsigned long long)mode,(unsigned long long)m[0],(unsigned long long)m[1]);
        return 1;
    }
    return 0;
}
int main(int argc,char **argv) {
    (void)argv;
    const uint64_t values[]={0,1,2,3,4,5,6,7,8,9,10,11,12,46,47,48,57,58,59,63,64,65,
        99,100,101,102,103,108,109,110,
        999,1000,1001,4095,4096,4097,65535,65536,65537,268435455,268435456,268435457,
        UINT32_MAX-1,UINT32_MAX,(uint64_t)UINT32_MAX+1,
        (UINT64_C(1)<<40)-4097,(UINT64_C(1)<<40)-4096,(UINT64_C(1)<<40)-4095,
        (UINT64_C(1)<<40)-1,UINT64_C(1)<<40,(UINT64_C(1)<<40)+1,UINT64_MAX-1,UINT64_MAX};
    uint64_t seeds[14][16]={{4096,268435456},{6,'m','e','m','f','d',':'},
        {1,1,1,1,2,256},{1,1},{1000},{0,8},{4,1,0,0},{0,0},{1},{1000000},
        {64,5},{UINT32_MAX},{1,1,3,MAP_SHARED,MAP_SHARED,MAP_ANONYMOUS},{0}};
    for(unsigned mode=0;mode<14;++mode) for(unsigned field=0;field<16;++field)
        for(unsigned i=0;i<sizeof values/sizeof values[0];++i) {
            uint64_t m[16]; memcpy(m,seeds[mode],sizeof m); m[field]=values[i];
            if(one(mode,m,argc>1)) return 1;
        }
    uint64_t m[16]={0};
    if(one(UINT64_MAX,m,0)) return 1;
    uint64_t full[16]={4,1,1,1};
    if(one(6,full,0)) return 1; /* All extra slots occupied: loop exhausts its full extent. */
    uint64_t result,q[16];
    if(omega_plat_rest(&result,0,m,15,q,16)!=32) return 1;
    if(omega_plat_rest(&result,0,m,16,q,15)!=32) return 1;
    int fd=memfd_create("platform-rest",MFD_CLOEXEC);
    if(fd<0) { perror("memfd_create"); return 1; }
    char path[64], link[64]={0};
    snprintf(path,sizeof path,"/proc/self/fd/%d",fd);
    ssize_t n=readlink(path,link,sizeof link-1);
    close(fd);
    if(n<7) { perror("readlink"); return 1; }
    m[0]=(uint64_t)n;
    for(unsigned i=0;i<7;++i) m[i+1]=(unsigned char)link[i];
    if(omega_plat_rest(&result,1,m,16,q,16) || result!=1) {
        fputs("FAIL platform rest live memfd marker\n",stderr); return 1;
    }
    if(n>=6 && !memcmp(link,"memfd:",6)) {
        fputs("FAIL platform rest old marker negative control\n",stderr); return 1;
    }
    puts("PASS platform rest live memfd: original marker parser rejected real /memfd: link");
    puts("PASS platform rest source-partition differential and ABI refusals");
    return 0;
}
