/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 * Allocation, addresses and bit transport only. Omega defines the hierarchy. */
#if defined(CUDAPP_MIGRATION_PENDING_C_NUMERIC_REST_DIFF)
#undef omega_reduce_identity
#undef omega_reduce_reference
#undef omega_reduce_cpu
#undef omega_reduce_rows_reference
#undef omega_reduce_rows_cpu
#undef omega_reduce_sequential_sum_not_contract
#endif
#if !defined(CUDAPP_MIGRATION_PENDING_C_NUMERIC_REST)
float omega_reduce_identity(OmegaReduceOp op) {
    return omega_bits_to_float((uint32_t)omega_numeric_rest(20, (unsigned)op, 0, 0, 0, 0));
}
static int numeric_reduce_adapter(OmegaReduceOp op, const float *x, size_t n, float *out, unsigned cpu) {
    uint64_t flags = (uint64_t)!!x | ((uint64_t)!!out << 3) | ((uint64_t)omega_numeric_fpenv_ok() << 4);
    int rc = -(int)omega_numeric_rest(22, (unsigned)op, n, flags, 0, 0);
    if (rc) return rc;
    size_t len = n, tiles = omega_numeric_rest(23, op, len, 0, 0, 0);
    const float *cur = x;
    float *owned = NULL;
    uint64_t result = omega_numeric_rest(21, op, 0, 0, 0, 0);
    while (tiles) {
        float *next = malloc(tiles * sizeof(*next));
        if (!next) { free(owned); return OMEGA_NUMERIC_ERR_BAD_ARGS; }
        for (size_t j = 0; j < tiles; ++j) {
            uint64_t tile[32];
            size_t valid = omega_numeric_rest(18, 0, len - j*32, 0, 0, 0);
            for (size_t k = 0; k < valid; ++k) tile[k] = omega_float_to_bits(cur[j*32+k]);
            if (omega_numeric_reduce(&result, op, valid, tile, 32, 0, 0) != OMEGA_OK) __builtin_trap();
            next[j] = omega_bits_to_float((uint32_t)result);
        }
        free(owned);
        owned = next; cur = next; len = tiles;
        tiles = omega_numeric_rest(30, 0, len, 0, 0, 0);
    }
    if (n) result = omega_numeric_rest(25, op, n, result, 0, cpu);
    *out = omega_bits_to_float((uint32_t)result);
    free(owned);
    return OMEGA_NUMERIC_OK;
}
int omega_reduce_reference(OmegaReduceOp op, const float *x, size_t n, float *out) {
    return numeric_reduce_adapter(op, x, n, out, 0);
}
int omega_reduce_cpu(OmegaReduceOp op, const float *x, size_t n, float *out) {
    return numeric_reduce_adapter(op, x, n, out, 1);
}
static int numeric_reduce_rows_adapter(OmegaReduceOp op, const float *x, size_t rows, size_t n,
                                       size_t stride, float *out, unsigned cpu) {
    uint64_t flags = (uint64_t)!!x | ((uint64_t)!!out << 3);
    int rc = -(int)omega_numeric_rest(26, (unsigned)op, n, rows, stride, flags);
    for (size_t r = 0; rc == OMEGA_NUMERIC_OK && r < rows; ++r)
        rc = numeric_reduce_adapter(op, n ? x + r*stride : NULL, n, &out[r], cpu);
    return rc;
}
int omega_reduce_rows_reference(OmegaReduceOp op, const float *x, size_t rows, size_t n, size_t stride, float *out) {
    return numeric_reduce_rows_adapter(op, x, rows, n, stride, out, 0);
}
int omega_reduce_rows_cpu(OmegaReduceOp op, const float *x, size_t rows, size_t n, size_t stride, float *out) {
    return numeric_reduce_rows_adapter(op, x, rows, n, stride, out, 1);
}
float omega_reduce_sequential_sum_not_contract(const float *x, size_t n) {
    uint64_t result = omega_numeric_rest(31, 0, n, 0, 0, 0);
    for (size_t base = 0; base < n;) {
        uint64_t tile[32];
        size_t valid = omega_numeric_rest(18, 0, n - base, 0, 0, 0);
        for (size_t k = 0; k < valid; ++k) tile[k] = omega_float_to_bits(x[base+k]);
        if (omega_numeric_reduce(&result, 0, valid, tile, 32, 1, result) != OMEGA_OK) __builtin_trap();
        base += valid;
    }
    return omega_bits_to_float((uint32_t)result);
}
#endif
