/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Independent host oracle: enumerate overlapping half-open intervals, then drain
 * all outstanding dispatches at the first conflicting dispatch. No GPU access. */
#include "cudapp_native.h"
#include "omega_blackwell_qmd.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX 4096u
#define STRIDE 49u
#define SLOTS (MAX * STRIDE)
static uint64_t program[SLOTS], barriers[MAX], reference[MAX];
static unsigned failures;
static uint64_t seed = 0x9e3779b97f4a7c15ull;
static uint64_t rnd(void) { seed ^= seed << 13; seed ^= seed >> 7; seed ^= seed << 17; return seed; }
static void check(int ok, const char *s) { if (!ok) { fprintf(stderr,"FAIL %s\n",s); failures++; } }
static unsigned conflict(unsigned a, unsigned b) {
    const uint64_t *x=program+a*STRIDE,*y=program+b*STRIDE;
    if (x[0]==17 || y[0]==17) return 7;
    unsigned hazard=0;
    for (unsigned i=0;i<x[0];i++) for (unsigned j=0;j<y[0];j++) {
        const uint64_t *p=x+1+3*i,*q=y+1+3*j;
        if (p[0] < q[0]+q[1] && q[0] < p[0]+p[1]) {
            if ((q[2]&2) && (p[2]&1)) hazard|=1; /* RAW */
            if ((q[2]&1) && (p[2]&2)) hazard|=2; /* WAR */
            if ((q[2]&2) && (p[2]&2)) hazard|=4; /* WAW */
        }
    }
    return hazard;
}
static uint64_t ref(unsigned n, unsigned mode, unsigned stamps, uint64_t *waits) {
    unsigned first=0; *waits=n && stamps ? 1 : 0;
    uint64_t words=35+(stamps?8:0);
    for(unsigned i=0;i<n;i++) {
        unsigned h=0;
        for(unsigned j=first;j<i;j++) h|=conflict(i,j);
        reference[i]=i==0?8:h;
        if(reference[i]) { ++*waits; first=i; }
        words+=(mode?198:4)+(reference[i]?4:0)+(stamps?6:0);
    }
    return n?words:0;
}
static int plan(unsigned n,unsigned cap,unsigned mode,unsigned stamps,uint64_t budget,uint64_t out[16]) {
    uint64_t a[16]={n,cap,mode,stamps,budget,0},r=999;
    memset(out,0,16*sizeof *out);
    return omega_cmdlist_plan(&r,a,16,program,SLOTS,barriers,MAX,out,16)==0 && r==0;
}
static void compare(unsigned n,unsigned mode,unsigned stamps) {
    uint64_t out[16],waits,words=ref(n,mode,stamps,&waits);
    check(plan(n,MAX,mode,stamps,1000000,out),"plan accepted");
    check(out[0]==words && out[1]==waits && out[2]==(n?1u:0u),"exact words/waits/submissions");
    check(memcmp(reference,barriers,n*sizeof *barriers)==0,"hazard classes and outstanding epoch");
}
static void edge_tests(void) {
    uint64_t out[16]; memset(program,0,sizeof program);
    compare(0,0,0); compare(1,0,0); compare(MAX,0,0);
    check(!plan(MAX+1,MAX,0,0,1000000,out),"capacity+1 refused");
    check(!plan(1,0,0,0,1000000,out),"zero capacity refused");
    check(plan(1,1,0,0,43,out),"exact word capacity accepted");
    check(!plan(1,1,0,0,42,out),"word capacity+1 refused");
    program[0]=1;program[1]=100;program[2]=16;program[3]=2;
    program[STRIDE]=1;program[STRIDE+1]=108;program[STRIDE+2]=8;program[STRIDE+3]=1;
    compare(2,0,0); check(barriers[1]==1,"overlapping VA alias RAW");
    program[3]=1;program[STRIDE+3]=2;compare(2,0,0);check(barriers[1]==2,"WAR");
    program[3]=2;compare(2,0,0);check(barriers[1]==4,"WAW");
    program[STRIDE+1]=116;compare(2,0,0);check(barriers[1]==0,"adjacent ranges do not alias");
    program[2]=0;check(!plan(2,MAX,0,0,1000000,out),"zero-byte access refused");
    program[2]=32;program[1]=UINT64_MAX-7;check(!plan(2,MAX,0,0,1000000,out),"wrapping access refused");
    memset(program,0,sizeof program);program[0]=17;compare(3,0,0);check(barriers[1]==7,"unknown access conservative");
    memset(program,0,sizeof program);
    uint64_t a[16]={1,1,0,0,47,4},r=0;
    check(omega_cmdlist_plan(&r,a,16,program,SLOTS,barriers,MAX,out,16)==0&&r==0,"platform prefix exact fit");
    a[5]=5;omega_cmdlist_plan(&r,a,16,program,SLOTS,barriers,MAX,out,16);check(r!=0,"platform prefix overflow refused");
    a[5]=0;
    for(unsigned k=6;k<=9;k++) {
        a[k]=1;omega_cmdlist_plan(&r,a,16,program,SLOTS,barriers,MAX,out,16);
        check(r!=0,"manual barrier/skip/first-none/cross-wait refused");a[k]=0;
    }
    a[0]=32;a[1]=5;a[2]=10;
    omega_cmdlist_layout(&r,3,a,16,out,16);check(r==15,"bounded next chunk");
    a[1]=30;omega_cmdlist_layout(&r,3,a,16,out,16);check(r==32,"final chunk clamps to count");
    a[2]=0;omega_cmdlist_layout(&r,3,a,16,out,16);check(r==UINT64_MAX,"zero chunk room refused");
}
static void records(void) {
    uint64_t a[16]={4,0,32,128,1,3,2,0x1234567800ull,81,2049,0x20000000,0x30000000,0x40000000,0x50000000};
    uint64_t args[32],words[512],r=0; uint32_t bank[224],q1[96],q0[96];
    for(unsigned round=0;round<1000;round++) {
        a[1]=round%4;a[8]=round==0?UINT32_MAX:rnd()%250;a[9]=rnd()%32769;a[4]=rnd()%4;a[5]=1+rnd()%31;
        for(unsigned j=0;j<32;j++) args[j]=(uint32_t)rnd();
        check(omega_cmdlist_record(&r,a,16,args,32,words,512)==0 && r==0,"record accepted");
        uint64_t bv=a[10]+a[1]*1024,qv=a[11]+a[1]*1024;
        omega_blackwell_build_cbank_driver_2d(bank,bv,(uint32_t)a[3],a[4]?(uint32_t)a[4]:1,(uint32_t)a[5],(uint32_t)a[6]);
        for(unsigned j=0;j<224;j++) check(words[j]==bank[j],"bank differential");
        for(unsigned j=0;j<32;j++) check(words[224+j]==args[j],"argument record");
        uint32_t gp=a[8]>64?((uint32_t)a[8]+7)/8*8:64;
        OmegaBlackwellQmdConfig c={.code_va=a[7],.cbank_va=bv,.scratch_va=a[13],.sem_va=a[12]+a[1]*16,.qmd0_va=qv+512,.qmd1_va=qv,.threads_x=(uint32_t)a[3],.threads_y=a[4]?(uint32_t)a[4]:1,.grid_x=(uint32_t)a[5],.grid_y=(uint32_t)a[6],.gpr_count=gp,.shared_bytes=(uint32_t)a[9]};
        check(omega_blackwell_build_qmd1(q1,&c)==0,"C QMD oracle");omega_blackwell_build_qmd0(q0,qv+512,qv);
        for(unsigned j=0;j<96;j++) { check(words[256+j]==q1[j],"QMD1 differential");check(words[384+j]==q0[j],"QMD0 differential"); }
    }
    a[1]=4;check(omega_cmdlist_record(&r,a,16,args,32,words,512)==0 && r==2,"record full refusal");
    a[1]=0;a[9]=32769;check(omega_cmdlist_record(&r,a,16,args,32,words,512)==0 && r!=0,"shared bound refusal");
    a[9]=0;a[0]=MAX;a[1]=MAX-1;
    check(omega_cmdlist_record(&r,a,16,args,32,words,512)==0 && r==0,"full-capacity final record accepted");
    a[1]=MAX;check(omega_cmdlist_record(&r,a,16,args,32,words,512)==0 && r==2,"capacity+1 record refused");
}
int main(int argc,char **argv) {
    (void)argc;(void)argv; edge_tests();
    for(unsigned t=0;t<12000;t++) {
        unsigned n=1+rnd()%32; memset(program,0,n*STRIDE*sizeof *program);
        for(unsigned i=0;i<n;i++) { uint64_t *p=program+i*STRIDE;p[0]=rnd()%17;
            for(unsigned j=0;j<p[0];j++) { p[1+3*j]=4096+(rnd()%32)*16;p[2+3*j]=1+rnd()%64;p[3+3*j]=1+rnd()%3; }
        }
        compare(n,t&1,(t>>1)&1);
        if(failures) break;
    }
    records();
    printf("%s cmdlist independent differential: 12000 randomized programs, edges, 1000 launch records\n",failures?"FAIL":"PASS");
    return failures?1:0;
}
