/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* omega_gpu_scan.h -- reductions, prefix sums and radix sort of any length on the GB10 (row C3 of docs/OMEGA.md).
 *
 * The sum, the maximum and the minimum of n float values, the inclusive prefix sum (the running total
 * y[i] = x[0] + ... + x[i]) of n float or n u32 values, and the stable sort of n u32 keys (alone or with u32 values),
 * for every n from 1 to 2^28. The order of the additions is frozen: it depends only on n and the element index, never
 * on the grid, the scheduling or the chip, so the chip result equals the host reference below bit for bit.
 *
 * Reductions (the order of docs/numeric/E1_REDUCTION_CONTRACT.md, omega_numeric_reduce.h): one level turns len values
 * into ceil(len / 32) values; value j is the 32-lane tree over values [32j, 32j + 32) with lane i combining lane i + d
 * for d = 16, 8, 4, 2, 1 (lower lane on the left), positions past len filled with the identity (-0.0 for the sum, a
 * quiet NaN for the maximum and minimum, which those operations ignore). Levels repeat until one value is left; at
 * least one level runs.
 *
 * Prefix sum, same tile structure:
 *   level scan  inside each tile of 32 (positions past len filled with -0.0, or 0 for u32), for d = 1, 2, 4, 8, 16 in
 *               that order, every position i >= d takes v[i - d] + v[i] (both from before the step); the tile total
 *               T[j] is the value at position 31.
 *   carry       when there is more than one tile, the totals T are prefix-summed by the same definition (recursively)
 *               into S, and every element of tile j >= 1 becomes S[j - 1] + (its tile value).
 * f32 inputs are any float values. Results with no NaN are bit-exact; a NaN result is a NaN (the chip writes one
 * canonical NaN, the host may keep another payload). u32 additions wrap modulo 2^32 and are exact.
 *
 * Radix sort: least significant digit first, 4-bit digits, 8 passes between the input buffer and a scratch copy,
 * ending in the input buffer. One pass is a digit histogram per 32-key tile (counts digit-major, cnt[0] = 0 and
 * cnt[1 + digit * tiles + tile]), the u32 prefix sum above over those 16 * tiles counts in place, and a scatter that
 * writes key i to cnt[digit * tiles + tile] + (the earlier keys of its tile with the same digit): stable, so equal
 * keys keep their input order and the result is unique. Key-value pairs move the value with its key.
 *
 * The kernels and the launch plans are seeded Omega: om/scan/scan_ir.om emits the BlackwellIR records of each kernel
 * (replayed and register-allocated by omega_gpu_om_ir_replay, encoded by the .om sm_121 encoder) and
 * om/scan/scan_plan.om writes the launch plan of each call; both reach C as the checked generated units
 * src/cudapp_om_scan_ir.c and src/cudapp_om_scan_plan.c. No CUDA. The reduction tree and the tile scan are warp
 * shuffles (SHFL.DOWN; the scan holds each tile in reversed lane order so a shuffle down reads the lower element),
 * the carry is a separate pass. omega_gpu_scan_drive runs a plan through a launch callback, so the same plan runs on
 * the chip (omega_gpu_reduce, omega_gpu_prefix_sum, omega_gpu_prefix_sum_u32, omega_gpu_radix_sort_*) and in the
 * warp simulator (tests/gpu_scan_test.c).
 *
 * MIGRATION-PENDING C: the earlier C generator and level drivers of kinds 0..4 and ops 0..3 are kept in
 * omega_gpu_scan.c as the differential oracle, compiled only with -DCUDAPP_MIGRATION_PENDING_C_SCAN (C primary for
 * those, Omega for the rest) or -DCUDAPP_MIGRATION_PENDING_SCAN_DIFF (both run; any difference in the IR, the
 * register allocation, the plan or the scratch size aborts). The default build has no C kernel generator. */
#ifndef OMEGA_GPU_SCAN_H
#define OMEGA_GPU_SCAN_H

#include <stddef.h>
#include <stdint.h>
#include "omega_blackwell_codegen.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OMEGA_GPU_SCAN_MAX_N ((size_t)1 << 28)
#define OMEGA_GPU_SCAN_THREADS 256u /* 8 warps per CTA; each warp walks tiles gw, gw + warps, ... */
#define OMEGA_GPU_SCAN_MAX_CTAS 1024u

enum {
    OMEGA_GPU_SCAN_OK = 0,
    OMEGA_GPU_SCAN_BAD_ARGS = -1,     /* NULL pointer, n == 0, unknown op */
    OMEGA_GPU_SCAN_TOO_LARGE = -2,    /* n > OMEGA_GPU_SCAN_MAX_N */
    OMEGA_GPU_SCAN_CODEGEN_FAIL = -3, /* our codegen refused a kernel */
    OMEGA_GPU_SCAN_CHIP_FAIL = -4     /* device, allocation, launch, or an output the chip never wrote or got wrong */
};

typedef enum { OMEGA_GPU_RED_SUM = 0, OMEGA_GPU_RED_MAX = 1, OMEGA_GPU_RED_MIN = 2 } OmegaGpuRedOp;

/* The kernels. Argument words at constant bank 0x380 (OMEGA_GPU_SCAN_A_*). */
typedef enum {
    OMEGA_SCAN_K_RED_SUM = 0, OMEGA_SCAN_K_RED_MAX, OMEGA_SCAN_K_RED_MIN, /* one reduction level: in -> out[tile] */
    OMEGA_SCAN_K_TILE,          /* one f32 scan level: in -> out (may alias), tot[tile] */
    OMEGA_SCAN_K_ADD,           /* f32 carry: in[e] = out[tile - 1] + in[e], tiles >= 1 */
    OMEGA_SCAN_K_TILE_U32,      /* the same two over u32 */
    OMEGA_SCAN_K_ADD_U32,
    OMEGA_SCAN_K_SORT_HIST,     /* digit counts of in into tot (cnt[0] = 0, cnt[1 + digit * tiles + tile]) */
    OMEGA_SCAN_K_SORT_SCATTER,  /* keys in -> out at the scanned counts tot */
    OMEGA_SCAN_K_SORT_SCATTER_KV, /* the same for [keys len | values len] */
    OMEGA_SCAN_K_COUNT
} OmegaScanKind;
#define OMEGA_GPU_SCAN_A_IN 0x380u     /* u64: level input (K_ADD*: the level being carried into) */
#define OMEGA_GPU_SCAN_A_OUT 0x388u    /* u64: level output (K_ADD*: S, the scanned totals) */
#define OMEGA_GPU_SCAN_A_TOT 0x390u    /* u64: tile totals (K_TILE*), the digit counts (K_SORT_*) */
#define OMEGA_GPU_SCAN_A_LEN 0x398u    /* u32: values in this level */
#define OMEGA_GPU_SCAN_A_TILES 0x39cu  /* u32: ceil(len / 32) */
#define OMEGA_GPU_SCAN_A_WARPS 0x3a0u  /* u32: warps in the grid (the tile stride) */
#define OMEGA_GPU_SCAN_A_SHIFT 0x3a4u  /* u32: K_SORT_*: 28 - 4 * pass, the digit is (key << shift) >> 28 */
#define OMEGA_GPU_SCAN_ARGS_WORDS 10u

/* Mutants for the red checks (each must change some result): 1 the reduction tree skips d = 1, 2 the tile scan skips
 * d = 16, 3 the carry adds S[tile] instead of S[tile - 1], 4 the scatter rank counts the key itself, 5 the scatter
 * ranks keys in lane order instead of position order (unstable: only equal keys with values show it), 6 the histogram
 * scan skips d = 16. 0 = the real kernels. */
typedef struct { OmegaScanKind kind; int mutant; } OmegaScanSpec;

/* Builds the IR program of one kernel (om/scan/scan_ir.om) and allocates its registers; 0 or -1. */
int omega_gpu_scan_codegen(const OmegaScanSpec *spec, BlackwellIRProgram *prog);

/* Host references, the frozen order above. 0, or -1 for bad arguments or out of memory. */
int omega_gpu_reduce_reference(OmegaGpuRedOp op, const float *x, size_t n, float *out);
int omega_gpu_prefix_sum_reference(const float *x, size_t n, float *y);
int omega_gpu_prefix_sum_u32_reference(const uint32_t *x, size_t n, uint32_t *y);
/* Stable sort of keys[0..n) in place; vals (may be NULL) moves with its key. */
int omega_gpu_radix_sort_reference(uint32_t *keys, uint32_t *vals, size_t n);

/* The calls a plan serves. */
typedef enum {
    OMEGA_SCAN_OP_RED_SUM = 0, OMEGA_SCAN_OP_RED_MAX, OMEGA_SCAN_OP_RED_MIN, /* n f32 at in; result in scratch */
    OMEGA_SCAN_OP_PREFIX_F32,   /* inclusive prefix sum of n f32, in -> out (may alias) */
    OMEGA_SCAN_OP_PREFIX_U32,   /* the same over u32 */
    OMEGA_SCAN_OP_SORT_KEYS,    /* n u32 keys at in, sorted in place (out unused) */
    OMEGA_SCAN_OP_SORT_PAIRS,   /* [keys n | values n] at in, sorted by key in place (out unused) */
    OMEGA_SCAN_OP_COUNT
} OmegaScanOp;

/* A launch plan (om/scan/scan_plan.om): plan[0] launches, plan[1] scratch words (u32) used from scratch_va,
 * plan[2] the reduction result VA; launch i at plan[16 + 12 i]: kind, grid_x, the OMEGA_GPU_SCAN_ARGS_WORDS argument
 * words. Device addresses are plain numbers (host pointers in the simulator). */
#define OMEGA_GPU_SCAN_PLAN_WORDS 2048u
#define OMEGA_GPU_SCAN_PLAN_LAUNCH(i) (16u + 12u * (unsigned)(i))
int omega_gpu_scan_plan(OmegaScanOp op, size_t n, uint64_t in_va, uint64_t out_va, uint64_t scratch_va,
                        uint64_t plan[OMEGA_GPU_SCAN_PLAN_WORDS]);
/* Scratch words a call needs beyond its input and output; 0 for bad arguments. */
size_t omega_gpu_scan_scratch_words(OmegaScanOp op, size_t n);

/* The driver: plans the call and runs its launches in order. launch runs one kernel over grid_x CTAs of
 * OMEGA_GPU_SCAN_THREADS threads with the given argument words and returns 0 when it completed. A reduction's result
 * is the scratch word returned through *result_va (may be NULL for the other ops). */
typedef int (*OmegaScanLaunchFn)(void *ctx, OmegaScanKind kind, const uint32_t args[OMEGA_GPU_SCAN_ARGS_WORDS], uint32_t grid_x);
int omega_gpu_scan_drive(OmegaScanOp op, size_t n, uint64_t in_va, uint64_t out_va, uint64_t scratch_va,
                         uint64_t *result_va, OmegaScanLaunchFn launch, void *ctx, uint32_t *launches);

typedef struct {
    uint32_t launches;      /* kernels run */
    uint64_t elapsed_ns;    /* sum of launch-to-completion over the launches */
    uint64_t call_ns;       /* the whole call, uploads and readback included */
} OmegaGpuScanInfo;

/* On the chip. x is read, out / y written; nothing else is kept between calls except the kernels. The u32 results
 * have no spare value to poison, so their readback is checked instead: y[i] - y[i - 1] == x[i] for the prefix sum,
 * nondecreasing keys for the sort (CHIP_FAIL otherwise). */
int omega_gpu_reduce(OmegaGpuRedOp op, const float *x, size_t n, float *out, OmegaGpuScanInfo *info);
int omega_gpu_prefix_sum(const float *x, size_t n, float *y, OmegaGpuScanInfo *info);
int omega_gpu_prefix_sum_u32(const uint32_t *x, size_t n, uint32_t *y, OmegaGpuScanInfo *info);
int omega_gpu_radix_sort_u32(uint32_t *keys, size_t n, OmegaGpuScanInfo *info);
int omega_gpu_radix_sort_pairs_u32(uint32_t *keys, uint32_t *vals, size_t n, OmegaGpuScanInfo *info);
/* Red checks: every later call builds its kernels with this mutant (clears the kernel cache). */
void omega_gpu_scan_test_set_mutant(int mutant);
const char *omega_gpu_scan_rc_name(int rc);

#ifdef __cplusplus
}
#endif
#endif /* OMEGA_GPU_SCAN_H */
