/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Verification-only reuse of omega-mv@e5b32567's ternary/algebra battery with
 * CUDApp's current warp simulator. The pinned Omega simulator predates the
 * scalar LDG immediate fix and fails the same row-major cases for the C oracle.
 * Its include guard ensures the battery uses this current simulator instead.
 * Unreachable original main/chip sections are discarded with --gc-sections. */
#include "bw_warp_sim_k.h"
#define main omega_tern_reference_main
#include OMEGA_TERN_REFERENCE
#undef main
int main(void) {
    sim_battery();
    printf("CODEGEN TERN SIM: %d checks, %d failed: %s\n", g_checks, g_failed, g_failed ? "FAIL" : "PASS");
    return g_failed ? 1 : 0;
}
