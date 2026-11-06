/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "cudapp_native.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"FAIL provider line %d\n",__LINE__); return 1; } } while (0)
int main(void) {
    uint64_t r, p[12] = {0,1,1,16,16};
    CHECK(omega_cg_pklen(&r,p,12)==0 && r==0 && p[8]==2 && p[9]==256);
    uint16_t s=0x1234,d[256],before[256];
    memset(d,0x5a,sizeof d); memcpy(before,d,sizeof d);
    CHECK(omega_cg_pkh(&r,p,12,&s,1,d,256)==0 && (int64_t)r==-1 && !memcmp(d,before,sizeof d));
    CHECK(omega_cg_pkh(&r,p,12,&s,2,d,255)==0 && (int64_t)r==-1 && !memcmp(d,before,sizeof d));
    p[0]=9; CHECK(omega_cg_pklen(&r,p,12)==0 && (int64_t)r==-1);
    p[0]=0;p[1]=17; CHECK(omega_cg_pklen(&r,p,12)==0 && (int64_t)r==-1);
    p[1]=1;p[3]=17; CHECK(omega_cg_pklen(&r,p,12)==0 && (int64_t)r==-1);
    p[3]=16;p[2]=17; CHECK(omega_cg_pklen(&r,p,12)==0 && (int64_t)r==-1);
    p[2]=1;p[4]=17; CHECK(omega_cg_pklen(&r,p,12)==0 && (int64_t)r==-1);
    p[0]=2;p[4]=16;p[6]=2;p[7]=1; CHECK(omega_cg_pklen(&r,p,12)==0 && (int64_t)r==-1);
    const uint32_t dims[]={0,1,2,7,8,15,16,17,31};
    for(unsigned mi=0;mi<sizeof dims/sizeof *dims;++mi) for(unsigned ni=0;ni<sizeof dims/sizeof *dims;++ni) {
        uint64_t m=dims[mi],n=dims[ni],np=n+3,q[3]={m,n,np};
        size_t sn=m*np,dn=m*n;
        float *src=malloc((sn+1)*sizeof *src),*dst=malloc((dn+1)*sizeof *dst),*exp=malloc((dn+1)*sizeof *exp);
        CHECK(src&&dst&&exp);
        for(size_t i=0;i<sn+1;++i) { uint32_t bits=0x7fc00000u+(uint32_t)i; memcpy(src+i,&bits,4); }
        memset(dst,0xa5,(dn+1)*sizeof *dst);memcpy(exp,dst,(dn+1)*sizeof *dst);
        for(size_t row=0;row<m;++row) memcpy(exp+row*n,src+row*np,n*sizeof *exp);
        CHECK(omega_mrow(&r,q,3,src,sn,dst,dn)==0 && r==0 && !memcmp(dst,exp,(dn+1)*sizeof *dst));
        if(sn) CHECK(omega_mrow(&r,q,3,src,sn-1,dst,dn)==0 && (int64_t)r==-1);
        if(dn) CHECK(omega_mrow(&r,q,3,src,sn,dst,dn-1)==0 && (int64_t)r==-1);
        free(src);free(dst);free(exp);
    }
    uint64_t q[3]={1,2,1};float a[4]={0};
    CHECK(omega_mrow(&r,q,3,a,2,a+2,2)==0 && (int64_t)r==-1);
    q[0]=UINT64_MAX;q[1]=q[2]=0;
    CHECK(omega_mrow(&r,q,3,a,0,a+2,0)==0 && (int64_t)r==-1);
    q[0]=q[1]=q[2]=1;
    CHECK(omega_mrow(&r,q,3,a,1,a,1)!=0);
    puts("PASS fragment metadata, short-view refusal and row-compaction bitwise differential");
    return 0;
}
