/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef GPU_RESIDENT_GRID_H
#define GPU_RESIDENT_GRID_H
#include "bw_warp_sim_k.h"
/* Additive one-warp/block grid scheduler. Unlike ws_run, a polling CTA yields.
 * Memory is coherent/SC in this simulator; fence/cache hardware remains NOT_RUN.
 * Both CTA orders and several instruction quanta exercise actual generated spin loops. */
static int resident_grid(WsSim *s, unsigned ctas, unsigned reverse) {
    (void)ws_run;
    WsWarp *w=calloc(ctas,sizeof *w);
    if(!w || !s->quantum || s->threads!=32) { free(w);return WS_E_OP; }
    for(unsigned c=0;c<ctas;c++) { w[c].nfrag=1;w[c].frag[0].mask=0xffffffffu; }
    for(unsigned epoch=0;epoch<100000;epoch++) {
        unsigned live=0;
        for(unsigned v=0;v<ctas;v++) {
            unsigned c=reverse ? ctas-1-v:v;
            if(!w[c].nfrag) continue;
            live++;
            int rc=ws_run_warp(s,&w[c],0,c,0,0);
            if(rc<0||rc==1) { free(w);return s->err ? s->err:WS_E_STUCK; }
        }
        if(!live) { free(w);return s->err; }
    }
    free(w);ws_fail(s,WS_E_STEPS,-1,"resident grid scheduling budget");return s->err;
}
#endif
