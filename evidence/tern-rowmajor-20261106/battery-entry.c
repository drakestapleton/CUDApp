/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Verification-only probe; unreachable chip entry discarded.
 * Expand ~ to HOME before compiling, as with the receipt logs.
 * The SPDX and path-expansion comments are archival, not behavioral changes. */
#define main omega_tern_original_main
#include "~/work/cudastack/omega-mv/tests/gpu_tern_test.c"
#undef main
int main(void) { sim_battery(); printf("gpu_tern_test: %d checks, %d failed: %s\n", g_checks, g_failed, g_failed ? "FAIL" : "PASS"); return g_failed ? 1 : 0; }
