/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "cudapp_native.h"
#include <stdint.h>
#include <stdio.h>
static uint32_t perm(uint32_t i) { uint32_t p=32; if(i<128)return i<p?i:64+i-p; return i-128<p?p+i-128:i; }
int main(void) {
 uint32_t tab[14336];
 const uint32_t heads[]={16,24};
 for(unsigned h=0;h<2;h++) {
  uint32_t q=heads[h],qn=q*256,kn=1024,n=(q+4)*512; uint64_t r=0;
  if(omega_grtable(&r,1,q,4,tab,n)||r!=1)return 1;
  for(uint32_t i=0;i<n;i++) {
   uint32_t s;
   if(i<qn)s=i/256*512+perm(i%256);
   else if(i<qn+kn)s=q*512+(i-qn)/256*256+perm(i%256);
   else if(i<qn+2*kn)s=q*512+i-qn;
   else s=(i-qn-2*kn)/256*512+256+(i-qn-2*kn)%256;
   if(tab[i]!=s)return 2;
  }
 }
 puts("PASS graph table rotary tail and complete q/k/v/gate mapping");return 0;
}
