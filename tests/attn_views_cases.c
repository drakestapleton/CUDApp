/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "attn_views_fixture.h"
#include "omega_attn_tables.h"
#include <stdlib.h>
#ifndef AV_ENTRY
#error AV_ENTRY must name the differential arm
#endif
void AV_ENTRY(AttnViewsFixture *r) {
    uint8_t *bytes=malloc(AV_N);uint16_t *bf=malloc(AV_N*sizeof *bf);int32_t *si=malloc(AV_N*sizeof *si);
    if(!bytes || !bf || !si)abort();
    for(size_t i=0;i<AV_N;i++){bytes[i]=(uint8_t)(i*37u);bf[i]=(uint16_t)(i%0x7f80u);si[i]=i%3?(int32_t)i:-1;}
    uint64_t p[32];for(size_t i=0;i<32;i++)p[i]=0xdeadbeef12345678ull^((uint64_t)i<<40);
    r->result[0]=av_args(r->args,p,9,25);
    r->result[1]=av_layout(r->layout,p,0x5555u);
    r->result[28]=r->layout[1];
    av_memcpy(r->bytes,bytes,AV_N);av_memset(r->bytes+17,0xe7,19);
    av_memcpy(r->bytes,r->bytes,AV_N);av_memcpy(NULL,NULL,0);av_memset(NULL,0,0);
    av_memset(bytes,0xe7,AV_N);
    r->result[30]=(uint64_t)bytes[0]+bytes[AV_N-1];
    av_fill32(r->ui,AV_N,0xa1234567u);r->result[2]=av_count32(r->ui,AV_N,0xa1234567u);
    r->result[3]=av_any32(r->ui,AV_N);r->result[4]=av_compact32(r->compact,si,AV_N);
    av_fillf32(r->ff,AV_N,-123.25f);r->result[5]=av_countf32(r->ff,AV_N,-123.25f);
    r->result[6]=av_routerpoison(r->ff,r->ui,AV_N,-123.25f,0xffffffffu);
    r->result[7]=av_badids(r->ui,AV_N,1024);uint64_t zeros[5]={0,1,0,UINT64_MAX,0};r->result[8]=av_zero64(zeros,5);
    r->ff[0]=0;r->ff[1]=0;r->ui[0]=UINT32_MAX;r->ui[1]=0;
    r->result[25]=av_routerpoison(r->ff,r->ui,AV_N,-123.25f,UINT32_MAX);
    r->result[26]=av_count32(r->ui,AV_N,0xa1234567u);
    r->result[27]=av_countf32(r->ff,AV_N,-123.25f);
    r->result[9]=av_bffinite(bf,AV_N);bf[AV_N-1]=0x7f80u;r->result[10]=av_bffinite(bf,AV_N);bf[AV_N-1]=0x3f80u;
    av_bfexpand(r->ff,bf,AV_N);av_rowtab(r->table,4,19);
    int32_t signed_ctx[8]={INT32_MIN,-1,0,1,2,INT32_MAX,31,32};
    av_context(r->compact+AV_N-8,signed_ctx,8);
    av_ropegen(r->rope,0,10000.0,128,128,2);av_ropegen(r->rope+256,129,500000.0,64,256,3);
    uint32_t lo[4]={0,1,2,4},hi[4]={3,4,5,6};av_gentab(lo,hi,r->table+8,4,29,3);
    uint64_t ranges[16]={8,12,0,3,2};av_ranges(r->range,8,ranges);
    av_rowtab(r->range+12,2,UINT32_MAX);
    av_gentab(lo,hi,r->table+16,2,UINT32_MAX,1);
    uint32_t ctx[8]={0,1,2,3,100,64,65,4096},nb[8]={0,1,2,1,3,4,1,2};av_eff(r->eff,ctx,nb,8,32);
    r->result[11]=av_total(r->eff,8,32);r->result[12]=av_best(r->eff,8,128);r->result[13]=av_max(nb,8);
    av_headmap(r->headmap,8,2,128,0);av_headmap(r->headmap+8,8,2,128,1);
    uint32_t ids[4]={2,2,5,1};av_memset(r->slots,0xff,sizeof r->slots);
    uint64_t tq[16]={4,1,0,24,91};uint32_t g=av_table(ids,r->table,32,r->slots,r->touched,8,tq,0);r->result[14]=g;
    tq[1]=0;tq[2]=4;tq[3]=25;r->result[15]=av_table(ids,r->table,32,NULL,NULL,0,tq,g);
    tq[1]=2;tq[2]=28;tq[3]=26;r->result[16]=av_table(ids,r->table,32,NULL,NULL,0,tq,0);
    uint64_t block[16]={0,8,64,1,16,512};r->result[17]=av_blocks(ids,4,block);block[1]=5;r->result[18]=av_blocks(ids,4,block);
    r->result[19]=av_resfit(ids,4,64,16);r->result[20]=av_resfit(ids,4,UINT32_MAX,16);
    r->result[20]+=av_resfit(ids,4,UINT64_C(4294967296),0);
    uint64_t gather[16]={4,2,8,20,5,2};av_gather(bytes,146,r->gather,64,gather);
    uint64_t stage[16]={3,64,16,16,2};av_stage(bytes,512,r->stage,512,r->touched,stage);
    av_reset(r->slots,8,r->touched,g);
    float parts[4*2*10];for(size_t i=0;i<80;i++)parts[i]=(float)(i%7)*0.125f;
    const float gaps[4]={0.0f,0.375f,126.75f,200.0f};
    for(size_t row=0;row<4;row++)for(size_t s=0;s<2;s++){parts[row*20+s*10+8]=row==0?-__builtin_inff():-(float)s*gaps[row];parts[row*20+s*10+9]=1.0f;}
    av_fold(parts,r->fold,4,2,8);
    r->result[21]=av_args(r->args,p,17,32);
    r->result[22]=av_args(r->args,p,0,33);
    r->result[23]=av_args(r->args,p,2,3);
    r->result[24]=av_layout(r->layout,p,UINT64_C(1)<<31);
    r->result[31]=av_max(r->ui,AV_N);
    r->result[29]=av_total(r->compact,AV_N,32)^av_best(r->ui,AV_N,128);
    av_eff(r->compact,r->ui,r->ui,AV_N,32);
    r->result[30]=((uint64_t)r->compact[999999]<<32)|r->compact[AV_N-1];
    av_fill32((uint32_t*)si,AV_N,0);
    uint64_t long_stage[16]={AV_N,1,0,1,0};
    av_stage(r->bytes,AV_N,bytes,AV_N,(const uint32_t*)si,long_stage);
    r->result[30]^=(uint64_t)bytes[999999]+bytes[AV_N-1];
    int32_t one_slot=0;uint32_t one_touched=UINT32_MAX;
    uint64_t long_table[16]={AV_N,1,0,0,0};
    r->result[28]=av_table((uint32_t*)si,r->compact,AV_N,&one_slot,&one_touched,1,long_table,1);
    r->result[28]|=(uint64_t)one_touched<<32;
    one_slot=-1;one_touched=UINT32_MAX;
    r->result[28]+=av_table((uint32_t*)si,r->compact,AV_N,&one_slot,&one_touched,1,long_table,0);
    r->touched[7]=one_touched;r->slots[7]=one_slot;
    uint64_t empty[16]={0};av_stage(NULL,0,NULL,0,NULL,empty);
    empty[1]=1;empty[3]=31;empty[4]=97;
    r->result[28]+=av_table(NULL,r->table,32,NULL,NULL,0,empty,0);
    free(si);free(bf);free(bytes);
}
