/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#define CUDAPP_MIGRATION_PENDING_C_CM_SORT 1
#define CUDAPP_MIGRATION_PENDING_C_CM_MATH 1
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
