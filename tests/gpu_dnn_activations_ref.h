/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef GPU_DNN_ACTIVATIONS_REF_H
#define GPU_DNN_ACTIVATIONS_REF_H
#include <math.h>
#include <stdint.h>
#include <string.h>
/* Oracle only: double libm; no polynomial or emitted-kernel helper reused. */
static double da_input(const void *p,size_t i,unsigned dtype) {
    if (!dtype) return ((const float *)p)[i];
    uint16_t u=((const uint16_t *)p)[i];
    if (dtype==1) { uint32_t v=(uint32_t)u<<16;float f;memcpy(&f,&v,4);return f; }
    int exponent=(u>>10)&31; unsigned mantissa=u&1023;
    double value=exponent ? ldexp(1.0+(double)mantissa/1024.0,exponent-15) : ldexp((double)mantissa,-24);
    return u&32768 ? -value : value;
}
static double da_reference(double x,unsigned op) {
    if (op==0) return x<0.0 ? 0.0 : x;
    if (op==1) return x*0.5*erfc(-x/sqrt(2.0)); /* erfc avoids negative-tail cancellation. */
    if (op==2) return x*0.5*(1.0+tanh(sqrt(2.0/acos(-1.0))*(x+0.044715*x*x*x)));
    return x>=0.0 ? x/(1.0+exp(-x)) : x*exp(x)/(1.0+exp(x));
}
static int da_equal(float got,double want) {
    return isfinite(got) && fabs((double)got-want)<=0.000003+0.000003*fabs(want);
}
#endif
