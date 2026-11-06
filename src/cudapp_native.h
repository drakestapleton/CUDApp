/* Generated checked portable Omega declarations. Regenerate with om/units.sh.
 * Status: 0 done, 1..14 trap, 32 argument refused, 33 reentry.
 * Only a successful call writes result. Attention data views use exact runtime lengths;
 * all other roots keep their fixed-length profile.
 * Implementation is Omega lowered by omega-bridge, not the native AArch64 ABI. */
#ifndef CUDAPP_PORTABLE_OMEGA_H
#define CUDAPP_PORTABLE_OMEGA_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define OMEGA_OK 0u
#define OMEGA_BAD_ARG 32u
#define OMEGA_REENTRY 33u
uint32_t omega_bw_enc_sm121(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, uint64_t p2, uint64_t p3, void *p4, uint64_t p4_n);
uint32_t omega_cubin_elf(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, void *p3, uint64_t p3_n);
uint32_t omega_mathlibs_ir(uint64_t  *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_ampere_enc_sm86(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, uint64_t p2, uint64_t p3, void *p4, uint64_t p4_n);
uint32_t omega_amp_runtime(uint64_t  *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n);
uint32_t omega_native_repack(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, void *p3, uint64_t p3_n, void *p4, uint64_t p4_n);
uint32_t omega_native_quant(uint64_t  *result, uint64_t p0, void *p1, uint64_t p1_n, void *p2, uint64_t p2_n);
uint32_t omega_native_compose(uint64_t  *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n, void *p2, uint64_t p2_n, void *p3, uint64_t p3_n, void *p4, uint64_t p4_n);
uint32_t omega_bc_encode(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, uint64_t p2, uint64_t p3, void *p4, uint64_t p4_n);
uint32_t omega_native_cache(uint64_t  *result, void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, uint64_t p3);
uint32_t omega_native_lease(uint64_t  *result, void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, uint64_t p3);
uint32_t omega_native_slotrev(uint64_t  *result, const void *p0, uint64_t p0_n, uint64_t p1, uint64_t p2);
uint32_t omega_native_event(uint64_t  *result, void *p0, uint64_t p0_n, void *p1, uint64_t p1_n, uint64_t p2, uint64_t p3);
uint32_t omega_native_validate(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, const void *p3, uint64_t p3_n, const void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_native_key(uint64_t  *result, const void *p0, uint64_t p0_n, uint64_t p1, void *p2, uint64_t p2_n);
uint32_t omega_native_codekey(uint64_t  *result, const void *p0, uint64_t p0_n, uint64_t p1, void *p2, uint64_t p2_n);
uint32_t omega_native_templates(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, const void *p3, uint64_t p3_n);
uint32_t omega_native_graph(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, void *p2, uint64_t p2_n, const void *p3, uint64_t p3_n, const void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_native_context_lookup(uint64_t  *result, const void *p0, uint64_t p0_n, uint64_t p1, const void *p2, uint64_t p2_n);
uint32_t omega_native_binding(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, void *p3, uint64_t p3_n);
uint32_t omega_moe_plan(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, void *p2, uint64_t p2_n, void *p3, uint64_t p3_n, void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_moe_group(uint64_t  *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n);
uint32_t omega_moe_ids32(uint64_t  *result, const void *p0, uint64_t p0_n, uint64_t p1, void *p2, uint64_t p2_n);
uint32_t omega_moe_maps32(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, void *p2, uint64_t p2_n);
uint32_t omega_moe_segs32(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, void *p3, uint64_t p3_n);
uint32_t omega_moe_windows(uint64_t  *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n, void *p2, uint64_t p2_n);
uint32_t omega_moe_admit(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, void *p3, uint64_t p3_n, void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_native_mcpack(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, void *p3, uint64_t p3_n);
uint32_t omega_native_mcbind(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, const void *p3, uint64_t p3_n, uint64_t p4, void *p5, uint64_t p5_n);
uint32_t omega_native_mcblob(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, uint64_t p3, const void *p4, uint64_t p4_n, uint64_t p5);
uint32_t omega_native_mccover(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, const void *p3, uint64_t p3_n);
uint32_t omega_native_mcintent(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, const void *p3, uint64_t p3_n, const void *p4, uint64_t p4_n);
uint32_t omega_native_mcseal(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, uint64_t p2, void *p3, uint64_t p3_n);
uint32_t omega_native_mcreseal(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, uint64_t p2, void *p3, uint64_t p3_n, void *p4, uint64_t p4_n);
uint32_t omega_native_mccode(uint64_t  *result, const void *p0, uint64_t p0_n, uint64_t p1, void *p2, uint64_t p2_n);
uint32_t omega_native_mclaunch(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, const void *p3, uint64_t p3_n, void *p4, uint64_t p4_n);
uint32_t omega_native_mcdir(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, void *p2, uint64_t p2_n);
uint32_t omega_native_mcalloc(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, void *p2, uint64_t p2_n);
uint32_t omega_native_mcseries(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, void *p3, uint64_t p3_n);
uint32_t omega_native_qgshape(uint64_t  *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3);
uint32_t omega_native_qgreading(uint64_t  *result, uint64_t p0, uint64_t p1);
uint32_t omega_native_qgworkspace(uint64_t  *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4);
uint32_t omega_native_qgorder(uint64_t  *result, void *p0, uint64_t p0_n, uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_native_attnmirplan(uint64_t  *result, const void *p0, uint64_t p0_n);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_native_attnmirnext(uint64_t  *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3);
uint32_t omega_q38_runtime(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, void *p2, uint64_t p2_n);
uint32_t omega_q38_emit(uint64_t  *result, void *p0, uint64_t p0_n);
uint32_t omega_artifact_policy(uint64_t  *result, uint64_t p0, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, const void *p3, uint64_t p3_n);
uint32_t omega_artifact_parse(uint64_t  *result, uint64_t p0, uint64_t p1, void *p2, uint64_t p2_n);
uint32_t omega_api_policy(uint64_t  *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3);
uint32_t omega_cm_sort(uint64_t  *result, void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, void *p2, uint64_t p2_n, void *p3, uint64_t p3_n);
uint32_t omega_cm_math(uint64_t  *result, uint64_t p0, uint64_t p1);
uint32_t omega_cm_fit(uint64_t  *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n);
uint32_t omega_cm_stats(uint64_t  *result, uint64_t p0, void *p1, uint64_t p1_n);
uint32_t omega_cm_features(uint64_t  *result, uint64_t p0, void *p1, uint64_t p1_n);
uint32_t omega_cm_quantile(uint64_t  *result, uint64_t p0, uint64_t p1);
uint32_t omega_cm_decision(uint64_t  *result, uint64_t p0, void *p1, uint64_t p1_n);
uint32_t omega_cm_prediction(uint64_t  *result, void *p0, uint64_t p0_n);
uint32_t omega_cm_calibration(uint64_t  *result, uint64_t p0, void *p1, uint64_t p1_n);
uint32_t omega_cm_blob(uint64_t  *result, uint64_t p0, void *p1, uint64_t p1_n);
uint32_t omega_cm_digest(uint64_t  *result, uint64_t p0, void *p1, uint64_t p1_n, void *p2, uint64_t p2_n);
uint32_t omega_api_mm_plan(uint64_t  *result, uint64_t p0, void *p1, uint64_t p1_n);
uint32_t omega_api_at_plan(uint64_t  *result, uint64_t p0, void *p1, uint64_t p1_n);
uint32_t omega_api_ew_plan(uint64_t  *result, uint64_t p0, void *p1, uint64_t p1_n);
uint32_t omega_api_bf_plan(uint64_t  *result, uint64_t p0, void *p1, uint64_t p1_n);
uint32_t omega_api_da_plan(uint64_t  *result, uint64_t p0, void *p1, uint64_t p1_n);
uint32_t omega_api_rt_plan(uint64_t  *result, uint64_t p0, void *p1, uint64_t p1_n);
uint32_t omega_gemm_ex_plan(uint64_t  *result, uint64_t p0, const void *p1, uint64_t p1_n, void *p2, uint64_t p2_n);
uint32_t omega_gemm_ex_row(uint64_t  *result, uint64_t p0, uint64_t p1, const void *p2, uint64_t p2_n, const void *p3, uint64_t p3_n, void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_gemm_ex_x3(uint64_t  *result, uint64_t p0, uint64_t p1, uint64_t p2, const void *p3, uint64_t p3_n, void *p4, uint64_t p4_n);
uint32_t omega_gemm_ex_acc(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, void *p3, uint64_t p3_n);
uint32_t omega_gemm_ex_dot(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n);
uint32_t omega_gemm_ex_epi(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, const void *p3, uint64_t p3_n, const void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_gemm_ex_fp8(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, void *p3, uint64_t p3_n);
uint32_t omega_apishp(uint64_t  *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4, uint64_t p5);
uint32_t omega_apishk(uint64_t  *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4, uint64_t p5);
uint32_t omega_apishl(uint64_t  *result, uint64_t p0, uint64_t p1, void *p2, uint64_t p2_n);
uint32_t omega_apishb(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, uint64_t p2);
uint32_t omega_apishr(uint64_t  *result, const void *p0, uint64_t p0_n);
uint32_t omega_av_pack(uint64_t  *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n, uint64_t p2, uint64_t p3);
uint32_t omega_av_lay(uint64_t  *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n, uint64_t p2);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_av_copy(uint64_t  *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n, uint64_t p2);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_av_rope(uint64_t  *result, void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_av_fold(uint64_t  *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_av_set(uint64_t  *result, void *p0, uint64_t p0_n, uint64_t p1, uint64_t p2);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_av_fill(uint64_t  *result, void *p0, uint64_t p0_n, uint64_t p1, uint64_t p2);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_av_fillf(uint64_t  *result, void *p0, uint64_t p0_n, uint64_t p1, float p2);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_av_count(uint64_t  *result, const void *p0, uint64_t p0_n, uint64_t p1, uint64_t p2);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_av_cntf(uint64_t  *result, const void *p0, uint64_t p0_n, uint64_t p1, float p2);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_av_bfexp(uint64_t  *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n, uint64_t p2);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_av_bffin(uint64_t  *result, const void *p0, uint64_t p0_n, uint64_t p1);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_av_any(uint64_t  *result, const void *p0, uint64_t p0_n, uint64_t p1);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_av_comp(uint64_t  *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n, uint64_t p2);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_av_rpois(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, uint64_t p2, uint64_t p3);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_av_z64(uint64_t  *result, const void *p0, uint64_t p0_n, uint64_t p1);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_av_bids(uint64_t  *result, const void *p0, uint64_t p0_n, uint64_t p1, uint64_t p2);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_av_rows(uint64_t  *result, void *p0, uint64_t p0_n, uint64_t p1, uint64_t p2);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_av_gtab(uint64_t  *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, uint64_t p3);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_av_range(uint64_t  *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n, uint64_t p2);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_av_total(uint64_t  *result, const void *p0, uint64_t p0_n, uint64_t p1, uint64_t p2);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_av_table(uint64_t  *result, void *p0, uint64_t p0_n, void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, uint64_t p3);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_av_touch(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, void *p2, uint64_t p2_n, uint64_t p3);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_av_blks(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, uint64_t p2);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_av_rfit(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, uint64_t p2);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_av_gath(uint64_t  *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_av_stage(uint64_t  *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, const void *p3, uint64_t p3_n);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_av_reset(uint64_t  *result, void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, uint64_t p2);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_av_hmap(uint64_t  *result, void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_av_eff(uint64_t  *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n, uint64_t p2);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_av_best(uint64_t  *result, const void *p0, uint64_t p0_n, uint64_t p1, uint64_t p2);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_av_max(uint64_t  *result, const void *p0, uint64_t p0_n, uint64_t p1);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_av_ctx(uint64_t  *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n, uint64_t p2);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_ds_round(uint64_t  *result, uint64_t p0);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_ds_slot(uint64_t  *result, uint64_t p0, uint64_t p1);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_ds_vecf(uint64_t  *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n, uint64_t p2);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_ds_vecb(uint64_t  *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n, uint64_t p2);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_na_read(uint64_t  *result, const void *p0, uint64_t p0_n, uint64_t p1);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_na_cubin(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_na_layout(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, uint64_t p2);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_na_ver(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_na_json(uint64_t  *result, uint64_t p0, void *p1, uint64_t p1_n);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_mm_dot(uint64_t  *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, void *p3, uint64_t p3_n);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_mm_err(uint64_t  *result, const void *p0, uint64_t p0_n, uint64_t p1, void *p2, uint64_t p2_n);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_mm_init(uint64_t  *result, void *p0, uint64_t p0_n);
/* Omega Bridge runtime views: numeric extents are capacities; * is unsized. */
uint32_t omega_mm_next(uint64_t  *result, void *p0, uint64_t p0_n);
uint32_t omega_native_revision(uint64_t *result, uint64_t p0);
uint32_t omega_native_epoch(uint64_t *result, uint64_t p0);
uint32_t omega_native_f16(uint64_t *result, uint64_t p0);
#ifdef __cplusplus
}
#endif
#endif
