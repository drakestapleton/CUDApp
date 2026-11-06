#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Regenerate checked portable Omega C and declarations for all canonical roots.
# Seeded e5b32567 validates exactly the composed bytes lowered by bridge36.
# No native x86 compiler or executable-code trampoline participates.
set -eu
OMC_PIN=2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f
CONVERTER_PIN=d84fd6ebd258170a023ed5235880cd3e1ca8664703a2542542a7f8d103120538
FRONTEND_PIN=a3a455bd77381e132215924295763e8728889d7d578278ae376de78324be8629
FREEST_CONVERTER_PIN=836cbf9ef8a3c4ee82b0458070884405bc8296f83b10f933556bf27750bb3661
FREEST_FRONTEND_PIN=51b41794e7001c5606b715fd7de8a34ec935a6e4e4bc1da36ad08102c009537d
ROOT=$(cd "$(dirname "$0")/.." && pwd)
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
PURE_UNITS="revision=om/pipeline/revision.om epoch=om/pipeline/epoch.om f16=om/numeric/f16.om"
GXNUM=om/gemmex/num.om
GEMMEX="gemm_ex_plan=$GXNUM,om/gemmex/plan.om gemm_ex_row=$GXNUM,om/gemmex/row.om gemm_ex_x3=$GXNUM,om/gemmex/x3.om gemm_ex_acc=$GXNUM,om/gemmex/acc.om gemm_ex_dot=$GXNUM,om/gemmex/dot.om gemm_ex_epi=$GXNUM,om/gemmex/epi.om gemm_ex_fp8=$GXNUM,om/gemmex/fp8.om"
[ -n "${OMEGA_UNITS:-}" ] || UNITS="$UNITS $GEMMEX"
if [ -z "${OMEGA_UNITS:-}" ]; then
UNITS="$UNITS apish_shp=om/api/apish_shp.om apish_pack=om/api/apish_pack.om apish_slot=om/api/apish_slot.om apish_blk=om/api/apish_blk.om apish_trit=om/api/apish_trit.om"
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
[ -n "${OMEGA_UNITS:-}" ] || UNITS="$UNITS $REST"
DEST="${OMEGA_UNITS_DEST:-$ROOT/src}"
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
  converter_pin=$CONVERTER_PIN
  frontend_pin=$FRONTEND_PIN
  file=cudapp_om_$name.c
  case "$name" in
    revision|epoch|f16)
      frontend=bridge-freest
      converter_pin=$FREEST_CONVERTER_PIN
      frontend_pin=$FREEST_FRONTEND_PIN
      file=cudapp_freest_$name.c ;;
  esac
  opts=
  case "$name" in ar_*) opts=--runtime-views ;; esac
  printf 'generating portable unit %s (%s)\n' "$name" "$frontend" >&2
  OMEGA_SEEDED_OMC="$OMEGA_OMC" sh "$OMEGA_BRIDGE/tools/$frontend" $opts c "$src" > "$T/$file"
  OMEGA_SEEDED_OMC="$OMEGA_OMC" sh "$OMEGA_BRIDGE/tools/$frontend" $opts h "$src" >> "$T/cudapp_native.h"
  identity="omega-ident 2 unit=$name abi=$frontend src=$(sha256sum < "$src" | cut -c1-64) c=$(sha256sum < "$T/$file" | cut -c1-64) omc=$OMC_PIN converter=$converter_pin frontend=$frontend_pin path=$u"
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
