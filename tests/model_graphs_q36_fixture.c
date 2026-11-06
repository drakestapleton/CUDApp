/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "model_graphs_fixture.h"
#ifdef MODEL_GRAPHS_NESTED
#include "model_graphs_nested_objects.h"
#endif
#include "../src/omega_gpu_q36_graph.c"
int mg_q36_run(const MgCase *c) {
    OmegaGpuQ36Graph g={0};
    g.cfg.layers=c->layers;g.cfg.vocab=248320;g.cfg.eps=1e-6f;g.cfg.eps_l2=1e-6f;
    g.cfg.router=(void*)0x100;g.cfg.lm_head=(void*)0x200;g.am_ctas=122;
    g.heads=g.stroke_heads=c->heads;g.in_token=1;g.final_set=1;g.batch=1;g.cl=mg_fixture_list();
    g.buf.va=c->seed;g.dbuf.va=c->seed+UINT64_C(0x100000000);g.lo.split=UINT64_C(0x1000000);
    g.lo.lb_stride=32768;g.lo.bytes=UINT64_C(0x10000000);
    g.lo.x=UINT64_C(0x100000);
    g.lo.ab=UINT64_C(0x200000);
    g.lo.hidden=UINT64_C(0x300000);
    g.lo.logits=UINT64_C(0x400000);
    g.lo.am_v=UINT64_C(0x500000);
    g.lo.am_i=UINT64_C(0x600000);
    g.lo.token=UINT64_C(0x700000);
    g.lo.norms=UINT64_C(0x800000);
    g.lo.fnorm=UINT64_C(0x900000);
    g.lo.tabs=UINT64_C(0xa00000);
    g.lo.gtab=UINT64_C(0xb00000);
    g.lo.idtab=UINT64_C(0xc00000);
    g.lo.stage=UINT64_C(0xd00000);
    g.lo.segd=UINT64_C(0xe00000);
    g.lo.ptab=UINT64_C(0xf00000);
    g.lo.xq=UINT64_C(0x1000000);
    g.lo.mix=UINT64_C(0x1100000);
    g.lo.agg=UINT64_C(0x1200000);
    g.lo.att=UINT64_C(0x1300000);
    g.lo.o=UINT64_C(0x1400000);
    g.lo.gu=UINT64_C(0x1500000);
    g.lo.act=UINT64_C(0x1600000);
    g.lo.dn=UINT64_C(0x1700000);
    g.lo.xp=UINT64_C(0x1800000);
    g.lo.mixN=UINT64_C(0x1900000);
    g.lo.abN=UINT64_C(0x1a00000);
    g.lo.aggN=UINT64_C(0x1b00000);
    g.lo.attN=UINT64_C(0x1c00000);
    g.lo.op=UINT64_C(0x1d00000);
    g.lo.oN=UINT64_C(0x1e00000);
    g.lo.rlb=UINT64_C(0x1f00000);
    g.lo.sgu=UINT64_C(0x2000000);
    g.lo.sdn=UINT64_C(0x2100000);
    g.lo.xmap=UINT64_C(0x2200000);
    g.lo.ymap=UINT64_C(0x2300000);
    g.lo.xp2=UINT64_C(0x2400000);
    g.lo.guN=UINT64_C(0x2500000);
    g.lo.actN=UINT64_C(0x2600000);
    g.lo.actp=UINT64_C(0x2700000);
    g.lo.dnN=UINT64_C(0x2800000);
    g.pf.x=UINT64_C(0x4100000);
    g.pf.rlb=UINT64_C(0x4200000);
    g.pf.sgu=UINT64_C(0x4300000);
    g.pf.sdn=UINT64_C(0x4400000);
    g.pf.sgu2=UINT64_C(0x4500000);
    g.pf.sdn2=UINT64_C(0x4600000);
    g.pf.xmap=UINT64_C(0x4700000);
    g.pf.ymap=UINT64_C(0x4800000);
    g.pf.xmap2=UINT64_C(0x4900000);
    g.pf.ymap2=UINT64_C(0x4a00000);
    g.pf.xq=UINT64_C(0x4b00000);
    g.pf.xp=UINT64_C(0x4c00000);
    g.pf.mixN=UINT64_C(0x4d00000);
    g.pf.abN=UINT64_C(0x4e00000);
    g.pf.aggN=UINT64_C(0x4f00000);
    g.pf.attN=UINT64_C(0x5000000);
    g.pf.op=UINT64_C(0x5100000);
    g.pf.oN=UINT64_C(0x5200000);
    g.pf.xp2=UINT64_C(0x5300000);
    g.pf.guN=UINT64_C(0x5400000);
    g.pf.actN=UINT64_C(0x5500000);
    g.pf.actp=UINT64_C(0x5600000);
    g.pf.dnN=UINT64_C(0x5700000);
    g.pf.hb.va=c->seed+UINT64_C(0x200000000);g.pf.db.va=c->seed+UINT64_C(0x300000000);
    g.pf.cap=c->heads;g.pf.gdn_multi=(c->flags>>4)&1;g.pf.attn_multi=(c->flags>>5)&1;g.pf.mutant=(c->flags>>8)&7;
    g.mutant=c->mutant;
    g_overlap_ab=c->flags&1;g_cn_rows=(c->flags>>1)&1;
    if(c->flags&4)g.head8=(void*)0x300;
    g.xh.screen=(void*)0x400;g.xh.h8=c->seed+0x100;g.xh.s=c->seed+0x200;g.xh.c=c->seed+0x300;
    g.xh.hi=c->seed+0x400;g.xh.lo=c->seed+0x500;g.xh.tau=c->seed+0x600;g.xh.tab=c->seed+0x700;
    for(unsigned i=0;i<K_COUNT;i++){g.k[i].code.va=UINT64_C(0x500000000)+i*0x1000;g.k[i].gpr=64;}
    for(unsigned i=0;i<c->heads;i++){
        g.gdn_h[i]=(void*)(uintptr_t)(0x1000+i);g.attn_h[i]=(void*)(uintptr_t)(0x2000+i);
        g.xq_layer[i]=(c->flags&64)?(int)c->layer:-1;g.xh.on[i]=(c->flags>>3)&1;
    }
    for(unsigned i=0;i<c->layers;i++){
        g.layer[i].set=1;g.layer[i].attn=c->attn;g.layer[i].kind_index=i;
        g.layer[i].mix=(void*)0x10;g.layer[i].ab=(void*)0x20;g.layer[i].o=(void*)0x30;
        g.layer[i].gu=(void*)0x40;g.layer[i].dn=(void*)0x50;
    }
#ifdef MODEL_GRAPHS_NESTED
    g.cfg.router=mg_nested_router(c->layers,256,1);g.cfg.lm_head=mg_nested_bf16(0,g.cfg.vocab,H);
    g.head8=(c->flags&4)?mg_nested_fp8(4,1,g.cfg.vocab,H):NULL;g.xh.screen=mg_nested_fp8(5,1,g.cfg.vocab,H);
    for(unsigned h=0;h<c->heads;h++){g.gdn_h[h]=mg_nested_gdn(h,2,c->layers);g.attn_h[h]=mg_nested_attn256(h,c->layers,16,2,4);}
    for(unsigned l=0;l<c->layers;l++){
        g.layer[l].mix=mg_nested_fp8(0,1,c->attn?OMEGA_GPU_Q36_ATT_MIX:OMEGA_GPU_Q36_GDN_MIX,H);
        g.layer[l].ab=mg_nested_bf16(1,OMEGA_GPU_Q36_AB,H);g.layer[l].o=mg_nested_fp8(1,1,H,OMEGA_GPU_Q36_MIXER_OUT);
        g.layer[l].gu=mg_nested_fp8(2,257,2*EI,H);g.layer[l].dn=mg_nested_fp8(3,257,H,EI);
    }
    mg_nested_record_begin();
#endif
    uint32_t pos[OMEGA_GPU_Q36_MAX_HEADS];for(unsigned h=0;h<c->heads;h++)pos[h]=c->pos+h;
    uint32_t a=0,b=0,d=0;int rc;
    switch(c->mode){
    case 0:rc=omega_gpu_q36_graph_record_head_layer(&g,c->heads-1,c->layer,c->pos);break;
    case 1:g.hidden_ready[c->heads-1]=(c->flags>>7)&1;rc=omega_gpu_q36_graph_record_head_final(&g,c->heads-1);break;
    case 2:rc=omega_gpu_q36_graph_record_layer_heads(&g,c->layer,pos);break;
    case 3:rc=pf_record_mixer(&g,0,c->layer,c->heads,c->pos,&a,&b,&d,NULL);break;
    case 4:rc=pf_record_experts(&g,0,c->layer,c->heads,7,128,256);break;
    case 5:for(unsigned h=0;h<c->heads;h++)g.hidden_ready[h]=1;rc=record_final_heads(&g);break;
    case 6:case 7:{
        Q36MoeObservation m={0};g.moe=&m;m.layer=c->layer;m.head=0;m.tokens=c->heads;g.pf.cap=c->heads;
        m.router.ids_va=UINT64_C(0x710000000);m.router.wts_va=m.router.ids_va+128;
        m.router.xout_va=m.router.ids_va+256;m.router.tab_gu_va=m.router.ids_va+16384;m.router.tab_dn_va=m.router.ids_va+24576;
        m.windows[1]=c->heads;m.windows[2]=c->heads*NG;m.windows[3]=8192;m.windows[4]=16384;m.windows[5]=24576;m.windows[6]=32768;m.windows[7]=g.lo.lb_stride;
        m.pair[0].artifact[0]=(void*)0xb0;m.pair[0].artifact[1]=(void*)0xc0;
#ifdef MODEL_GRAPHS_NESTED
        if(c->mode==6 && mg_nested_window(c->heads*NG,m.pair[0].artifact,m.pair[0].lease))return -1;
#endif
        rc=c->mode==6?pf_record_moe_window(&g,0):pf_record_moe_combine(&g,128);break;
    }
    case 8:
        g.xq_layer[0]=-1;g.hidden_ready[0]=0;rc=0;
        for(unsigned l=0;l<c->layers&&!g.failed;l++)rc=omega_gpu_q36_graph_record_head_layer(&g,0,l,c->pos);
        if(!rc)rc=omega_gpu_q36_graph_record_head_final(&g,0);
        break;
    case 9:{
        uint64_t host[8192];memset(host,0xa5,sizeof host);g.pf.hb.cpu=host;g.lo.lb_stride=256;
        g.pf.rlb=0;g.pf.sgu=4096;g.pf.sdn=12288;g.pf.sgu2=20480;g.pf.sdn2=20512;
        g.pf.xmap=24576;g.pf.ymap=28672;g.pf.xmap2=32768;g.pf.ymap2=36864;
        for(unsigned t=0;t<c->heads;t++){uint32_t *ids=(void*)((uint8_t*)host+t*256);for(unsigned s=0;s<8;s++)ids[s]=(uint32_t)(c->seed>>(s*7))^(t*31+s);}
        unsigned segs=pf_group(&g,c->layer,c->heads,0);uint64_t group[2]={segs,g.pf.streamed};mg_record(202,group,2);
        for(unsigned i=0;i<8192;i+=32)mg_record(203,host+i,32);
        rc=0;break;
    }
    default:return -1;
    }
    uint64_t state[16]={g.n_kind,g.failed,g.xq_layer[c->heads-1]+1,g.hidden_ready[c->heads-1],a,b,d};
    mg_record(200,state,16);
    for(unsigned i=0;i<g.n_kind;i++){uint64_t kind=g.kind[i];mg_record(201,&kind,1);}
    return rc;
}
