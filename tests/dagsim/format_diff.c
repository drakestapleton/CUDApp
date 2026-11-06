/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 * Read-only C formatting oracles retained from the migrated expressions. */
#include "dagsim.h"
#include "dagsim_format.h"
#include <math.h>
#include <stdio.h>
int oracle_ds_day_from_date(const char *);
const char *oracle_ds_status_name(int);
const char *oracle_ds_prov_name(int);
static int checks, bad;
#define CHECK(x) do { checks++; if (!(x)) { fprintf(stderr,"format check line %d\n",__LINE__); bad++; } } while (0)
static void same(unsigned mode, uint64_t a[16], double expected) {
    double got=ds_unbits(ds_decide(mode,a));
    CHECK((isnan(got)&&isnan(expected)) || ds_bits(got)==ds_bits(expected));
}
int main(void) {
    const char *dates[]={"1970-01-01","1969-12-31","2000-02-29","1900-03-01","2026-11-06","2026-0-0","2026-13-32","2026--1-1","2026-11-06junk","-1-1-1","2026-11","2026-11-x","+2026-+11-+6"};
    for (unsigned i=0;i<sizeof dates/sizeof *dates;i++) CHECK(ds_day_from_date(dates[i])==oracle_ds_day_from_date(dates[i]));
    for (int i=-1;i<=10;i++) CHECK(!strcmp(ds_status_name(i),oracle_ds_status_name(i)));
    for (int i=-1;i<=5;i++) CHECK(!strcmp(ds_prov_name(i),oracle_ds_prov_name(i)));
    double v[]={0.0,-0.0,1.0,-1.0,0.5,0.05,1e-6,1e-9,1e15,1e14,INFINITY,-INFINITY,NAN};
    for (unsigned i=0;i<sizeof v/sizeof *v;i++) {
        uint64_t a[16]={ds_bits(v[i])};
        CHECK(ds_decide(7,a)==!(v[i]==floor(v[i])&&fabs(v[i])<1e15));
        same(18,a,v[i]*1e-9);
        for (unsigned j=0;j<sizeof v/sizeof *v;j++) {
            uint64_t p[16]={ds_bits(v[i]),ds_bits(v[j])};
            CHECK(ds_decide(8,p)==(v[i]>0&&v[j]>0));
            CHECK(ds_decide(9,p)==(v[i]+1e-6>=v[j]));
            same(17,p,v[i]-v[j]); same(19,p,v[j]>v[i]?v[j]:v[i]);
            for (unsigned k=0;k<sizeof v/sizeof *v;k++) {
                uint64_t s[16]={ds_bits(v[i]),ds_bits(v[j]),ds_bits(v[k])};
                same(16,s,v[j]>=0?fabs(v[i]-v[j])*(v[k]>0?v[k]:0.05):0);
            }
        }
    }
    for (unsigned kind=1;kind<=4;kind++) for (unsigned prov=0;prov<=4;prov++) {
        uint64_t w[16]={kind,ds_bits(10),ds_bits(8),ds_bits(12),prov};
        double width=kind==2?(12.0-8.0)/10.0:0;
        if(prov==4) width=1;
        same(15,w,width);
        char s[256]; ds_fmt(12,w,s); CHECK(strlen(s)>0);
    }
    for (unsigned kind=1;kind<=6;kind++) for (unsigned has=0;has<=1;has++) {
        uint64_t p[16]={kind,has,3}; CHECK(ds_decide(13,p)==(kind==3&&!has?2:3));
    }
    for (unsigned m=0;m<=1;m++) for(unsigned syn=0;syn<=1;syn++) {
        uint64_t a[16]={m,syn}; char s[256]; ds_fmt(10,a,s);
        CHECK(!strcmp(s,m?"MEASURED":syn?"SYNTHETIC_SENSITIVITY_ONLY":"UNMEASURED"));
        CHECK(ds_decide(11,a)==m);
    }
    for (unsigned i=0;i<3;i++) for(unsigned j=0;j<3;j++) for(unsigned e=0;e<2;e++) {
        uint64_t a[16]={ds_bits(v[i]),ds_bits(v[j]),ds_bits(1),ds_bits(v[j]),ds_bits(v[i]),ds_bits(0),e};
        int le=v[j]<=v[i]&&v[i]<=v[j]&&(!e||0<=1);
        int lt=v[j]<v[i]||v[i]<v[j]||(e&&0<1);
        CHECK(ds_decide(14,a)==(le&&lt));
    }
    for(unsigned n=0;n<=32;n++) {
        double scores[32]; unsigned ix[32]; char s[256]={0};
        for(unsigned i=0;i<n;i++) { scores[i]=(double)(i%5); ix[i]=i; }
        for(unsigned i=1;i<n;i++) { unsigned k=ix[i],j=i; while(j&&scores[ix[j-1]]<scores[k]){ix[j]=ix[j-1];j--;}ix[j]=k; }
        memcpy(s,scores,n*sizeof *scores); uint64_t a[16]={n}; CHECK(!ds_fmt(20,a,s));
        for(unsigned i=0;i<n;i++) CHECK((unsigned char)s[i]==ix[i]);
    }
    uint64_t over[16]={33}; char s[256]; CHECK(ds_fmt(20,over,s)==1);
    printf("format differential: %d checks, %d failures\n",checks,bad); return bad?1:0;
}
