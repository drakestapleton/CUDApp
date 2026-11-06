/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "cudapp_native.h"
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static unsigned long long buddy(const char *line,unsigned order,unsigned long long total) {
    const char *p=strstr(line,"zone"); if(!p)return total;
    p+=4; while(*p==' ')p++; while(*p&&*p!=' ')p++;
    unsigned long long n;
    for(unsigned o=0;o<32&&sscanf(p," %llu",&n)==1;o++) {
        if(o>=order)total+=n;
        while(*p==' ')p++;
        while(*p&&*p!=' ')p++;
    }
    return total;
}
int main(void) {
    const char *values[]={"", " ","0","1","-1","+1","18446744073709551615","18446744073709551616","-18446744073709551616","12junk","junk","\t12","+","-0","00000000000000000000000000000000000000000001"};
    for(size_t i=0;i<sizeof values/sizeof *values;i++) {
        char line[512]; snprintf(line,sizeof line,"MemFree: %s kB",values[i]);
        unsigned long long c=0; if(sscanf(line+8," %llu kB",&c)!=1)c=0;
        uint64_t state[2]={0,9},matched=0;
        assert(!omega_nvrm_counts(&matched,0,line,strlen(line),"MemFree",7,state,2));
        assert(matched==1&&state[1]==c);
        const char *zones[]={"Node 0, zone Normal ","zone DMA ","zone\tDMA ","no zone", "z", "zone DMA\t"};
        for(size_t z=0;z<sizeof zones/sizeof *zones;z++) {
            snprintf(line,sizeof line,"%s%s 2 3 4 5 6 7 8 9 10 11\n",zones[z],values[i]);
            for(unsigned order=0;order<=33;order++) {
                state[0]=order; state[1]=UINT64_MAX; uint64_t unused;
                assert(!omega_nvrm_counts(&unused,1,line,strlen(line),"",0,state,2));
                assert(state[1]==buddy(line,order,UINT64_MAX));
            }
        }
    }
    const char *mismatch[]={"MemFree", "MemFreeX: 12", "xMemFree: 12", "MemFre: 12", "", "MemFree: garbage"};
    for(size_t i=0;i<sizeof mismatch/sizeof *mismatch;i++) {
        uint64_t state[2]={0,9},matched;
        assert(!omega_nvrm_counts(&matched,0,mismatch[i],strlen(mismatch[i]),"MemFree",7,state,2));
        int m=strlen(mismatch[i])>7&&!strncmp(mismatch[i],"MemFree:",8);
        assert(matched==(uint64_t)m); assert(state[1]==(m?0:9));
    }
    puts("NVRM COUNTS DIFFERENTIAL PASS (decimal classes, token boundaries, orders 0..33; HOST_ONLY)");
}
