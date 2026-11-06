/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_GPU_API_HOSTREST_H
#define OMEGA_GPU_API_HOSTREST_H
#include "cudapp_native.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
/* Default-OFF frozen C oracle. Pointer acquisition and effects stay in callers. */
#if defined(CUDAPP_MIGRATION_PENDING_C_API_FP8_GDN_HOST) || defined(CUDAPP_API_HOSTREST_DIFF)
static inline uint64_t omega_ah_c(unsigned op, const uint64_t p[16]) {
    const uint32_t a=p[0], b=p[1], c=p[2], d=p[3], e=p[4];
    switch (op) {
    case 0: return p[1]==0 && p[0]=='0' ? 0 : p[1]==0 && p[0]=='1' ? 1 : p[2];
    case 1: return p[0]=='2' && !p[1] ? 2 : 1;
    case 2: return b ? b : c==0 ? (a%512==0 ? 16 : 8) : c==1 ? 1 : 8;
    case 3: return p[0]*p[1]<=65536;
    case 4: { if (b>0 && b<=256) return b; if (!p[2] || !a) return 6; uint32_t n=48/a; if (!n) n=1; return n<6?n:6; }
    case 5: return b<'0'||b>'9'||a>=1000 ? UINT64_C(4294967296) : (uint32_t)(a*10u+b-'0');
    case 6: return c ? c : p[4]==2 ? 16 : b>1 ? 32 : b==1 && a>4096 && p[3] ? 16 : 8;
    case 7: return p[0] && b*c<=32;
    case 8: {
        uint32_t ks=a/(32u*b);
        int partial=ks*c*b>128u && a>4096u && e==1 && p[5];
        if (partial) { do ks--; while (ks>1u && (a/(32u*b)%ks || ks*c*b>128u)); if (ks<2u) return 0; }
        if (ks*c*b>128u || a%(32u*d) || a>512u*d) return 0;
        return ks | ((uint64_t)partial<<32);
    }
    case 9: return a%b || !p[2];
    case 10: {
        uint32_t gx=(a/b+c-1u)/c, budget=d;
        if (p[4]) { uint32_t waves=(gx+383u)/384u; if (waves<2u) waves=2u; gx=(gx+waves-1u)/waves; budget=gx; }
        if (gx>budget) gx=budget;
        return gx ? gx : 1;
    }
    case 11: return p[0] && p[1] ? 1 : p[0] && p[2] ? 2 : 0;
    case 12: return (p[0]!=4 && p[0]!=5)||!p[1]||!p[2]||p[2]%p[3]||p[2]/p[3]>0x7fffffffu ? 0 : p[2]/p[3];
    case 13: return !(a>=b || c<d || c>e);
    case 14: return !(p[0] && (!p[1] || p[2]<p[3]));
    case 16: for (unsigned i=0;i<8;i++) if(p[i]) return 0; return 1;
    case 17: return p[0] && (p[1]==1 || p[1]==p[0]);
    case 18: return p[0] ? p[1] : p[2]>1 ? (p[3]==2 ? 12 : p[4]) : 256;
    case 19: return !p[0] && p[1]==1 && p[2]>4096 && p[3];
    case 20: return p[0]==1 && p[1] && p[2]<=p[3];
    case 21: return p[0]>p[1];
    case 22: return p[0]/p[1];
    case 23:
        if(p[0]==0) return !p[1] || (p[2] && p[3] && p[4]);
        if(p[0]==1) return p[1] && (p[2] || p[3]) && (p[4] || p[5] || p[6]) && (p[7] || p[8]);
        return p[1] ? p[2] && p[3] && p[4] : p[5] && p[6] && p[7] && p[8];
    case 24: return (uint32_t)(a/(32u*b)*c*b)>128u && a>4096u && d==1;
    default: return d==0 ? (a+b-1u)/b : d==1 ? c : a;
    }
}
#endif
static inline uint64_t omega_ah_call(unsigned op, const uint64_t p[16]) {
#if defined(CUDAPP_MIGRATION_PENDING_C_API_FP8_GDN_HOST) && !defined(CUDAPP_API_HOSTREST_DIFF)
    return omega_ah_c(op,p);
#else
    uint64_t r=0;
    if (omega_ah_rest(&r,op,p,16)!=OMEGA_OK) abort();
#ifdef CUDAPP_API_HOSTREST_RED
    r ^= 1;
#endif
#ifdef CUDAPP_API_HOSTREST_DIFF
    if (r!=omega_ah_c(op,p)) { fprintf(stderr,"hostrest differential op=%u\n",op); abort(); }
#endif
    return r;
#endif
}
#define OMEGA_AH(op, ...) omega_ah_call((op), (const uint64_t[16]){__VA_ARGS__})
static inline uint64_t omega_ah_flag(const char *s, unsigned dflt) {
    return OMEGA_AH(0, s ? (unsigned char)s[0] : 0, s && s[0] ? (unsigned char)s[1] : 1, dflt);
}
#endif
