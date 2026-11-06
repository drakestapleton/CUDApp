/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "model_graphs_nested_objects.h"
#include "omega_gpu_native_runtime.h"
#include "omega_native_artifact.h"
#include "omega_blackwell_qmd.h"
#include "sha256.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
/* Same independently hashed export/snapshot path as native_runtime_controls.h,
 * but with the real staged rowmap composer and the graph's GU/DN dimensions.
 * Cubins are temporary first-party host output, never device-loaded or filed. */
#define CHECK(x) do {if(!(x)){fprintf(stderr,"nested native refusal line=%u: %s\n",__LINE__,#x);return -1;}}while(0)
static OmegaGpuNativeArtifact artifacts[2];
static OmegaNativeArtifactSnapshot snapshots[2];
static void word(uint8_t *p,unsigned i,uint64_t v){for(unsigned j=0;j<8;j++)p[i*8+j]=(uint8_t)(v>>(j*8));}
static void u32(uint8_t *p,uint32_t v){for(unsigned j=0;j<4;j++)p[j]=(uint8_t)(v>>(j*8));}
static void digest(const void *p,size_t n,uint8_t out[32]){sha256_hash(p,n,out);}
static int admit(unsigned slot,uint32_t rows,uint32_t n,uint32_t k){
    OmegaGpuNativeArtifact *a=&artifacts[slot];
    uint64_t d[48]={0},status=0,cfg[16]={3,k,8,1,8,1},ins[8192]={0},meta[2048]={0},phys[288]={0},stats[8]={0};
    uint8_t quant[384]={0};
    d[0]=1;d[1]=2;d[2]=8;d[4]=1;d[5]=128;d[6]=128;d[11]=1;d[13]=1;d[14]=8;d[15]=1;d[17]=128;d[18]=256;d[19]=2;
    d[24]=3;d[25]=256;d[26]=4;d[27]=1;d[30]=32768;d[31]=4;d[33]=256;d[34]=8;d[35]=4;d[36]=32768;d[37]=8;d[40]=1;d[41]=256;
    CHECK(omega_native_quant(&status,0,d,48,quant,384)==OMEGA_OK && status==0);
    CHECK(omega_native_compose(&status,cfg,16,ins,8192,meta,2048,phys,288,stats,8)==OMEGA_OK && status==0);
    CHECK(omega_bc_encode(&status,ins,8192,phys,288,stats[0],1,a->code,16384)==OMEGA_OK && status>0 && status<=16384);
    a->sizes[0]=1152;a->sizes[1]=status;a->sizes[2]=256;
    uint64_t words[38]={1,1152,121,0,rows,n,k,1,3,1,1,1,0,0,1,1,256,8,2,3,7,1024,128,6,1,256,16,rows,1,7,1,2,0,4,5,6,7,8};
    for(unsigned i=0;i<38;i++)word(a->canonical,i,words[i]);
    memcpy(a->canonical+768,quant,384);
    digest(cfg,sizeof cfg,a->canonical+512);digest(ins,sizeof ins,a->canonical+544);digest(phys,sizeof phys,a->canonical+576);
    digest(d,sizeof d,a->canonical+608);digest(meta,sizeof meta,a->canonical+640);digest(a->code,a->sizes[1],a->canonical+672);digest(stats,sizeof stats,a->canonical+736);
    uint64_t launch[32]={1,16,rows,1,256,1,1,1024,128,6,1,896,12,1,1};
    uint8_t abi[136]={0},qpair[768];uint32_t q0[96],q1[96],cb[12]={0};OmegaCubinParam params[7];
    const uint32_t offsets[7]={0,8,16,24,28,32,40},sizes[7]={8,8,8,4,4,8,8};
    OmegaBlackwellQmdConfig qc={.threads_per_block=256,.grid_width=16,.threads_x=256,.threads_y=1,.grid_x=16,.grid_y=rows,.gpr_count=128,.shared_bytes=1024};
    CHECK(omega_blackwell_build_qmd0(q0,0,0)==0 && omega_blackwell_build_qmd1(q1,&qc)==0);
    u32(abi,14084u|(4u<<16));u32(abi+4,130);
    for(unsigned i=0;i<7;i++){unsigned pointer=i<3 || i>=5;params[i]=(OmegaCubinParam){896+offsets[i],sizes[i],pointer};u32(abi+8+16*i,5892u|(12u<<16));u32(abi+16+16*i,i|(offsets[i]<<16));u32(abi+20+16*i,pointer?2225408:1175552);}
    u32(abi+120,6403u|(48u<<16));u32(abi+124,2564u|(8u<<16));u32(abi+128,3);u32(abi+132,896u|(48u<<16));
    digest(abi,sizeof abi,(uint8_t*)(launch+15));digest(cb,sizeof cb,(uint8_t*)(launch+19));memcpy(qpair,q0,384);memcpy(qpair+384,q1,384);digest(qpair,sizeof qpair,(uint8_t*)(launch+23));
    char dir[]="/tmp/model-graphs-native-XXXXXX",prefix[256],path[300],id[65],actual[65],err[256],hash[65],shape[100];
    CHECK(mkdtemp(dir));snprintf(prefix,sizeof prefix,"%s/artifact",dir);memset(hash,'a',64);hash[64]=0;snprintf(shape,sizeof shape,"m=%u,k=%u,n=%u",rows,k,n);
    OmegaNativeArtifact artifact={.op="matmul",.shape=shape,.bounds=shape,.contract_name="synthetic-host-model-graphs",.contract_id_hex=hash,.semantic_spec_id_hex=hash,.kernel_function="nested_staged",.precision="fp8",.accumulation="fp32",.tolerance="0",.text=a->code,.text_len=a->sizes[1],.params=params,.nparams=7,.gpr_count=128,.grid_x=16,.grid_y=rows,.block_x=256,.block_y=1,.block_z=1,.shared_bytes=1024,.cbank_offset=896,.cbank_args=cb,.cbank_words=12,.qmd0=q0,.qmd1=q1,.min_driver="580.173.02",.requires="sm_121",.source_commit="UNKNOWN",.tool_version="host-model-graphs/1",.qualification="device-load=NOT_RUN"};
    int rc=omega_native_artifact_export(&artifact,prefix,id,err,sizeof err);if(rc)fprintf(stderr,"native export: %s\n",err);CHECK(rc==0);
    for(unsigned i=0;i<32;i++){unsigned v;CHECK(sscanf(id+2*i,"%2x",&v)==1);a->manifest[i]=(uint8_t)v;}
    rc=omega_native_artifact_snapshot(prefix,"580.173.02",actual,&snapshots[slot],err,sizeof err);if(rc)fprintf(stderr,"native snapshot: %s\n",err);CHECK(rc==0);a->admission=&snapshots[slot];
    static const char *suffix[]={".text.bin",".cubin",".qmd0.bin",".qmd1.bin",".cbank_args.bin",".json",".manifest",".sha256"};
    for(unsigned i=0;i<sizeof suffix/sizeof suffix[0];i++){snprintf(path,sizeof path,"%s%s",prefix,suffix[i]);CHECK(unlink(path)==0);}CHECK(rmdir(dir)==0);
    for(unsigned i=0;i<32;i++)word(a->launch,i,launch[i]);
    digest(a->launch,256,a->canonical+704);digest(a->canonical,1152,a->realization);
    uint8_t key[32];CHECK(omega_native_admitted(a,key)==0);
    return 0;
}
int mg_nested_window(uint32_t rows,const OmegaGpuNativeArtifact **a,OmegaGpuNativeLease *lease){
    CHECK(admit(0,rows,1024,2048)==0 && admit(1,rows,2048,512)==0);
    for(unsigned i=0;i<2;i++){a[i]=&artifacts[i];CHECK(omega_gpu_fp8gemm_prepare_native(a[i],&lease[i])==0);}
    return 0;
}
