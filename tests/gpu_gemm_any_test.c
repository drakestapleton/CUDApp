/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Reuse the independent tensor arithmetic oracle and simulator, not the kernel builder. */
#define main gemm_existing_test_main
#include "gpu_gemm_test.c"
#undef main

typedef struct { uint32_t m, n, k; } AnyShape;
static AnyShape shapes[300];
static unsigned shape_count;
static void add_shape(uint32_t m, uint32_t n, uint32_t k) {
    for (unsigned i = 0; i < shape_count; i++)
        if (shapes[i].m == m && shapes[i].n == n && shapes[i].k == k) return;
    if (shape_count >= sizeof shapes / sizeof shapes[0]) abort();
    shapes[shape_count++] = (AnyShape){m,n,k};
}
static void add_axis(uint32_t d) { add_shape(d,3,5); add_shape(3,d,5); add_shape(3,5,d); }
static void init_shapes(void) {
    static const uint32_t edges[] = {127,128,129,255,256,257,511,512,513,1023,1024,1025,2047,2048,2049,4095,4096,4097,8191,8192};
    for (uint32_t d = 1; d <= 64; d++) add_axis(d);
    for (unsigned i = 0; i < sizeof edges / sizeof edges[0]; i++) add_axis(edges[i]);
    for (uint32_t i = 1; i <= 30; i++) add_shape(i,2*i+1,3*i+7);
    static const AnyShape coupled[] = {{17,19,33},{31,33,65},{63,65,17},{65,67,33},{127,129,3},{129,127,5}};
    for (unsigned i = 0; i < sizeof coupled / sizeof coupled[0]; i++)
        add_shape(coupled[i].m,coupled[i].n,coupled[i].k);
    check(shape_count == 286, "Wolfram-verified distinct shape count");
}
/* The mathematical check supplements the stricter bitwise tensor-model comparison.
 * WolframLanguageEvaluator: (33*4/2^26+1/2^23)<1/2^18 -> True;
 * n=Ceiling[K/16]; gamma=n/(2^18-n). See om/gemm-any/README.md for assumptions. */
static uint64_t mathematical_check(OmegaGpuGemmType t, unsigned lay, AnyShape sh,
                                   const void *a, const void *b, const float *c) {
    double steps = (sh.k + 15u) / 16u, gamma = steps / (262144.0 - steps);
    uint64_t bad = 0;
    for (uint32_t m = 0; m < sh.m; m++) for (uint32_t n = 0; n < sh.n; n++) {
        double sum = 0, magnitude = 0;
        for (uint32_t k = 0; k < sh.k; k++) {
            double av = val(t,a,lay & OMEGA_GPU_GEMM_A_COL ? (uint64_t)k*sh.m+m : (uint64_t)m*sh.k+k);
            double bv = val(t,b,lay & OMEGA_GPU_GEMM_B_COL ? (uint64_t)n*sh.k+k : (uint64_t)k*sh.n+n);
            double p = av*bv; sum += p; magnitude += fabs(p);
        }
        if (!isfinite(c[(uint64_t)m*sh.n+n]) || fabs((double)c[(uint64_t)m*sh.n+n]-sum) > gamma*magnitude) bad++;
    }
    return bad;
}
static uint64_t digest_words(uint64_t h, const float *p, uint64_t n) {
    for (uint64_t i = 0; i < n; i++) { h ^= f2u(p[i]); h *= UINT64_C(1099511628211); }
    return h;
}
static int sweep(int chip, int digest) {
    const size_t cap = 8192u*5u;
    void *a = malloc(cap*2+2), *b = malloc(cap*2+2); float *c = malloc(cap*4);
    if (!a || !b || !c) { free(a); free(b); free(c); return 2; }
    unsigned runs = 0; uint64_t checked_total = 0;
    for (int ti = 0; ti < 2; ti++) {
        OmegaGpuGemmType t = (OmegaGpuGemmType)ti;
        OmegaGemmCfg cfg = { t,1,1,1,1,4,0,0 };
        SimCtx sim; memset(&sim,0,sizeof sim);
        if (!chip && sim_build(&sim,&cfg)) { fprintf(stderr,"kernel build failed type %d\n",ti); sim_free(&sim); return 1; }
        for (unsigned s = 0; s < shape_count; s++) for (unsigned lay = 0; lay < 4; lay++) {
            AnyShape sh = shapes[s]; uint64_t checked = 0; char first[200] = {0};
            fill(t,a,(uint64_t)sh.m*sh.k,s*4+lay); fill(t,b,(uint64_t)sh.k*sh.n,s*4+lay+1000);
            sim.order = s & 1u;
            int rc = chip ? omega_gpu_gemm(t,lay,sh.m,sh.n,sh.k,a,b,c,NULL) : sim_gemm(&sim,&cfg,lay,sh.m,sh.n,sh.k,a,b,c);
            uint64_t bad = rc ? 1 : compare(t,lay,sh.m,sh.n,sh.k,a,b,c,&checked,first,sizeof first);
            uint64_t math_bad = rc ? 1 : mathematical_check(t,lay,sh,a,b,c);
            if (rc || bad || math_bad) {
                fprintf(stderr,"FAIL any %s %ux%ux%u layout %u rc %d wrong %"PRIu64" math %"PRIu64" %s %s\n",tname(t),sh.m,sh.n,sh.k,lay,rc,bad,math_bad,first,sim.err);
                sim_free(&sim); free(a); free(b); free(c); return 1;
            }
            checked_total += checked; runs++;
            if (digest) printf("digest %d %u %u %016"PRIx64"\n",ti,s,lay,digest_words(UINT64_C(14695981039346656037),c,(uint64_t)sh.m*sh.n));
        }
        sim_free(&sim);
    }
    printf("GEMM ANY %s SWEEP PASS (%u distinct shapes, %u multiplies, %"PRIu64" output words, bitwise model + per-K bound)\n",chip ? "CHIP" : "SIM",shape_count,runs,checked_total);
    free(a); free(b); free(c); return 0;
}
static int mutants(int chip) {
    AnyShape sh = {17,9,33}; uint16_t a[17*33+1], b[33*9+1]; float c[17*9];
    for (int t = 0; t < 2; t++) for (int mut = 1; mut <= 2; mut++) {
        OmegaGemmCfg cfg = {(OmegaGpuGemmType)t,1,1,1,1,4,mut,0};
        SimCtx sim; memset(&sim,0,sizeof sim);
        if (!chip && sim_build(&sim,&cfg)) { sim_free(&sim); return 1; }
        fill(cfg.type,a,17*33,77); fill(cfg.type,b,33*9,78);
        if (chip) omega_gpu_gemm_test_set_cfg(cfg.type,&cfg);
        int rc = chip ? omega_gpu_gemm(cfg.type,0,sh.m,sh.n,sh.k,a,b,c,NULL) :
                        sim_gemm(&sim,&cfg,0,sh.m,sh.n,sh.k,a,b,c);
        if (chip) omega_gpu_gemm_test_set_cfg(cfg.type,NULL);
        uint64_t checked = 0; char first[200];
        uint64_t bad = rc ? 0 : compare(cfg.type,0,sh.m,sh.n,sh.k,a,b,c,&checked,first,sizeof first);
        sim_free(&sim);
        if (rc || !bad) { fprintf(stderr,"FAIL mutant %d type %d rc %d wrong %"PRIu64"\n",mut,t,rc,bad); return 1; }
        printf("GEMM ANY MUTANT CAUGHT %s %s (%"PRIu64" differing output words)\n",tname(cfg.type),mut == 1 ? "dropped K tail" : "off-by-one edge predicate",bad);
    }
    return 0;
}
int main(int argc, char **argv) {
    int sim = 0, chip = 0, digest = 0, list = 0; double bf = 0, fp = 0;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i],"--sim")) sim = 1;
        else if (!strcmp(argv[i],"--digest")) digest = 1;
        else if (!strcmp(argv[i],"--list")) list = 1;
        else if (!strcmp(argv[i],"--chip")) chip = 1;
        else if (!strcmp(argv[i],"--ceiling") && i+1 < argc && sscanf(argv[++i],"%lf,%lf",&bf,&fp) == 2 && isfinite(bf) && isfinite(fp) && bf > 0 && fp > 0) { CEIL_TFLOPS[0] = bf; CEIL_TFLOPS[1] = fp; }
        else return 2;
    }
    if ((chip && (sim || list || !bf || !fp)) || (!sim && !chip && !list)) {
        fprintf(stderr,"usage: %s --sim [--digest] | --list | --chip --ceiling MEASURED_BF16_TFLOPS,MEASURED_E4M3_TFLOPS\n",argv[0]); return 2;
    }
    init_shapes(); if (g_failed) return 1;
    if (list) { for (unsigned s = 0; s < shape_count; s++) printf("%u %u %u\n",shapes[s].m,shapes[s].n,shapes[s].k); return 0; }
    if (sweep(chip,digest)) return 1;
    if (!digest && mutants(chip)) return 1;
    if (chip) {
        /* Dense speed probe is separate from the exhaustive skinny/edge sweep. */
        size_t n = (size_t)8192*8192; void *a = malloc(n*2), *b = malloc(n*2); float *c = malloc(n*4);
        if (!a || !b || !c) { free(a); free(b); free(c); return 2; }
        for (int t = 0; t < 2; t++) chip_one((OmegaGpuGemmType)t,0,8192,8192,8192,a,b,c);
        free(a); free(b); free(c); omega_gpu_session_close();
    }
    printf("GEMM ANY %s %s\n",chip ? "CHIP" : digest ? "SIM/DIGEST" : "HOST/SIM/MUTANTS",g_failed ? "FAIL" : "PASS");
    return g_failed ? 1 : 0;
}
