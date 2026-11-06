/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Source-derived partitions for the REST continuation. The retained C fragment
 * packers are independent verification oracles, not production dependencies. */
#include "cudapp_native.h"
#define CUDAPP_MIGRATION_PENDING_C_API_SHELLS 1
#include "omega_api_shells.h"
#include "omega_api_views_rest.h"
#include "omega_blackwell_matmul.h"
#include "omega_blackwell_codegen.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
static unsigned checks;
#define EQ(a,b) do { uint64_t aa=(a),bb=(b); ++checks; if(aa!=bb){fprintf(stderr,"FAIL line %u: %llu != %llu\n",__LINE__,(unsigned long long)aa,(unsigned long long)bb);exit(1);} } while(0)
static void launch(void) {
 const uint64_t v[]={0,1,UINT32_MAX-1,UINT32_MAX,(uint64_t)UINT32_MAX+1,UINT64_MAX};
 for(size_t i=0;i<sizeof v/sizeof*v;i++) {
  uint64_t q[16]={v[i],255,32,1,17,3,v[i],v[i],20,v[i],96,5,7},r=0;
  uint32_t a[20],b[20]={0};
  b[0]=(uint32_t)q[0];b[1]=q[0]>>32;b[2]=255;b[3]=32;b[4]=1;b[5]=17;b[6]=3;
  b[8]=(uint32_t)v[i];b[9]=v[i]>>32;b[10]=(uint32_t)v[i];b[11]=v[i]>>32;b[12]=20;
  b[14]=(uint32_t)v[i];b[15]=v[i]>>32;b[16]=96;b[17]=5;b[18]=7;
  EQ(omega_arlaunch(&r,q,16,a,20),0);EQ(r,0);EQ(memcmp(a,b,sizeof a),0);
  b[0]^=1; EQ(memcmp(a,b,sizeof a)!=0,1); /* red: low-half corruption */
  uint64_t va[5]={v[i],v[i],v[i],v[i],v[i]},inf[16]={0};uint32_t sc[2]={7,9},ga[12],gi[22];
  EQ(omega_argargs(&r,va,5,sc,2,ga,12),0);EQ(r,0);
  for(unsigned j=0;j<5;j++){unsigned off=j<3?j*2:8+(j-3)*2;EQ(ga[off],(uint32_t)v[i]);EQ(ga[off+1],v[i]>>32);}
  EQ(ga[6],7);EQ(ga[7],9);inf[10]=v[i];inf[11]=v[i];
  EQ(omega_arginfo(&r,inf,16,gi,22),0);EQ(r,0);EQ(gi[10],(uint32_t)v[i]);EQ(gi[11],v[i]>>32);EQ(gi[12],(uint32_t)v[i]);EQ(gi[13],v[i]>>32);
  EQ(omega_arargsx(&r,va,5,sc,2,ga,12),0);EQ(r,0);
  for(unsigned j=0;j<5;j++){EQ(ga[j*2],(uint32_t)v[i]);EQ(ga[j*2+1],v[i]>>32);}
  EQ(ga[10],7);EQ(ga[11],9);
  EQ(omega_arargsx(&r,va,5,sc,2,ga,11),0);EQ(r,1);
  uint32_t spec[8]={(uint32_t)v[i],128,0,0,0,5,7,9},out[24],rr,cc;
  omega_api_tcsp(0,0,128,&rr,&cc);
  EQ(omega_artcsp(&r,spec,8,out,5),0);EQ(r,0);EQ(out[0],spec[0]);EQ(out[1],rr);EQ(out[2],cc);EQ(out[3],8);EQ(out[4],5);
  omega_api_tgspec(0,0,128,&rr,&cc);
  EQ(omega_artspec(&r,spec,8,out,10),0);EQ(r,0);EQ(out[0],spec[0]);EQ(out[1],128);EQ(out[2],rr);EQ(out[3],2);EQ(out[4],cc);EQ(out[5],32);EQ(out[6],1);EQ(out[7],5);EQ(out[8],7);EQ(out[9],9);
  uint64_t ti[16]={(uint32_t)v[i],65537,65539,65541,4,2,0,0,3,11,13};
  EQ(omega_artcinfo(&r,ti,16,out,24),0);EQ(r,0);
  uint64_t products[]={ti[1]*ti[3]*ti[4],ti[2]*ti[3]*ti[4],ti[2]*ti[8]*ti[5]*ti[4]*32};
  for(unsigned j=0;j<3;j++){EQ(out[12+j*2],(uint32_t)products[j]);EQ(out[13+j*2],products[j]>>32);}
 }
}
static void cache(void) {
 struct Rec {unsigned char valid,pinned,pad[2];uint32_t key;} a[17];
 const uint32_t vals[]={0,1,15,16,17,UINT32_MAX};
 for(unsigned n=0;n<=17;n++)for(unsigned z=0;z<sizeof vals/sizeof*vals;z++) {
  memset(a,0,sizeof a);uint32_t key=vals[z];for(unsigned i=0;i<n;i++){a[i].valid=1;a[i].pinned=1;a[i].key=i;}
  uint64_t q[8]={n,sizeof a[0],0,4,4},r=0;
  EQ(omega_arcache(&r,q,8,a,sizeof a,&key,sizeof key),0);EQ(r,key<n?key:UINT64_MAX);
  for(unsigned start=0;start<(n?n:1);start++) {
   uint64_t t[8]={n,sizeof a[0],0,1,start};
   EQ(omega_arvictim(&r,t,8,a,sizeof a),0); EQ(r,UINT64_MAX);
   if(n){a[start].pinned=0;EQ(omega_arvictim(&r,t,8,a,sizeof a),0);EQ(r,start);EQ(t[4],(start+1)%n);a[start].pinned=1;}
  }
 }
}
static void steps(void) {
 const uint32_t nt[]={0,1,63,64,65};
 for(unsigned j=0;j<sizeof nt/sizeof*nt;j++) {
  uint64_t q[16]={0,nt[j],32,64},r=0;unsigned next=0;
  for(;;){EQ(omega_armstep(&r,q,16,NULL,0),0);if(next>=nt[j]){EQ(r,0);break;}
   unsigned last=nt[j]-next<64?nt[j]:next+64;EQ(r,1);EQ(q[8],next);EQ(q[9],last);EQ(q[10],(uint64_t)next*32*8);EQ(q[11],(uint64_t)(last-next)*32*8);EQ(q[12],q[11]*2);next=last;
  }
 }
 const uint32_t rows[]={0,1,15,16,17,31,32,33};
 for(unsigned j=0;j<sizeof rows/sizeof*rows;j++) {
  uint64_t q[16]={0,rows[j],32,1,16,8,4096,8192},r=0;q[15]=2;unsigned next=0;
  for(;;){EQ(omega_armstep(&r,q,16,NULL,0),0);if(next>=rows[j]){EQ(r,0);break;}
   unsigned n=rows[j]-next<32?rows[j]-next:32;EQ(r,1);EQ(q[8],n);EQ(q[9],n/16);EQ(q[10],4096+(uint64_t)next*16*2);EQ(q[11],8192+(uint64_t)next*8*4);next+=32;
  }
 }
 uint32_t cols[]={0,8,16};
 for(int skew=-1;skew<=1;skew++) {
  uint64_t q[16]={0,3,16,2,8,8192,(uint64_t)(int64_t)skew},r=0;q[15]=1;
  for(unsigned i=0;i<3;i++){EQ(omega_armstep(&r,q,16,cols,3),0);EQ(r,1);EQ(q[8],i);EQ(q[9],8192+(uint64_t)((int64_t)(cols[i]/8)+skew)*16*16);EQ(q[10],i*2*8);}
  EQ(omega_armstep(&r,q,16,cols,3),0);EQ(r,0);
 }
}
static void packing(void) {
 const uint32_t dims[]={0,1,7,8,9,15,16,17};
 for(unsigned mi=0;mi<sizeof dims/sizeof*dims;mi++)for(unsigned ki=0;ki<sizeof dims/sizeof*dims;ki++) {
  uint32_t m=dims[mi],k=dims[ki],mp=(m+15)/16*16,kp=(k+15)/16*16;
  uint16_t src[17*17],actual[32*32],expect[32*32];float fs[17*17];
  for(unsigned i=0;i<17*17;i++){src[i]=(uint16_t)i;fs[i]=(float)(int)(i%31-15);}
  for(unsigned kind=0;kind<5;kind++){if(kind==2)continue;if((kind==1||kind==4)&&m!=1)continue;
   uint64_t q[12]={kind,m,k,mp,kp},r=0;EQ(omega_cg_pklen(&r,q,12),0);EQ(r,0);
   memset(actual,0,sizeof actual);memset(expect,0,sizeof expect);
   const void *in=kind>=3?(void*)fs:(void*)src;
   EQ(omega_cg_pkh(&r,q,12,in,q[8],actual,q[9]),0);EQ(r,0);
   if(kind>=3)omega_matmul_frag_pack_a_f32(fs,m,k,mp,kp,expect);else omega_matmul_frag_pack_a(src,m,k,mp,kp,expect);
   EQ(memcmp(actual,expect,q[9]*2),0);
  }
 }
 for(unsigned ki=1;ki<sizeof dims/sizeof*dims;ki++)for(unsigned ni=1;ni<sizeof dims/sizeof*dims;ni++){
  uint32_t k=dims[ki],n=dims[ni],kp=(k+15)/16*16,nt=(n+7)/8;uint16_t src[17*17],a[32*24],b[32*24];
  for(unsigned i=0;i<17*17;i++)src[i]=(uint16_t)i;
  uint64_t q[12]={2,0,k,0,kp,n,0,nt},r=0;EQ(omega_cg_pklen(&r,q,12),0);EQ(r,0);EQ(omega_cg_pkh(&r,q,12,src,q[8],a,q[9]),0);EQ(r,0);omega_matmul_frag_pack_b(src,k,n,kp,0,nt,b);EQ(memcmp(a,b,q[9]*2),0);
 }
 for(unsigned mi=0;mi<sizeof dims/sizeof*dims;mi++)for(unsigned ni=0;ni<sizeof dims/sizeof*dims;ni++){
  unsigned m=dims[mi],n=dims[ni],np=(n+7)/8*8;float s[17*24],d[17*17],e[17*17];for(unsigned i=0;i<17*24;i++)s[i]=(float)i;
  uint64_t q[3]={m,n,np},r=0;EQ(omega_mrow(&r,q,3,s,(uint64_t)m*np,d,(uint64_t)m*n),0);EQ(r,0);for(unsigned i=0;i<m;i++)memcpy(e+i*n,s+i*np,n*sizeof(float));EQ(memcmp(d,e,m*n*sizeof(float)),0);
 }
 uint64_t q[12]={0,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX},r=0;EQ(omega_cg_pklen(&r,q,12),0);EQ(r,UINT64_MAX);
}
static void prefill(void) {
 const unsigned dims[]={128,256,384};
 for(unsigned ni=0;ni<3;ni++)for(unsigned ki=0;ki<3;ki++)for(unsigned layout=0;layout<2;layout++) {
  unsigned n=dims[ni],k=dims[ki],kb=k/128;
  uint8_t *s=malloc((size_t)n*k),*a=malloc((size_t)n*k),*b=malloc((size_t)n*k);
  float sf[9],r[512],e[512];for(unsigned i=0;i<9;i++)sf[i]=(float)(i+1);
  for(size_t i=0;i<(size_t)n*k;i++)s[i]=(uint8_t)(i/k);
  memset(r,0,sizeof r);memset(e,0,sizeof e);
  uint64_t q[4]={n,k,layout},out=0;
  EQ(omega_arpfcopy(&out,q,4,s,(uint64_t)n*k,a,(uint64_t)n*k),0);
  EQ(omega_arpfrat(&out,q,4,sf,(size_t)(n/128)*kb,r,(size_t)(n/8)*(kb+1)),0);
  for(unsigned p=0;p<n;p++) {
   unsigned src=layout?omega_api_swsrc(p,n):p;
   memcpy(b+(size_t)p*k,s+(size_t)src*k,k);
   if(p%8==0) {
    float *rt=e+(p/16)*(kb+1)*2+(p/8)%2;const float *sc=sf+(src/128)*kb;
    rt[0]=1;for(unsigned j=1;j<kb;j++)rt[j*2]=sc[j-1]/sc[j];rt[2*kb]=sc[kb-1];
   }
  }
  EQ(memcmp(a,b,(size_t)n*k),0);EQ(memcmp(r,e,(size_t)(n/8)*(kb+1)*sizeof(float)),0);
  free(s);free(a);free(b);
 }
 const unsigned kvals[]={0,1,127,128,129,255,256,257,UINT32_MAX};
 for(unsigned i=0;i<sizeof kvals/sizeof*kvals;i++)for(unsigned group=0;group<4;group++) {
  uint64_t q[32]={13,kvals[i],group},out=0;EQ(omega_arcalc(&out,q,32),0);EQ(out,0);
  unsigned kb=kvals[i]/128,g=group?group:kb;
  EQ(q[16],kb);EQ(q[17],g);EQ(q[18],group?kb/group:1);EQ(q[19],(uint64_t)g*32);
 }
}
#include "api_rest_partitions.h"
int main(void){launch();cache();steps();packing();prefill();data_parts();tile_parts();state_parts();calc_parts();misc_parts();slot_parts();printf("PASS api-rest checks=%u red=low-half-corruption\n",checks);return 0;}
