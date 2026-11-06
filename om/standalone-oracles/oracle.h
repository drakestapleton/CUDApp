/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_STANDALONE_ORACLE_H
#define OMEGA_STANDALONE_ORACLE_H
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../../src/cudapp_native.h"
#ifndef CUDAPP_MIGRATION_PENDING_C_STANDALONE_ORACLES
#define CUDAPP_MIGRATION_PENDING_C_STANDALONE_ORACLES 0
#endif
static inline uint32_t so_bits(float f) { uint32_t u; memcpy(&u,&f,4); return u; }
static inline float so_float(uint64_t u) { uint32_t v=(uint32_t)u; float f; memcpy(&f,&v,4); return f; }
static inline double so_double(uint64_t u) { double f; memcpy(&f,&u,8); return f; }
#if CUDAPP_MIGRATION_PENDING_C_STANDALONE_ORACLES
/* Frozen scalar C oracle, deliberately default-OFF. Binary128 references in
 * q38-head are separately retained: Omega has no binary128 scalar type. */
static inline uint64_t so_call(unsigned op,uint64_t a,uint64_t b,uint64_t c,uint64_t d) {
    switch(op) {
    case 0:return (uint32_t)a*2654435761u;
    case 1:return UINT32_MAX-7u*(uint32_t)a;
    case 2:return (uint32_t)a+(uint32_t)b;
    case 3:return a==b;
    case 4:return a&&b&&c;
    case 5:return a+b;
    case 6:return (a+c)*b;
    case 7:return (a+255)/256;
    case 8:return a|(b<<32);
    case 9:return 2*(b-a);
    case 10:return (a==1||(a==2&&b>=1024))?17:0;
    case 11:return a==0?4:8;
    case 12:return a>1?a-1:0;
    case 13:return a<2;
    case 14:return a==b&&c==d;
    case 15:return a<=64?4:8;
    case 16:return a<=64?0:4;
    case 17:return UINT64_C(2)<<30;
    case 18:return so_bits((float)((int)(a%7)-3)*0.25f);
    case 19:return so_bits((float)((int)(a%5)-2)*0.5f);
    case 20:{double x=fabs((double)so_float(a)-(double)so_float(b));uint64_t u;memcpy(&u,&x,8);return u;}
    case 21:return so_double(a)>1e-4;
    case 22:return (uint32_t)a*1664525u+1013904223u;
    case 23:return so_bits(((float)((uint32_t)a>>8)/8388608.0f)-1.0f);
    case 24:return (a&0x7fffffffu)<0x7f800000u;
    case 25:{float x=so_float(a),y=so_float(b&0x7fffffffu);return so_bits(y>x?y:x);}
    case 26:return so_bits(b?(float)(a%17):(float)((int)(a%67)-33));
    case 27:return so_bits(b==0?1.0f:b==1?(a==5119?1.0f:0.0f):(a%2?1.0f:-1.0f));
    case 28:return b+!a;
    case 29:return so_float(a)>=so_float(b);
    case 30:return so_float(a)>so_float(b)?a:b;
    case 31:return so_bits((float)((int)(a%173)-90));
    case 32:return so_bits((float)((int)(a%143)-80));
    case 33:return a>b;
    case 34:return a<b;
    case 35:return a-b;
    case 36:return (!a)==(!b);
    case 37:return so_float(a)>so_float(b);
    case 38:return so_double(a)>so_double(b)?a:b;
    case 39:return a%2?(b^0x80000000u):b;
    case 40:return (uint64_t)-(int64_t)(a%64);
    case 41:return a==0?b:c;
    /* op42 narrows only the exactly representable quarter/half fixture inputs. */
    case 42:return a>>16;
    case 43:{float x=so_float(b<<16)*so_float(c<<16);return so_bits(so_float(a)+x);}
    case 44:return a||b||c;
    default:abort();
    }
}
#else
static inline uint64_t so_call(unsigned op,uint64_t a,uint64_t b,uint64_t c,uint64_t d) {
    uint64_t value;
    if(omega_so_oracles(&value,op,a,b,c,d)) abort();
    return value;
}
#endif
static inline int so_eq(uint64_t a,uint64_t b) { return (int)so_call(3,a,b,0,0); }
static inline int so_all(uint64_t a,uint64_t b,uint64_t c) { return (int)so_call(4,a,b,c,0); }
static inline int so_any(uint64_t a,uint64_t b,uint64_t c) { return (int)so_call(44,a,b,c,0); }
static inline unsigned so_bad(unsigned n,int ok) { return (unsigned)so_call(28,(unsigned)ok,n,0,0); }
static inline size_t so_bytes(uint64_t n,uint64_t size,uint64_t tail) {return (size_t)so_call(6,n,size,tail,0);}
#endif
