/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Exercise the real shell's record/plan/emit path without opening a device.
 * The existing Streams C oracle compares every generated method word. */
#define CUDAPP_MIGRATION_PENDING_STREAMS_DIFF 1
#include "../src/omega_gpu_cmdlist.c"
#include <stdio.h>
#include <stdlib.h>
void cudapp_streams_diff_note(int same,const char *line) { if(!same) { fprintf(stderr,"FAIL %s\n",line);abort(); } }
static uint64_t random_state=123456789;
static unsigned random_word(void) { random_state^=random_state<<13;random_state^=random_state>>7;random_state^=random_state<<17;return (unsigned)random_state; }
int main(void) {
    OmegaGpuCmdList cl={0};cl.cap=32;cl.words_cap=16384;
    cl.gen=omega_gpu_session_recoveries();cl.epoch=omega_gpu_session_epoch();cl.record_generation=1;
    cl.banks.cpu=calloc(cl.cap,1024);cl.banks.va=0x10000000;
    cl.qmds.cpu=calloc(cl.cap,1024);cl.qmds.va=0x20000000;
    cl.sems.cpu=calloc(cl.cap,16);cl.sems.va=0x30000000;
    cl.stamps.va=0x40000000;cl.scratch.va=0x50000000;
    cl.words=calloc(cl.words_cap,4);cl.native=calloc(cl.cap,sizeof *cl.native);
    cl.indep=calloc(cl.cap,1);cl.wait_va=calloc(cl.cap,sizeof *cl.wait_va);
    if(!cl.banks.cpu||!cl.qmds.cpu||!cl.sems.cpu||!cl.words||!cl.native||!cl.indep||!cl.wait_va) return 2;
    for(unsigned t=0;t<2000;t++) {
        omega_gpu_cmdlist_reset(&cl);unsigned n=random_word()%33;
        for(unsigned i=0;i<n;i++) {
            uint32_t args[32];for(unsigned j=0;j<32;j++) args[j]=random_word();
            OmegaGpuLaunch l={.code_va=0x60000000,.args=args,.n_args=1+random_word()%32,
                .threads_x=128,.threads_y=1,.grid_x=1+random_word()%32,.grid_y=1,
                .gpr_count=64,.shared_bytes=random_word()%32769};
            OmegaGpuCmdListAccess a[2]={{4096+64*(random_word()%16),64,1},{4096+64*(random_word()%16),64,2}};
            if(t&1) {
                if(omega_gpu_cmdlist_add(&cl,&l)!=(int)i) return 1;
                uint32_t bank[256],qmd[256];
                memcpy(bank,(uint8_t *)cl.banks.cpu+i*1024,sizeof bank);
                memcpy(qmd,(uint8_t *)cl.qmds.cpu+i*1024,sizeof qmd);
                if(omega_gpu_cmdlist_set_accesses(&cl,i,a,2)) return 1;
                if(memcmp(bank,(uint8_t *)cl.banks.cpu+i*1024,sizeof bank)||
                   memcmp(qmd,(uint8_t *)cl.qmds.cpu+i*1024,sizeof qmd)) return 1;
                uint64_t saved[CMDLIST_DESCRIPTOR_WORDS];
                memcpy(saved,cl.program+i*CMDLIST_DESCRIPTOR_WORDS,sizeof saved);
                OmegaGpuCmdListAccess bad={1,0,1};
                if(omega_gpu_cmdlist_set_accesses(&cl,i,&bad,1)==0||
                   memcmp(saved,cl.program+i*CMDLIST_DESCRIPTOR_WORDS,sizeof saved)) return 1;
            } else if(omega_gpu_cmdlist_add_accesses(&cl,&l,a,2)!=(int)i) return 1;
        }
        OmegaGpuCmdListOpts o={.mode=t&1,.stamps=(t>>1)&1};OmegaGpuCmdListProgramPlan p;
        if(omega_gpu_cmdlist_plan_program(&cl,&o,&p)) return 1;
        if(n) {
            prepare_qmd_mode(&cl,&o);cl.program_active=1;
            size_t words=emit_chunk(&cl,&o,0,n,0x70000000);cl.program_active=0;
            if(words!=p.words) { fprintf(stderr,"FAIL emitted words %zu != plan %u\n",words,p.words);return 1; }
        } else {
            OmegaGpuCmdListStats stats;
            if(p.words||p.waits||p.submissions||omega_gpu_cmdlist_submit_program(&cl,&o,&stats)||
               stats.submits||stats.first_missing!=-1) return 1;
        }
        o.first_barrier_none=1;
        if(omega_gpu_cmdlist_plan_program(&cl,&o,&p)==0) return 1;
    }
    free(cl.program);free(cl.barriers);free(cl.banks.cpu);free(cl.qmds.cpu);free(cl.sems.cpu);
    free(cl.words);free(cl.native);free(cl.indep);free(cl.wait_va);
    puts("PASS real command-list shell: 2000 record/plan/emit programs, PCAS/INLINE/stamps, exact C method differential; no device opened");
    return 0;
}
