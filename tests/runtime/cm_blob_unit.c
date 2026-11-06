/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "cudapp_native.h"
#include "sha256.h"
#include "runtime/rx_costmodel.h"
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void oracle_cm_init(RxCostModel *, unsigned);
int oracle_cm_serialize(const RxCostModel *, uint8_t *, size_t, size_t *);
int oracle_cm_deserialize(RxCostModel *, const uint8_t *, size_t);
void oracle_cm_digest(const RxCostModel *, uint8_t[32]);

static uint64_t step(unsigned mode, uint64_t s[16], uint8_t b[128]) {
    uint64_t result;
    assert(!omega_cm_digest(&result, mode, s, 16, b, 128));
    return result;
}
static void digest(const uint8_t *p, size_t n, uint8_t out[32]) {
    uint64_t state[16];
    uint8_t block[128];
    assert(!step(0, state, block));
    size_t offset = 0;
    for (size_t i = 0; i < n / 64; ++i) {
        memcpy(block, p + offset, 64);
        assert(!step(1, state, block));
        offset += 64;
    }
    state[9] = n - offset;
    memcpy(block, p + offset, n - offset);
    assert(!step(2, state, block));
    memcpy(out, block, 32);
}
static uint64_t policy(unsigned mode, uint64_t w[64]) {
    uint64_t result;
    assert(!omega_cm_blob(&result, mode, w, 64));
    return result;
}
static uint64_t pair(uint32_t a, uint32_t b, unsigned big) {
    return big ? ((uint64_t)a << 32) | b : ((uint64_t)b << 32) | a;
}
static unsigned header_ok(const uint64_t w[64], unsigned big) {
    uint32_t h[8];
    for (unsigned i = 0; i < 4; ++i) {
        h[2*i] = (uint32_t)(big ? w[16+i] >> 32 : w[16+i]);
        h[2*i+1] = (uint32_t)(big ? w[16+i] : w[16+i] >> 32);
    }
    return h[0] == 0x4d43474f && h[1] == 1 && h[2] > 0 && h[2] <= 8 &&
           h[3] == 8 && h[4] == 9 && h[5] == 8 && h[6] == 3 ? h[2] : 0;
}
int main(void) {
    uint8_t *input = malloc(65536);
    assert(input);
    uint64_t rng = 1;
    for (size_t i = 0; i < 65536; ++i) {
        rng = rng * UINT64_C(6364136223846793005) + 1;
        input[i] = (uint8_t)(rng >> 32);
    }
    const size_t lengths[] = {0,1,55,56,57,63,64,65,127,128,129,4095,4096,4097,28112,65536};
    for (size_t i = 0; i < sizeof lengths / sizeof *lengths; ++i) {
        uint8_t actual[32], expected[32];
        digest(input, lengths[i], actual);
        sha256_hash(input, lengths[i], expected);
        assert(!memcmp(actual, expected, 32));
    }
    for (size_t n = 0; n < 256; ++n) {
        uint8_t a[32], b[32];
        digest(input, n, a);
        sha256_hash(input, n, b);
        assert(!memcmp(a, b, 32));
    }
    uint64_t state[16] = {0};
    uint8_t block[128] = {0};
    assert(step(3, state, block) == 1);
    for (unsigned i = 0; i < 256; ++i) {
        block[32+i/8] ^= (uint8_t)(1u << (i%8));
        assert(step(3, state, block) == 0);
        block[32+i/8] ^= (uint8_t)(1u << (i%8));
    }
    state[9] = 64;
    assert(step(2, state, block) == 1);
    for (unsigned big = 0; big < 2; ++big) {
        for (unsigned n = 0; n < 11; ++n) {
            uint64_t words[64] = {0};
            words[0] = n;
            words[1] = UINT64_C(4562254508917369340);
            words[63] = big;
            assert(policy(0, words) == 1);
            uint64_t expected[5] = {pair(0x4d43474f,1,big),pair(n,8,big),pair(9,8,big),pair(3,0,big),words[1]};
            assert(!memcmp(words+16, expected, sizeof expected));
            assert(policy(1, words) == header_ok(words, big));
            for (unsigned j = 0; j < 5; ++j) for (unsigned bit = 0; bit < 64; ++bit) {
                words[16+j] ^= UINT64_C(1) << bit;
                assert(policy(1, words) == header_ok(words, big));
                words[16+j] ^= UINT64_C(1) << bit;
            }
        }
    }
    const double scales[] = {-INFINITY,-1,-0.0,0,nextafter(1,0),1,nextafter(1,2),nextafter(1e6,0),1e6,INFINITY,NAN};
    for (size_t i = 0; i < sizeof scales / sizeof *scales; ++i) {
        uint64_t words[64] = {0};
        memcpy(words, scales+i, sizeof(double));
        assert(policy(2, words) == (uint64_t)(scales[i] >= 1 && scales[i] < 1e6));
    }
    const uint64_t sizes[] = {0,1,28111,28112,28113,UINT64_MAX};
    for (size_t i = 0; i < sizeof sizes / sizeof *sizes; ++i) {
        uint64_t words[64] = {sizes[i],28112};
        assert(policy(3, words) == (uint64_t)(sizes[i] >= 28112));
        assert(policy(4, words) == (uint64_t)(sizes[i] == 28112));
    }
    const size_t blob_size = rx_cm_blob_size();
    assert(blob_size <= 32768);
    for (unsigned arms = 0; arms < 11; ++arms) {
        RxCostModel actual, expected;
        rx_cm_init(&actual, arms);
        oracle_cm_init(&expected, arms);
        assert(!memcmp(&actual, &expected, sizeof actual));
        for (size_t cap = blob_size - 1; cap <= blob_size + 1; ++cap) {
            size_t an = 0, en = 0;
            int ar = rx_cm_serialize(&actual, input, cap, &an);
            int er = oracle_cm_serialize(&actual, input + 32768, cap, &en);
            assert(ar == er && an == en);
            if (!ar) assert(!memcmp(input, input + 32768, an));
        }
        uint8_t ad[32], ed[32];
        rx_cm_digest(&actual, ad);
        oracle_cm_digest(&actual, ed);
        assert(!memcmp(ad, ed, 32));
        assert(rx_cm_deserialize(&actual, input, blob_size) ==
               oracle_cm_deserialize(&expected, input, blob_size));
        assert(!memcmp(&actual, &expected, sizeof actual));
    }
    for (size_t i = 0; i < 32; ++i) {
        RxCostModel actual, expected;
        rx_cm_init(&actual, 3);
        expected = actual;
        size_t length;
        assert(!oracle_cm_serialize(&actual, input, blob_size, &length));
        input[blob_size - 32 + i] ^= 1;
        assert(rx_cm_deserialize(&actual, input, blob_size) == RX_CM_ERR_DIGEST);
        assert(oracle_cm_deserialize(&expected, input, blob_size) == RX_CM_ERR_DIGEST);
        assert(!memcmp(&actual, &expected, sizeof actual));
    }
    free(input);
    puts("CM BLOB/DIGEST differential PASS (native/big-endian headers, bit mutants, SHA-256 block/tail boundaries)");
    return 0;
}
