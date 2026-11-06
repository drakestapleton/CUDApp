/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#define CUDAPP_API_HOSTREST_DIFF 1
#include "omega_gpu_api_hostrest.h"
#include <assert.h>
#include <string.h>
#include <math.h>
static const uint32_t edges[]={0,1,2,5,6,7,15,16,17,31,32,33,47,48,49,50,51,56,57,58,127,128,129,255,256,257,511,512,513,999,1000,1001,4095,4096,4097,65535,65536,65537,UINT32_MAX};
int main(void) {
    for(unsigned i=0;i<sizeof edges/sizeof *edges;i++) for(unsigned j=0;j<sizeof edges/sizeof *edges;j++) {
        uint64_t p[16]={edges[i],edges[j],1,8,1,1};
        for(unsigned op=0;op<23;op++) {
            uint64_t q[16]; memcpy(q,p,sizeof q);
            if(op==0)q[2]=1;
            if(op==2)q[2]=i%3;
            if(op==3){q[0]=edges[i];q[1]=edges[j];}
            if(op==8){q[0]=(edges[i]%129)*512u;q[1]=16;q[2]=1;q[3]=8;q[4]=j%3;q[5]=1;}
            if(op==9)q[1]=edges[j]?edges[j]:1;
            if(op==10){q[1]=1;q[2]=8;q[3]=edges[j];q[4]=i%2;}
            if(op==12){q[0]=i%7;q[1]=1;q[2]=edges[j];q[3]=4096;}
            if(op==13){q[1]=edges[j];q[2]=edges[i];q[3]=32;q[4]=4096;}
            if(op==15){q[1]=32;q[3]=j%3;}
            if(op==22)q[1]=edges[j]?edges[j]:1;
            (void)omega_ah_call(op,q);
        }
    }
    for(unsigned kind=0;kind<3;kind++) for(unsigned mask=0;mask<256;mask++) {
        uint64_t p[16]={kind};
        for(unsigned b=0;b<8;b++)p[b+1]=(mask>>b)&1;
        (void)omega_ah_call(23,p);
        uint64_t q[16]={4096u+mask*512u,16,1,kind};
        (void)omega_ah_call(24,q);
    }
    puts("API_HOSTREST differential PASS");
    return 0;
}
