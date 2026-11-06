/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/*
 * m16_requalify.c -- M16 libcuda-free causality qualification test.
 *
 * Sequence:
 *   1. Initialize raw RM client (zero libcuda).
 *   2. Create independent channel, USERD, and obtain work-submit token.
 *   3. Allocate coherent marker memory, initialize marker = 0.
 *   4. Construct M16 semaphore-release method stream (expected payload: 0x16c0ffee).
 *   5. Enqueue work to GPFIFO ring WITHOUT ringing doorbell.
 *   6. Negative control: wait 100ms, assert marker remains strictly 0.
 *   7. Positive stimulus: ring doorbell with session work-submit token.
 *   8. Assert marker changes to 0x16c0ffee.
 *   9. Inspect /proc/self/maps to prove libcuda.so was never loaded.
 *  10. Output machine-parsable receipts.
 */
#include "m16_native.h"
/* The pass/fail decisions (word count, withheld marker, positive wait, libcuda
 * audit) and the 0x16c0ffee payload come from om/engine/eng.om (op REQF,
 * src/omega_eng_gates.h); the C oracle is behind CUDAPP_MIGRATION_PENDING_C_REQUALIFY.
 * Exit codes are unchanged. */
#if defined(CUDAPP_MIGRATION_PENDING_C_REQUALIFY)
#define ENG_C_PART 1
#endif
#include "omega_eng_gates.h"
#include "omega_fixture_rest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int check_proc_maps(void) {
    FILE *f = fopen("/proc/self/maps", "r");
    if (!f) return -1;
    unsigned char line[512] = {0};
    int hit = 0;
    while (fgets((char *)line, sizeof(line), f)) {
        if (omega_fixture_text(line, (const unsigned char[32]){"libcuda.so"}, 10) ||
            omega_fixture_text(line, (const unsigned char[32]){"libcudart.so"}, 12)) {
            hit = 1;
            break;
        }
    }
    fclose(f);
    return hit;
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    printf("=== M16 CORRECTIVE REQUALIFICATION: CONTROLLED CAUSALITY TRANSACTION ===\n");

    M16NativeContext ctx;
    if (m16_native_open(&ctx) != 0) {
        fprintf(stderr, "FATAL: m16_native_open failed: %s\n", ctx.rm.err);
        return 1;
    }
    printf("[OBSERVED] RM client opened: GPU minor=%u compute_class=0x%x sm_version=0x%x\n",
           ctx.rm.minor, ctx.rm.compute_class, ctx.rm.sm_version);

    if (m16_native_create_channel(&ctx) != 0) {
        fprintf(stderr, "FATAL: m16_native_create_channel failed: %s\n", ctx.rm.err);
        m16_native_close(&ctx);
        return 1;
    }
    printf("[OBSERVED] Channel initialized: token=0x%08x gpfifo=0x%08x fifo_va=0x%lx doorbell=%p\n",
           ctx.rm.token, ctx.rm.gpfifo, (unsigned long)ctx.rm.fifo.va, (void *)ctx.rm.doorbell);

    void *marker_cpu = NULL;
    uint64_t marker_va = 0;
    if (m16_native_alloc_memory(&ctx, 0x1000, &marker_cpu, &marker_va) != 0) {
        fprintf(stderr, "FATAL: alloc marker failed: %s\n", ctx.rm.err);
        m16_native_close(&ctx);
        return 1;
    }
    volatile uint32_t *marker = (volatile uint32_t *)marker_cpu;
    *marker = 0;
    printf("[OBSERVED] Coherent marker allocated at VA=0x%lx CPU=%p (initial=0x%08x)\n",
           (unsigned long)marker_va, marker_cpu, *marker);

    uint32_t methods[32];
    uint32_t payload = omega_gate_reqf_payload();
    if (!omega_fixture_calc(FR_EQ, payload, 0x16c0ffeeu, 0)) {
        fprintf(stderr, "FATAL: requalify payload unavailable (0x%08x)\n", payload);
        m16_native_close(&ctx);
        return 1;
    }
    int nwords = m16_native_build_release(methods, marker_va, payload, ctx.rm.compute_class);
    if (!omega_gate_reqf(OMEGA_REQF_WORDS, nwords)) {
        fprintf(stderr, "FATAL: build release failed\n");
        m16_native_close(&ctx);
        return 1;
    }

    /* Step 1: Enqueue to GPFIFO without ringing doorbell */
    printf("\n--- STEP 1: NEGATIVE CONTROL (DOORBELL WITHHELD) ---\n");
    if (m16_native_enqueue_methods(&ctx, methods, (size_t)nwords) != 0) {
        fprintf(stderr, "FATAL: enqueue failed: %s\n", ctx.rm.err);
        m16_native_close(&ctx);
        return 1;
    }
    printf("[OBSERVED] Enqueued %d words to GPFIFO index %u. Withholding doorbell notification...\n",
           nwords, (uint32_t)omega_fixture_calc(FR_PREV32, ctx.rm.put, 0, 0));

    /* Wait 100ms and verify marker has NOT changed */
    usleep(100000);
    printf("[OBSERVED] Marker after 100ms without doorbell: 0x%08x (expected 0x00000000)\n", *marker);
    if (!omega_gate_reqf(OMEGA_REQF_MARKER_ZERO, (int64_t)*marker)) {
        fprintf(stderr, "FATAL: Negative control failed: marker changed without doorbell!\n");
        m16_native_close(&ctx);
        return 2;
    }
    printf("NEGATIVE_CONTROL_PASS: Marker strictly remained 0x00000000 while doorbell was withheld.\n");

    /* Step 2: Positive stimulus (Ring doorbell) */
    printf("\n--- STEP 2: POSITIVE STIMULUS (DOORBELL RUNG) ---\n");
    printf("[OBSERVED] Storing work-submit token 0x%08x to doorbell MMIO at %p...\n",
           ctx.rm.token, (void *)ctx.rm.doorbell);
    m16_native_ring_doorbell(&ctx);

    if (!omega_gate_reqf(OMEGA_REQF_WAIT_OK, m16_native_wait_marker(marker, payload, 2000))) {
        fprintf(stderr, "FATAL: Positive control failed: timeout waiting for marker 0x16c0ffee! Actual=0x%08x\n",
                *marker);
        m16_native_close(&ctx);
        return 3;
    }
    printf("[OBSERVED] Marker after doorbell rung: 0x%08x (expected 0x16c0ffee)\n", *marker);
    printf("POSITIVE_CONTROL_PASS: Marker successfully updated to 0x16c0ffee upon doorbell store.\n");

    /* Step 3: Process mappings audit */
    int cuda_mapped = check_proc_maps();
    printf("\n--- STEP 3: RUNTIME LIBCUDA AUDIT ---\n");
    printf("[OBSERVED] libcuda.so / libcudart.so present in /proc/self/maps: %s\n",
           cuda_mapped ? "YES (FAIL)" : "NO (PASS)");
    if (!omega_gate_reqf(OMEGA_REQF_NO_LIBCUDA, cuda_mapped)) {
        fprintf(stderr, "FATAL: libcuda detected in process address space!\n");
        m16_native_close(&ctx);
        return 4;
    }
    printf("NO_LIBCUDA_PASS: Zero libcuda / libcudart mappings present in live process.\n");

    printf("\nCAUSALITY_VERIFICATION_PASS: Doorbell causality strictly confirmed.\n");
    printf("M16_REQUALIFICATION_PASS: Milestone 16 native execution verified.\n");

    nvrm_retire(&ctx.rm, ctx.rm.put);
    m16_native_close(&ctx);
    return 0;
}
