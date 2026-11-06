/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
uint64_t om_plan(uint64_t[32],uint32_t);uint64_t c_plan(uint64_t[32],uint32_t);
size_t om_check(uint32_t,const uint32_t*,const uint32_t*,size_t);size_t c_check(uint32_t,const uint32_t*,const uint32_t*,size_t);
uint64_t om_clock(uint64_t,uint64_t);uint64_t c_clock(uint64_t,uint64_t);
#define REQUIRE(x) do{if(!(x)){fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);return 1;}}while(0)
static int policy(void){
 const uint32_t edge[]={0,1,2,3,4,15,16,17,31,32,33,63,64,65,127,128,129,255,256,257,1023,1024,1025,UINT32_MAX-1,UINT32_MAX};
 for(size_t i=0;i<sizeof edge/sizeof*edge;i++){
  uint64_t a[32]={0,0,edge[i]},b[32];memcpy(b,a,sizeof a);
  REQUIRE(om_plan(a,0)==c_plan(b,0)&&!memcmp(a,b,sizeof a));
 }
 for(size_t i=0;i<sizeof edge/sizeof*edge;i++)for(size_t j=0;j<sizeof edge/sizeof*edge;j++)for(unsigned ids=0;ids<2;ids++){
  uint64_t a[32]={edge[i],edge[j],129,31,edge[j],ids},b[32];memcpy(b,a,sizeof a);
  REQUIRE(om_plan(a,1)==c_plan(b,1)&&!memcmp(a,b,sizeof a));
  /* Cursor partitions include the final row boundary without walking billions of rows. */
  uint64_t curs[]={0,edge[i]?edge[i]-1:0,edge[i],(uint64_t)edge[i]+1};
  for(size_t z=0;z<sizeof curs/sizeof*curs;z++){
   a[10]=b[10]=curs[z];REQUIRE(om_plan(a,2)==c_plan(b,2)&&!memcmp(a,b,sizeof a));
   REQUIRE(om_plan(a,3)==c_plan(b,3)&&!memcmp(a,b,sizeof a));
  }
 }
 /* Source launch policy boundaries per-1/per/per+1 and 32-bit offset wrap. */
 for(size_t j=1;j<sizeof edge/sizeof*edge;j++)for(unsigned z=0;z<3;z++){
  uint64_t a[32]={(uint64_t)edge[j]+z-1,1024,256,edge[j],17,1},b[32];memcpy(b,a,sizeof a);
  REQUIRE(om_plan(a,1)==c_plan(b,1)&&!memcmp(a,b,sizeof a));
  REQUIRE(om_plan(a,2)==c_plan(b,2)&&!memcmp(a,b,sizeof a));
 }
 return 0;
}
static int scans(void){
 const size_t ns[]={0,1,2,31,32,33,255,256,257,999999,1000000,1000001};
 uint32_t*w=malloc(1000001*sizeof*w),*x=malloc(1000001*sizeof*x);REQUIRE(w&&x);
 for(size_t k=0;k<sizeof ns/sizeof*ns;k++){
  size_t n=ns[k];uint32_t sum=0;
  for(size_t i=0;i<n;i++){x[i]=(i%3)?UINT32_MAX:17;sum+=x[i];w[i]=sum;}
  for(uint32_t op=0;op<3;op++)REQUIRE(om_check(op,w,x,n)==c_check(op,w,x,n));
  REQUIRE(om_check(1,w,x,n)==0);
  if(n){w[n-1]^=1;REQUIRE(om_check(1,w,x,n)>0);w[n-1]^=1;}
  for(size_t i=0;i<n;i++)w[i]=(uint32_t)i;
  REQUIRE(om_check(2,w,w,n)==0);
  if(n>1){w[0]=1;w[n-1]=0;REQUIRE(om_check(2,w,w,n)==c_check(2,w,w,n)&&om_check(2,w,w,n)>0);}
  for(size_t i=0;i<n;i++)w[i]=0x7fa00bad;
  REQUIRE(om_check(0,w,w,n)==n&&om_check(0,w,w,n)==c_check(0,w,w,n));
  if(n){w[n-1]=0x7fa00bac;REQUIRE(om_check(0,w,w,n)==c_check(0,w,w,n));w[n-1]=0x7fa00bae;REQUIRE(om_check(0,w,w,n)==c_check(0,w,w,n));}
 }
 free(w);free(x);return 0;
}
int main(void){
 REQUIRE(policy()==0);REQUIRE(scans()==0);
 const uint64_t ts[]={0,1,999999999,1000000000,1000000001,UINT32_MAX,UINT64_MAX-1,UINT64_MAX};
 for(size_t i=0;i<sizeof ts/sizeof*ts;i++)for(size_t j=0;j<sizeof ts/sizeof*ts;j++)REQUIRE(om_clock(ts[i],ts[j])==c_clock(ts[i],ts[j]));
 REQUIRE(om_clock(1,0)!=om_clock(0,0));
 puts("ATTN-HOST-DIFF PASS");return 0;
}
