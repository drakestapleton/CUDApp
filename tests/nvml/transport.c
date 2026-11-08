/* ORACLE: foreign C caller of the generated ABI; not shipped */
/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Test transport for the CUDA++ NVML shell: the three omega2_nvrm_* entry points of
 * native/nvrm/transport.om (aien-dev/CUDApp#281), answered by the Omega RM fake
 * tests/nvml/rmfake.om instead of /dev/nvidia*. Each call copies the request into the fake's
 * views, runs one omega_nvml_fake step and copies the reply back. No device file is opened. */
#include <stdint.h>
#include <string.h>

#include "transport.h"

enum { FAKE_HN = 4608, FAKE_PN = 55888 };

uint32_t omega_nvml_fake(uint64_t *result, void *p0, uint64_t p0_n, void *p1, uint64_t p1_n,
                         void *p2, uint64_t p2_n);

static uint64_t fake_st[64];
static uint8_t fake_h[FAKE_HN];
static uint8_t fake_p[FAKE_PN];
static uint64_t fake_traps;

void fake_reset(int fault) {
    memset(fake_st, 0, sizeof fake_st);
    fake_st[5] = (uint64_t)(unsigned char)fault;
    fake_traps = 0;
}

uint64_t fake_get(unsigned i) { return i < 64 ? fake_st[i] : UINT64_MAX; }

uint64_t fake_trapped(void) { return fake_traps; }

/* One fake step; a trap or refusal counts as a violation and reads as transport status 1. */
static uint32_t fake_step(uint64_t *result) {
    uint64_t r = 1;
    if (omega_nvml_fake(&r, fake_st, 64, fake_h, FAKE_HN, fake_p, FAKE_PN) != 0 || r != 0) {
        fake_traps++;
        fake_st[13]++;
        return 1;
    }
    *result = fake_st[6];
    return 0;
}

uint32_t omega2_nvrm_open_file(uint64_t *result, void *o_path, uint64_t o_path_len) {
    if (o_path_len == 0 || o_path_len > FAKE_PN) return 32;
    memcpy(fake_p, o_path, o_path_len);
    fake_st[0] = 2;
    fake_st[7] = o_path_len;
    return fake_step(result);
}

uint32_t omega2_nvrm_close_file(uint64_t *result, uint64_t o_fd) {
    fake_st[0] = 3;
    fake_st[1] = o_fd;
    return fake_step(result);
}

uint32_t omega2_nvrm_ioctl(uint64_t *result, uint64_t o_fd, uint64_t o_nr, void *o_h,
                           uint64_t o_h_len, void *o_p, uint64_t o_p_len) {
    if (o_h_len > FAKE_HN || o_p_len > FAKE_PN) return 32;
    /* Fault S: the transport itself refuses the GPU name control (omega2 status 1). */
    if (fake_st[5] == 'S' && o_nr == 42 && o_h_len >= 12) {
        uint32_t cmd;
        memcpy(&cmd, (const uint8_t *)o_h + 8, 4);
        if (cmd == 0x20800110u) return 1;
    }
    memcpy(fake_h, o_h, o_h_len);
    memcpy(fake_p, o_p, o_p_len);
    fake_st[0] = 1;
    fake_st[1] = o_fd;
    fake_st[2] = o_nr;
    fake_st[3] = o_h_len;
    fake_st[4] = o_p_len;
    uint32_t st = fake_step(result);
    memcpy(o_h, fake_h, o_h_len);
    memcpy(o_p, fake_p, o_p_len);
    return st;
}
