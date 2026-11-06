/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#define omega_cm_math oracle_cm_math_trace
#define CUDAPP_MIGRATION_PENDING_C_CM_SORT 1
/* Omega's per-operation binary64 and first-party libm are canonical. This
 * retained C algorithm is compiled with -ffp-contract=off; its math calls bind
 * the same canonical routines. cm_math_unit independently measures glibc drift. */
#define CUDAPP_MIGRATION_PENDING_C_CM_MATH 0
#define CUDAPP_MIGRATION_PENDING_C_CM_FIT 1
#define CUDAPP_MIGRATION_PENDING_C_CM_STATS 1
#define CUDAPP_MIGRATION_PENDING_C_CM_FEATURES 1
#define CUDAPP_MIGRATION_PENDING_C_CM_QUANTILE 1
#define CUDAPP_MIGRATION_PENDING_C_CM_DECISION 1
#define rx_cm_t_quantile oracle_cm_t_quantile
#define CUDAPP_MIGRATION_PENDING_C_CM_PREDICTION 1
#define CUDAPP_MIGRATION_PENDING_C_CM_CALIBRATION 1
#define CUDAPP_MIGRATION_PENDING_C_CM_BLOB 1
#define CUDAPP_MIGRATION_PENDING_C_CM_DIGEST 1
#define CUDAPP_MIGRATION_PENDING_C_CM_ROUTE 1
#define rx_cm_default_policy oracle_cm_default_policy
#define rx_cm_init oracle_cm_init
#define rx_cm_observe oracle_cm_observe
#define rx_cm_predict oracle_cm_predict
#define rx_cm_decide oracle_cm_decide
#define rx_cm_calibrate oracle_cm_calibrate
#define rx_cm_charge oracle_cm_charge
#define rx_cm_blob_size oracle_cm_blob_size
#define rx_cm_serialize oracle_cm_serialize
#define rx_cm_deserialize oracle_cm_deserialize
#define rx_cm_digest oracle_cm_digest
#define rx_cm_basis oracle_cm_basis
#define rx_cm_cell oracle_cm_cell
#include "../../src/runtime/rx_costmodel.c"
#undef omega_cm_math
#undef log
#undef log2
#undef ceil
#undef exp
#undef exp2
#undef sqrt
#undef erfc
#undef fabs
#include <stdio.h>
uint32_t omega_cm_math(uint64_t *, uint64_t, uint64_t);
static uint64_t math_calls[8], math_ulp[8], math_input[8], math_got[8], math_reference[8];
uint32_t oracle_cm_math_trace(uint64_t *result, uint64_t mode, uint64_t input) {
    uint32_t status=omega_cm_math(result,mode,input);
    if (status!=OMEGA_OK || mode>=8) return status;
    double x, reference;
    memcpy(&x,&input,sizeof x);
    switch (mode) {
    case 0: reference=log(x); break;
    case 1: reference=log2(x); break;
    case 2: reference=ceil(x); break;
    case 3: reference=exp(x); break;
    case 4: reference=exp2(x); break;
    case 5: reference=sqrt(x); break;
    case 6: reference=erfc(x); break;
    default: reference=fabs(x); break;
    }
    uint64_t bits;
    memcpy(&bits,&reference,sizeof bits);
    uint64_t a=*result, b=bits;
    a=a>>63 ? ~a : a|UINT64_C(0x8000000000000000);
    b=b>>63 ? ~b : b|UINT64_C(0x8000000000000000);
    uint64_t distance=a>b ? a-b : b-a;
    ++math_calls[mode];
    if (distance>math_ulp[mode]) {
        math_ulp[mode]=distance; math_input[mode]=input;
        math_got[mode]=*result; math_reference[mode]=bits;
    }
    return status;
}
static void __attribute__((destructor)) report_math_inputs(void) {
    for (unsigned mode=0;mode<8;++mode) if (math_calls[mode])
        printf("CM LIBM actual-inputs mode=%u calls=%llu max_ulp=%llu input=%016llx candidate=%016llx glibc=%016llx\n",
               mode,(unsigned long long)math_calls[mode],(unsigned long long)math_ulp[mode],
               (unsigned long long)math_input[mode],(unsigned long long)math_got[mode],
               (unsigned long long)math_reference[mode]);
}
