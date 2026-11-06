/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Verification-only reuse of omega-mv@e5b32567's existing ternary simulator
 * battery. The retained entry has no command-line/device branch. Unreachable
 * original main/chip sections are discarded with --gc-sections. */
/* The pinned simulator also recognizes TC's conversion marker, absent from
 * CUDApp's older TC header. Exact verification-only constant from the pinned
 * omega_blackwell_codegen_tc.h:110; no ternary instruction uses this marker. */
#define OMEGA_TC_MARK_F2FP 0x46320000u
#define main omega_tern_reference_main
#include OMEGA_TERN_REFERENCE
#undef main
int main(void) {
    sim_battery();
    printf("CODEGEN TERN SIM: %d checks, %d failed: %s\n", g_checks, g_failed, g_failed ? "FAIL" : "PASS");
    return g_failed ? 1 : 0;
}
