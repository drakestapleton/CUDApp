/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "dagsim.h"
#include "runtime/rx_costmodel.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef CUDAPP_MIGRATION_PENDING_C_DAGSIM_RESOLVE
#error resolve migration guard is not implemented
#endif
extern int ds_resolved_pack(const ds_model *, const ds_opts *, uint64_t *, ds_result *);
extern int oracle_ds_resolved_pack(const ds_model *, const ds_opts *, uint64_t *, ds_result *);
extern int oracle_ds_diff_models(const ds_model *, const ds_model *, const ds_opts *, ds_diff *);
extern double oracle_ds_counterfactual(const ds_model *, const ds_opts *, int);
extern int oracle_ds_find_op(const ds_model *, const char *);
extern int oracle_ds_find_res(const ds_model *, const char *);
static unsigned checks, failed;
static int fault;
static int fail_alloc;
void *__real_malloc(size_t n);
void *__wrap_malloc(size_t n) { return fail_alloc ? NULL : __real_malloc(n); }
static RxCmFeatures feature;
static const char *label;
#define CK(c, msg) do { checks++; if (!(c)) { failed++; fprintf(stderr,"FAIL %s: %s\n",label,msg); } } while (0)
int __wrap_rx_cm_predict(const RxCostModel *m, const RxCmFeatures *f, uint32_t arm, RxCmPrediction *p) {
    (void)m; (void)arm; feature=*f; memset(p,0,sizeof *p);
    p->known=1; p->n=32; p->mean_log2_ps=20.0; p->sd_log2=0.25; p->dof=12.0;
    if (fault==1) return RX_CM_ERR_UNSUPPORTED;
    if (fault==2) p->refused=1;
    if (fault==3) p->known=0;
    if (fault==4) p->n=0;
    return fault==5 ? RX_CM_ERR_ARG : RX_CM_OK;
}
static void init(ds_model *m) {
    memset(m,0,sizeof *m); m->n_res=4; m->n_ops=2;
    m->has_calibration=1; m->cal_taken_day=100; m->cal_valid_days=20;
    strcpy(m->cal_id,"cal"); strcpy(m->instrument,"none");
    for(int i=0;i<4;i++) {
        ds_resource *r=&m->res[i]; snprintf(r->id,sizeof r->id,"r%d",i);
        r->kind=i+1; r->slots=1; r->cap=1000000000.0; r->lo=500000000.0; r->hi=2000000000.0; r->prov=DS_PROV_SPECIFIED;
    }
    for(int i=0;i<2;i++) {
        ds_op *p=&m->ops[i]; snprintf(p->id,sizeof p->id,"op%d",i);
        p->kind=DS_OP_COMPUTE; p->engine=0; p->t_ns=16.0; p->lo=8.0; p->hi=32.0;
        p->t_prov=DS_PROV_SPECIFIED; p->has_t=1; p->bytes=8.0; p->bw=1;
        p->launch_ns=4.0; p->launcher=3; p->M=64; p->N=16; strcpy(p->src,"source");
    }
}
static void packcheck(ds_model *m, ds_opts *o) {
    uint64_t a[8192],b[8192]; ds_result x,y; memset(a,0xa5,sizeof a); memset(b,0xa5,sizeof b);
    memset(&x,0,sizeof x); memset(&y,0,sizeof y);
    memset(&feature,0,sizeof feature);
    int sy=oracle_ds_resolved_pack(m,o,b,&y); RxCmFeatures expected=feature;
    int sx=ds_resolved_pack(m,o,a,&x);
    if(getenv("DS_RESOLVE_RED")) a[0]^=1;
    CK(sx==sy,"pack status"); CK(!memcmp(&x,&y,sizeof x),"refusal bytes/state");
    if(!sx) CK(!memcmp(a,b,sizeof a),"packed model bytes");
    CK(!memcmp(&feature,&expected,sizeof feature),"costmodel features");
}
static void diffcheck(ds_model *b, ds_model *t, ds_opts *o) {
    ds_diff x,y; int sy=oracle_ds_diff_models(b,t,o,&y); int sx=ds_diff_models(b,t,o,&x);
    if(getenv("DS_RESOLVE_RED")) x.n_added^=1;
    CK(sx==sy,"diff status"); CK(!memcmp(&x,&y,sizeof x),"diff fields/reason/accounting");
}
static void resolve_cases(void) {
    ds_model m; ds_opts o={.asof_day=120,.interval_q=0.9};
    label="resolve modes/provenance";
    for(int mode=-1;mode<=1;mode++) for(int prov=0;prov<=4;prov++) for(int where=0;where<3;where++) {
        init(&m); o.mode=mode;
        if(where==0) m.res[1].prov=prov;
        if(where==1) m.ops[0].t_prov=prov;
        if(where==2) { m.ops[0].t_prov=prov; m.ops[0].src[0]=0; }
        packcheck(&m,&o);
    }
    label="resolve calibration boundary";
    for(int has=0;has<=1;has++) for(int day=119;day<=121;day++) for(int via=0;via<2;via++) {
        init(&m); m.has_calibration=has; o.asof_day=day;
        if(via) m.ops[0].t_prov=DS_PROV_MEASURED; else m.res[2].prov=DS_PROV_FITTED;
        packcheck(&m,&o);
    }
    label="resolve host clock"; init(&m); m.ops[0].t_prov=DS_PROV_MEASURED;
    for(int day=-1;day<=0;day++) { o.asof_day=day; packcheck(&m,&o); }
    o.asof_day=120;
    label="resolve engine refusals";
    for(int kind=1;kind<=6;kind++) for(int launcher=-1;launcher<=0;launcher++) {
        init(&m); m.ops[0].kind=kind; m.ops[0].engine=-1; m.ops[0].launcher=launcher; packcheck(&m,&o);
    }
    label="resolve prediction branches";
    double qs[]={-1.0,0.0,0.5,0.9,1.0,2.0,NAN};
    for(int mode=-1;mode<=1;mode++) for(int f=0;f<=5;f++) for(unsigned q=0;q<sizeof qs/sizeof *qs;q++) {
        init(&m); m.ops[0].kind=DS_OP_MATVEC; m.ops[0].has_t=0; o.mode=mode; o.interval_q=qs[q];
        fault=f; o.costmodel=(void *)1; packcheck(&m,&o);
    }
    init(&m); m.ops[0].kind=DS_OP_MATVEC; m.ops[0].has_t=0; o.costmodel=NULL; packcheck(&m,&o);
    fault=0; o.costmodel=(void *)1;
    label="resolve u32 shape boundaries";
    uint32_t limits[]={0,1,UINT32_MAX-1,UINT32_MAX};
    for(int fld=0;fld<4;fld++) for(unsigned v=0;v<sizeof limits/sizeof *limits;v++) {
        init(&m); m.ops[0].kind=DS_OP_MATVEC; m.ops[0].has_t=0;
        if(fld==0)m.ops[0].M=limits[v]; if(fld==1)m.ops[0].N=limits[v];
        if(fld==2)m.ops[0].core=limits[v]; if(fld==3)m.ops[0].arm=(uint32_t)v;
        packcheck(&m,&o);
    }
    o.costmodel=NULL; o.mode=0;
    label="resolve count boundaries";
    int counts[]={0,1,DS_MAX_OPS-1,DS_MAX_OPS};
    for(unsigned k=0;k<sizeof counts/sizeof *counts;k++) {
        init(&m); ds_op tmpl=m.ops[0]; m.n_ops=counts[k];
        for(int i=0;i<m.n_ops;i++) {m.ops[i]=tmpl; snprintf(m.ops[i].id,DS_ID,"op%d",i);}
        packcheck(&m,&o);
    }
    label="resolve resource/dependency/buffer ABI boundaries";
    init(&m);
    ds_resource rt=m.res[0];m.n_res=DS_MAX_RES;
    for(int i=0;i<m.n_res;i++){m.res[i]=rt;snprintf(m.res[i].id,DS_ID,"r%d",i);}
    m.ops[0].bw=DS_MAX_RES-1;m.n_deps=DS_MAX_DEPS;m.n_buf=DS_MAX_BUF;
    for(int i=0;i<m.n_deps;i++)m.deps[i]=(ds_dep){0,1};
    for(int i=0;i<m.n_buf;i++){m.buf[i].bytes=4;m.buf[i].cap=2;m.buf[i].first=0;m.buf[i].last=1;}
    packcheck(&m,&o);
    for(int i=0;i<m.n_res;i++) {
        CK(ds_find_res(&m,m.res[i].id)==oracle_ds_find_res(&m,m.res[i].id),"max resource lookup");
    }
    init(&m);m.n_res=0;m.ops[0].engine=m.ops[1].engine=-1;
    m.ops[0].launcher=m.ops[1].launcher=-1;m.ops[0].bw=m.ops[1].bw=-1;
    m.ops[0].kind=m.ops[1].kind=DS_OP_BARRIER;packcheck(&m,&o);
    label="resolve absent bandwidth and energy";
    for(int energy=0;energy<2;energy++) for(int instr=0;instr<2;instr++) {
        init(&m); m.ops[0].bw=-1; m.ops[0].bytes=0; m.has_energy=energy;
        m.joules=2.0; m.power_w=4.0; m.energy_prov=DS_PROV_MEASURED;
        if(instr)strcpy(m.instrument,"meter"); packcheck(&m,&o);
    }
    label="lookup scans"; init(&m);
    const char *ids[]={"op0","op1","missing","","op","op00","r0","r3","r4","1234567890123456789012345678901234567890"};
    for(unsigned i=0;i<sizeof ids/sizeof *ids;i++) {
        CK(ds_find_op(&m,ids[i])==oracle_ds_find_op(&m,ids[i]),"op lookup");
        CK(ds_find_res(&m,ids[i])==oracle_ds_find_res(&m,ids[i]),"resource lookup");
    }
    label="counterfactual kinds/index";
    double demands[]={0.0,8.0,128.0};
    for(unsigned k=0;k<sizeof demands/sizeof *demands;k++) for(int r=-1;r<=4;r++) {
        m.ops[0].bytes=m.ops[1].bytes=demands[k];
        double x=ds_counterfactual(&m,&o,r), y=oracle_ds_counterfactual(&m,&o,r);
        CK(!memcmp(&x,&y,sizeof x),"counterfactual bit equality");
    }
    label="counterfactual unmatched launcher/refusal/default kind";
    init(&m);m.ops[0].launcher=-1;
    for(int kind=0;kind<=5;kind++) {
        m.res[3].kind=kind;
        double x=ds_counterfactual(&m,&o,3),y=oracle_ds_counterfactual(&m,&o,3);
        CK(!memcmp(&x,&y,sizeof x),"counterfactual unmatched/default");
    }
    m.res[0].prov=DS_PROV_UNKNOWN;
    double x=ds_counterfactual(&m,&o,0),y=oracle_ds_counterfactual(&m,&o,0);
    CK(!memcmp(&x,&y,sizeof x),"counterfactual refused evaluation");
    label="counterfactual allocation refusal";init(&m);fail_alloc=1;
    x=ds_counterfactual(&m,&o,0);y=oracle_ds_counterfactual(&m,&o,0);fail_alloc=0;
    CK(!memcmp(&x,&y,sizeof x),"counterfactual allocation failure");
}
static void diff_cases(void) {
    ds_model b,t; ds_opts o={.asof_day=120};
    label="diff removal evidence partitions";
    for(int reason=0;reason<=7;reason++) for(int ref=0;ref<4;ref++) for(int proof=0;proof<2;proof++) for(int consumer=0;consumer<4;consumer++) for(int changed=0;changed<3;changed++) {
        init(&b); init(&t); t.ops[0]=t.ops[1]; t.n_ops=1;
        t.n_removed=1; strcpy(t.removed[0].op,"op0"); t.removed[0].reason=reason;
        const char *refs[]={"","missing","op1","op0"}; strcpy(t.removed[0].ref,refs[ref]);
        if(proof)strcpy(t.removed[0].proof,"proof");
        if(consumer==1){b.n_deps=1;b.deps[0]=(ds_dep){0,1};}
        if(consumer==2){b.n_deps=1;b.deps[0]=(ds_dep){1,0};}
        if(consumer==3){
            b.n_ops=3;b.ops[2]=b.ops[1];strcpy(b.ops[2].id,"op2");
            b.n_deps=1;b.deps[0]=(ds_dep){0,2};t.n_removed=2;
            strcpy(t.removed[1].op,"op2");t.removed[1].reason=DS_RM_ELIMINATED;
        }
        const char *lists[]={NULL,"op1","x,op10,op0,,"}; o.changed=lists[changed]; diffcheck(&b,&t,&o);
    }
    label="diff nullable options";init(&b);init(&t);t.n_ops=1;t.ops[0]=t.ops[1];t.n_removed=1;
    strcpy(t.removed[0].op,"op0");t.removed[0].reason=DS_RM_UNCHANGED_DEP;
    strcpy(t.removed[0].ref,"op1");diffcheck(&b,&t,NULL);t.removed[0].ref[0]=0;diffcheck(&b,&t,NULL);
    label="diff duplicate missing fabricated";
    for(int records=0;records<=2;records++) {
        init(&b); init(&t); t.n_ops=1; t.ops[0]=t.ops[1]; t.n_removed=records;
        for(int i=0;i<records;i++){strcpy(t.removed[i].op,"op0");t.removed[i].reason=DS_RM_ELIMINATED;}
        diffcheck(&b,&t,&o);
    }
    init(&b); init(&t); t.n_removed=1; strcpy(t.removed[0].op,"fabricated"); diffcheck(&b,&t,&o);
    label="diff shared double count"; init(&b); init(&t); t.n_ops=1; t.ops[0]=t.ops[1]; t.n_removed=2;
    strcpy(t.removed[0].op,"op0"); t.removed[0].reason=DS_RM_SHARED; strcpy(t.removed[0].ref,"op1");
    strcpy(t.removed[1].op,"op1"); t.removed[1].reason=DS_RM_ELIMINATED; diffcheck(&b,&t,&o);
    label="diff kept changed and added";
    init(&b); init(&t); t.ops[0].t_ns=17; t.ops[1].bytes=9; diffcheck(&b,&t,&o);
    strcpy(t.ops[0].id,"added"); t.n_removed=1;strcpy(t.removed[0].op,"op0");t.removed[0].reason=1;diffcheck(&b,&t,&o);
    label="diff accounting bandwidth branches";
    for(int bw=-1;bw<=1;bw+=2) for(int bytes=0;bytes<=32;bytes+=8) {
        init(&b);init(&t); b.ops[0].bw=bw; b.ops[0].bytes=bytes;t.n_ops=1;t.ops[0]=t.ops[1];
        t.n_removed=1;strcpy(t.removed[0].op,"op0");t.removed[0].reason=1;diffcheck(&b,&t,&o);
    }
    label="diff streaming changed list";
    char list[2048];
    for(int n=0;n<=1024;n++) {
        memset(list,'x',n); strcpy(list+n,",op1,");
        init(&b);init(&t);t.n_ops=1;t.ops[0]=t.ops[1];t.n_removed=1;strcpy(t.removed[0].op,"op0");
        t.removed[0].reason=DS_RM_UNCHANGED_DEP;strcpy(t.removed[0].ref,"op1");o.changed=list;diffcheck(&b,&t,&o);
    }
    label="diff zero/max counts";o.changed=NULL;
    int counts[]={0,1,DS_MAX_OPS-1,DS_MAX_OPS};
    for(unsigned k=0;k<sizeof counts/sizeof *counts;k++) {
        init(&b);b.n_ops=counts[k];ds_op tmpl=b.ops[0];
        for(int i=0;i<b.n_ops;i++){b.ops[i]=tmpl;snprintf(b.ops[i].id,DS_ID,"op%d",i);}
        t=b;diffcheck(&b,&t,&o);t.n_ops=0;t.n_removed=b.n_ops;
        for(int i=0;i<b.n_ops;i++){strcpy(t.removed[i].op,b.ops[i].id);t.removed[i].reason=1;}
        diffcheck(&b,&t,&o);
    }
}
int main(void) {
    resolve_cases();diff_cases();
    printf("resolve differential: %u checks, %u failures\n",checks,failed);
    return failed ? 1 : 0;
}
