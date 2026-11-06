/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Reuse the established C-vs-Omega partition, adding explicit target ABI edges. */
#define main no_posix_cubin_sweep
#include "om_cubin_sweep_test.c"
#undef main

int main(void) {
    if (no_posix_cubin_sweep()) return 1;
    /* WolframLanguageEvaluator: {86+{-1,0,1},121+{-1,0,1},2^32-1}
     * -> {{85,86,87},{120,121,122},4294967295}. */
    static const uint32_t targets[] = {0,85,86,87,120,121,122,UINT32_MAX};
    for (size_t i = 0; i < sizeof targets / sizeof targets[0]; i++) {
        uint8_t *out = NULL;
        size_t len = 0;
        char err[256];
        uint32_t sm = targets[i];
        int expected = sm == OMEGA_CUBIN_SM_86 || sm == OMEGA_CUBIN_SM_121
                     ? OMEGA_CUBIN_OK : OMEGA_CUBIN_E_TARGET;
        int rc = omega_cubin_pack_target(sm, "target_edge", text, 16, NULL, 0,
                                        1024, 1, 0, &out, &len, err, sizeof err);
        if (rc != expected || (rc == 0 && (!out || !len)) || (rc != 0 && (out || len))) return 2;
        omega_cubin_free(out);
    }
    uint8_t *out = NULL;
    size_t len = 0;
    int rc = omega_cubin_pack_target(OMEGA_CUBIN_SM_121, "null_params", text, 16,
                                    NULL, 1, 1024, 1, 0, &out, &len, NULL, 0);
    if (rc != OMEGA_CUBIN_E_ARG || out || len) return 3;
    for (int status = OMEGA_CUBIN_OK; status <= OMEGA_CUBIN_E_TARGET; status++)
        if (!omega_cubin_rc_name(status)) return 4;
    puts("PASS explicit sm_86/sm_121 image differential, target boundaries and null parameter refusal");
    return 0;
}
