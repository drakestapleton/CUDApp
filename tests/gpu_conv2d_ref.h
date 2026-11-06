/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef GPU_CONV2D_REF_H
#define GPU_CONV2D_REF_H
#include <math.h>
#include <stdint.h>
#include <string.h>
/* Independent numeric decoding; no emitted IR, seeded plan, or simulator math. */
static double cv_value(const void *p,size_t i,unsigned t) {
 if(!t) return ((const float*)p)[i];
 uint16_t u=((const uint16_t*)p)[i];
 if(t==1) {uint32_t x=(uint32_t)u<<16;float f;memcpy(&f,&x,4);return f;}
 unsigned e=u>>10&31,m=u&1023; double x=e?ldexp(1.0+(double)m/1024,(int)e-15):ldexp((double)m,-24);
 return u&32768?-x:x;
}
static size_t cv_index(unsigned layout,size_t n,size_t c,size_t y,size_t x,size_t C,size_t H,size_t W) {
 return layout?((n*H+y)*W+x)*C+c:((n*C+c)*H+y)*W+x;
}
static void cv_reference(const OmegaGpuConv2dShape *s,const void *a,const void *b,double *out,double *mag,int wrong) {
 size_t oh=(s->height+s->pad_top+s->pad_bottom-s->dilation_h*(s->kernel_h-1)-1)/s->stride_h+1;
 size_t ow=(s->width+s->pad_left+s->pad_right-s->dilation_w*(s->kernel_w-1)-1)/s->stride_w+1;
 size_t ci=s->in_channels/s->groups,co=s->out_channels/s->groups;
 for(size_t n=0;n<s->batch;n++) for(size_t o=0;o<s->out_channels;o++)
 for(size_t y=0;y<oh;y++) for(size_t x=0;x<ow;x++) {
  double sum=0,absum=0;
  for(size_t c=0;c<ci;c++) for(size_t ky=0;ky<s->kernel_h;ky++) for(size_t kx=0;kx<s->kernel_w;kx++) {
   int64_t iy=(int64_t)(y*s->stride_h+ky*s->dilation_h)-s->pad_top;
   int64_t ix=(int64_t)(x*s->stride_w+kx*s->dilation_w)-s->pad_left;
   if(iy<0||ix<0||iy>=s->height||ix>=s->width) continue;
   size_t ic=c+(wrong?0:o/co)*ci;
   double product=cv_value(a,cv_index(s->layout,n,ic,iy,ix,s->in_channels,s->height,s->width),s->dtype)*
     cv_value(b,((o*ci+c)*s->kernel_h+ky)*s->kernel_w+kx,s->dtype);
   sum+=product;absum+=fabs(product);
  }
  size_t dst=cv_index(s->layout,n,o,y,x,s->out_channels,oh,ow);out[dst]=sum;mag[dst]=absum;
 }
}
#endif
