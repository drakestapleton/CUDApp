/* ngcap.c: REFEREE INSTRUMENT, not part of any build or gate (Hive NG lane ng-gpu, 2026-11-07).
 * Linked into a scratch build of CUDApp@b774723f tests with
 *   -Wl,--wrap=omega_bw_enc_sm121 -Wl,--wrap=omega_cubin_elf
 * It records every call CUDApp's own C kernel builder / artifact code makes into its seeded .om encoder and
 * cubin packer (the generated omega-bridge binding), in the frame formats the NG tools read, plus the result the
 * CUDApp unit returned. In CUDAPP_*_DIFF builds the CUDApp C oracle runs beside it and aborts on any difference,
 * so every recorded result is agreed by CUDApp's .om unit and its C oracle.
 * Files: $NGCAP_DIR/enc.in enc.out cubin.in cubin.out (appended). Unset NGCAP_DIR: records nothing.
 * Frame formats: seed/ng/gpu/README.md (NGBWENC1 / NGCUBIN1). Little-endian host (AArch64). */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint32_t __real_omega_bw_enc_sm121(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n,
                                   uint64_t p2, uint64_t p3, void *p4, uint64_t p4_n);
uint32_t __real_omega_cubin_elf(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n,
                                const void *p2, uint64_t p2_n, void *p3, uint64_t p3_n);

static FILE *cap(const char *name) {
    const char *d = getenv("NGCAP_DIR");
    if (!d || !d[0]) return NULL;
    char p[4096];
    snprintf(p, sizeof p, "%s/%s", d, name);
    FILE *f = fopen(p, "ab");
    if (!f) { fprintf(stderr, "ngcap: cannot open %s\n", p); abort(); }
    return f;
}

static void w64(FILE *f, uint64_t v) { if (fwrite(&v, 8, 1, f) != 1) abort(); }
static void wb(FILE *f, const void *b, size_t n) { if (n && fwrite(b, 1, n, f) != n) abort(); }

uint32_t __wrap_omega_bw_enc_sm121(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n,
                                   uint64_t p2, uint64_t p3, void *p4, uint64_t p4_n) {
    uint32_t st = __real_omega_bw_enc_sm121(result, p0, p0_n, p1, p1_n, p2, p3, p4, p4_n);
    FILE *fi = cap("enc.in");
    if (!fi) return st;
    if (st != 0 || p0_n != 8192 || p1_n != 288 || p4_n != 16384) { fprintf(stderr, "ngcap: enc shape/status\n"); abort(); }
    uint64_t n = p2, rows = n <= 1024 ? n : 0;
    wb(fi, "NGBWENC1", 8);
    w64(fi, n);
    w64(fi, p3);
    wb(fi, p1, 288 * 8);
    wb(fi, p0, rows * 64);
    fclose(fi);
    FILE *fo = cap("enc.out");
    w64(fo, *result);
    wb(fo, p4, (size_t)*result);
    fclose(fo);
    return st;
}

uint32_t __wrap_omega_cubin_elf(uint64_t *result, const void *p0, uint64_t p0_n, const void *p1, uint64_t p1_n,
                                const void *p2, uint64_t p2_n, void *p3, uint64_t p3_n) {
    uint32_t st = __real_omega_cubin_elf(result, p0, p0_n, p1, p1_n, p2, p2_n, p3, p3_n);
    FILE *fi = cap("cubin.in");
    if (!fi) return st;
    if (st != 0 || p0_n != 104 || p1_n != 64 || p2_n != 16777216) { fprintf(stderr, "ngcap: cubin shape/status\n"); abort(); }
    const uint64_t *m = p0;
    uint64_t tl = m[1] <= 16777216 ? m[1] : 0;
    wb(fi, "NGCUBIN1", 8);
    wb(fi, m, 104 * 8);
    wb(fi, p1, 64);
    wb(fi, p2, tl);
    fclose(fi);
    FILE *fo = cap("cubin.out");
    w64(fo, *result);
    if (*result > 7) wb(fo, p3, (size_t)*result);
    else wb(fo, p3, 24);
    fclose(fo);
    return st;
}
