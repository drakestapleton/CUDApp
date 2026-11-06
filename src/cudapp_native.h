/* Generated checked portable Omega declarations. Regenerate with om/units.sh.
 * Status: 0 done, 1..14 trap, 32 argument refused, 33 reentry.
 * Only a successful call writes result. Views use exact declared lengths.
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
uint32_t omega_bw_enc_sm121(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, uint64_t p2, uint64_t p3, void *p4, uint64_t p4_n);
uint32_t omega_cubin_elf(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, void *p3, uint64_t p3_n);
uint32_t omega_mathlibs_ir(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_ampere_enc_sm86(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, uint64_t p2, uint64_t p3, void *p4, uint64_t p4_n);
uint32_t omega_amp_runtime(uint64_t *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n);
uint32_t omega_native_repack(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, void *p3, uint64_t p3_n, void *p4, uint64_t p4_n);
uint32_t omega_native_quant(uint64_t *result, uint64_t p0, void *p1, uint64_t p1_n, void *p2, uint64_t p2_n);
uint32_t omega_native_compose(uint64_t *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n, void *p2, uint64_t p2_n, void *p3, uint64_t p3_n, void *p4, uint64_t p4_n);
uint32_t omega_bc_encode(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, uint64_t p2, uint64_t p3, void *p4, uint64_t p4_n);
uint32_t omega_native_cache(uint64_t *result, void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, uint64_t p3);
uint32_t omega_native_lease(uint64_t *result, void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, uint64_t p3);
uint32_t omega_native_slotrev(uint64_t *result, const void *p0, uint64_t p0_n, uint64_t p1, uint64_t p2);
uint32_t omega_native_event(uint64_t *result, void *p0, uint64_t p0_n, void *p1, uint64_t p1_n, uint64_t p2, uint64_t p3);
uint32_t omega_native_validate(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, const void *p3, uint64_t p3_n, const void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_native_key(uint64_t *result, const void *p0, uint64_t p0_n, uint64_t p1, void *p2, uint64_t p2_n);
uint32_t omega_native_codekey(uint64_t *result, const void *p0, uint64_t p0_n, uint64_t p1, void *p2, uint64_t p2_n);
uint32_t omega_native_templates(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, const void *p3, uint64_t p3_n);
uint32_t omega_native_graph(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, void *p2, uint64_t p2_n, const void *p3, uint64_t p3_n, const void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_native_context_lookup(uint64_t *result, const void *p0, uint64_t p0_n, uint64_t p1, const void *p2, uint64_t p2_n);
uint32_t omega_native_binding(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, void *p3, uint64_t p3_n);
uint32_t omega_moe_plan(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, void *p2, uint64_t p2_n, void *p3, uint64_t p3_n, void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_moe_group(uint64_t *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n);
uint32_t omega_moe_ids32(uint64_t *result, const void *p0, uint64_t p0_n, uint64_t p1, void *p2, uint64_t p2_n);
uint32_t omega_moe_maps32(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, void *p2, uint64_t p2_n);
uint32_t omega_moe_segs32(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, void *p3, uint64_t p3_n);
uint32_t omega_moe_windows(uint64_t *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n, void *p2, uint64_t p2_n);
uint32_t omega_moe_admit(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, void *p3, uint64_t p3_n, void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_native_mcpack(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, void *p3, uint64_t p3_n);
uint32_t omega_native_mcbind(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, const void *p3, uint64_t p3_n, uint64_t p4, void *p5, uint64_t p5_n);
uint32_t omega_native_mcblob(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, uint64_t p3, const void *p4, uint64_t p4_n, uint64_t p5);
uint32_t omega_native_mccover(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, const void *p3, uint64_t p3_n);
uint32_t omega_native_mcintent(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, const void *p3, uint64_t p3_n, const void *p4, uint64_t p4_n);
uint32_t omega_native_mcseal(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, uint64_t p2, void *p3, uint64_t p3_n);
uint32_t omega_native_mcreseal(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, uint64_t p2, void *p3, uint64_t p3_n, void *p4, uint64_t p4_n);
uint32_t omega_native_mccode(uint64_t *result, const void *p0, uint64_t p0_n, uint64_t p1, void *p2, uint64_t p2_n);
uint32_t omega_native_mclaunch(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, const void *p3, uint64_t p3_n, void *p4, uint64_t p4_n);
uint32_t omega_native_mcdir(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, void *p2, uint64_t p2_n);
uint32_t omega_native_mcalloc(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, void *p2, uint64_t p2_n);
uint32_t omega_native_mcseries(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, void *p3, uint64_t p3_n);
uint32_t omega_native_qgshape(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3);
uint32_t omega_native_qgreading(uint64_t *result, uint64_t p0, uint64_t p1);
uint32_t omega_native_qgworkspace(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4);
uint32_t omega_native_qgorder(uint64_t *result, void *p0, uint64_t p0_n, uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4);
uint32_t omega_native_attnmirplan(uint64_t *result, const void *p0, uint64_t p0_n);
uint32_t omega_native_attnmirnext(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3);
uint32_t omega_q38_runtime(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, void *p2, uint64_t p2_n);
uint32_t omega_q38_emit(uint64_t *result, void *p0, uint64_t p0_n);
uint32_t omega_ra_run(uint64_t *result, void *p0, uint64_t p0_n);
uint32_t omega_ra_ir(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2);
uint32_t omega_ra_live(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4, uint64_t p5);
uint32_t omega_cg_matmul(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3);
uint32_t omega_cg_par(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4);
uint32_t omega_cg_i32(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_cg_tens(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_cg_loop(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_cg_frag(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_cg_block(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_cg_vec(uint64_t *result, void *p0, uint64_t p0_n);
uint32_t omega_scan_ir(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_scan_plan(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4, void *p5, uint64_t p5_n);
uint32_t omega_cg_tc(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_gemm_any_ir(uint64_t *result, const void *p0, uint64_t p0_n, uint64_t p1, void *p2, uint64_t p2_n, void *p3, uint64_t p3_n);
uint32_t omega_dnn_rows(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_dr_plan(uint64_t *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n);
uint32_t omega_dnn_activations(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_da_plan(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, void *p3, uint64_t p3_n);
uint32_t omega_cg_attn(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_cg_pattn(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_cg_pfhost(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_attn_any(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_aa_plan(uint64_t *result, const void *p0, uint64_t p0_n, uint64_t p1, void *p2, uint64_t p2_n);
uint32_t omega_gemm_scalar(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_conv_ir(uint64_t *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n, void *p2, uint64_t p2_n);
uint32_t omega_conv_plan(uint64_t *result, const void *p0, uint64_t p0_n, uint64_t p1, void *p2, uint64_t p2_n);
uint32_t omega_cg_attn256(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_cg_pattn256(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_cg_router(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_cg_gemm(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_cg_bf16(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_cg_qg(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_omsha(uint64_t *result, void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, uint64_t p2, uint64_t p3, void *p4, uint64_t p4_n, const void *p5, uint64_t p5_n);
uint32_t omega_omcanon(uint64_t *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n, uint64_t p2);
uint32_t omega_omcidcmp(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n);
uint32_t omega_omcidhex(uint64_t *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n);
uint32_t omega_omparse(uint64_t *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n);
uint32_t omega_omcver(uint64_t *result, uint64_t p0);
uint32_t omega_omcheck(uint64_t *result, uint64_t p0, uint64_t p1);
uint32_t omega_omobject(uint64_t *result, void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, uint64_t p3);
uint32_t omega_omgraph(uint64_t *result, void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, uint64_t p2, uint64_t p3);
uint32_t omega_ompay(uint64_t *result, void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, uint64_t p3);
uint32_t omega_omparam(uint64_t *result, void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, uint64_t p2);
uint32_t omega_omeffect(uint64_t *result, void *p0, uint64_t p0_n, void *p1, uint64_t p1_n, uint64_t p2, uint64_t p3);
uint32_t omega_omeval(uint64_t *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n);
uint32_t omega_omvec(uint64_t *result, void *p0, uint64_t p0_n, void *p1, uint64_t p1_n, void *p2, uint64_t p2_n, uint64_t p3, uint64_t p4, uint64_t p5);
uint32_t omega_omvspec(uint64_t *result, void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, uint64_t p2);
uint32_t omega_omvpay(uint64_t *result, void *p0, uint64_t p0_n, uint64_t p1);
uint32_t omega_omfmt(uint64_t *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n);
uint32_t omega_omnum(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, void *p3, uint64_t p3_n);
uint32_t omega_omclip(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2);
uint32_t omega_omhost(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, void *p3, uint64_t p3_n);
uint32_t omega_cubmsg(uint64_t *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n);
uint32_t omega_mg_wire(uint64_t *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n, uint64_t p2);
uint32_t omega_mg_node(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, void *p2, uint64_t p2_n, uint64_t p3);
uint32_t omega_mg_batch(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, void *p2, uint64_t p2_n, uint64_t p3);
uint32_t omega_mg_lauch(uint64_t *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n);
uint32_t omega_mg_copy(uint64_t *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n);
uint32_t omega_mg_xfer(uint64_t *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n);
uint32_t omega_mg_final(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, void *p2, uint64_t p2_n, uint64_t p3);
uint32_t omega_mg_exp(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, void *p2, uint64_t p2_n, uint64_t p3);
uint32_t omega_mg_group(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, void *p2, uint64_t p2_n, void *p3, uint64_t p3_n);
uint32_t omega_mg_addr(uint64_t *result, const void *p0, uint64_t p0_n);
uint32_t omega_mg_finb(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, void *p2, uint64_t p2_n, uint64_t p3);
uint32_t omega_mg_win(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, void *p3, uint64_t p3_n, uint64_t p4);
uint32_t omega_mg_layout(uint64_t *result, const void *p0, uint64_t p0_n, void *p1, uint64_t p1_n);
uint32_t omega_mg_aux(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, void *p2, uint64_t p2_n);
uint32_t omega_resident(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_resident_check(uint64_t *result, const void *p0, uint64_t p0_n, uint64_t p1, uint64_t p2, uint64_t p3);
uint32_t omega_gemm_ex_plan(uint64_t *result, uint64_t p0, const void *p1, uint64_t p1_n, void *p2, uint64_t p2_n);
uint32_t omega_gemm_ex_row(uint64_t *result, uint64_t p0, uint64_t p1, const void *p2, uint64_t p2_n, const void *p3, uint64_t p3_n, void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_gemm_ex_x3(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, const void *p3, uint64_t p3_n, void *p4, uint64_t p4_n);
uint32_t omega_gemm_ex_acc(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, void *p3, uint64_t p3_n);
uint32_t omega_gemm_ex_dot(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n);
uint32_t omega_gemm_ex_epi(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, const void *p3, uint64_t p3_n, const void *p4, uint64_t p4_n, void *p5, uint64_t p5_n);
uint32_t omega_gemm_ex_fp8(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n, const void *p2, uint64_t p2_n, void *p3, uint64_t p3_n);
uint32_t omega_native_revision(uint64_t *result, uint64_t p0);
uint32_t omega_native_epoch(uint64_t *result, uint64_t p0);
uint32_t omega_native_f16(uint64_t *result, uint64_t p0);
uint32_t omega_native_c5(uint64_t *result, uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4, uint64_t p5);
#ifdef __cplusplus
}
#endif
#endif
