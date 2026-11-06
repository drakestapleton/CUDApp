/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "cudapp_native.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
int main(void) {
 const uint32_t a[]={0,1,2,7,8,9,31,32,33,63,64,65,255,256,257,65535,65536,65537,UINT32_MAX};
 for(unsigned i=0;i<sizeof a/sizeof *a;i++) for(unsigned j=0;j<sizeof a/sizeof *a;j++) {
  uint64_t r=99; uint64_t want=(uint64_t)32*a[i]*a[j]>65536;
  if(omega_arpfreg(&r,a[i],a[j])||r!=want) return 1;
 }
 float x[4096];
 const uint32_t cls[]={0,0x80000000,1,0x80000001,0x7f800000,0xff800000,0x7fc00000,0x7f800001,0xc479c000,0xc479bfff,0xc479c001};
 for(unsigned i=0;i<4096;i++) memcpy(x+i,cls+i%(sizeof cls/sizeof *cls),4);
 for(unsigned n=0;n<=4096;n++) {
  uint64_t want=0,r=99; for(unsigned i=0;i<n;i++) if(x[i]==-999.0f) want++;
  if(omega_pfcount(&r,x,n)||r!=want) return 2;
 }
 puts("PASS prefill host register policy and float-class poison differential"); return 0;
}
