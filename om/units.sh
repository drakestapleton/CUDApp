#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Regenerate checked portable Omega C and declarations for all canonical roots.
# Seeded e5b32567 validates exactly the composed bytes lowered by bridge36.
# The default portable mode uses no native compiler or executable-code trampoline.
set -eu
OMC_PIN=2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f

CONVERTER_PIN=d84fd6ebd258170a023ed5235880cd3e1ca8664703a2542542a7f8d103120538
FRONTEND_PIN=a3a455bd77381e132215924295763e8728889d7d578278ae376de78324be8629
FREEST_CONVERTER_PIN=836cbf9ef8a3c4ee82b0458070884405bc8296f83b10f933556bf27750bb3661
FREEST_FRONTEND_PIN=51b41794e7001c5606b715fd7de8a34ec935a6e4e4bc1da36ad08102c009537d
ROOT=$(cd "$(dirname "$0")/.." && pwd)
# Native NO_POSIX roots have separate bridge pins and import-free object custody.
if [ "${1:-}" = --native-no-posix ]; then
    exec sh "$ROOT/om/no-posix.sh"
fi
QUANT=om/quant/quant.om
REAL="$QUANT,om/components/realize.om"
CACHE="$REAL,om/cache/digest.om,om/cache/residency.om,om/cache/artifact.om"
PIPE="$CACHE,om/pipeline/select.om,om/pipeline/graph.om,om/pipeline/consume.om"
COMP="om/components/bw_alloc.om,om/components/bw_effect.om,om/components/bw_check.om,om/components/bw_project.om,om/components/rowpipe.om,om/components/staged.om,om/components/compose.om,om/components/runtime.om"
ENC="om/encoder/bw_enc_sm121.om,om/encoder/bw_spec_sm121.om,om/components/encode.om"
MOE="om/moe/moe_plan.om om/moe/moe_group.om om/moe/moe_ids32.om om/moe/moe_maps32.om om/moe/moe_segs32.om om/moe/moe_windows.om moe_admission=$PIPE,om/moe/moe_group.om,om/moe/moe_admit.om"
CLOSURE="$REAL,om/cache/digest.om,om/moe/moe_windows.om,om/moe/moe_group.om,native/learn53/moe-closure.om"
OBSERVE="closure_code=$CLOSURE,om/moe/closure_code.om closure_launch=$CLOSURE,om/moe/closure_launch.om closure_dir=$CLOSURE,om/moe/closure_dir.om closure_alloc=$CLOSURE,om/moe/closure_alloc.om closure_series=$CLOSURE,om/moe/closure_series.om"
UNITS="${OMEGA_UNITS:-om/encoder/bw_enc_sm121.om om/cubin/cubin_elf.om om/mathlibs/mathlibs_ir.om om/encoder/ampere_enc_sm86.om om/encoder/amp_runtime.om repack=$QUANT,om/quant/repack.om quant_wire=$QUANT,om/quant/runtime.om compose=$COMP encode=$ENC cache_runtime=om/cache/residency.om,om/cache/runtime.om lease=om/cache/residency.om,om/cache/runtime.om,om/cache/lease.om slotrev=om/cache/residency.om,om/cache/slotrev.om command=om/cache/residency.om,om/cache/command.om validate=$CACHE,$COMP,$ENC,om/components/admit.om,om/cache/validate.om digest=$REAL,om/cache/digest.om,om/cache/key.om code_digest=$REAL,om/cache/digest.om,om/cache/codekey.om templates=$REAL,om/cache/digest.om,om/cache/templates.om graph=$PIPE,om/pipeline/runtime.om context_lookup=om/pipeline/context.om,om/pipeline/context_abi.om binding=$REAL,om/pipeline/binding.om $MOE closure_pack=$CLOSURE,om/moe/closure_pack.om closure_bind=$CLOSURE,om/moe/closure_bind.om closure_blob=$CLOSURE,om/moe/closure_blob.om closure_cover=$CLOSURE,om/pipeline/mccover.om,om/pipeline/coverage_entry.om closure_intent=$CLOSURE,om/pipeline/mccover.om closure_seal=$CLOSURE,om/pipeline/mccover.om,om/pipeline/seal_entry.om closure_reseal=$CLOSURE,om/pipeline/mccover.om,om/pipeline/seal_reuse.om $OBSERVE prefill_shape=om/prefill-shape/admit.om prefill_reading=om/prefill-shape/reading.om prefill_workspace=om/prefill-shape/workspace.om prefill_order=om/prefill-shape/order.om attn_mirror_plan=om/attention-mirror/plan.om attn_mirror_step=om/attention-mirror/step.om q38_runtime=om/q38-head/envelope.om,om/q38-head/plan.om q38_emit=om/components/bw_alloc.om,om/components/bw_effect.om,om/components/bw_check.om,om/components/bw_project.om,$ENC,om/q38-head/bounds.om,om/q38-head/emit.om}"
if [ -z "${OMEGA_UNITS:-}" ]; then
  UNITS="$UNITS cg_qmma=om/codegen/cg_qmma.om cg_qmma_patch=om/codegen/cg_qmma_patch.om"
fi
if [ -z "${OMEGA_UNITS:-}" ]; then
  UNITS="$UNITS cg_tgemm_spec=om/codegen/cg_tgemm_spec.om cg_tgemm=om/codegen/cg_qmma.om,om/codegen/cg_tgemm_emit.om,om/codegen/cg_tgemm.om"
  UNITS="$UNITS cg_pf=om/codegen/cg_pf.om"
  UNITS="$UNITS cg_tern=om/codegen/cg_tern_common.om,om/codegen/cg_tern.om cg_tern_sched=om/codegen/cg_tern_sched.om cg_tern_policy=om/codegen/cg_tern_common.om,om/codegen/cg_tern_policy.om"
fi
[ -n "${OMEGA_UNITS:-}" ] || UNITS="$UNITS om/streams/streams.om"
[ -n "${OMEGA_UNITS:-}" ] || UNITS="$UNITS numeric=om/numeric/numeric.om numeric_tile=om/numeric/numeric.om,om/numeric/tile.om"
if [ -z "${OMEGA_UNITS:-}" ]; then
    CM_HOST_MATH=om/components/cm_math.om,om/components/cm_math_extra.om,om/components/cm_math_erfc.om
    UNITS="$UNITS artifact_policy=om/components/artifact_policy.om artifact_parse=om/components/artifact_parse.om api_policy=om/components/api_policy.om cm_sort=om/components/cm_sort.om cm_math=$CM_HOST_MATH cm_fit=$CM_HOST_MATH,om/components/cm_fit.om cm_stats=$CM_HOST_MATH,om/components/cm_stats.om cm_features=$CM_HOST_MATH,om/components/cm_features.om"
    UNITS="$UNITS cm_quantile=$CM_HOST_MATH,om/components/cm_quantile.om cm_decision=$CM_HOST_MATH,om/components/cm_decision.om"
    UNITS="$UNITS cm_prediction=$CM_HOST_MATH,om/components/cm_prediction.om cm_calibration=$CM_HOST_MATH,om/components/cm_quantile.om,om/components/cm_calibration.om"
    UNITS="$UNITS cm_blob=om/components/cm_blob.om cm_digest=om/cache/digest.om,om/components/cm_digest.om"
    UNITS="$UNITS artifact_format=om/components/artifact_format.om artifact_admit=om/components/artifact_parse.om,om/components/artifact_policy.om,om/components/artifact_format.om,om/components/artifact_admit.om"
    UNITS="$UNITS artifact_elf=om/components/artifact_elf.om"
    UNITS="$UNITS api_mm_plan=om/components/api_mm_plan.om api_at_plan=om/components/api_at_plan.om api_ew_plan=om/components/api_ew_plan.om api_bf_plan=om/components/api_bf_plan.om api_da_plan=om/components/api_da_plan.om api_rt_plan=om/components/api_rt_plan.om"
fi
if [ -z "${OMEGA_UNITS:-}" ]; then UNITS="$UNITS cmdlist_record=om/cmdlist/record.om cmdlist_plan=om/cmdlist/plan.om cmdlist_layout=om/cmdlist/layout.om cmdlist_access=om/cmdlist/plan.om,om/cmdlist/access.om"; fi
[ -n "${OMEGA_UNITS:-}" ] || UNITS="$UNITS nvrm_wire_pack=native/nvrm/wire.om,native/nvrm/wire_pack.om nvrm_wire_reply=native/nvrm/wire.om nvrm_heap_wire=native/nvrm/heap_wire.om"
[ -n "${OMEGA_UNITS:-}" ] || UNITS="$UNITS nvrm_uvm_wire=native/nvrm/uvm_fields.om,native/nvrm/uvm_wire.om nvrm_uvm_map_wire=native/nvrm/uvm_fields.om,native/nvrm/uvm_map_wire.om nvrm_struct=native/nvrm/uvm_fields.om,native/nvrm/struct.om"
if [ -z "${OMEGA_UNITS:-}" ]; then
  UNITS="$UNITS plat_policy=om/platform-linux/plat_policy.om"
  UNITS="$UNITS plat_rest=om/platform-linux/plat_rest.om"
fi
NVML="nvml_step=native/nvml/nvml.om,native/nvml/query.om,native/nvml/step.om nvml_errstr=native/nvml/errstr.om"
[ -n "${OMEGA_UNITS:-}" ] || UNITS="$UNITS $NVML"
[ -n "${OMEGA_UNITS:-}" ] || UNITS="$UNITS om/admission/chip_admit.om"
# RegAlloc #309 (seeded allocator, core matmul builders, vecadd writer): appended to the default list only.
[ -n "${OMEGA_UNITS:-}" ] || UNITS="$UNITS om/codegen/regalloc.om om/codegen/regalloc_ir.om om/codegen/regalloc_live.om om/codegen/cg_matmul.om om/codegen/cg_matmul_params.om om/codegen/cg_matmul_i32.om om/codegen/cg_matmul_tensor.om om/codegen/cg_matmul_loop.om om/codegen/cg_matmul_frag.om om/codegen/cg_matmul_block.om om/codegen/cg_vecadd.om om/codegen/bw_hazard.om bw_hazard_dec=om/codegen/bw_hazard.om,om/codegen/bw_hazard_dec.om om/codegen/cg_bind.om"
# C3 scan and sort (CUB/Thrust row): the kernel IR and the launch plans
if [ -z "${OMEGA_UNITS:-}" ]; then UNITS="$UNITS om/scan/scan_ir.om om/scan/scan_plan.om"; fi
[ -z "${OMEGA_UNITS:-}" ] && UNITS="$UNITS aiengpu=native/aiengpu/context.om"
# C6 (CUDApp#10): the sm_121 disassembler; appended only to the default list, so OMEGA_UNITS subsets still work.
[ -z "${OMEGA_UNITS:-}" ] && UNITS="$UNITS om/disasm/sm121_dis.om"
# dag_eval is the dagsim scheduler; the other om/tools units are the decisions
# of the host tools (tools-om-2). Append only when the caller did not
# override the unit list, so a selective OMEGA_UNITS run stays selective.
if [ -z "${OMEGA_UNITS:-}" ]; then
  UNITS="$UNITS cg_moe=om/codegen/cg_moe_emit.om,om/codegen/cg_moe_core.om,om/codegen/cg_moe.om cg_moe2=om/codegen/cg_moe_emit.om,om/codegen/cg_moe_core.om,om/codegen/cg_moe2.om cg_moe_sched=om/codegen/cg_moe_sched.om"
  UNITS="$UNITS cg_gdn=om/codegen/cg_gdn_shape.om,om/codegen/cg_gdn_common.om,om/codegen/cg_gdn.om cg_gdn_policy=om/codegen/cg_gdn_shape.om,om/codegen/cg_gdn_policy.om"
  UNITS="$UNITS cg_dec=om/codegen/cg_dec_emit.om,om/codegen/cg_dec.om gdn_admit=om/gdn/admit.om"
  UNITS="$UNITS om/tools/dag_eval.om om/tools/own30b_prof.om doctor=om/tools/fmt.om,om/tools/doctor.om om/tools/ampcorp.om om/tools/ampfam.om om/tools/artexp.om"
  UNITS="$UNITS bw_qmd=om/qmd/bw_qmd.om eng=om/engine/eng.om"
fi
if [ -z "${OMEGA_UNITS:-}" ]; then
  UNITS="$UNITS cg_tc=om/codegen/cg_tc_emit.om,om/codegen/cg_tc.om"
    UNITS="$UNITS gemm_any=om/gemm-any/gemm_ir.om"
    UNITS="$UNITS dnn_rows=om/attn-any/emit.om,om/dnn-rows/kernel.om dnn_rows_plan=om/dnn-rows/plan.om"
fi
if [ -z "${OMEGA_UNITS:-}" ]; then
    UNITS="$UNITS dnn_activations=om/attn-any/emit.om,om/dnn-activations/kernel.om dnn_activations_plan=om/dnn-activations/plan.om"
fi
if [ -z "${OMEGA_UNITS:-}" ]; then
  UNITS="$UNITS cg_attn=om/codegen/cg_attn_emit.om,om/codegen/cg_attn.om cg_pattn=om/codegen/cg_pattn_emit.om,om/codegen/cg_pattn.om"
fi
if [ -z "${OMEGA_UNITS:-}" ]; then
  UNITS="$UNITS cg_pfhost=om/codegen/cg_pfhost_emit.om,om/codegen/cg_pfhost_rl.om,om/codegen/cg_pfhost_mc.om,om/codegen/cg_pfhost.om"
fi
[ -n "${OMEGA_UNITS:-}" ] || UNITS="$UNITS nvrm_policy=native/nvrm/policy.om"
if [ -z "${OMEGA_UNITS:-}" ]; then UNITS="$UNITS session_admit=om/session/admit.om session_words=om/session/words.om session_park=om/session/park.om cmdlist_book=om/cmdlist/book.om"; fi
if [ -z "${OMEGA_UNITS:-}" ]; then
  UNITS="$UNITS om/codegen/cg_fields.om om/codegen/cg_replay.om om/codegen/cg_life.om om/codegen/cg_invar.om om/codegen/cg_budget.om"
    UNITS="$UNITS cm_route=om/components/cm_route.om"
fi
PURE_UNITS="revision=om/pipeline/revision.om epoch=om/pipeline/epoch.om f16=om/numeric/f16.om"
[ -n "${OMEGA_UNITS:-}" ] || UNITS="$UNITS attn_any=om/attn-any/emit.om,om/attn-any/kernel.om attn_any_plan=om/attn-any/plan.om"
if [ -z "${OMEGA_UNITS:-}" ]; then
    UNITS="$UNITS gemm_scalar=om/gemm-any/gemm_ir.om,om/conv2d/dense.om conv2d=om/gemm-any/gemm_ir.om,om/conv2d/conv.om conv_plan=om/gemm-any/gemm_ir.om,om/conv2d/conv.om,om/conv2d/plan.om"
  UNITS="$UNITS cg_attn256=om/codegen/cg_rest_emit.om,om/codegen/cg_attn256.om cg_pattn256=om/codegen/cg_rest_emit.om,om/codegen/cg_pattn256.om cg_router=om/codegen/cg_rest_emit.om,om/codegen/cg_router.om cg_gemm=om/codegen/cg_rest_emit.om,om/codegen/cg_gemm.om cg_bf16=om/codegen/cg_rest_emit.om,om/codegen/cg_bf16.om cg_qg=om/codegen/cg_rest_emit.om,om/prefill-shape/admit.om,om/prefill-shape/order.om,om/codegen/cg_qg.om"
  UNITS="$UNITS core_sha=om/core/sha256.om"
  CORE=om/core
  for entry in omcanon omcidcmp omcidhex; do
    UNITS="$UNITS core_$entry=$CORE/canonical.om,$CORE/$entry.om"
  done
  for entry in omparse omcver omcheck; do
    UNITS="$UNITS core_$entry=$CORE/idparse.om,$CORE/$entry.om"
  done
  for unit in object graph payload param effect eval vector vecspec vpay fmt fmtnum fmtclip hostmath; do
    UNITS="$UNITS core_$unit=$CORE/$unit.om"
  done
  UNITS="$UNITS core_cubinmsg=$CORE/fmtnum.om,$CORE/cubinmsg.om"
  GRAPH_REC=om/graph/record.om
  GRAPH_NODE=$GRAPH_REC,om/graph/scalar.om
  UNITS="$UNITS model_graph_wire=$GRAPH_REC,om/graph/entry_wire.om model_graph_node=$GRAPH_NODE,om/graph/entry_node.om model_graph_batch=$GRAPH_NODE,om/graph/pf_order.om,om/graph/batch.om,om/graph/entry_batch.om model_graph_launch=$GRAPH_REC,om/graph/entry_launch.om model_graph_copy=om/graph/prefill.om,om/graph/entry_copy.om model_graph_xfer=om/graph/prefill.om,om/graph/entry_xfer.om model_graph=$GRAPH_NODE,om/graph/final.om,om/graph/entry_final.om"
  UNITS="$UNITS model_graph_expert=$GRAPH_NODE,om/graph/pf_order.om,om/graph/batch.om,om/graph/experts.om"
  UNITS="$UNITS model_graph_group=om/graph/group.om"
  UNITS="$UNITS model_graph_address=om/graph/address.om"
  UNITS="$UNITS model_graph_final_batch=$GRAPH_NODE,om/graph/final_batch.om"
  UNITS="$UNITS model_graph_window=$GRAPH_NODE,om/graph/window.om"
  UNITS="$UNITS model_graph_layout=$GRAPH_REC,om/graph/layout.om"
  UNITS="$UNITS model_graph_aux=$GRAPH_NODE,om/graph/aux.om"
  UNITS="$UNITS om/resident/resident.om resident_check=om/resident/resident.om,om/resident/check.om"
  UNITS="$UNITS model_graph_residue=om/graph/residue.om model_graph_table=om/graph/table.om hello_host=om/graph/hello.om hello_plan=om/graph/hello-plan.om"
  UNITS="$UNITS core_cubadmit=$CORE/fmtnum.om,$CORE/cubadmit.om"
  UNITS="$UNITS core_ampack=$CORE/ampack.om"
fi
[ -n "${OMEGA_UNITS:-}" ] || PURE_UNITS="$PURE_UNITS c5=tools/c5/policy.om,tools/c5/runtime.om"
GXNUM=om/gemmex/num.om
GEMMEX="gemm_ex_plan=$GXNUM,om/gemmex/plan.om gemm_ex_row=$GXNUM,om/gemmex/row.om gemm_ex_x3=$GXNUM,om/gemmex/x3.om gemm_ex_acc=$GXNUM,om/gemmex/acc.om gemm_ex_dot=$GXNUM,om/gemmex/dot.om gemm_ex_epi=$GXNUM,om/gemmex/epi.om gemm_ex_fp8=$GXNUM,om/gemmex/fp8.om"
[ -n "${OMEGA_UNITS:-}" ] || UNITS="$UNITS $GEMMEX"
if [ -z "${OMEGA_UNITS:-}" ]; then
UNITS="$UNITS apish_shp=om/api/apish_shp.om apish_pack=om/api/apish_pack.om apish_slot=om/api/apish_slot.om apish_blk=om/api/apish_blk.om apish_trit=om/api/apish_trit.om"
fi
PURE_UNITS="$PURE_UNITS leftovers=om/components/leftovers.om"
AV=om/apiviews
APIVIEWS="av_ewarg=$AV/ewarg.om av_fp8am=$AV/fp8am.om av_fp8en=$AV/fp8en.om av_fp8q=$AV/fp8q.om av_e4m3=$AV/e4m3.om av_fp8sc=$AV/fp8sc.om av_gdnch=$AV/gdnch.om av_gdnsl=$AV/gdnsl.om av_scan=$AV/scan.om"
APIVIEWS_RT="av_bf16=$AV/bf16.om av_widen=$AV/widen.om av_rope=$AV/rope.om av_gdnsc=$AV/gdnsc.om av_gdnex=$AV/gdnex.om av_gdnpt=$AV/gdnpt.om av_gdnut=$AV/gdnut.om av_fp8x=$AV/fp8x.om av_fp8xb=$AV/fp8xb.om"
APIVIEWS="$APIVIEWS ah_rest=$AV/hostrest.om"
APIVIEWS_RT="$APIVIEWS_RT ar_carry=$AV/scancarry.om"
[ -n "${OMEGA_UNITS:-}" ] || UNITS="$UNITS $APIVIEWS $APIVIEWS_RT"
UNITS="$UNITS bw_identity=om/cache/digest.om,om/blackwell/identity.om"
UNITS="$UNITS bw_cpui=om/blackwell/views.om,om/blackwell/shape.om,om/blackwell/cpui.om bw_cpuf16=om/blackwell/views.om,om/blackwell/scalar.om,om/blackwell/shape.om,om/blackwell/cpuf16.om bw_cpubf16=om/blackwell/views.om,om/blackwell/shape.om,om/blackwell/cpubf16.om"
UNITS="$UNITS bw_shape=om/blackwell/views.om,om/blackwell/shape.om,om/blackwell/shape_probe.om bw_name=om/blackwell/name.om"
PURE_UNITS="$PURE_UNITS bw_scalar=om/blackwell/scalar.om"
if [ -z "${OMEGA_UNITS:-}" ]; then
UNITS="$UNITS attn_args=om/api/attn_args.om"
UNITS="$UNITS attn_lay=om/api/attn_lay.om"
UNITS="$UNITS attn_buf=om/api/attn_buf.om"
UNITS="$UNITS attn_math=om/api/attn_math.om"
UNITS="$UNITS attn_tables=om/api/attn_om2.om,om/api/attn_tables.om"
UNITS="$UNITS attn_set=om/api/attn_set.om"
UNITS="$UNITS attn_fill=om/api/attn_fill.om"
UNITS="$UNITS attn_fillf=om/api/attn_fillf.om"
UNITS="$UNITS attn_count=om/api/attn_count.om"
UNITS="$UNITS attn_cntf=om/api/attn_cntf.om"
UNITS="$UNITS attn_bfexp=om/api/attn_bfexp.om"
UNITS="$UNITS attn_bffin=om/api/attn_bffin.om"
UNITS="$UNITS attn_view_any=om/api/attn_any.om"
UNITS="$UNITS attn_comp=om/api/attn_comp.om"
UNITS="$UNITS attn_rpois=om/api/attn_rpois.om"
UNITS="$UNITS attn_z64=om/api/attn_z64.om"
UNITS="$UNITS attn_bids=om/api/attn_bids.om"
UNITS="$UNITS attn_rows=om/api/attn_rows.om"
UNITS="$UNITS attn_gtab=om/api/attn_om2.om,om/api/attn_gtab.om"
UNITS="$UNITS attn_range=om/api/attn_om2.om,om/components/api_at_plan.om,om/api/attn_range.om"
UNITS="$UNITS attn_total=om/api/attn_om2.om,om/api/attn_total.om"
UNITS="$UNITS attn_table=om/api/attn_om2.om,om/api/attn_table.om"
UNITS="$UNITS attn_touch=om/api/attn_om2.om,om/api/attn_touch.om"
UNITS="$UNITS attn_blks=om/api/attn_om2.om,om/components/api_at_plan.om,om/api/attn_blks.om"
UNITS="$UNITS attn_rfit=om/api/attn_om2.om,om/components/api_at_plan.om,om/api/attn_rfit.om"
UNITS="$UNITS attn_gath=om/api/attn_om2.om,om/api/attn_cseg.om,om/api/attn_gath.om"
UNITS="$UNITS attn_stage=om/api/attn_om2.om,om/api/attn_cseg.om,om/api/attn_stage.om"
UNITS="$UNITS attn_reset=om/api/attn_om2.om,om/api/attn_reset.om"
UNITS="$UNITS attn_hmap=om/api/attn_om2.om,om/components/api_at_plan.om,om/api/attn_hmap.om"
UNITS="$UNITS attn_eff=om/api/attn_om2.om,om/components/api_at_plan.om,om/api/attn_eff.om"
UNITS="$UNITS attn_best=om/api/attn_om2.om,om/components/api_at_plan.om,om/api/attn_best.om"
UNITS="$UNITS attn_max=om/api/attn_om2.om,om/api/attn_max.om"
UNITS="$UNITS attn_ctx=om/api/attn_ctx.om"
UNITS="$UNITS attn_ds_round=om/api/attn_om2.om,om/gemmex/num.om,om/api/attn_scalar.om,om/api/ds_round.om"
UNITS="$UNITS attn_ds_slot=om/api/attn_om2.om,om/gemmex/num.om,om/api/attn_scalar.om,om/api/ds_slot.om"
UNITS="$UNITS attn_ds_vecf=om/api/attn_om2.om,om/gemmex/num.om,om/api/attn_scalar.om,om/api/ds_vecf.om"
UNITS="$UNITS attn_ds_vecb=om/api/attn_om2.om,om/gemmex/num.om,om/api/attn_scalar.om,om/api/ds_vecb.om"
UNITS="$UNITS attn_na_read=om/api/attn_art.om,om/api/na_read.om"
UNITS="$UNITS attn_na_cubin=om/api/attn_art.om,om/api/na_cubin.om"
UNITS="$UNITS attn_na_layout=om/api/attn_art.om,om/api/na_layout.om"
UNITS="$UNITS attn_na_ver=om/api/attn_art.om,om/api/na_ver.om"
UNITS="$UNITS attn_na_json=om/api/attn_art.om,om/api/na_json.om"
UNITS="$UNITS attn_mm_dot=om/api/attn_oracle.om,om/api/mm_dot.om"
UNITS="$UNITS attn_mm_err=om/api/attn_oracle.om,om/api/mm_err.om"
UNITS="$UNITS attn_mm_init=om/api/mm_init.om"
UNITS="$UNITS attn_mm_next=om/api/mm_next.om"
fi
REST="ar_bf16=om/apiviews/bf16.om,om/apirest/bf16.om"
for entry in fill count args argsx pad idle route perm widen next key; do
  REST="$REST ar_$entry=om/apirest/data.om,om/apirest/$entry.om"
done
for entry in nan scale fold foldh acc mag; do
  REST="$REST ar_$entry=om/apirest/pack.om,om/api/apish_pack.om,om/apirest/$entry.om"
done
for entry in tiles tg tile groups run; do
  REST="$REST ar_$entry=om/apirest/tiledata.om,om/apirest/$entry.om"
done
for entry in tern trits rev; do
  REST="$REST ar_$entry=om/apirest/terndata.om,om/apirest/$entry.om"
done
for entry in pfcopy pfratio segok segb tspec; do
  REST="$REST ar_$entry=om/apirest/data.om,om/api/apish_shp.om,om/apirest/$entry.om"
done
REST="$REST ar_calc=om/apirest/calc.om ar_launch=om/apirest/launch.om ar_cache=om/apirest/cache.om"
REST="$REST ar_tcspec=om/apirest/data.om,om/api/apish_shp.om,om/apirest/tcspec.om ar_tcperm=om/apiviews/bf16.om,om/apirest/tcperm.om ar_tcinfo=om/apirest/tcinfo.om ar_gtile=om/apirest/tiledata.om,om/apirest/gtile.om"
REST="$REST ar_gspec=om/apirest/data.om,om/api/apish_shp.om,om/apirest/gspec.om"
for entry in gargs gseg ginfo xnext gath32 gath16; do
  REST="$REST ar_$entry=om/apirest/$entry.om"
done
for entry in slot sgate; do
  REST="$REST ar_$entry=om/apirest/data.om,om/api/apish_slot.om,om/apirest/$entry.om"
done
REST="$REST ar_copy=om/apirest/copy.om ar_sstats=om/apirest/sstats.om"
REST="$REST ar_victim=om/apirest/victim.om ar_mstep=om/apirest/data.om,om/api/apish_shp.om,om/apirest/mstep.om"
REST="$REST om/codegen/cg_pack_len.om om/codegen/cg_pack_h.om om/codegen/cg_mrow.om"
[ -n "${OMEGA_UNITS:-}" ] || UNITS="$UNITS $REST"
DEST="${OMEGA_UNITS_DEST:-$ROOT/src}"
[ -n "${OMEGA_UNITS:-}" ] || UNITS="$UNITS om/codegen/cg_em.om om/codegen/cg_reconv.om om/codegen/cg_sentin.om"
OUTS="cudapp_native.h cudapp_om_units.ident"
[ -n "${OMEGA_OMC:-}" ] && [ -n "${OMEGA_BRIDGE:-}" ] || { echo "NOT_RUN: set OMEGA_OMC and OMEGA_BRIDGE"; exit 3; }
[ "$(sha256sum < "$OMEGA_OMC" | cut -c1-64)" = "$OMC_PIN" ] || { echo "NOT_RUN: omc is not the pinned compiler"; exit 3; }
[ "$(sha256sum < "$OMEGA_BRIDGE/src/toc.om" | cut -c1-64)" = "$CONVERTER_PIN" ] || { echo "NOT_RUN: toc.om is not the pinned converter"; exit 3; }
[ "$(sha256sum < "$OMEGA_BRIDGE/tools/bridge-seeded" | cut -c1-64)" = "$FRONTEND_PIN" ] || { echo "NOT_RUN: bridge-seeded is not the pinned frontend"; exit 3; }
[ "$(sha256sum < "$OMEGA_BRIDGE/src/freest.om" | cut -c1-64)" = "$FREEST_CONVERTER_PIN" ] || { echo "NOT_RUN: freest.om is not the pinned converter"; exit 3; }
[ "$(sha256sum < "$OMEGA_BRIDGE/tools/bridge-freest" | cut -c1-64)" = "$FREEST_FRONTEND_PIN" ] || { echo "NOT_RUN: bridge-freest is not the pinned frontend"; exit 3; }
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT

cat > "$T/cudapp_native.h" <<'HEADER'
/* Generated checked portable Omega declarations. Regenerate with om/units.sh.
 * Status: 0 done, 1..14 trap, 32 argument refused, 33 reentry.
 * Only a successful call writes result. Fixed views use the declared length.
 * Runtime-extent units pass the live count under the bridge runtime-views profile.
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
HEADER
: > "$T/cudapp_om_units.ident"
for u in $UNITS $PURE_UNITS; do
  case "$u" in
    *=*) name=${u%%=*}; parts=${u#*=} ;;
    *) name=$(basename "$u" .om | tr -c 'A-Za-z0-9_\n' '_'); parts=$u ;;
  esac
  src="$T/$name.om"
  : > "$src"
  old_ifs=$IFS; IFS=,
  for part in $parts; do
    case "$part" in /*) input=$part ;; *) input=$ROOT/$part ;; esac
    cat "$input" >> "$src"
    printf '\n' >> "$src"
  done
  IFS=$old_ifs
  frontend=bridge-seeded
  abi_name=bridge-seeded
  converter_pin=$CONVERTER_PIN
  frontend_pin=$FRONTEND_PIN
  bridge=$OMEGA_BRIDGE
  flags=
  file=cudapp_om_$name.c
  profile=
  case "$name" in
    revision|epoch|f16|c5|leftovers|bw_scalar)
      frontend=bridge-freest
      abi_name=bridge-freest
      converter_pin=$FREEST_CONVERTER_PIN
      frontend_pin=$FREEST_FRONTEND_PIN
      file=cudapp_freest_$name.c ;;
    av_bf16|av_widen|av_rope|av_gdnsc|av_gdnex|av_gdnpt|av_gdnut|av_fp8x|av_fp8xb|ar_carry|bw_cpui|bw_cpuf16|bw_cpubf16|bw_shape|bw_name|model_graph_table)
      flags=--runtime-views
      abi_name=bridge-seeded-runtime ;;
    attn_buf|attn_math|attn_tables|attn_set|attn_fill|attn_fillf|attn_count|attn_cntf|attn_bfexp|attn_bffin|attn_view_any|attn_comp|attn_rpois|attn_z64|attn_bids|attn_rows|attn_gtab|attn_range|attn_total|attn_table|attn_touch|attn_blks|attn_rfit|attn_gath|attn_stage|attn_reset|attn_hmap|attn_eff|attn_best|attn_max|attn_ctx|attn_ds_*|attn_na_*|attn_mm_*)
      flags=--runtime-views
      abi_name=bridge-seeded-runtime ;;
    ar_*|cg_pack_h|cg_mrow|cg_sentin|nvrm_struct|nvrm_uvm_wire|nvrm_uvm_map_wire)
      flags=--runtime-views
      abi_name=bridge-seeded-runtime ;;
  esac
  printf 'generating portable unit %s (%s%s)\n' "$name" "$frontend" "$flags" >&2
  OMEGA_SEEDED_OMC="$OMEGA_OMC" sh "$bridge/tools/$frontend" $flags c "$src" > "$T/$file"
  OMEGA_SEEDED_OMC="$OMEGA_OMC" sh "$bridge/tools/$frontend" $flags h "$src" >> "$T/cudapp_native.h"
  identity="omega-ident 2 unit=$name abi=$abi_name src=$(sha256sum < "$src" | cut -c1-64) c=$(sha256sum < "$T/$file" | cut -c1-64) omc=$OMC_PIN converter=$converter_pin frontend=$frontend_pin path=$u"
  printf '%s\n' "$identity" > "$T/cudapp_om_$name.ident"
  printf '%s\n' "$identity" >> "$T/cudapp_om_units.ident"
  OUTS="$OUTS $file cudapp_om_$name.ident"
done
cat >> "$T/cudapp_native.h" <<'HEADER'
#ifdef __cplusplus
}
#endif
#endif
HEADER
if [ "${1:-}" = --check ]; then
  bad=0
  for f in $OUTS; do cmp -s "$T/$f" "$DEST/$f" || { echo "FAIL $DEST/$f differs from pinned tools"; bad=1; }; done
  [ "$bad" = 0 ] || exit 1
  echo "PASS portable Omega units regenerate byte for byte"; exit 0
fi
for f in $OUTS; do cp "$T/$f" "$DEST/$f"; done
echo "wrote checked portable Omega units to $DEST"
