/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Link the unchanged rx_costmodel_unit through --wrap: every public test input
 * reaches both implementations. Numeric distance is measured, not substituted
 * for exact agreement of refusal, eligibility, ranking, actions or reasons. */
#include "runtime/rx_costmodel.h"
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef NDEBUG
#error "cm_decision_diff requires enabled assertions"
#endif
struct DiffCase {
    RxCostModel model;
    RxCmFeatures features;
    RxCmPolicy policy;
    uint32_t kind, arm;
    int allow;
};
static struct DiffCase worst;
static const RxCostModel *active_model;
static const RxCmFeatures *active_features;
static const RxCmPolicy *active_policy;
static uint32_t active_kind, active_arm;
static int active_allow;
static const char *worst_field = "none";
static double worst_a, worst_b;
void __real_rx_cm_decide(const RxCostModel *, const RxCmPolicy *, const RxCmFeatures *, int, RxCmDecision *);
void oracle_cm_decide(const RxCostModel *, const RxCmPolicy *, const RxCmFeatures *, int, RxCmDecision *);
int __real_rx_cm_predict(const RxCostModel *, const RxCmFeatures *, uint32_t, RxCmPrediction *);
int oracle_cm_predict(const RxCostModel *, const RxCmFeatures *, uint32_t, RxCmPrediction *);
static unsigned decisions, predictions;
static uint64_t max_ulp;
static void numeric(const char *field, double a, double b) {
    uint64_t x, y;
    memcpy(&x, &a, 8); memcpy(&y, &b, 8);
    x = x >> 63 ? ~x : x | UINT64_C(0x8000000000000000);
    y = y >> 63 ? ~y : y | UINT64_C(0x8000000000000000);
    uint64_t distance = x > y ? x-y : y-x;
    if (distance > max_ulp) {
        max_ulp = distance;
        worst_field = field; worst_a = a; worst_b = b;
        memset(&worst,0,sizeof worst);
        if (active_model) worst.model = *active_model;
        if (active_features) worst.features = *active_features;
        if (active_policy) worst.policy = *active_policy;
        worst.kind = active_kind; worst.arm = active_arm; worst.allow = active_allow;
    }
    if (distance != 0) {
        FILE *output=fopen("build/cm_decision_max.case","wb");
        assert(output && fwrite(&worst,sizeof worst,1,output)==1);
        assert(fclose(output)==0);
        fprintf(stderr,"CM DECISION floating differential FAIL field=%s candidate=%a oracle=%a ulp=%llu\n",
                field,a,b,(unsigned long long)distance);
        abort();
    }
}
static void prediction(const RxCmPrediction *a, const RxCmPrediction *b) {
    assert(a->n == b->n && a->known == b->known && a->borrowed == b->borrowed && a->refused == b->refused);
    numeric("mean_log2_ps",a->mean_log2_ps,b->mean_log2_ps); numeric("sd_log2",a->sd_log2,b->sd_log2);
    numeric("dof",a->dof,b->dof); numeric("energy_pj",a->energy_pj,b->energy_pj); numeric("fail_p",a->fail_p,b->fail_p);
}
void __wrap_rx_cm_decide(const RxCostModel *m, const RxCmPolicy *p, const RxCmFeatures *f, int allow, RxCmDecision *out) {
    RxCmDecision expected;
    active_model=m; active_features=f; active_policy=p; active_kind=1; active_allow=allow;
    __real_rx_cm_decide(m,p,f,allow,out);
    oracle_cm_decide(m,p,f,allow,&expected);
    assert(out->action == expected.action && out->arm == expected.arm && out->why == expected.why &&
           out->measure[0] == expected.measure[0] && out->measure[1] == expected.measure[1]);
    numeric("p_runner_up_better",out->p_runner_up_better, expected.p_runner_up_better);
    numeric("expected_improvement",out->expected_improvement, expected.expected_improvement);
    for (unsigned i=0;i<RX_CM_ARMS;++i) prediction(out->pred+i,expected.pred+i);
    ++decisions;
}
int __wrap_rx_cm_predict(const RxCostModel *m, const RxCmFeatures *f, uint32_t arm, RxCmPrediction *out) {
    RxCmPrediction expected = {0};
    active_model=m; active_features=f; active_policy=NULL; active_kind=2; active_arm=arm;
    int a = __real_rx_cm_predict(m,f,arm,out);
    int b = oracle_cm_predict(m,f,arm,out ? &expected : NULL);
    assert(a == b);
    if (out) prediction(out,&expected);
    ++predictions;
    return a;
}
static uint64_t seed = 1;
static uint32_t random_word(void) {
    seed = seed * UINT64_C(6364136223846793005) + 1;
    return (uint32_t)(seed >> 32);
}
__attribute__((constructor)) static void random_cases(void) {
    const char *replay = getenv("CM_DIFF_REPLAY");
    if (replay) {
        struct DiffCase c;
        FILE *input=fopen(replay,"rb");
        assert(input && fread(&c,sizeof c,1,input)==1);
        assert(fclose(input)==0);
        if (c.kind==1) {
            RxCmDecision d;
            if (getenv("CM_DIFF_THRESHOLD_PROBE")) {
                RxCmDecision candidate, reference;
                __real_rx_cm_decide(&c.model,&c.policy,&c.features,c.allow,&candidate);
                oracle_cm_decide(&c.model,&c.policy,&c.features,c.allow,&reference);
                fprintf(stderr,"CM DECISION threshold probe EI candidate=%a oracle=%a\n",
                        candidate.expected_improvement,reference.expected_improvement);
                if (candidate.expected_improvement >= 0 && reference.expected_improvement >= 0) {
                    c.policy.ei_min=fmin(candidate.expected_improvement,reference.expected_improvement);
                    c.policy.explore_frac=1;
                    c.policy.explore_allow_ps=UINT64_MAX;
                    c.model.work_ps=UINT64_MAX; c.model.explore_ps=0; c.allow=1;
                }
            }
            __wrap_rx_cm_decide(&c.model,&c.policy,&c.features,c.allow,&d);
        } else {
            RxCmPrediction p;
            __wrap_rx_cm_predict(&c.model,&c.features,c.arm,&p);
        }
        return;
    }
    RxCostModel m;
    RxCmPolicy p;
    rx_cm_init(&m,RX_CM_ARMS);
    rx_cm_default_policy(&p);
    RxCmFeatures f = {.M=32,.N=4,.cache_bytes=4096,.eligible=255};
    for (unsigned arm=0;arm<RX_CM_ARMS;++arm) for (unsigned i=0;i<128;++i) {
        f.M=1+random_word()%4096; f.N=1+random_word()%32;
        f.state_bytes=random_word();
        RxCmObservation o = {.ps_per_call=1+random_word(),.failed=(i==0 && arm==1)};
        int status=rx_cm_observe(&m,&f,arm,&o);
        assert(status == RX_CM_OK);
    }
    for (unsigned i=0;i<1024;++i) {
        f.op=i%17 ? 0 : UINT32_MAX;
        f.M=random_word(); f.N=random_word(); f.state_bytes=random_word();
        f.cache_bytes=i%5 ? random_word() : 0;
        f.core=random_word()%5; f.pressure=random_word()%5;
        f.eligible=random_word(); f.latency_budget_ps=i%3 ? 0 : random_word();
        f.energy_budget_pj=i%7 ? 0 : random_word();
        p.fallback_arm=random_word()%11;
        p.explore_allow_ps=i%2 ? 0 : UINT64_MAX;
        m.work_ps=random_word(); m.explore_ps=random_word();
        RxCmDecision d;
        __wrap_rx_cm_decide(&m,&p,&f,(i/2)%2,&d);
    }
    RxCmPrediction poisoned;
    memset(&poisoned,0xa5,sizeof poisoned);
    assert(__wrap_rx_cm_predict(NULL,&f,0,&poisoned)==RX_CM_ERR_ARG);
    memset(&poisoned,0xa5,sizeof poisoned);
    assert(__wrap_rx_cm_predict(&m,NULL,0,&poisoned)==RX_CM_ERR_ARG);
    memset(&poisoned,0xa5,sizeof poisoned);
    assert(__wrap_rx_cm_predict(&m,&f,RX_CM_ARMS,&poisoned)==RX_CM_ERR_ARG);
    assert(__wrap_rx_cm_predict(&m,&f,0,NULL)==RX_CM_ERR_ARG);
}
__attribute__((destructor)) static void report(void) {
    printf("CM DECISION differential PASS decisions=%u predictions=%u max_numeric_ulp=%llu field=%s candidate=%a oracle=%a\n",
           decisions,predictions,(unsigned long long)max_ulp,worst_field,worst_a,worst_b);
}
#ifdef CM_DIFF_REPLAY_MAIN
int main(void) { return 0; }
#endif
