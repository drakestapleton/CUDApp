#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Existing portable roots, explicitly composed as OM2 for caller-stack native ABI.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
MODE=${1:---install}
OUT=${2:-$ROOT/build/no_posix}
mkdir -p "$OUT/sources" "$OUT/native"
for pair in encoder/bw_enc_sm121 cubin/cubin_elf numeric/no_bf16 session/admit session/words session/park cmdlist/book streams/streams apiviews/no_avbf; do
  name=${pair##*/}
  { printf 'om2\n'; cat "$ROOT/om/$pair.om"; } > "$OUT/sources/$name.om"
done
# Core encoder/codegen roots; the C implementations are explicit oracles in both profiles.
for name in regalloc regalloc_ir regalloc_live cg_matmul cg_matmul_params cg_matmul_i32 cg_matmul_tensor cg_matmul_loop cg_matmul_frag cg_matmul_block cg_vecadd cg_bind cg_pack_len; do
  { printf 'om2\n'; cat "$ROOT/om/codegen/$name.om"; } > "$OUT/sources/$name.om"
done
for name in cg_pack_h cg_sentin; do
  cp "$ROOT/om/codegen/$name.om" "$OUT/sources/$name.om"
done
{ printf 'om2\n'; cat "$ROOT/om/cache/digest.om" "$ROOT/om/blackwell/identity.om" "$ROOT/om/blackwell/id_native.om"; } > "$OUT/sources/bw_identity.om"
set -- "$OUT/sources/regalloc.om" "$OUT/sources/regalloc_ir.om" "$OUT/sources/regalloc_live.om" \
  "$OUT/sources/cg_matmul.om" "$OUT/sources/cg_matmul_params.om" "$OUT/sources/cg_matmul_i32.om" \
  "$OUT/sources/cg_matmul_tensor.om" "$OUT/sources/cg_matmul_loop.om" \
  "$OUT/sources/cg_matmul_frag.om" "$OUT/sources/cg_matmul_block.om" "$OUT/sources/cg_vecadd.om" \
  "$OUT/sources/cg_bind.om" "$OUT/sources/cg_pack_len.om" "$OUT/sources/cg_pack_h.om" "$OUT/sources/cg_sentin.om" \
  "$OUT/sources/bw_identity.om"
# ham/cubin-qmd-om: additive QMD roots; cg_bind and bw_identity come from the core block above.
{ printf 'om2\n'; cat "$ROOT/om/qmd/bw_qmd.om"; } > "$OUT/sources/bw_qmd.om"
{ printf 'om2\n'; cat "$ROOT/om/encoder/amp_runtime.om" "$ROOT/om/encoder/amp_qmd.om"; } > "$OUT/sources/amp_qmd.om"
if [ "$MODE" = --install ]; then
  case "$(uname -m)" in
    aarch64) target=aarch64linux ;;
    x86_64) target=x86linux ;;
    *) echo 'unsupported NO_POSIX target' >&2; exit 2 ;;
  esac
  cp "$ROOT/om/no-posix/gen/$target/"* "$OUT/native/"
  exit 0
fi
case "$MODE" in --generate|--check|--tools) ;; *) exit 2 ;; esac
: "${OMEGA_BRIDGE:?set OMEGA_BRIDGE to the locked converter}"
: "${OMEGA_OMC:?set OMEGA_OMC to the pinned seeded compiler}"
[ "$(sha256sum < "$OMEGA_OMC" | cut -c1-64)" = 2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f ]
[ "$(sha256sum < "$OMEGA_BRIDGE/tools/native" | cut -c1-64)" = 14b2b51ed062ebaa7ea99e83752cc075452465b9ddba4939eb171692358ecb6b ]
[ "$(sha256sum < "$OMEGA_BRIDGE/src/native.om" | cut -c1-64)" = e2077722aaeffb1121b70160e22357ea47f369e0039ab1ea06dc700f6f4e45d7 ]
# Verification oracles are immutable compiler blobs from their published revisions.
# An ordinary portable build consumes only the checked generated sources.
repo=$(CDPATH= cd -- "$(dirname "$OMEGA_OMC")/../.." && pwd)
mkdir -p "$OUT/tools"
git -C "$repo" show e17039d9ceeb30e0637ef4145e37afc5821c3773:seed/out/omc > "$OUT/tools/omc-gen"
git -C "$repo" show 474197e4fa0207e0e2fb3c0494594a975f5031fa:seed/out/omc > "$OUT/tools/omc-x86"
chmod +x "$OUT/tools/omc-gen" "$OUT/tools/omc-x86"
export OMEGA_NATIVE_OMC="$OUT/tools/omc-gen"
export OMEGA_NATIVE_X86_OMC="$OUT/tools/omc-x86"
[ "$MODE" != --tools ] || exit 0
for target in aarch64linux x86linux; do
  case "$target" in aarch64linux) triple=aarch64-linux-gnu ;; x86linux) triple=x86_64-linux-gnu ;; esac
  if command -v "$triple-gcc" >/dev/null 2>&1; then native_cc="$triple-gcc"
  elif command -v clang >/dev/null 2>&1; then native_cc="clang --target=$triple -ffreestanding"
  else echo "NO_POSIX regeneration requires $triple-gcc or clang" >&2; exit 2; fi
  # ham/cubin-qmd-om: native argv roots; install-time sources are the earlier block.
  CC="$native_cc" OMEGA_NATIVE_OMC2="$OMEGA_OMC" sh "$OMEGA_BRIDGE/tools/native" --target "$target" --freestanding "$OUT/$target" \
    "$OUT/sources/bw_enc_sm121.om" "$OUT/sources/cubin_elf.om" "$OUT/sources/no_bf16.om" \
    "$OUT/sources/bw_qmd.om" "$OUT/sources/amp_qmd.om" \
    "$OUT/sources/admit.om" "$OUT/sources/words.om" "$OUT/sources/park.om" "$OUT/sources/book.om" "$OUT/sources/streams.om" "$OUT/sources/no_avbf.om" "$@"
  dest="$ROOT/om/no-posix/gen/$target"
  if [ "$MODE" = --generate ]; then mkdir -p "$dest"; fi
  for file in omega_native.c omega_native.h omega_api_decl.h omega_api_defs.c; do
    if [ "$MODE" = --generate ]; then cp "$OUT/$target/$file" "$dest/$file"
    else cmp "$OUT/$target/$file" "$dest/$file"; fi
  done
done
echo 'PASS NO_POSIX checked native sources regenerate byte for byte (both ISAs)'
