/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include <stdio.h>
#include <string.h>
#include "omega_blackwell_codegen.h"
#include "omega_cubin.h"
static BlackwellIRProgram captured,actual;
static int capture_encode(const BlackwellIRProgram *p,uint8_t *b,size_t cap,size_t *n){captured=*p;return omega_bw_encode_program(p,b,cap,n);}
#define CUDAPP_MIGRATION_PENDING_C_CODEGEN_QMMA 1
#define CUDAPP_MIGRATION_PENDING_C_QMMA_PROBE_REST 1
#define omega_qmma_is_mark oracle_mark
#define omega_qmma_patch_code oracle_patch
#define omega_qmma_patch_selftest oracle_self
#define omega_blackwell_codegen_qmma_probe_prog oracle_probe
#define omega_blackwell_codegen_qmma_peak_prog oracle_peak
#define omega_blackwell_codegen_f2e4_probe_prog oracle_f2
#define omega_bw_encode_program capture_encode
#include "../src/omega_blackwell_codegen_qmma.c"
#undef omega_bw_encode_program
#undef omega_qmma_is_mark
#undef omega_qmma_patch_code
#undef omega_qmma_patch_selftest
extern int omega_qmma_is_mark(const BlackwellIRInsn *);
extern int omega_qmma_patch_code(const BlackwellIRProgram *,uint8_t *,size_t);
extern int omega_qmma_patch_selftest(char *,size_t);
static unsigned cases;
static int check(uint64_t mode,uint64_t a,uint64_t b,uint64_t c,uint64_t d,uint64_t e,uint64_t want,int red){
 uint64_t got=0;if(omega_qm_host(&got,mode,a,b,c,d,e)!=OMEGA_OK)return 0;
 cases++;if(red&&cases==1)got^=1;
 if(got!=want){fprintf(stderr,"QMMA-REST DIFF FAIL case=%u mode=%llu\n",cases,(unsigned long long)mode);return 0;}return 1;
}
static uint64_t oracle_word(uint64_t s,uint32_t op,uint32_t imm,uint32_t w0,uint32_t w1,uint32_t w2){
 if(op==BW_IR_FMNMX_MAX){if((w0&0xffff)!=0x723e||(w1&0xffffff00)!=0||w2!=0x048070ff)return UINT64_MAX;return s+65536;}
 if(op!=BW_IR_HMMA_BF16)return s;
 if(imm==OMEGA_QMMA_MARK){if((w0&0xffff)!=0x727a||(w1&0xffffff00)!=0||(w2&0xffffff00)!=0x2c00)return UINT64_MAX;return s+1;}
 if((w0&0xffff)!=0x723c||(w2&0xffffff00)!=0x41800)return UINT64_MAX;
 return s+256;
}
static int self_identity(void){
 char x[256],y[256];if(oracle_self(x,sizeof x)||omega_qmma_patch_selftest(y,sizeof y)||strcmp(x,y))return 0;
 if(omega_gpu_om_ir_replay(omega_qm_self,0,0,0,0,&actual,NULL)!=OMEGA_OM_IR_OK)return 0;
 static uint8_t ca[1024],cb[1024];size_t na=0,nb=0;
 if(omega_bw_encode_program(&captured,ca,sizeof ca,&na)||omega_bw_encode_program(&actual,cb,sizeof cb,&nb)||na!=nb||memcmp(ca,cb,na))return 0;
 if(oracle_patch(&captured,ca,na)!=2||omega_qmma_patch_code(&actual,cb,nb)!=2||memcmp(ca,cb,na))return 0;
 uint8_t *aa=NULL,*bb=NULL;size_t an=0,bn=0;char err[128];
 const OmegaCubinParam p[]={{OMEGA_CUBIN_PARAM_BASE,8,1}};
 int ar=omega_cubin_pack("self",ca,na,p,1,OMEGA_CUBIN_PARAM_BASE+8,omega_bw_gpr_declared(captured.regalloc.peak_gpr_usage),0,&aa,&an,err,sizeof err);
 int br=omega_cubin_pack("self",cb,nb,p,1,OMEGA_CUBIN_PARAM_BASE+8,omega_bw_gpr_declared(actual.regalloc.peak_gpr_usage),0,&bb,&bn,err,sizeof err);
 int ok=!ar&&!br&&an==bn&&!memcmp(aa,bb,an);omega_cubin_free(aa);omega_cubin_free(bb);
 if(ok)puts("QMMA-REST SELFTEST IR/ENCODED/CUBIN IDENTITY PASS");
 return ok;
}
int main(int argc,char **argv){(void)argv;int red=argc>1;
 const uint32_t v[]={0,1,15,16,17,36,37,38,0xfffe,0xffff,0x10000,0xfffffeff,0xffffff00,0xffffff01,UINT32_MAX,OMEGA_QMMA_MARK-1,OMEGA_QMMA_MARK,OMEGA_QMMA_MARK+1,0x723d,0x723e,0x723f,0x7279,0x727a,0x727b,0x723b,0x723c,0x048070fe,0x048070ff,0x04807100,0x2bff,0x2c00,0x2c01,0x417ff,0x41800,0x41801};
 for(size_t i=0;i<sizeof v/sizeof v[0];i++)for(size_t j=0;j<sizeof v/sizeof v[0];j++){
  BlackwellIRInsn in;memset(&in,0,sizeof in);in.op=v[i];in.imm=v[j];
  if(!check(0,v[i],v[j],0,0,0,oracle_mark(&in),red)||omega_qmma_is_mark(&in)!=oracle_mark(&in))return 1;
 }
 if(omega_qmma_is_mark(NULL))return 1;
 for(unsigned axis=0;axis<4;axis++)for(unsigned n=0;n<256;n++){
  uint8_t b[4]={0x12,0x34,0x56,0x78};b[axis]=(uint8_t)n;
  if(!check(1,b[0],b[1],b[2],b[3],0,rd32q(b),red))return 1;
 }
 for(unsigned kind=0;kind<4;kind++)for(unsigned axis=0;axis<5;axis++)for(size_t i=0;i<sizeof v/sizeof v[0];i++){
  uint32_t z[5]={BW_IR_HMMA_BF16,OMEGA_QMMA_MARK,0x727a,0,0x2c00};
  if(kind==1){z[1]=0;z[2]=0x723c;z[4]=0x41800;}
  if(kind==2){z[0]=BW_IR_FMNMX_MAX;z[2]=0x723e;z[4]=0x048070ff;}
  if(kind==3)z[0]=0;
  z[axis]=v[i];
  if(!check(2,z[0],z[1],z[2],z[3],z[4],oracle_word(2,z[0],z[1],z[2],z[3],z[4]),red))return 1;
 }
 if(!self_identity()){fputs("QMMA-REST SELFTEST FAIL\n",stderr);return 1;}
 printf("QMMA-REST DIFF PASS cases=%u\n",cases);return 0;
}
