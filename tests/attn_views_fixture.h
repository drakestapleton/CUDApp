/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef ATTN_VIEWS_FIXTURE_H
#define ATTN_VIEWS_FIXTURE_H
#include <stdint.h>
#define AV_N 1000003u
typedef struct {
    uint32_t args[32],layout[32],ui[AV_N],compact[AV_N];
    uint8_t bytes[AV_N],gather[512],stage[512];
    float ff[AV_N],rope[1024],fold[32];
    uint64_t result[32];
    uint32_t table[32],touched[8],headmap[16],eff[8],range[16];
    int32_t slots[8];
} AttnViewsFixture;
void attn_omega(AttnViewsFixture *r);
void attn_oracle(AttnViewsFixture *r);
#endif
