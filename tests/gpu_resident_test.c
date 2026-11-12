/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_gpu_resident.h"
#include "gpu_resident_grid.h"
#include "gpu_resident_fixture.h"
#include "omega_bw_hazard.h"
/* The static scoreboard checker over the encoded kernel. The warp simulator has no instruction latency, so a
 * variable-latency producer (S2R, loads, atomics) read without its wait is invisible to it and is caught only here
 * or on the chip (2026-11-12: S2R emitted fixed-latency, GB10 marker timeout). */
static int hazards(unsigned n, unsigned ctas) {
    static BlackwellIRProgram p; static OmegaBwHazardReport rep;
    static uint8_t code[65536]; OmegaBlackwellKernel kernel; uint64_t err=0;
    if(omega_gpu_resident_build(resident_model,n,64,ctas,&p,&err)) { fprintf(stderr,"resident build refused err=%llu\n",(unsigned long long)err);return 1; }
    size_t bytes=omega_bw_program_code_bytes(&p);
    if(bytes>sizeof code||omega_blackwell_kernel_encode_into(&p,code,bytes,&kernel)) { fprintf(stderr,"resident encode refused\n");return 1; }
    int rc=omega_bw_hazard_check(code,bytes,&rep);
    if(rc||rep.count) {
        fprintf(stderr,"resident hazards layers=%u ctas=%u: rc=%d findings=%zu\n",n,ctas,rc,rep.count);
        for(size_t i=0;i<rep.count&&i<OMEGA_BW_HAZARD_MAX;i++)
            fprintf(stderr,"  IR %u %s R%u release 0x%x (producer/reader IR %u)\n",rep.f[i].pc,omega_bw_hazard_kind(rep.f[i].kind),rep.f[i].reg,rep.f[i].release,rep.f[i].src_pc);
        return 1;
    }
    return 0;
}
static int run(int drop, unsigned order, unsigned quantum, unsigned witness) {
    float arena[64], expected[64];
    resident_input(arena,witness);
    memcpy(expected,arena,sizeof arena); resident_reference(resident_model,5,expected);
    static BlackwellIRProgram p;
    uint64_t err=0;
    if(omega_gpu_resident_build(resident_model,5,64,2,&p,&err)) { fprintf(stderr,"resident build refused err=%llu\n",(unsigned long long)err);return 1; }
    uint8_t code[65536];OmegaBlackwellKernel kernel;
    if(omega_blackwell_kernel_encode_into(&p,code,sizeof code,&kernel)) { fprintf(stderr,"resident encode refused\n");return 1; }
    if(drop) {
        int end=-1,begin=-1;
        for(unsigned i=0;i<p.count;i++) {
            if(p.insns[i].op==BW_IR_MEMBAR_ALL_SYS && begin<0) begin=(int)i;
            if(p.insns[i].op==BW_IR_ATOMG_EXCH_STRONG_SYS) end=(int)i;
        }
        /* The barrier region includes its spin-loop backward BRA; remove all of it. */
        if(begin<0||end<0) return 1;
        int stop=end+1;
        while(stop<(int)p.count && p.insns[stop].op!=BW_IR_IADD3) stop++;
        for(int i=begin;i<stop;i++) p.insns[i].op=BW_IR_NOP;
    }
    uint32_t sync[2]={0};
    WsSim s={0}; s.p=&p;s.threads=32;s.order=(int)order;s.quantum=quantum;
    uintptr_t args[]={ (uintptr_t)resident_model,(uintptr_t)arena,(uintptr_t)sync };
    for(unsigned i=0;i<3;i++) { s.cbank[(896+8*i)/4]=(uint32_t)args[i];s.cbank[(900+8*i)/4]=(uint32_t)(args[i]>>32); }
    s.ranges[0]=(WsRange){(uintptr_t)resident_model,(uintptr_t)resident_model+sizeof resident_model};
    s.ranges[1]=(WsRange){(uintptr_t)arena,(uintptr_t)arena+sizeof arena};
    s.ranges[2]=(WsRange){(uintptr_t)sync,(uintptr_t)sync+sizeof sync};s.nranges=3;
    int rc=resident_grid(&s,2,order);
    if(!drop && (rc||memcmp(arena,expected,sizeof arena)||sync[1]!=5)) {
        fprintf(stderr,"resident rc=%d %s gen=%u order=%u quantum=%u\n",rc,s.msg,sync[1],order,quantum);
        for(unsigned i=32;i<46;i++) fprintf(stderr,"%u %g expected %g\n",i,arena[i],expected[i]);
        return 1;
    }
    return drop ? (!rc && !memcmp(arena,expected,sizeof arena)) : 0;
}
int main(int argc,char **argv) {
    if(argc!=2||strcmp(argv[1],"--sim")) { fprintf(stderr,"use --sim; chip tier NOT_RUN\n");return 2; }
    for(unsigned n=1;n<=5;n+=4) for(unsigned c=1;c<=2;c++) if(hazards(n,c)) return 1;
    for(unsigned w=0;w<2;w++) for(unsigned o=0;o<2;o++) for(unsigned q=1;q<=17;q+=8) if(run(0,o,q,w)) return 1;
    if(run(1,0,10000,0)) { fprintf(stderr,"dropped barrier survived\n");return 1; }
    uint32_t bad[40];memcpy(bad,resident_model,sizeof bad);bad[3]=64;
    static BlackwellIRProgram p;uint64_t err=0;
    if(!omega_gpu_resident_build(bad,5,64,2,&p,&err)) return 1;
    memcpy(bad,resident_model,sizeof bad);bad[0]=99;
    if(!omega_gpu_resident_build(bad,5,64,2,&p,&err)) return 1;
    const unsigned fields[]={0,1,2,3,4,4,5,5,6,7,13,28};
    const uint32_t values[]={0,64,64,0,0,33,0,33,1,1,2,2};
    for(unsigned i=0;i<sizeof fields/sizeof fields[0];i++) {
        memcpy(bad,resident_model,sizeof bad);bad[fields[i]]=values[i];
        if(!omega_gpu_resident_build(bad,5,64,2,&p,&err)) return 1;
    }
    if(!omega_gpu_resident_build(resident_model,0,64,2,&p,&err) ||
       !omega_gpu_resident_build(resident_model,5,0,2,&p,&err) ||
       !omega_gpu_resident_build(resident_model,5,64,0,&p,&err) ||
       !omega_gpu_resident_build(resident_model,5,64,3,&p,&err)) return 1;
    puts("GPU RESIDENT PASS (scheduling hazards, reference, interleaved CTAs, barrier and descriptor controls); chip tier is gpu_resident_chip");return 0;
}
