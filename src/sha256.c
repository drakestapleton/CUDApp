/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 * Copyright 2026 Drake Stapleton (AIEN / aien-dev)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "sha256.h"
#include <string.h>

#ifndef CUDAPP_MIGRATION_PENDING_C_CORE
#include "omega_core_binding.h"
#include <stdlib.h>
_Static_assert(sizeof(sha256_ctx) == 104, "seeded SHA context ABI");
_Static_assert(offsetof(sha256_ctx, count) == 32, "seeded SHA count ABI");
_Static_assert(offsetof(sha256_ctx, buffer) == 40, "seeded SHA buffer ABI");
static void sha_call(sha256_ctx *ctx, const uint8_t *data, size_t len,
                     uint64_t op, uint8_t *digest) {
    uint64_t input[512], scratch[64], result;
    if (len) memcpy(input, data, len);
    if (CORE_CALL(omsha, &result, (uint8_t *)ctx, 104, (const uint8_t *)input, 4096,
                    len, op, scratch, 64, input, 512) != OMEGA_OK || result) CORE_STOP();
    if (digest) memcpy(digest, scratch, SHA256_DIGEST_SIZE);
}
void sha256_init(sha256_ctx *ctx) { sha_call(ctx, NULL, 0, 0, NULL); }
void sha256_update(sha256_ctx *ctx, const uint8_t *data, size_t len) {
    while (len) {
        size_t n = len > 4096 ? 4096 : len;
        sha_call(ctx, data, n, 1, NULL);
        data += n;
        len -= n;
    }
}
void sha256_final(sha256_ctx *ctx, uint8_t digest[SHA256_DIGEST_SIZE]) {
    sha_call(ctx, NULL, 0, 2, digest);
}
void sha256_hash(const uint8_t *data, size_t len, uint8_t digest[SHA256_DIGEST_SIZE]) {
    sha256_ctx ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, data, len);
    sha256_final(&ctx, digest);
}
#else

#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define EP1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define SIG0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ ((x) >> 3))
#define SIG1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ ((x) >> 10))

static const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static void sha256_transform(sha256_ctx *ctx, const uint8_t *data) {
    uint32_t a, b, c, d, e, f, g, h, t1, t2, m[64];
    size_t i, j;

    for (i = 0, j = 0; i < 16; ++i, j += 4) {
        m[i] = ((uint32_t)data[j] << 24) |
               ((uint32_t)data[j + 1] << 16) |
               ((uint32_t)data[j + 2] << 8) |
               ((uint32_t)data[j + 3]);
    }
    for (; i < 64; ++i) {
        m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];
    }

    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];
    f = ctx->state[5];
    g = ctx->state[6];
    h = ctx->state[7];

    for (i = 0; i < 64; ++i) {
        t1 = h + EP1(e) + CH(e, f, g) + K[i] + m[i];
        t2 = EP0(a) + MAJ(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

void sha256_init(sha256_ctx *ctx) {
    ctx->count = 0;
    ctx->state[0] = 0x6a09e667;
    ctx->state[1] = 0xbb67ae85;
    ctx->state[2] = 0x3c6ef372;
    ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f;
    ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab;
    ctx->state[7] = 0x5be0cd19;
}

void sha256_update(sha256_ctx *ctx, const uint8_t *data, size_t len) {
    size_t buffer_index = (size_t)(ctx->count & 0x3f);
    ctx->count += len;

    if (buffer_index > 0) {
        size_t left = SHA256_BLOCK_SIZE - buffer_index;
        if (len < left) {
            memcpy(&ctx->buffer[buffer_index], data, len);
            return;
        }
        memcpy(&ctx->buffer[buffer_index], data, left);
        sha256_transform(ctx, ctx->buffer);
        data += left;
        len -= left;
    }

    while (len >= SHA256_BLOCK_SIZE) {
        sha256_transform(ctx, data);
        data += SHA256_BLOCK_SIZE;
        len -= SHA256_BLOCK_SIZE;
    }

    if (len > 0) {
        memcpy(ctx->buffer, data, len);
    }
}

void sha256_final(sha256_ctx *ctx, uint8_t digest[SHA256_DIGEST_SIZE]) {
    uint8_t count_bytes[8];
    uint64_t bit_count = ctx->count * 8;
    size_t pad_len;
    size_t buffer_index = (size_t)(ctx->count & 0x3f);
    static const uint8_t padding[64] = { 0x80 };

    for (int i = 0; i < 8; ++i) {
        count_bytes[7 - i] = (uint8_t)(bit_count >> (i * 8));
    }

    pad_len = (buffer_index < 56) ? (56 - buffer_index) : (120 - buffer_index);
    sha256_update(ctx, padding, 1);
    if (pad_len > 1) {
        static const uint8_t zeroes[64] = { 0 };
        sha256_update(ctx, zeroes, pad_len - 1);
    }
    sha256_update(ctx, count_bytes, 8);

    for (int i = 0; i < 8; ++i) {
        digest[i * 4] = (uint8_t)(ctx->state[i] >> 24);
        digest[i * 4 + 1] = (uint8_t)(ctx->state[i] >> 16);
        digest[i * 4 + 2] = (uint8_t)(ctx->state[i] >> 8);
        digest[i * 4 + 3] = (uint8_t)(ctx->state[i]);
    }
}

void sha256_hash(const uint8_t *data, size_t len, uint8_t digest[SHA256_DIGEST_SIZE]) {
    sha256_ctx ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, data, len);
    sha256_final(&ctx, digest);
}
#endif
