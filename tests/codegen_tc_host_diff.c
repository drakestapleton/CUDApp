/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_blackwell_codegen_tc.h"
#include "omega_blackwell_codegen_tc2.h"
#include "cudapp_native.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
int oracle_omega_tcgemm_spec_ok(const OmegaTcGemmSpec *);
uint32_t oracle_omega_tcgemm_shared_bytes(const OmegaTcGemmSpec *);
uint32_t oracle_omega_tcgemm_span(const OmegaTcGemmSpec *);
void oracle_omega_tcgemm_permute_x(const float *,uint32_t,uint16_t *);
uint16_t oracle_omega_tcgemm_w_bf16(uint8_t);
int oracle_omega_tc2_spec_ok(const OmegaTc2Spec *);
uint32_t oracle_omega_tc2_shared_bytes(const OmegaTc2Spec *);
uint32_t oracle_omega_tc2_span(const OmegaTc2Spec *);
static unsigned cases;
static int red;
static int same(uint64_t a,uint64_t b) { cases++; return (a ^ (red && cases==1))==b; }
static int tc(OmegaTcGemmSpec s) {
 uint64_t v[16]={s.k,s.rt,s.ct,s.warps,(uint32_t)s.mutant},r;
 uint64_t want[3]={oracle_omega_tcgemm_spec_ok(&s),oracle_omega_tcgemm_shared_bytes(&s),oracle_omega_tcgemm_span(&s)};
 uint64_t got[3]={omega_tcgemm_spec_ok(&s),omega_tcgemm_shared_bytes(&s),omega_tcgemm_span(&s)};
 for(unsigned j=0;j<3;j++) if(omega_tc_host(&r,j,v,16)||!same(r,want[j])||got[j]!=want[j]) return 0;
 return 1;
}
static int tc2(OmegaTc2Spec s) {
 uint64_t v[16]={s.k,s.rt,s.ct,s.warps,(uint32_t)s.mutant,s.n,s.aqn,s.epi,s.direct},r;
 uint64_t want[3]={oracle_omega_tc2_spec_ok(&s),oracle_omega_tc2_shared_bytes(&s),oracle_omega_tc2_span(&s)};
 uint64_t got[3]={omega_tc2_spec_ok(&s),omega_tc2_shared_bytes(&s),omega_tc2_span(&s)};
 for(unsigned j=0;j<3;j++) if(omega_tc_host(&r,j+3,v,16)||!same(r,want[j])||got[j]!=want[j]) return 0;
 return 1;
}
static int nan16(uint16_t x) { return (x&0x7fff)>0x7f80; }
static int conv(uint32_t u,uint32_t k) {
 uint32_t raw[8]={u,u^0x80000000u,u+1,u-1,u,u,u,u}; float x[8]; uint16_t a[8],b[8];
 memcpy(x,raw,sizeof x); memset(a,0xa5,sizeof a);memset(b,0xa5,sizeof b);
 omega_tcgemm_permute_x(x,k,a);oracle_omega_tcgemm_permute_x(x,k,b);
 for(unsigned j=0;j<8;j++) if(!same(nan16(a[j])?0x7fff:a[j],nan16(b[j])?0x7fff:b[j])) return 0;
 return 1;
}
int main(int argc,char **argv) {
 (void)argv; red=argc>1;
 const uint32_t edge[]={0,1,2,3,4,5,7,8,9,15,16,17,31,32,33,63,64,65,127,128,129,255,256,257,4095,4096,4097,16383,16384,16385,32767,32768,32769,UINT32_MAX};
 if(omega_tcgemm_spec_ok(NULL)||omega_tcgemm_shared_bytes(NULL)||omega_tcgemm_span(NULL)||omega_tc2_spec_ok(NULL)||omega_tc2_shared_bytes(NULL)||omega_tc2_span(NULL)) return 1;
 for(uint32_t k=128;k<=32768;k+=128) for(uint32_t rt=1;rt<=8;rt*=2) for(uint32_t ct=1;ct<=4;ct*=2) for(uint32_t w=1;w<=16;w*=2) for(int m=-1;m<=5;m++) {
  OmegaTcGemmSpec s={.k=k,.rt=rt,.ct=ct,.warps=w,.mutant=m}; if(!tc(s)) goto fail;
 }
 for(uint32_t k=128;k<=16384;k+=128) for(uint32_t rt=2;rt<=8;rt*=2) for(uint32_t w=4;w<=16;w*=2) for(uint32_t aq=1;aq<=2;aq++) for(uint32_t ep=0;ep<=1;ep++) for(uint32_t dr=0;dr<=1;dr++) for(int m=0;m<=4;m++) {
  OmegaTc2Spec s={.k=k,.rt=rt,.ct=2,.warps=w,.mutant=m,.n=w*16,.aqn=aq,.epi=ep,.direct=dr}; if(!tc2(s)) goto fail;
 }
 for(unsigned f=0;f<9;f++) for(unsigned i=0;i<sizeof edge/sizeof *edge;i++) {
  uint32_t v[9]={128,2,2,4,0,64,1,0,0};v[f]=edge[i];
  OmegaTcGemmSpec s={.k=v[0],.rt=v[1],.ct=v[2],.warps=v[3],.mutant=(int)v[4]};if(!tc(s))goto fail;
  OmegaTc2Spec t={.k=v[0],.rt=v[1],.ct=v[2],.warps=v[3],.mutant=(int)v[4],.n=v[5],.aqn=v[6],.epi=v[7],.direct=v[8]};if(!tc2(t))goto fail;
 }
 /* Every strict output-size boundary, including adjacent and alignment values. */
 for(uint32_t rt=2;rt<=8;rt*=2) for(uint32_t w=4;w<=16;w*=2) {
  uint32_t lim=(1u<<23)/(64*rt); const uint32_t ns[]={lim-1,lim,lim+1,(lim/(16*w))*(16*w),(lim/(16*w)-1)*(16*w)};
  for(unsigned j=0;j<sizeof ns/sizeof *ns;j++) { OmegaTc2Spec t={.k=128,.rt=rt,.ct=2,.warps=w,.n=ns[j],.aqn=1};if(!tc2(t))goto fail; }
 }
 for(unsigned i=0;i<256;i++) { uint64_t v[16]={i},r; if(omega_tc_host(&r,7,v,16)||!same(r,oracle_omega_tcgemm_w_bf16(i))||r!=omega_tcgemm_w_bf16(i))goto fail; }
 const uint32_t fp[]={0,0x80000000,1,0x007fffff,0x00800000,0x3f800000,0x5f000000,0x5f7fffff,0x7f7fffff,0x7f800000,0xff800000,0x7f800001,0x7fc12345,0xffc12345};
 for(unsigned i=0;i<sizeof fp/sizeof *fp;i++)for(unsigned k=0;k<=8;k++)if(!conv(fp[i],k))goto fail;
 for(uint32_t u=0,i=0;i<65536;i++,u+=65537)if(!conv(u,8))goto fail;
 printf("TC-HOST-DIFF PASS cases=%u\n",cases);return 0;
fail: fprintf(stderr,"TC-HOST-DIFF FAIL case=%u\n",cases);return 1;
}
