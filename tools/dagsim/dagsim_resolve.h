/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Internal fixed-view ABI shell. All selection, evidence, lookup and arithmetic
 * decisions live in dag_res.om / dag_diff.om. The host owns bytes and messages. */
#include "cudapp_native.h"
static uint64_t dr_call(uint64_t mode,uint64_t *w,uint64_t *s,unsigned char *t,uint64_t *a) {
    uint64_t result;
    if(omega_dag_res(&result,mode,w,16384,s,8192,t,131072,a,64))abort();
    return result;
}
static double dr_double_bits(uint64_t x) { double d; memcpy(&d,&x,sizeof d); return d; }
static uint64_t dr_bits(double d) { uint64_t x; memcpy(&x,&d,sizeof x); return x; }
static void dr_model(const ds_model *m, uint64_t *w, int side) {
    uint64_t *h=w+side*8192;
    h[0]=m->n_ops; h[1]=m->n_res; h[2]=m->n_deps; h[3]=m->n_removed;
    h[4]=(uint64_t)(int64_t)m->has_calibration; h[5]=(uint64_t)(int64_t)m->cal_taken_day;
    h[6]=(uint64_t)(int64_t)m->cal_valid_days; h[7]=(uint64_t)(int64_t)m->has_energy;
    h[8]=dr_bits(m->joules);h[9]=dr_bits(m->power_w);h[10]=m->energy_prov;h[12]=m->n_buf;
    for(int i=0;i<5;i++)h[13+i]=(unsigned char)m->instrument[i];
    for(int i=0;i<m->n_res;i++) {
        const ds_resource *r=&m->res[i]; uint64_t *p=h+32+i*8;
        p[0]=r->kind;p[1]=(uint64_t)(int64_t)r->slots;p[2]=dr_bits(r->cap);
        p[3]=dr_bits(r->lo);p[4]=dr_bits(r->hi);p[5]=r->prov;
    }
    for(int i=0;i<m->n_ops;i++) {
        const ds_op *r=&m->ops[i];uint64_t *p=h+288+i*16;
        p[0]=r->kind;p[1]=(uint64_t)(int64_t)r->engine;p[2]=dr_bits(r->t_ns);p[3]=dr_bits(r->lo);p[4]=dr_bits(r->hi);
        p[5]=r->t_prov;p[6]=r->has_t;p[7]=dr_bits(r->bytes);p[8]=(uint64_t)(int64_t)r->bw;
        p[9]=dr_bits(r->launch_ns);p[10]=(uint64_t)(int64_t)r->launcher;
        p[11]=r->M;p[12]=r->N;p[13]=r->core;p[14]=r->arm;
    }
    for(int i=0;i<m->n_deps;i++){h[4384+i*2]=m->deps[i].from;h[4385+i*2]=m->deps[i].to;}
    for(int i=0;i<m->n_removed;i++) {
        h[6432+i*4]=m->removed[i].reason;
    }
}
static void dr_names(const ds_model *m,unsigned char *t,int side) {
    unsigned char *names=t+side*65536;
    for(int i=0;i<m->n_ops;i++)memcpy(names+i*40,m->ops[i].id,DS_ID);
    for(int i=0;i<m->n_removed;i++) {
        memcpy(names+11520+i*208,m->removed[i].op,DS_ID);
        memcpy(names+11560+i*208,m->removed[i].ref,DS_ID);
        memcpy(names+11600+i*208,m->removed[i].proof,DS_SRC);
    }
}
static int dr_lookup(const ds_model *m, const char *id, int kind) {
    uint64_t w[16384],s[8192],a[64]={0};unsigned char t[131072];
    w[0]=m->n_ops;w[1]=m->n_res;a[1]=kind;
    if(kind==0) {
        for(int i=0;i<m->n_ops;i++)memcpy(t+i*40,m->ops[i].id,DS_ID);
    } else {
        for(int i=0;i<m->n_res;i++)memcpy(t+10240+i*40,m->res[i].id,DS_ID);
    }
    /* Copy at most DS_ID bytes: a longer query cannot equal a valid model id. */
    size_t n=strnlen(id,DS_ID);memcpy(t+130816,id,n);if(n<DS_ID)t[130816+n]=0;
    return (int)(int32_t)dr_call(2,w,s,t,a);
}
static int dr_refusal(const ds_model *m, const uint64_t *a, ds_result *r) {
    int i=(int)a[1],status=(int)a[2];const ds_op *op=&m->ops[i];
    switch(a[0]) {
    case 1:return refuse(r,status,"resource %s has prov=unknown; unknown stays unknown%s",m->res[i].id,"");
    case 2:return refuse(r,status,"op %s is a matvec with no rx_costmodel blob supplied (calibration model=)%s",op->id,"");
    case 3:return refuse(r,status,"op %s: rx_costmodel has no known cell for this (op, core, pressure) and arm; measure it first%s",op->id,"");
    case 4:return refuse(r,status,"op %s has prov=unknown (%s); unknown stays unknown",op->id,op->src[0]?op->src:"no source");
    case 5:return refuse(r,status,"op %s has no engine%s",op->id,"");
    case 6:return refuse(r,status,"op %s: launch without an engine%s",op->id,"");
    case 7:return refuse(r,status,"measured or fitted values need a calibration record naming when and under what load they were taken%s%s","","");
    case 8:{char d[64];snprintf(d,sizeof d,"asof is %d day(s) past the validity envelope",(int)(int32_t)a[34]);return refuse(r,status,"calibration %s is stale: %s",m->cal_id,d);}
    case 10:return refuse(r,status,"empty DAG%s%s","","");
    }
    return refuse_msg(r,status,"invalid Omega resolver refusal");
}
static int dr_resolved(const ds_model *m,const ds_opts *o,uint64_t *s,ds_result *r) {
    uint64_t w[16384],a[64]={0};unsigned char t[131072];
    dr_model(m,w,0);memset(s,0,8192*sizeof *s);
    a[5]=(uint64_t)(int64_t)o->mode;a[6]=(uint64_t)(int64_t)o->asof_day;
    a[7]=(uintptr_t)o->costmodel;a[9]=dr_bits(o->interval_q);a[35]=(uint64_t)time(NULL);
    for(;;) {
        uint64_t rc=dr_call(0,w,s,t,a);
        switch(rc) {
        case 0:return DS_OK;
        case 1:{
            RxCmFeatures f;memset(&f,0,sizeof f);
            f.op=RX_CM_OP_MATVEC;f.M=a[16];f.N=a[17];f.state_bytes=a[18];f.cache_bytes=a[19];
            f.core=a[20];f.eligible=a[21];f.current_arm=a[22];RxCmPrediction p;
            int st=rx_cm_predict((const RxCostModel *)o->costmodel,&f,(uint32_t)a[22],&p);
            a[24]=(uint64_t)(int64_t)st;a[25]=p.refused;a[26]=p.known;a[27]=p.n;
            a[28]=dr_bits(p.mean_log2_ps);a[29]=dr_bits(p.sd_log2);a[30]=dr_bits(p.dof);break;}
        case 2:a[31]=dr_bits(rx_cm_t_quantile(dr_double_bits(a[9]),dr_double_bits(a[30])));break;
        case 3:a[33]=dr_bits(pow(2.0,dr_double_bits(a[32])));break;
        default:return dr_refusal(m,a,r);
        }
    }
}
static int resolve(const ds_model *m,const ds_opts *o,ds_num *num,ds_result *r) {
    uint64_t s[8192];int rc=dr_resolved(m,o,s,r);if(rc)return rc;
    for(int i=0;i<m->n_ops;i++) {
        const uint64_t *p=s+32+i*10;num[i].t=dr_double_bits(p[0]);num[i].bytes=dr_double_bits(p[1]);
        num[i].launch=dr_double_bits(p[2]);num[i].cap=dr_double_bits(p[3]);num[i].prov=(int)p[8];
    }
    return DS_OK;
}
static double dr_counter(const ds_model *m,const ds_opts *o,int idx) {
    uint64_t w[16384],s[8192],a[64]={0};unsigned char t[131072];dr_model(m,w,0);
    a[1]=(uint64_t)(int64_t)idx;
    if(dr_call(1,w,s,t,a))return -DS_REFUSED_UNSUPPORTED;
    ds_model *c=malloc(sizeof *c);if(!c)return -DS_REFUSED_UNSUPPORTED;*c=*m;
    for(int i=0;i<m->n_res;i++) {
        uint64_t *p=w+32+i*8;c->res[i].slots=(int)(int32_t)p[1];c->res[i].cap=dr_double_bits(p[2]);
        c->res[i].lo=dr_double_bits(p[3]);c->res[i].hi=dr_double_bits(p[4]);
    }
    for(int i=0;i<m->n_ops;i++)c->ops[i].launch_ns=dr_double_bits(w[288+i*16+9]);
    ds_result r;ds_opts x=*o;x.mode=0;int rc=ds_eval(c,&x,&r);free(c);
    return rc==DS_OK?r.makespan_ns:-(double)rc;
}
static int dr_diff(const ds_model *b,const ds_model *t,const ds_opts *o,ds_diff *d) {
    uint64_t w[16384],s[8192],a[64]={0};unsigned char names[131072];
    dr_model(b,w,0);dr_model(t,w,1);dr_names(b,names,0);dr_names(t,names,1);
    memset(w+7600,0,DS_MAX_OPS*sizeof *w);
    const char *list=o?o->changed:NULL;
    if(list) {
        size_t left=strlen(list);
        do {
            size_t n=left>256?256:left;memcpy(names+130816,list,n);a[41]=n;a[42]=(n==left);
            dr_call(3,w,s,names,a);list+=n;left-=n;
        }while(left);
    }
    memset(a,0,sizeof a);uint64_t result;
    if(omega_dag_diff(&result,w,16384,names,131072,a,64))abort();
    int rc=(int)result;memset(d,0,sizeof *d);
    d->n_added=(int)a[10];d->n_kept=(int)a[11];d->n_removed=(int)a[12];d->double_counted=(int)a[13];d->required_removed=(int)a[14];
    for(int i=0;i<8;i++)d->removed_ns[i]=dr_double_bits(a[32+i]);
    int i=(int)a[1];const ds_removed *rec=&t->removed[a[3]];
    switch(a[0]) {
    case 1:snprintf(d->reason,sizeof d->reason,"op %s kept but its demand changed; trace conservation broken",b->ops[i].id);break;
    case 2:snprintf(d->reason,sizeof d->reason,"op %s vanished without a removed record",b->ops[i].id);break;
    case 3:snprintf(d->reason,sizeof d->reason,"op %s removed %d times (double counted)",b->ops[i].id,(int)a[4]);break;
    case 4:snprintf(d->reason,sizeof d->reason,"op %s: removal reason %d not supported by its evidence (ref=%s proof=%s changed=%s)",b->ops[i].id,rec->reason,rec->ref,rec->proof,o&&o->changed?o->changed:"");break;
    case 5:snprintf(d->reason,sizeof d->reason,"removed record for %s, which was never in the baseline",rec->op);break;
    }
    return rc;
}
