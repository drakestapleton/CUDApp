/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 * Diagnostic oracle copied unchanged from CUDApp b774723f537cec16a359fe082b8100317a5b5490. */
static void ref_cubin_message(uint32_t target_sm, int rc, const uint64_t *d, size_t text_len, const OmegaCubinParam *params,
                             size_t nparams, uint32_t cbank_size, uint32_t gpr_count, uint32_t shared_reserved,
                             char *err, size_t err_cap) {
    size_t i = (size_t)d[1];
    if (!err) return;
    if (rc == OMEGA_CUBIN_E_PARAMS && d[0] != 0 && i >= nparams) {
        snprintf(err, err_cap, "cubin_elf.om refusal %d names parameter %zu of %zu", rc, i, nparams);
        return;
    }
    switch (rc) {
    case OMEGA_CUBIN_E_ARG: snprintf(err, err_cap, "invalid function name or text"); break;
    case OMEGA_CUBIN_E_TEXT: snprintf(err, err_cap, "text length %zu is not a nonzero multiple of 16", text_len); break;
    case OMEGA_CUBIN_E_PARAMS:
        switch (d[0]) {
        case 0: snprintf(err, err_cap, "too many parameters: %zu", nparams); break;
        case 1: snprintf(err, err_cap, "parameter %zu offset 0x%x overlaps or is unsorted", i, params[i].offset); break;
        case 2: snprintf(err, err_cap, "parameter %zu has unsupported size %u", i, params[i].size); break;
        case 3: snprintf(err, err_cap, "parameter %zu is a %u-byte pointer", i, params[i].size); break;
        default: snprintf(err, err_cap, "parameter %zu offset 0x%x is not naturally aligned", i, params[i].offset); break;
        }
        break;
    case OMEGA_CUBIN_E_CBANK:
        if (d[0] == 5)
            snprintf(err, err_cap, "parameter area ends at 0x%llx beyond cbank size 0x%x", (unsigned long long)d[2],
                     cbank_size);
        else
            snprintf(err, err_cap, "cbank size 0x%x outside 0x%x..0x%x", cbank_size,
                     target_sm == OMEGA_CUBIN_SM_86 ? OMEGA_CUBIN_PARAM_BASE_SM86 : OMEGA_CUBIN_PARAM_BASE,
                     OMEGA_CUBIN_CBANK_MAX);
        break;
    case OMEGA_CUBIN_E_GPR: snprintf(err, err_cap, "gpr count %u exceeds 255", gpr_count); break;
    case OMEGA_CUBIN_E_SHARED: snprintf(err, err_cap, "reserved shared bytes %u not a multiple of 4", shared_reserved); break;
    default:
        snprintf(err, err_cap, "cubin image of %llu bytes exceeds the Omega packer's %llu-byte output view",
                 (unsigned long long)d[2], (unsigned long long)CUBIN_OM_WORDS * 8u);
        break;
    }
}
