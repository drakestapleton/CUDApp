/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef RX_CM_SORT_H
#define RX_CM_SORT_H
#include "cudapp_native.h"
#include <stdlib.h>
#include <string.h>
#ifndef CUDAPP_MIGRATION_PENDING_C_CM_SORT
#define CUDAPP_MIGRATION_PENDING_C_CM_SORT 0
#endif
#if CUDAPP_MIGRATION_PENDING_C_CM_SORT
/* Total comparator: resolves formerly unspecified NaN and signed-zero ties. */
static int rx_cm_cmp(const void *pa, const void *pb) {
    double a, b; uint64_t ab, bb;
    memcpy(&a,pa,8); memcpy(&b,pb,8); memcpy(&ab,pa,8); memcpy(&bb,pb,8);
    if (a != a) { if (b == b) return 1; }
    else { if (b != b) return -1; if (a < b) return -1; if (a > b) return 1; }
    return (ab > bb) - (ab < bb);
}
#endif
static inline int rx_cm_sort(double *values, size_t n) {
#if CUDAPP_MIGRATION_PENDING_C_CM_SORT
    qsort(values,n,sizeof *values,rx_cm_cmp); return 0;
#else
    if (n < 2) return 0;
    if (n > SIZE_MAX / sizeof *values) return -1;
    uint64_t left[256]={0}, right[256]={0}, out[256]={0}, meta[4]={0}, done;
    for (size_t i=0;i<n;i+=256) {
        size_t count=n-i<256?n-i:256;
        memcpy(left,values+i,count*sizeof *values);meta[0]=count;meta[1]=0;meta[2]=0;
        if (omega_cm_sort(&done,left,256,right,256,out,256,meta,4) || done!=count) return -1;
        memcpy(values+i,left,count*sizeof *values);
    }
    if (n <= 256) return 0;
    double *scratch = malloc(n * sizeof *values);
    if (!scratch) return -1;
    double *src=values,*dst=scratch;
    for (size_t width=256;width<n;) {
        for (size_t start=0;start<n;) {
            size_t mid=n-start<width?n:start+width;
            size_t end=n-mid<width?n:mid+width;
            size_t a=start,b=mid,k=start;
            while(a<mid || b<end) {
                size_t na=mid-a<256?mid-a:256,nb=end-b<256?end-b:256;
                memcpy(left,src+a,na*sizeof *values);memcpy(right,src+b,nb*sizeof *values);
                meta[0]=na;meta[1]=nb;meta[2]=1;
                if (omega_cm_sort(&done,left,256,right,256,out,256,meta,4) || !done || done>256 || meta[2]>na || meta[3]>nb || done!=meta[2]+meta[3]) { free(scratch);return -1; }
                memcpy(dst+k,out,(size_t)done*sizeof *values);
                a+=(size_t)meta[2];b+=(size_t)meta[3];k+=(size_t)done;
            }
            start=end;
        }
        double *swap=src;src=dst;dst=swap;
        if(width>n/2)break;
        width*=2;
    }
    if(src!=values)memcpy(values,src,n*sizeof *values);
    free(scratch);return 0;
#endif
}
#endif
