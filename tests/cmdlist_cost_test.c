/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Count-only replay of existing decode workloads, not measured chip timing.
 * Sources and deliberately conservative legacy effects: om/cmdlist/README.md. */
#include "cudapp_native.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static uint64_t p[200704],b[4096];
static void access(unsigned i,unsigned slot,unsigned buffer,unsigned flags) {
    uint64_t *a=p+i*49;p[i*49]=slot+1;a[1+3*slot]=4096+buffer*4096;
    a[2+3*slot]=4096;a[3+3*slot]=flags;
}
static int report(const char *name,unsigned n,unsigned stamps) {
    uint64_t a[16]={n,4096,0,stamps,16384},o[16],r=1;
    if(omega_cmdlist_plan(&r,a,16,p,200704,b,4096,o,16)||r) return 1;
    printf("%s,%u,%u,%u,1,%llu,%llu,%llu\n",name,n,n,n+stamps,
        (unsigned long long)o[2],(unsigned long long)o[1],(unsigned long long)o[0]);
    return 0;
}
int main(void) {
    puts("program,launches,old_per_kernel_submits,old_list_waits,old_list_submits,new_submits,new_waits,new_words");
    for(unsigned n=1;n<=16;n*=2) {
        memset(p,0,sizeof p);
        /* gpu_launch_cost_test.c: disjoint weight/output slices, common read-only x. */
        for(unsigned i=0;i<n;i++) { access(i,0,0,1);access(i,1,1+i,1);access(i,2,32+i,2); }
        char name[64];snprintf(name,sizeof name,"decode-fp8-gemv-split-%u",n);
        if(report(name,n,1)) return 1;
    }
    memset(p,0,sizeof p);
    /* gpu_cmdlist_test.c:537-569. RES, SwiGLU, RES, ARGMAX stage0, stage1.
     * Buffer IDs encode actual aliases; omitted immutable weights are read-only
     * and do not introduce hazards with another launch. */
    access(0,0,0,3);access(0,1,1,1);access(0,2,2,2);
    access(1,0,3,1);access(1,1,4,2);
    access(2,0,0,3);access(2,1,1,1);access(2,2,2,2);
    access(3,0,5,1);access(3,1,6,2);access(3,2,7,2);
    access(4,0,6,1);access(4,1,7,1);access(4,2,8,2);access(4,3,9,2);
    if(report("decode-glue",5,0)) return 1;
    /* Historical full-token launch count from omega_gpu_cmdlist.h:8. Until
     * graph producers attach complete sets, unknown means no unsafe elision. */
    memset(p,0,sizeof p);for(unsigned i=0;i<289;i++) p[i*49]=17;
    if(report("qwen3-30b-token-legacy-unknown",289,0)) return 1;
    return 0;
}
