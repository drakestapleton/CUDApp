/* encref.c: REFEREE INSTRUMENT, not part of any build or gate (Hive NG lane ng-gpu, 2026-11-07).
 * Reads an NGBWENC1 frame stream on stdin and writes, in the ngenc result-frame format, what CUDApp's C encoder
 * oracle (static bw_c_encode_insn of src/omega_blackwell_codegen.c @ CUDApp b774723f, compiled with
 * CUDAPP_MIGRATION_PENDING_ENCODER_DIFF) produces for it: program mode (pad 1) as bw_encode_program_bytes'
 * oracle loop does (NOP words 0x7918,0,0,0x000fc000 past n, padded to a multiple of 8, at least 32),
 * pad 0 as exactly n instructions; any instruction refusal gives r = 0. Built from the CUDApp tree:
 *   gcc -std=gnu11 -O2 -pthread -Isrc -DCUDAPP_MIGRATION_PENDING_ENCODER_DIFF -o encref encref.c \
 *       src/omega_blackwell_matmul.c src/sha256.c src/cudapp_om_*.c src/cudapp_freest_{revision,epoch,f16}.c
 * With argument -v it also prints, on stderr, every instruction whose C words differ from a second stream
 * given as argv[2] (the CUDApp .om or NG output of the same input). */
#include "omega_blackwell_codegen.c"

static int rd(void *b, size_t n) { return fread(b, 1, n, stdin) == n; }

int main(int argc, char **argv) {
    FILE *cmp = (argc > 2 && strcmp(argv[1], "-v") == 0) ? fopen(argv[2], "rb") : NULL;
    static uint64_t ph[288], ins[8192];
    static uint8_t code[16384], other[16384];
    char mg[8];
    uint64_t frame = 0;
    while (fread(mg, 1, 8, stdin) == 8) {
        uint64_t n, pad;
        if (memcmp(mg, "NGBWENC1", 8) || !rd(&n, 8) || !rd(&pad, 8) || !rd(ph, sizeof ph)) return 3;
        uint64_t rows = n <= 1024 ? n : 0;
        memset(ins, 0, sizeof ins);
        if (rows && !rd(ins, rows * 64)) return 3;
        static OmegaRegAlloc ra;
        memset(&ra, 0, sizeof ra);
        for (int i = 0; i < 256; i++) ra.vreg_to_phys[i] = (int)(int64_t)ph[i];
        for (int i = 0; i < 32; i++) ra.uvreg_to_phys[i] = (int)(int64_t)ph[256 + i];
        uint64_t r = 0;
        if (n <= 1024) {
            uint64_t slots = n;
            if (pad) { slots = (n + 7) & ~7ULL; if (slots < 32) slots = 32; }
            int bad = 0;
            for (uint64_t i = 0; i < slots && !bad; i++) {
                uint32_t w[4] = { 0x00007918, 0, 0, 0x000fc000 };
                if (i < n) {
                    BlackwellIRInsn in;
                    memset(&in, 0, sizeof in);
                    const uint64_t *q = &ins[i * 8];
                    in.op = (BlackwellIROpcode)(int)q[0];
                    in.dst_vreg = (int)(int64_t)q[1]; in.src1_vreg = (int)(int64_t)q[2];
                    in.src2_vreg = (int)(int64_t)q[3]; in.src3_vreg = (int)(int64_t)q[4];
                    in.ureg = (int)(int64_t)q[5];
                    in.imm = (uint32_t)q[6]; in.control = (uint32_t)(q[6] >> 32);
                    in.is_uniform = q[7] & 1; in.predicate_p0 = (q[7] >> 1) & 1; in.predicate_not = (q[7] >> 2) & 1;
                    in.bar_reg = (uint8_t)(q[7] >> 8); in.dst_subreg = (uint8_t)(q[7] >> 16);
                    in.src1_subreg = (uint8_t)(q[7] >> 24); in.src2_subreg = (uint8_t)(q[7] >> 32);
                    in.src3_subreg = (uint8_t)(q[7] >> 40);
                    if (bw_c_encode_insn(&in, &ra, w) != 0) bad = 1;
                }
                for (int k = 0; k < 16; k++) code[16 * i + (uint64_t)k] = (uint8_t)(w[k / 4] >> (8 * (k % 4)));
            }
            if (!bad) r = slots * 16;
        }
        fwrite(&r, 8, 1, stdout);
        fwrite(code, 1, r, stdout);
        if (cmp) {
            uint64_t r2 = 0;
            if (fread(&r2, 8, 1, cmp) != 1 || (r2 && fread(other, 1, r2, cmp) != r2)) { fprintf(stderr, "short cmp\n"); return 4; }
            if (r2 != r) fprintf(stderr, "frame %llu n=%llu: C r=%llu other r=%llu\n", (unsigned long long)frame,
                                 (unsigned long long)n, (unsigned long long)r, (unsigned long long)r2);
            else
                for (uint64_t i = 0; i < r / 16; i++)
                    if (memcmp(code + 16 * i, other + 16 * i, 16)) {
                        const uint64_t *q = &ins[i * 8];
                        fprintf(stderr, "frame %llu insn %llu op %llu: C", (unsigned long long)frame,
                                (unsigned long long)i, (unsigned long long)(i < n ? q[0] : 999));
                        for (int k = 0; k < 16; k += 4) fprintf(stderr, " %08x", *(uint32_t *)(code + 16 * i + k));
                        fprintf(stderr, " other");
                        for (int k = 0; k < 16; k += 4) fprintf(stderr, " %08x", *(uint32_t *)(other + 16 * i + k));
                        fprintf(stderr, " | ins %llx %llx %llx %llx %llx %llx %llx %llx\n",
                                (unsigned long long)q[0], (unsigned long long)q[1], (unsigned long long)q[2],
                                (unsigned long long)q[3], (unsigned long long)q[4], (unsigned long long)q[5],
                                (unsigned long long)q[6], (unsigned long long)q[7]);
                    }
        }
        frame++;
    }
    return 0;
}
