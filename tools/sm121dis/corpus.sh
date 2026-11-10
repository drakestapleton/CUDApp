#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# corpus.sh <build-out-dir> <kernel-dir>: collects every kernel the library builds, without a GPU, into <kernel-dir>.
# Each GPU test runs in its host-only (and simulator) modes with OMEGA_KERNEL_DUMP_DIR set, so every program the
# library encodes lands as <sha16>.bin (src/omega_blackwell_codegen.c omega_kernel_dump). In CUDA++ the corpus is the
# GPU library's kernels; the numeric kernels (DIV, SQRT, load/store, reductions, own encoder) and gpu_tern_test stay
# in omega-mv with their producers. Run from the repo root after
# `make OUT_DIR=<build-out-dir> sm121dis-corpus-bins` (mk/sm121dis.mk). Any failed producer invalidates the corpus.
set -u
OUT=$1; K=$2
mkdir -p "$K"; K=$(cd "$K" && pwd)
export OMEGA_KERNEL_DUMP_DIR="$K"
# Manifest (CUDA++ C6, DRA-475): each kernel is credited to the first producer that wrote it (a later producer that
# builds the same bytes finds the file already there); PRODUCERS.tsv counts the new kernels of each run.
: > "$K/.seen"; : > "$K/.firstby"; printf 'producer\tnew_kernels\tkernels_so_far\n' > "$K/PRODUCERS.tsv"
run() {
    printf '%s' "$*: "
    "$@" > "$K/producer.log" 2>&1; rc=$?
    echo "exit $rc, $(ls "$K"/*.bin 2>/dev/null | wc -l) kernels so far"
    if [ "$rc" -ne 0 ]; then
        cat "$K/producer.log" >&2
        echo "VERDICT FAIL: corpus producer failed: $*" >&2
        exit "$rc"
    fi
    # src/omega_blackwell_codegen.c omega_kernel_dump: a kernel that could not be written whole is missing from the corpus
    if grep -q '^omega_kernel_dump: ' "$K/producer.log"; then
        grep '^omega_kernel_dump: ' "$K/producer.log" >&2
        echo "VERDICT FAIL: kernels not captured whole: $*" >&2
        exit 1
    fi
    rm -f "$K/producer.log"
    p=$(basename "$1"); shift; [ $# -gt 0 ] && p="$p $*"
    (cd "$K" && ls -1 | grep '\.bin$' | LC_ALL=C sort) > "$K/.now"
    LC_ALL=C comm -13 "$K/.seen" "$K/.now" > "$K/.new"
    while read -r f; do printf '%s\t%s\n' "$f" "$p" >> "$K/.firstby"; done < "$K/.new"
    printf '%s\t%s\t%s\n' "$p" "$(wc -l < "$K/.new")" "$(wc -l < "$K/.now")" >> "$K/PRODUCERS.tsv"
    mv "$K/.now" "$K/.seen"
}
for t in gpu_matmul_api_test gpu_elementwise_test gpu_reconv_test gpu_attention_test gpu_moe_window_test gpu_fp8_api_test \
         gpu_decode_attn_test gpu_router_test gpu_bf16_gemv_test gpu_fp8gemm_test gpu_tc_probe_test \
         gpu_tc_peak_test gpu_pf_mem_test gpu_qmma_probe_test gpu_cmdlist_test gpu_decode_graph_test; do
    run "$OUT/$t" --host-only
done
run "$OUT/gpu_attention_test" --hd 128 --host-only
for t in gpu_tcgemm_test gpu_prefill_attn_test gpu_pfgemm_test gpu_qgemm_test gpu_pfhost_test gpu_cmdlist_test; do
    run "$OUT/$t" --host-only; run "$OUT/$t" --sim
done
run "$OUT/gpu_gpr_boundary_test"
for t in gpu_scan_test gpu_gemm_test gpu_streams_test gpu_mathlibs_test; do
    run "$OUT/$t" --host-only; run "$OUT/$t" --sim
done
run "$OUT/gpu_isolation_test" --host-only
# the hazard checker's sweep (every generator over its accepted shapes, patched as the launchers do): bw_hazard_test
# --disasm reads each of these kernels through our disassembler, so each is compared with nvdisasm here
run "$OUT/bw_hazard_test"
# MANIFEST.tsv: one row per distinct kernel file, its full SHA-256 (the file name is its first 8 bytes), size, words and
# first producer. Its own SHA-256 is printed, so a receipt can name the exact corpus.
printf 'kernel\tsha256\tbytes\twords\tfirst_producer\n' > "$K/MANIFEST.tsv"
bad=0
for f in "$K"/*.bin; do
    b=$(basename "$f")
    h=$(sha256sum < "$f" | cut -c1-64)
    [ "$(printf '%s' "$h" | cut -c1-16).bin" = "$b" ] || { echo "VERDICT FAIL: $b does not carry its content hash ($h)" >&2; bad=1; }
    s=$(wc -c < "$f")
    pr=$(grep -F "$b	" "$K/.firstby" | head -1 | cut -f2-)
    [ -n "$pr" ] || { echo "VERDICT FAIL: $b has no producer" >&2; bad=1; }
    printf '%s\t%s\t%s\t%s\t%s\n' "$b" "$h" "$s" $((s / 16)) "$pr" >> "$K/MANIFEST.tsv"
done
rm -f "$K/.seen" "$K/.new" "$K/.firstby"
[ "$bad" = 0 ] || exit 1
echo "manifest: $(($(wc -l < "$K/MANIFEST.tsv") - 1)) kernels, $(awk -F'\t' 'NR>1{w+=$4} END{print w+0}' "$K/MANIFEST.tsv") words, sha256 $(sha256sum < "$K/MANIFEST.tsv" | cut -c1-64)"
echo "corpus: $(ls "$K"/*.bin | wc -l) kernels in $K"
