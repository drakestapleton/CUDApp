/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Exhaustive differential check of om/gemmex/num.om against the C number formats of src/omega_gpu_gemm_ex.c.
 * Build: cat om/gemmex/num.om om/gemmex/probe.om > t.om; OMEGA_SEEDED_OMC=$OMEGA_OMC sh $OMEGA_BRIDGE/tools/bridge-seeded c t.om > t.c;
 *        gcc -std=gnu11 -O2 -ffp-contract=off -o h tests/gemm_ex_om_num_test.c t.c -lm; GX_STEP=13 ./h */
#include <string.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define OMEGA_GEMM_DT_F32 0
#define OMEGA_GEMM_DT_F16 2

static float u2f(uint32_t u) { float f; memcpy(&f, &u, 4); return f; }
static uint32_t f2u(float f) { uint32_t u; memcpy(&u, &f, 4); return u; }
static int fin(float f) { return (f2u(f) & 0x7f800000u) != 0x7f800000u; }
static double dabs(double v) { return v < 0 ? -v : v; }

static float h2f(uint16_t h) {
    uint32_t s = (uint32_t)(h >> 15) << 31, e = (h >> 10) & 31u, m = h & 0x3ffu;
    if (e == 31u) return u2f(s | 0x7f800000u | m << 13);
    if (e == 0u) { float v = (float)m * (1.0f / 16777216.0f); return s ? -v : v; } /* m 2^-24, exact */
    return u2f(s | (e + 112u) << 23 | m << 13);
}
static float e4m3f(uint8_t b) {
    uint32_t e = (b >> 3) & 15u, m = b & 7u;
    if (e == 15u && m == 7u) return u2f(0x7fc00000u);
    float v = e == 0u ? (float)m * (1.0f / 512.0f) : u2f((e + 120u) << 23 | m << 20);
    return b & 0x80u ? -v : v;
}
static uint16_t f2bf_rne(float f) {
    uint32_t u = f2u(f);
    if (!fin(f)) return (uint16_t)(u >> 16 | ((u & 0x7fffffu) ? 0x40u : 0u));
    return (uint16_t)((u + 0x7fffu + ((u >> 16) & 1u)) >> 16);
}
static uint16_t f2h_rne(float f) {
    uint32_t u = f2u(f), s = (u >> 16) & 0x8000u, a = u & 0x7fffffffu;
    if (a >= 0x7f800000u) return (uint16_t)(s | 0x7c00u | (a > 0x7f800000u ? 0x200u : 0u));
    if (a >= 0x477ff000u) return (uint16_t)(s | 0x7c00u);      /* >= 65520 rounds past the f16 maximum */
    if (a < 0x38800000u) {                                     /* below 2^-14: an f16 subnormal (unit 2^-24) or zero */
        if (a < 0x33000000u) return (uint16_t)s;               /* below 2^-25 */
        uint32_t mant = (a & 0x7fffffu) | 0x800000u, sh = 126u - (a >> 23);
        uint32_t q = mant >> sh, rem = mant & ((1u << sh) - 1u), half = 1u << (sh - 1u);
        if (rem > half || (rem == half && (q & 1u))) q++;
        return (uint16_t)(s | q);
    }
    uint32_t r = a + 0xfffu + ((a >> 13) & 1u);
    return (uint16_t)(s | (((r >> 23) - 112u) << 10 | ((r >> 13) & 0x3ffu)));
}
/* double to float rounding to odd: one more rounding to bf16 or f16 is then the correct rounding of v */
static float d2f_odd(double v) {
    float f = (float)v;
    if (v != v || !fin(f) || (double)f == v) return f;
    uint32_t u = f2u(f);
    if (u & 1u) return f;
    return u2f(dabs(v) > dabs((double)f) ? u + 1u : u - 1u);
}
/* exact split of a finite f32 into three bf16 planes by truncation: f = hi + mid + lo, except the parts below 2^-133 */
static void split3(float f, uint16_t p[3]) {
    if (!fin(f)) { p[0] = p[1] = p[2] = 0; return; }
    uint32_t u = f2u(f);
    p[0] = (uint16_t)(u >> 16);
    float r1 = f - u2f(u & 0xffff0000u);
    uint32_t u1 = f2u(r1);
    p[1] = (uint16_t)(u1 >> 16);
    p[2] = (uint16_t)(f2u(r1 - u2f(u1 & 0xffff0000u)) >> 16);
}
uint32_t omega_gx_probe(uint64_t *result, uint64_t p0, uint64_t p1);
static uint64_t P(uint64_t op, uint64_t a){ uint64_t r; if (omega_gx_probe(&r, op, a)) { printf("trap\n"); exit(2);} return r; }
static uint64_t bad=0, n=0;
#define CK(c,msg,x) do{ n++; if(!(c)){ if(bad++<5) printf("FAIL %s %llx\n",msg,(unsigned long long)(x)); } }while(0)
int main(void){
  for (uint32_t h=0; h<65536; h++) CK(P(0,h)==f2u(h2f(h)),"h2f",h);
  for (uint32_t b=0; b<256; b++) CK(P(1,b)==f2u(e4m3f(b)),"e4m3",b);
  uint64_t step = getenv("GX_STEP") ? strtoull(getenv("GX_STEP"),0,10) : 13;
  for (uint64_t x=0; x<(1ull<<32); x+=step) { uint32_t u=(uint32_t)x;
    CK(P(2,u)==f2bf_rne(u2f(u)),"f2bf",u);
    CK(P(3,u)==f2h_rne(u2f(u)),"f2h",u);
    uint16_t p[3]; split3(u2f(u),p); CK(P(4,u)==((uint64_t)p[0]|(uint64_t)p[1]<<16|(uint64_t)p[2]<<32),"split",u); }
  uint64_t s=88172645463325252ull;
  for (int i=0;i<20000000;i++){ s^=s<<13; s^=s>>7; s^=s<<17; uint64_t d=s;
    if (i&1) { double f=(double)u2f((uint32_t)(s>>32)); memcpy(&d,&f,8); d ^= (s & 0xfff) * (i&2?0:1); }
    double v; memcpy(&v,&d,8); CK(P(5,d)==f2u(d2f_odd(v)),"d2fo",d); }
  static const uint32_t m[5]={0x7f800000u,0x7f80u,0x7c00u,0x7fu,0x7cu};
  for (uint64_t t=0;t<5;t++) for (uint32_t u=0;u<65536;u++){ uint32_t v= t==0 ? u<<16|u : u; CK(P(6,t<<32|v)==((v&m[t])==m[t]),"bad",t<<32|v);}
  printf("%s gemmex num: %llu checks, %llu failed\n", bad?"FAIL":"PASS", (unsigned long long)n,(unsigned long long)bad); return bad!=0;
}
