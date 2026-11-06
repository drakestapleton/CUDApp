/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_gpu_scan.h"
#include "cudapp_native.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
int scan_c_prefix(const float *,size_t,float *);
static uint32_t bits(float f){uint32_t u;memcpy(&u,&f,4);return u;}
static float value(uint32_t u){float f;memcpy(&f,&u,4);return f;}
static void equal(float a,float b){assert((isnan(a)&&isnan(b))||bits(a)==bits(b));}
int main(void){
 const uint32_t cls[]={0,0x80000000u,1,0x80000001u,0x3f800000u,0xbf800000u,0x7f800000u,0xff800000u,0x7fc00001u,0x4b000000u};
 const size_t ns[]={1,31,32,33,63,64,65,1023,1024,1025,2049};
 for(unsigned k=0;k<sizeof ns/sizeof *ns;k++) for(unsigned c=0;c<sizeof cls/sizeof *cls;c++){
  size_t n=ns[k],nt=(n+31)/32;
  float *x=malloc(n*4),*a=malloc(n*4),*b=malloc(n*4),*tot=malloc(nt*4);
  assert(x&&a&&b&&tot);
  for(size_t i=0;i<n;i++)x[i]=value(cls[(i+c)%(sizeof cls/sizeof *cls)]);
  assert(!scan_c_prefix(x,n,a));assert(!omega_gpu_prefix_sum_reference(x,n,b));
  for(size_t i=0;i<n;i++)equal(a[i],b[i]);
  memcpy(a,x,n*4);memcpy(b,x,n*4);
  for(size_t t=0;t<nt;t++)tot[t]=value(cls[(t+c)%(sizeof cls/sizeof *cls)]);
  for(size_t i=32;i<n;i++)a[i]=tot[i/32-1]+a[i];
  uint64_t r=9;assert(!omega_ar_carry(&r,tot,nt,b,n)&&!r);
  for(size_t i=0;i<n;i++)equal(a[i],b[i]);
  if(nt>1){r=0;assert(!omega_ar_carry(&r,tot,0,b,n)&&r==1);}
  free(x);free(a);free(b);free(tot);
 }
 puts("SCAN_REFERENCE_REST differential PASS");return 0;
}
