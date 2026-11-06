/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Included after the private effect types. No plan/layout arithmetic lives here:
 * Omega emits bounded unchunked records, then expands each launch on demand. */
/* Like g_words, this byte-storage workspace is protected by the session lock.
 * many_build materializes owned launches/tables before any public callback. */
static uint64_t g_mhplan[8192];

static int rng_args_ok(OmegaGpuRngKind kind, uint32_t nstreams, uint32_t per) {
    return -(int)omega_mh_rng((unsigned)kind, nstreams, per, OMEGA_GPU_RNG_NORMAL);
}

int omega_gpu_rng_codegen(OmegaGpuRngKind kind, OmegaGpuMathInfo *info) {
    if (rng_args_ok(kind, 1, 1)) return OMEGA_GPU_ML_BAD_ARGS;
    return codegen_only(0, (uint64_t)kind, 0, OMEGA_GPU_RNG_THREADS, info);
}

int omega_gpu_rng_generate(OmegaGpuRngKind kind, uint64_t seed, uint64_t stream0, uint32_t nstreams,
                           uint32_t offset, uint32_t per_stream, void *out, OmegaGpuMathInfo *info) {
    if (!out) return OMEGA_GPU_ML_BAD_ARGS;
    int rc = rng_args_ok(kind, nstreams, per_stream);
    if (rc) return rc;
    uint64_t t0 = now_ns();
    size_t count = omega_mh_rngval(kind, nstreams, per_stream, 0);
    size_t esize = omega_mh_rngval(kind, nstreams, per_stream, 1);
    size_t bytes = omega_mh_mul(count, esize);
    omega_gpu_session_lock();
    Slot *s;
    rc = begin(0, (uint64_t)kind, 0, OMEGA_GPU_RNG_THREADS, &s, info);
    if (!rc && omega_gpu_session_scratch(&g_out, bytes)) rc = OMEGA_GPU_ML_CHIP_FAIL;
    if (!rc) {
        uint32_t *dev = (uint32_t *)g_out.mem.cpu;
        size_t nw = omega_mh_div(bytes, 4);
        for (size_t i = 0; i < nw; i++) dev[i] = ML_POISON32;
        if (omega_gpu_session_sync_to_gpu(&g_out.mem)) rc = OMEGA_GPU_ML_CHIP_FAIL;
        uint32_t args[13];
        put64(args, g_out.mem.va);
        for (uint32_t i = 0; i < 4; i++) put64(&args[2 + 2 * i], omega_mh_seed(seed, stream0, i));
        args[10] = offset; args[11] = per_stream; args[12] = nstreams;
        if (!rc) rc = launch(s, args, 13, omega_mh_rngval(kind, nstreams, per_stream, 2), count, 0, info);
        if (!rc && omega_gpu_session_sync_to_cpu(&g_out.mem)) rc = OMEGA_GPU_ML_CHIP_FAIL;
        if (!rc) {
            memcpy(out, g_out.mem.cpu, bytes);
            uint64_t unwritten = 0;
            for (size_t i = 0; i < count; i++) {
                uint64_t v = esize == 8 ? ((const uint64_t *)out)[i] : ((const uint32_t *)out)[i];
                unwritten = omega_mh_add(unwritten, omega_mh_poison(v, 0, esize == 8, 0));
            }
            if (info) info->unwritten_words = unwritten;
            if (unwritten) rc = OMEGA_GPU_ML_UNWRITTEN;
        }
    }
    omega_gpu_session_unlock();
    if (info) info->call_ns = omega_mh_sub(now_ns(), t0);
    return rc;
}

int omega_gpu_fft_c2c(const float *in, float *out, uint32_t n, uint32_t batch, int inverse, OmegaGpuMathInfo *info) {
    uint64_t v[8] = {0};
    int rc = -(int)omega_mh_fft((uintptr_t)in, (uintptr_t)out, n, batch, (unsigned)inverse, v);
    if (rc == -4) return many_basic(OMEGA_GPU_FFT_TYPE_C2C, 1, &n, batch, inverse, in, out, info);
    if (rc) return rc;
    float *tw = (float *)malloc(v[3]);
    if (!tw) return OMEGA_GPU_ML_CHIP_FAIL;
    rc = omega_gpu_fft_twiddles(n, inverse, tw);
    if (rc) { free(tw); return rc; }
    uint64_t t0 = now_ns();
    omega_gpu_session_lock();
    Slot *s;
    rc = begin(1, v[0], (uint64_t)inverse, v[4], &s, info);
    if (!rc && (omega_gpu_session_scratch(&g_in, v[2]) || omega_gpu_session_scratch(&g_out, v[2]) ||
                omega_gpu_session_scratch(&g_tw, v[3]))) rc = OMEGA_GPU_ML_CHIP_FAIL;
    if (!rc) {
        memcpy(g_in.mem.cpu, in, v[2]);
        memcpy(g_tw.mem.cpu, tw, v[3]);
        uint32_t *dev = (uint32_t *)g_out.mem.cpu;
        for (size_t i = 0; i < v[1]; i++) dev[i] = ML_POISON32;
        if (omega_gpu_session_sync_to_gpu(&g_in.mem) || omega_gpu_session_sync_to_gpu(&g_tw.mem) ||
            omega_gpu_session_sync_to_gpu(&g_out.mem)) rc = OMEGA_GPU_ML_CHIP_FAIL;
        for (uint64_t first = 0; first < batch && !rc; first += OMEGA_GPU_FFT_MAX_CTAS) {
            uint32_t args[6];
            uint64_t off = omega_mh_mul(first, v[3]);
            uint32_t grid = omega_mh_min(omega_mh_sub(batch, first), OMEGA_GPU_FFT_MAX_CTAS);
            put64(args, omega_mh_add(g_in.mem.va, off));
            put64(&args[2], omega_mh_add(g_out.mem.va, off));
            put64(&args[4], g_tw.mem.va);
            rc = launch(s, args, 6, grid, omega_mh_mul(omega_mh_mul(grid, n), 2), v[3], info);
        }
        if (!rc && omega_gpu_session_sync_to_cpu(&g_out.mem)) rc = OMEGA_GPU_ML_CHIP_FAIL;
        if (!rc) {
            memcpy(out, dev, v[2]);
            uint64_t unwritten = 0;
            for (size_t i = 0; i < v[1]; i++) unwritten = omega_mh_add(unwritten, omega_mh_poison(dev[i], 0, 0, 0));
            if (info) info->unwritten_words = unwritten;
            if (unwritten) rc = OMEGA_GPU_ML_UNWRITTEN;
        }
    }
    omega_gpu_session_unlock();
    free(tw);
    if (info) info->call_ns = omega_mh_sub(now_ns(), t0);
    return rc;
}

int omega_gpu_philox_generate_dist(OmegaGpuPhiloxDist dist, uint64_t seed, uint64_t subseq0, uint32_t nstreams,
                                   uint64_t offset, uint32_t per_stream, void *out, OmegaGpuMathInfo *info) {
    if (!out) return OMEGA_GPU_ML_BAD_ARGS;
    int rc = -(int)omega_mh_rng((unsigned)dist, nstreams, per_stream, OMEGA_GPU_PHILOX_NORMAL);
    if (rc) return rc;
    uint64_t t0 = now_ns();
    size_t count = omega_mh_phil(dist, nstreams, per_stream, offset, 0);
    size_t bytes = omega_mh_mul(count, 4);
    uint32_t esz = omega_mh_phil(dist, nstreams, per_stream, offset, 5);
    uint32_t *words = (uint32_t *)out;
    omega_gpu_session_lock();
    Slot *s;
    rc = begin(3, dist, omega_mh_phil(dist, nstreams, per_stream, offset, 1), OMEGA_GPU_PHILOX_THREADS, &s, info);
    if (!rc && omega_gpu_session_scratch(&g_out, bytes)) rc = OMEGA_GPU_ML_CHIP_FAIL;
    if (!rc) {
        uint32_t args[11];
        put64(args, g_out.mem.va); put64(&args[2], seed); put64(&args[4], subseq0);
        put64(&args[6], omega_mh_phil(dist, nstreams, per_stream, offset, 2));
        args[8] = omega_mh_phil(dist, nstreams, per_stream, offset, 3);
        args[9] = nstreams;
        args[10] = omega_mh_phil(dist, nstreams, per_stream, offset, 4);
        uint32_t grid = omega_mh_phil(dist, nstreams, per_stream, offset, 6);
        rc = philox_pass(s, args, grid, count, ML_POISON32, words, info);
        uint64_t unwritten = 0;
        if (dist == OMEGA_GPU_PHILOX_RAW) {
            uint64_t hits = 0;
            for (size_t i = 0; !rc && i < count; i++) hits = omega_mh_add(hits, omega_mh_poison(words[i], 0, 0, 0));
            if (!rc && hits) {
                uint32_t *first = (uint32_t *)malloc(bytes);
                if (!first) rc = OMEGA_GPU_ML_CHIP_FAIL;
                if (first) {
                    memcpy(first, words, bytes);
                    rc = philox_pass(s, args, grid, count, ML_POISON32_ALT, words, info);
                    for (size_t i = 0; !rc && i < count; i++)
                        unwritten = omega_mh_add(unwritten, omega_mh_poison(first[i], words[i], 0, 1));
                    free(first);
                }
            }
        } else if (esz == 1) {
            for (size_t i = 0; !rc && i < count; i++) unwritten = omega_mh_add(unwritten, omega_mh_poison(words[i], 0, 0, 0));
        } else {
            const uint64_t *d = (const uint64_t *)out;
            size_t nd = omega_mh_div(count, 2);
            for (size_t i = 0; !rc && i < nd; i++) unwritten = omega_mh_add(unwritten, omega_mh_poison(d[i], 0, 1, 0));
        }
        if (info) info->unwritten_words = unwritten;
        if (!rc && unwritten) rc = OMEGA_GPU_ML_UNWRITTEN;
    }
    omega_gpu_session_unlock();
    if (info) info->call_ns = omega_mh_sub(now_ns(), t0);
    return rc;
}

static int walk_locked(OmegaGpuFftPlanMode mode, uint32_t ndim, const uint32_t *dims, uint32_t batch, int inverse,
                       size_t bytes[3], OmegaGpuFftLaunchFn fn, void *ctx) {
    if (!dims || !bytes) return OMEGA_GPU_ML_BAD_ARGS;
    uint64_t p[8] = {mode, ndim, batch, (unsigned)inverse, 0, 0, 0, 0}, v[8] = {0};
    if (ndim <= 3) for (uint32_t i = 0; i < ndim; i++) p[4 + i] = dims[i];
    int rc = -(int)omega_mh_walk(p, v);
    if (rc) return rc;
    for (uint32_t i = 0; i < 3; i++) bytes[i] = v[1 + i];
    rc = omega_gpu_om_ir_data(6, v[4], v[0], (uint64_t)inverse, g_words);
    if (rc == OMEGA_OM_IR_REFUSED) return OMEGA_GPU_ML_BAD_ARGS;
    if (rc != OMEGA_OM_IR_OK || !g_words[0] || g_words[0] > 6) return OMEGA_GPU_ML_CODEGEN_FAIL;
    uint64_t steps[6];
    size_t nsteps = g_words[0];
    memcpy(steps, &g_words[1], nsteps * sizeof *steps);
    for (size_t i = 0; fn && i < nsteps; i++) {
        uint64_t d[16] = {0};
        for (uint64_t cursor = 0;; cursor += OMEGA_GPU_FFT_MAX_CTAS) {
            rc = -(int)omega_mh_walkstep(steps[i], batch, cursor, d);
            if (rc) return rc;
            if (cursor >= d[15]) break;
            OmegaGpuFftLaunch l = {0};
            l.kind=d[0]; l.a=d[1]; l.b=d[2]; l.src=d[3]; l.dst=d[4];
            l.src_off=d[5]; l.dst_off=d[6]; l.threads=d[7]; l.grid=d[8];
            l.shared_bytes=d[9]; l.total=d[10]; l.first=d[11]; l.tw_n=d[12]; l.tw_inverse=d[13]; l.elements=d[14];
            if (l.tw_n) {
                if (!cursor) {
                    rc = twiddles_locked(l.a, l.b);
                    if (rc) return rc;
                }
                l.tw = g_twf;
            }
            rc = fn(ctx, &l);
            if (rc) return rc;
        }
    }
    return OMEGA_GPU_ML_OK;
}

static int many_each(uint32_t rank, const Lay *l, const uint32_t *ext, uint32_t batch,
                     int (*f)(void *, uint32_t), void *ctx) {
    uint64_t layout[8] = {l->base, l->bs, l->st[0], l->st[1], l->st[2], 0, 0, 0};
    uint64_t shape[4] = {ext[0], ext[1], ext[2], 1};
    uint64_t count = omega_mh_eachcnt(shape, rank, batch);
    for (uint64_t i = 0; i < count; i++) {
        int rc = f(ctx, omega_mh_index(layout, shape, rank, i));
        if (rc) return rc;
    }
    return 0;
}

static int many_mark(void *ctx, uint32_t idx) {
    uint8_t *bits = (uint8_t *)ctx;
    size_t at = omega_mh_div(idx, 8);
    uint64_t v = omega_mh_fill(bits[at], idx);
    if (v > UINT8_MAX) return 1;
    bits[at] = (uint8_t)v;
    return 0;
}

static int many_build(const OmegaGpuFftMany *p, int in_place, int alias, Many *m, size_t bytes[OMEGA_GPU_FFT_MANY_BUFS],
                      ManyOut *out) {
    memset(m, 0, sizeof *m);
    if (!p || !bytes) return OMEGA_GPU_ML_BAD_ARGS;
    uint64_t args[32] = {p->rank, p->batch, (unsigned)p->type, (unsigned)p->inverse, in_place, alias};
    for (uint32_t i = 0; i < 3; i++) { args[6+i]=p->n[i]; args[9+i]=p->inembed[i]; args[14+i]=p->onembed[i]; }
    args[12]=p->istride; args[13]=p->idist; args[17]=p->ostride; args[18]=p->odist;
    /* The caller holds the session lock, so the fixed ABI workspace needs
     * neither a per-plan allocation nor a large per-thread stack frame. */
    uint64_t *d = g_mhplan;
    int rc = -(int)omega_mh_many(args, d);
    if (rc) return rc;
    ManyOut o = {0};
    o.rank=d[13]; o.batch=d[14]; o.owords=d[15]; o.oextent=d[16]; o.obasic=d[17];
    for (uint32_t i=0; i<3; i++) o.oext[i]=d[18+i];
    o.olay.base=d[21]; o.olay.bs=d[22];
    for (uint32_t i=0; i<3; i++) o.olay.st[i]=d[23+i];
    if (!o.obasic) {
        size_t nb = omega_mh_div(omega_mh_add(o.oextent, 7), 8);
        uint8_t *bits = (uint8_t *)calloc(nb, 1);
        if (!bits) rc = OMEGA_GPU_ML_CHIP_FAIL;
        else { if (many_each(o.rank, &o.olay, o.oext, o.batch, many_mark, bits)) rc = OMEGA_GPU_ML_BAD_ARGS; free(bits); }
    }
    if (!rc) {
        for (uint32_t i=0; i<OMEGA_GPU_FFT_MANY_BUFS; i++) bytes[i]=d[5+i];
        m->tab = (uint64_t *)malloc(bytes[OMEGA_GPU_FFT_BUF_TABLE]);
        if (!m->tab) rc = OMEGA_GPU_ML_CHIP_FAIL;
    }
    for (uint64_t i=0; !rc && i<d[0]; i++) {
        const uint64_t *r = &d[32 + 40*i]; /* serialized record addressing, not plan arithmetic */
        if (r[0] == 10 || r[0] == 11) {
            if (omega_gpu_om_ir_data(r[0], r[1], r[2], 0, g_words) != OMEGA_OM_IR_OK) rc = OMEGA_GPU_ML_CODEGEN_FAIL;
            else memcpy((uint8_t *)m->tab + r[5], g_words, omega_mh_mul(r[1], 8));
            continue;
        }
        OmegaGpuFftManyLaunch l = {0};
        l.kind=r[0]; l.a=r[1]; l.b=r[2]; l.nptr=r[3]; l.nwords=r[4]; l.threads=OMEGA_GPU_FFT_MANY_THREADS;
        for (uint32_t k=0; k<3; k++) { l.buf[k]=r[7+k]; l.off[k]=r[10+k]; }
        for (uint32_t k=0; k<18; k++) l.words[k]=r[13+k];
        l.words[0]=r[5];
        uint64_t ctas=omega_mh_div(omega_mh_add(r[5], OMEGA_GPU_FFT_MANY_THREADS-1), OMEGA_GPU_FFT_MANY_THREADS);
        for (uint64_t first=0; !rc && first<ctas; first+=OMEGA_GPU_FFT_MAX_CTAS) {
            l.grid=omega_mh_chunk(r[5], first, OMEGA_GPU_FFT_MANY_THREADS, r[6], 0);
            l.words[1]=omega_mh_chunk(r[5], first, OMEGA_GPU_FFT_MANY_THREADS, r[6], 1);
            l.elements=omega_mh_chunk(r[5], first, OMEGA_GPU_FFT_MANY_THREADS, r[6], 2);
            if (m->n == m->cap) {
                size_t cap = m->cap ? omega_mh_mul(m->cap, 2) : 64;
                OmegaGpuFftManyLaunch *v = (OmegaGpuFftManyLaunch *)realloc(m->l, omega_mh_mul(cap, sizeof *v));
                if (!v) { rc=OMEGA_GPU_ML_CHIP_FAIL; break; }
                m->l=v; m->cap=cap;
            }
            m->l[m->n++]=l;
        }
    }
    if (!rc && out) *out=o;
    return rc;
}
