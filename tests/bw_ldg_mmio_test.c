/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/*
 * bw_ldg_mmio_test (issue #205): pins the word src/omega_blackwell_codegen.c encodes for BW_IR_LDG_MMIO, the load
 * whose IR contract is "LDG.E.MMIO.GPU: do not keep a stale cached copy" (src/omega_blackwell_codegen.h).
 *
 * The two trees disagree: CUDA++ encodes w[2] = 0x0c1f1900, omega-mv b5ab3fe94304 encodes w[2] = 0x0c0e1900. The
 * memory-access fields are read with Mesa NAK's sm_80+ layout (src/nouveau/compiler/nak/sm70_encode.rs, Mesa
 * 34d9ed80fc65, SM70Encoder::set_mem_access):
 *   bits 73..75 memory type      (set_mem_type:          B32 = 4, B64 = 5, B128 = 6)
 *   bits 77..80 memory order     (set_mem_order, sm>=80: Weak = 0, Constant = 4, Strong CTA = 5, GPU = 7, SYS = 0xa)
 *   bits 84..86 eviction priority (set_eviction_priority: First = 0, Normal = 1, Last = 2, ...)
 * With that citation omega-mv's word is a weak, evict-first load (LDG.E.EF): it has no MMIO or strong ordering, so
 * it cannot keep the IR contract. CUDA++'s word has normal eviction and memory order 0x8, a value NAK's sm_80+
 * table does not list; NVIDIA's offline nvdisasm 13.0.85 (-b SM121) decodes it as LDG.E.MMIO.GPU, as checked by
 * make test-sm121-mmio on the .om encoder's op 21 (om/encoder/mmio_test.om, the same word). nvdisasm is a
 * read-only checker (ADR 0043 C6); the memory-order value has no chip test yet, so GB10 behaviour is NOT_RUN.
 *
 * Checks, host only, no device:
 *   1. the C encoder's words for LDG_MMIO equal the .om encoder's (w[2] = 203364608 = 0x0c1f1900, mmio_test.om);
 *   2. the NAK field read reproduces NAK's values on the two other forms this encoder writes (LDG.E, LDG.E.STRONG.SYS);
 *   3. LDG_MMIO's fields keep the contract: 32-bit, normal eviction (not .EF), ordering neither weak nor constant;
 *   4. Omega's own sm_121 disassembler prints LDG.E.MMIO.GPU for it.
 * --word 0xW2: check only the contract (3) on an LDG.E word with that w[2]; exit 1 when it fails. The Makefile's
 * red check runs it on omega-mv's 0x0c0e1900 and requires the failure.
 */
#include "omega_blackwell_codegen.h"
#include "omega_sm121_disasm.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* instruction bits [at, at + n) of a 128-bit word; every field used here lies inside one 32-bit word */
static unsigned field(const uint32_t w[4], unsigned at, unsigned n) { return (w[at / 32] >> (at % 32)) & ((1u << n) - 1u); }

#define NAK_MEM_TYPE(w) field((w), 73, 3)
#define NAK_MEM_ORDER(w) field((w), 77, 4)
#define NAK_EVICTION(w) field((w), 84, 3)
enum { NAK_B32 = 4, NAK_WEAK = 0x0, NAK_CONSTANT = 0x4, NAK_STRONG_SYS = 0xa, NAK_EVICT_FIRST = 0, NAK_EVICT_NORMAL = 1 };

static int failures;
#define CHECK(cond, ...) do { if (!(cond)) { failures++; printf("FAIL: " __VA_ARGS__); printf("\n"); } } while (0)

/* the IR contract of BW_IR_LDG_MMIO, read through NAK's fields; 0 = kept */
static int mmio_contract(const uint32_t w[4], int verbose) {
    unsigned type = NAK_MEM_TYPE(w), order = NAK_MEM_ORDER(w), ev = NAK_EVICTION(w);
    int bad = 0;
    if (type != NAK_B32) { bad = 1; if (verbose) printf("  memory type %u, want B32 (%u)\n", type, NAK_B32); }
    if (ev != NAK_EVICT_NORMAL) {
        bad = 1;
        if (verbose) printf("  eviction priority %u%s, want normal (%u)\n", ev, ev == NAK_EVICT_FIRST ? " (EF)" : "", NAK_EVICT_NORMAL);
    }
    if (order == NAK_WEAK || order == NAK_CONSTANT) {
        bad = 1;
        if (verbose) printf("  memory order 0x%x (%s): a cached load, not MMIO\n", order, order == NAK_WEAK ? "weak" : "constant");
    }
    return bad;
}

static void encode(BlackwellIROpcode op, uint32_t w[4]) {
    BlackwellIRInsn in;
    memset(&in, 0, sizeof in);
    in.op = op; in.dst_vreg = 4; in.src1_vreg = 8; in.src2_vreg = -1; in.src3_vreg = -1; in.ureg = 2;
    memset(w, 0, 4 * sizeof w[0]);
    if (omega_blackwell_encode_one(&in, w) != 0) { failures++; printf("FAIL: op %d refused\n", (int)op); }
}

int main(int argc, char **argv) {
    if (argc == 3 && strcmp(argv[1], "--word") == 0) {
        uint32_t w[4];
        encode(BW_IR_LDG_MMIO, w);
        char *end;
        unsigned long v = strtoul(argv[2], &end, 0);
        if (*end || v > 0xffffffffUL) { fprintf(stderr, "bad word %s\n", argv[2]); return 2; }
        w[2] = (uint32_t)v;
        int bad = mmio_contract(w, 1);
        printf("w[2] = 0x%08" PRIx32 ": %s\n", w[2], bad ? "breaks the LDG_MMIO contract" : "keeps the LDG_MMIO contract");
        return bad ? 1 : 0;
    }
    if (argc != 1) { fprintf(stderr, "usage: %s [--word 0xW2]\n", argv[0]); return 2; }

    uint32_t w[4];
    /* 1. LDG.E.MMIO.GPU R4, desc[UR2][R8.64]: the .om encoder's op 21 with the same operands (mmio_test.om) */
    encode(BW_IR_LDG_MMIO, w);
    CHECK(w[0] == 0x08047981u && w[1] == 0x00000002u && w[2] == 0x0c1f1900u,
          "LDG_MMIO words 0x%08" PRIx32 " 0x%08" PRIx32 " 0x%08" PRIx32 ", want 0x08047981 0x00000002 0x0c1f1900", w[0], w[1], w[2]);
    CHECK(w[2] == 203364608u, "LDG_MMIO w[2] differs from om/encoder/mmio_test.om (203364608)");

    /* 2. the field read agrees with NAK on the forms NAK does encode */
    uint32_t e[4];
    encode(BW_IR_LDG_E, e);
    CHECK(NAK_MEM_TYPE(e) == NAK_B32 && NAK_MEM_ORDER(e) == NAK_WEAK && NAK_EVICTION(e) == NAK_EVICT_NORMAL,
          "LDG.E fields type %u order 0x%x eviction %u, want B32 weak normal", NAK_MEM_TYPE(e), NAK_MEM_ORDER(e), NAK_EVICTION(e));
    encode(BW_IR_LDG_STRONG_SYS, e);
    CHECK(NAK_MEM_TYPE(e) == NAK_B32 && NAK_MEM_ORDER(e) == NAK_STRONG_SYS && NAK_EVICTION(e) == NAK_EVICT_NORMAL,
          "LDG.E.STRONG.SYS fields type %u order 0x%x eviction %u, want B32 strong.sys normal", NAK_MEM_TYPE(e), NAK_MEM_ORDER(e), NAK_EVICTION(e));

    /* 3. the contract */
    CHECK(mmio_contract(w, 1) == 0, "LDG_MMIO breaks its IR contract");
    printf("LDG_MMIO NAK fields: type %u, order 0x%x, eviction %u\n", NAK_MEM_TYPE(w), NAK_MEM_ORDER(w), NAK_EVICTION(w));

    /* 4. Omega's own decoder */
    char text[128];
    uint64_t lo = (uint64_t)w[1] << 32 | w[0], hi = (uint64_t)w[3] << 32 | w[2];
    int rc = omega_sm121_disasm(lo, hi, 0, text, sizeof text);
    CHECK(rc == 0 && strcmp(text, "LDG.E.MMIO.GPU R4, desc[UR2][R8.64]") == 0, "disassembler rc %d text \"%s\"", rc, text);

    printf("bw_ldg_mmio_test: %s (GB10 device NOT_RUN)\n", failures ? "FAIL" : "PASS");
    return failures ? 1 : 0;
}
