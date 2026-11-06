/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "omega_blackwell_realize.h"
#include "sha256.h"
#include <string.h>
#include "omega_bw_policy.h"
#include "omega_bw_bind.h"

#ifndef BW_BIND_C_PRIMARY
int omega_blackwell_get_machine_id(SemanticId *out_machine_id) {
    uint8_t w[256] = {0};
    if (!out_machine_id) return -1;
    if (omega_bw_bind(2, 0, 0, w) != 27) return -1;
    if (omega_bw_id(w, 27, out_machine_id->bytes) != 0) return -1;
    return 0;
}

/* The Omega message carries sm_architecture little-endian; the C oracle hashes its host bytes (equal on LE hosts). */
int omega_blackwell_realization_compute_id(OmegaBlackwellRealization *real) {
    uint8_t w[256] = {0};
    if (!real) return -1;
    memcpy(w + 128, real->spec_id.bytes, OMEGA_ID_BYTES);
    memcpy(w + 160, real->machine_id.bytes, OMEGA_ID_BYTES);
    memcpy(w + 192, real->code_digest, sizeof(real->code_digest));
    if (omega_bw_bind(1, real->sm_architecture, 0, w) != 100) return -1;
    if (omega_bw_id(w, 100, real->realization_digest) != 0) return -1;
    memcpy(real->realization_id.bytes, real->realization_digest, OMEGA_ID_BYTES);
    return 0;
}
#else

int omega_blackwell_get_machine_id(SemanticId *out_machine_id) {
    if (!out_machine_id) return -1;
    sha256_ctx ctx;
    sha256_init(&ctx);
    const char *machine_str = "NVIDIA_DGX_SPARK_GB10_SM121";
    sha256_update(&ctx, (const uint8_t *)machine_str, strlen(machine_str));
    sha256_final(&ctx, out_machine_id->bytes);
    return 0;
}

int omega_blackwell_realization_compute_id(OmegaBlackwellRealization *real) {
    if (!real) return -1;
    sha256_ctx ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, real->spec_id.bytes, OMEGA_ID_BYTES);
    sha256_update(&ctx, real->machine_id.bytes, OMEGA_ID_BYTES);
    sha256_update(&ctx, real->code_digest, sizeof(real->code_digest));
    sha256_update(&ctx, (const uint8_t *)&real->sm_architecture, sizeof(real->sm_architecture));
    sha256_final(&ctx, real->realization_digest);

    memcpy(real->realization_id.bytes, real->realization_digest, OMEGA_ID_BYTES);
    return 0;
}
#endif /* BW_BIND_C_PRIMARY */

int omega_blackwell_realize_vector(const OmegaVectorSpec *spec, OmegaBlackwellRealization *out_real) {
    if (!spec || !out_real) return -1;
    memset(out_real, 0, sizeof(*out_real));

    out_real->spec_id = spec->spec_id;
    if (omega_blackwell_get_machine_id(&out_real->machine_id) != 0) return -1;

#ifndef BW_BIND_C_PRIMARY
    uint8_t w[256] = {0};
    int64_t arch = omega_bw_bind(9, 0, 0, w);
    if (arch < 0) return -1;
    out_real->sm_architecture = (uint32_t)arch;
#else
    out_real->sm_architecture = OMEGA_BW_SM_ARCH_121;
#endif
    out_real->code_len = OMEGA_BW_VECADD_CODE_SIZE;

    /* Verify and digest generated Blackwell machine code */
    uint8_t code_buf[OMEGA_BW_VECADD_CODE_SIZE];
    size_t encoded_len = 0;
    if (omega_blackwell_encode_vecadd(code_buf, sizeof(code_buf), &encoded_len) != 0) return -1;
    if (omega_blackwell_compute_code_digest(code_buf, encoded_len, out_real->code_digest) != 0) return -1;

    /* Default launch geometry */
    out_real->qmd_cfg.num_elements = spec->element_count;
#ifndef BW_BIND_C_PRIMARY
    int64_t geo = omega_bw_bind(4, spec->element_count, 0, w);
    if (geo < 0) return -1;
    out_real->qmd_cfg.grid_width = (uint32_t)geo;
    out_real->qmd_cfg.threads_per_block = (uint32_t)((uint64_t)geo >> 32);
#else
    out_real->qmd_cfg.threads_per_block = 64;
    out_real->qmd_cfg.grid_width = (spec->element_count + 63) / 64;
    if (out_real->qmd_cfg.grid_width == 0) out_real->qmd_cfg.grid_width = 1;
#endif

    return omega_blackwell_realization_compute_id(out_real);
}
