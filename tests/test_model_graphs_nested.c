/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#define main mg_timeline_reference_main
#include "test_omega_gpu_decode_graph_timeline.c"
#undef main
#include "model_graphs_fixture.h"
#include "model_graphs_nested_objects.h"
#include "omega_gpu_cmdlist_native.h"
#include "cudapp_native.h"
#include <fcntl.h>
#include <unistd.h>
static OmegaGpuCmdList *list;
static FILE *wire;
void *mg_fixture_list(void){return list;}
int mg_record(uint64_t kind,const uint64_t *words,size_t n){
    /* Graph-only CPU observations supplement the real nested launch stream. */
    if(kind<200)return -1;
    return fwrite(&kind,sizeof(kind),1,wire)!=1 || fwrite(&n,sizeof(n),1,wire)!=1 || fwrite(words,sizeof(*words),n,wire)!=n ? -1:0;
}
void mg_nested_record_begin(void){omega_gpu_session_lock();}
static int capture_submit(void *dev,OmegaGpuQueueHandle queue,const uint32_t *words,uint32_t n){
    if(fwrite(&n,sizeof(n),1,wire)!=1 || fwrite(words,sizeof(*words),n,wire)!=n)return -1;
    return f_submit(dev,queue,words,n);
}
#ifdef MG_NEGATIVE
uint32_t __real_omega_mg_node(uint64_t*,const void*,uint64_t,const void*,uint64_t,void*,uint64_t,uint64_t);
uint32_t __wrap_omega_mg_node(uint64_t *r,const void *c,uint64_t cn,const void *d,uint64_t dn,void *o,uint64_t on,uint64_t p){
    static int flipped;
    uint32_t rc=__real_omega_mg_node(r,c,cn,d,dn,o,on,p);
    if(!rc && *r==1 && !flipped && on>32){((uint64_t*)o)[32]^=1;flipped=1;}
    return rc;
}
#endif
int main(int argc,char **argv){
    if(argc!=9)return 2;
    unsigned family=(unsigned)strtoul(argv[1],NULL,0);
    MgCase c={.seed=UINT64_C(0x900000000),.mode=(uint32_t)strtoul(argv[2],NULL,0),.attn=(uint32_t)strtoul(argv[3],NULL,0),.heads=(uint32_t)strtoul(argv[4],NULL,0),.flags=(uint32_t)strtoul(argv[5],NULL,0),.pos=(uint32_t)strtoul(argv[6],NULL,0),.layers=(uint32_t)strtoul(argv[7],NULL,0)};
    int fd=open(argv[8],O_WRONLY|O_CREAT|O_TRUNC|O_CLOEXEC,0600);
    if(fd<0)return 2;
    if(!(wire=fdopen(fd,"wb"))){close(fd);return 2;}
    memset(&fk,0,sizeof(fk));g_plat.queue_submit=capture_submit;
    if(install() || omega_gpu_cmdlist_create(4096,&list))return 3;
    int rc=family==36?mg_q36_run(&c):family==38?mg_q38_run(&c):mg_decode_run(&c);
    if(fk.locks_held)omega_gpu_session_unlock();
    if(rc){fprintf(stderr,"nested refused family=%u mode=%u attn=%u flags=%u rc=%d detail=%s\n",family,c.mode,c.attn,c.flags,rc,omega_gpu_session_last_error());return 5;}
    unsigned count=omega_gpu_cmdlist_count(list);
    if(!count)return 6;
    if(fwrite(&count,sizeof(count),1,wire)!=1)return 7;
    for(unsigned i=0;i<count;i++){
        OmegaGpuCmdListEntry e;if(omega_gpu_cmdlist_entry(list,i,&e))return 7;
        if(fwrite(&e.code_va,sizeof(e.code_va),1,wire)!=1 || fwrite(&e.pending_wait,sizeof(e.pending_wait),1,wire)!=1 || fwrite(&e.n_args,sizeof(e.n_args),1,wire)!=1 || fwrite(&e.independent,sizeof(e.independent),1,wire)!=1 || fwrite(e.bank,1024,1,wire)!=1 || fwrite(e.qmd_slot,1024,1,wire)!=1)return 7;
    }
    unsigned before=fk.submits;OmegaGpuCmdListStats st;
    rc=omega_gpu_cmdlist_submit(list,NULL,&st);
    unsigned submissions=fk.submits-before;
    if(fwrite(&submissions,sizeof(submissions),1,wire)!=1 || fclose(wire))return 7;
    fprintf(stderr,"nested family=%u mode=%u attention=%u heads=%u flags=%u position=%u nodes=%u submissions=%u rc=%d\n",family,c.mode,c.attn,c.heads,c.flags,c.pos,count,submissions,rc);
    return rc || submissions!=1 || fk.double_locks || fk.bad_unlocks ? 8:0;
}
