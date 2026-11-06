/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef GPU_DNN_ROWS_REF_H
#define GPU_DNN_ROWS_REF_H
#include <math.h>
#include <stdlib.h>
#include <string.h>
static double dr_input(const void *p, size_t i, unsigned dtype) {
    if (dtype == 0) return ((const float*)p)[i];
    uint16_t h=((const uint16_t*)p)[i];
    if (dtype == 1) { uint32_t b=(uint32_t)h<<16; float f; memcpy(&f,&b,4); return f; }
    _Float16 f; memcpy(&f,&h,2); return (double)f;
}
static void dr_reference(const OmegaGpuDnnRowsShape *s,const void *p,double *o) {
    for (unsigned r=0;r<s->rows;++r) {
        size_t base=(size_t)r*s->length;
        double mean=0, v=0, mx=-INFINITY;
        for (unsigned j=0;j<s->length;++j) { double x=dr_input(p,base+j,s->dtype); mean+=x; if(x>mx) mx=x; }
        mean/=s->length;
        for (unsigned j=0;j<s->length;++j) {
            double x=dr_input(p,base+j,s->dtype);
            if(s->op==0) v+=exp(x-mx);
            else { double d=s->op==1?x-mean:x; v+=d*d; }
        }
        if(s->op) v=sqrt(v/s->length+(double)s->epsilon);
        for(unsigned j=0;j<s->length;++j) { double x=dr_input(p,base+j,s->dtype); o[base+j]=s->op==0?exp(x-mx)/v:(s->op==1?x-mean:x)/v; }
    }
}
static int dr_equal(const float *o,const double *r,size_t n) {
    /* MODELED 16 gamma_2053 envelope; README records Wolfram derivation. */
    const double tol=16.0*(2053.0/16775163.0);
    for(size_t i=0;i<n;++i) if(!isfinite(o[i]) || fabs((double)o[i]-r[i])>tol*(1+fabs(r[i]))) return 0;
    return 1;
}
#endif
