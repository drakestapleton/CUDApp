/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "model_graphs_fixture.h"
#include "../src/omega_gpu_decode_graph.c"
int mg_decode_run(const MgCase *c){
    OmegaGpuDecodeGraph g={0};
    g.cfg.layers=c->layers;g.cfg.vocab=248320;g.cfg.eps=1e-6f;g.cfg.attn=(void*)0x2000;g.cfg.lm_head=(void*)0x200;
    g.am_ctas=122;g.in_token=1;g.final_set=1;g.cl=mg_fixture_list();g.buf.va=c->seed;
    g.xq_layer=(c->flags&64)?(int)c->layer:-1;g.mutant=c->mutant;
    g.lo.x=UINT64_C(0x100000);
    g.lo.xq=UINT64_C(0x200000);
    g.lo.qkv=UINT64_C(0x300000);
    g.lo.attn=UINT64_C(0x400000);
    g.lo.o=UINT64_C(0x500000);
    g.lo.gu=UINT64_C(0x600000);
    g.lo.act=UINT64_C(0x700000);
    g.lo.dn=UINT64_C(0x800000);
    g.lo.hidden=UINT64_C(0x900000);
    g.lo.logits=UINT64_C(0xa00000);
    g.lo.am_v=UINT64_C(0xb00000);
    g.lo.am_i=UINT64_C(0xc00000);
    g.lo.token=UINT64_C(0xd00000);
    g.lo.norms=UINT64_C(0xe00000);
    g.lo.fnorm=UINT64_C(0xf00000);
    g.lo.tabs=UINT64_C(0x1000000);
    g.lo.pf_x=UINT64_C(0x1100000);
    g.lo.pf_y=UINT64_C(0x1200000);
    for(unsigned i=0;i<K_COUNT;i++){g.k[i].code.va=UINT64_C(0x500000000)+i*0x1000;g.k[i].gpr=64;}
    g.cfg.router=(void*)0x100;g.cfg.norm_topk=(c->flags>>1)&1;g_prefetch=c->flags&1;g_prefetch_red=(c->flags>>2)&1;
    for(unsigned i=0;i<c->layers;i++){g.layer[i].set=1;g.layer[i].qkv=(void*)0x10;g.layer[i].o=(void*)0x20;g.layer[i].gu=(void*)0x30;g.layer[i].dn=(void*)0x40;}
    if(c->mode)g.hidden_ready=(c->flags>>7)&1;
    int rc=0;if(c->mode<2)rc=c->mode?omega_gpu_decode_graph_record_final(&g):omega_gpu_decode_graph_record_layer(&g,c->layer,c->pos);
    if(c->mode==2){g.xq_layer=-1;g.hidden_ready=0;rc=0;for(unsigned l=0;l<c->layers&&!rc;l++)rc=omega_gpu_decode_graph_record_layer(&g,l,c->pos);if(!rc)rc=omega_gpu_decode_graph_record_final(&g);}
    uint64_t state[16]={g.n_kind,g.failed,g.xq_layer+1,g.hidden_ready};mg_record(200,state,16);
    for(unsigned i=0;i<g.n_kind;i++){uint64_t kind=g.kind[i];mg_record(201,&kind,1);}
    return rc;
}
