/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CUDAPP_MIGRATION_PENDING_C_CM_SORT 1
#define rx_cm_sort rx_cm_sort_oracle
#include "runtime/rx_cm_sort.h"
#undef rx_cm_sort
#undef RX_CM_SORT_H
#undef CUDAPP_MIGRATION_PENDING_C_CM_SORT
#define CUDAPP_MIGRATION_PENDING_C_CM_SORT 0
#include "runtime/rx_cm_sort.h"
static uint64_t state=0x5258534f5254ULL;
static uint64_t next(void){state^=state<<13;state^=state>>7;state^=state<<17;return state;}
static void test(size_t n,unsigned mode){
    double *a=calloc(n?n:1,sizeof *a),*b=calloc(n?n:1,sizeof *b);
    if(!a||!b)exit(2);
    const uint64_t edge[]={0,UINT64_C(0x8000000000000000),UINT64_C(0x7ff0000000000000),UINT64_C(0xfff0000000000000),UINT64_C(0x7ff8000000000000),UINT64_C(0xfff8000000000000),1,UINT64_C(0x8000000000000001)};
    for(size_t i=0;i<n;i++){
        uint64_t u=next();
        if(mode==0)memcpy(a+i,&u,8);
        else if(mode==1)a[i]=(double)(i%7);
        else if(mode==2)a[i]=(double)(n-i);
        else memcpy(a+i,&edge[i%(sizeof edge/sizeof *edge)],8);
    }
    memcpy(b,a,n*sizeof *a);
    if(rx_cm_sort(a,n)||rx_cm_sort_oracle(b,n)||memcmp(a,b,n*sizeof *a)){fprintf(stderr,"CM SORT differential FAIL n=%zu mode=%u\n",n,mode);exit(1);}
    free(a);free(b);
}
int main(void){
    const size_t n[]={0,1,2,10,255,256,257,511,512,513,767,768,769,1023,1024,1025,2047,2048,2049,4095,4096,4097};
    for(size_t i=0;i<sizeof n/sizeof *n;i++)for(unsigned mode=0;mode<4;mode++)test(n[i],mode);
    for(unsigned i=0;i<100;i++)test((size_t)(next()%8193),i%4);
    puts("CM SORT differential PASS");return 0;
}
