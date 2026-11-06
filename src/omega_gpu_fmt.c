/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* omega_gpu_fmt.c -- see omega_gpu_fmt.h. Freestanding: stdarg only. */
#include "omega_gpu_fmt.h"
#include <stdarg.h>
#include <stdint.h>
/* Freestanding (OMEGA_GPU_NO_POSIX) keeps the C oracle: MIGRATION-PENDING, see omega_core_binding.h. */
#ifndef OMEGA_GPU_NO_POSIX
#ifndef CUDAPP_MIGRATION_PENDING_C_CORE
#include "omega_core_binding.h"
#include <stdlib.h>
#include <string.h>
#endif
#endif
#if !defined(CUDAPP_MIGRATION_PENDING_C_CORE) && !defined(OMEGA_GPU_NO_POSIX)
static void fmt_status(uint32_t r) { if(r!=OMEGA_OK)CORE_STOP(); }
static void fmt_copy(char *out,size_t cap,size_t *used,const char *p,size_t len) {
    uint64_t n;fmt_status(CORE_CALL(omclip, &n,cap,*used,len));
    memcpy(out+*used,p,(size_t)n);*used+=(size_t)n;
}
int omega_gpu_fmt(char *out,size_t cap,const char *fmt,...) {
    if(!out||!cap)return 0;
    if(!fmt)fmt="";
    size_t used=0;va_list ap;va_start(ap,fmt);
    while(*fmt) {
        uint8_t input[4]={0};size_t avail=0;
        while(avail<4&&fmt[avail])avail++;
        memcpy(input,fmt,avail);
        uint64_t d[4];fmt_status(CORE_VOID(omfmt, input,4,d,4));
        uint64_t kind=d[1],size=d[2],bits=0;
        char number[24],ch;const char *p=number;size_t len=0;
        switch(kind) {
        case 0: p="%";len=1;break;
        case 1: p=va_arg(ap,const char *);if(!p)p="(null)";len=strlen(p);break;
        case 2: ch=(char)va_arg(ap,int);p=&ch;len=1;break;
        case 3:
            if(size==3)bits=(uint64_t)va_arg(ap,ptrdiff_t);
            else if(size==2)bits=(uint64_t)va_arg(ap,long long);
            else if(size==1)bits=(uint64_t)va_arg(ap,long);
            else bits=(uint64_t)va_arg(ap,int);
            break;
        case 4: case 5:
            if(size==3)bits=va_arg(ap,size_t);
            else if(size==2)bits=va_arg(ap,unsigned long long);
            else if(size==1)bits=va_arg(ap,unsigned long);
            else bits=va_arg(ap,unsigned int);
            break;
        default:p=fmt;len=(size_t)d[0];break;
        }
        if(kind>=3&&kind<=5) {
            uint64_t n;fmt_status(CORE_CALL(omnum, &n,bits,kind==5?16:10,kind==3,number,24));len=(size_t)n;
        }
        fmt_copy(out,cap,&used,p,len);fmt+=d[0];
    }
    va_end(ap);out[used]=0;return (int)used;
}
#else

typedef struct { char *out; size_t cap, n; } Sink;

static void put(Sink *s, char c) {
    if (s->n + 1 < s->cap) s->out[s->n++] = c;
}
static void put_str(Sink *s, const char *p) {
    if (!p) p = "(null)";
    while (*p) put(s, *p++);
}
static void put_u(Sink *s, unsigned long long v, unsigned base) {
    char tmp[24];
    int i = 0;
    do { unsigned d = (unsigned)(v % base); tmp[i++] = (char)(d < 10 ? '0' + d : 'a' + d - 10); v /= base; } while (v);
    while (i) put(s, tmp[--i]);
}
static void put_d(Sink *s, long long v) {
    if (v < 0) { put(s, '-'); put_u(s, (unsigned long long)(-(v + 1)) + 1ull, 10); }
    else put_u(s, (unsigned long long)v, 10);
}

int omega_gpu_fmt(char *out, size_t cap, const char *fmt, ...) {
    Sink s = { out, cap, 0 };
    va_list ap;
    if (!out || cap == 0) return 0;
    if (!fmt) fmt = "";
    va_start(ap, fmt);
    for (const char *p = fmt; *p; p++) {
        if (*p != '%') { put(&s, *p); continue; }
        const char *start = p++;
        int l = 0, z = 0;
        if (*p == 'z') { z = 1; p++; }
        else while (*p == 'l' && l < 2) { l++; p++; }
        switch (*p) {
        case 's': put_str(&s, va_arg(ap, const char *)); break;
        case 'c': put(&s, (char)va_arg(ap, int)); break;
        case 'd':
            if (z) put_d(&s, (long long)va_arg(ap, ptrdiff_t));
            else if (l == 2) put_d(&s, va_arg(ap, long long));
            else if (l == 1) put_d(&s, va_arg(ap, long));
            else put_d(&s, va_arg(ap, int));
            break;
        case 'u': case 'x': {
            unsigned base = *p == 'x' ? 16u : 10u;
            if (z) put_u(&s, va_arg(ap, size_t), base);
            else if (l == 2) put_u(&s, va_arg(ap, unsigned long long), base);
            else if (l == 1) put_u(&s, va_arg(ap, unsigned long), base);
            else put_u(&s, va_arg(ap, unsigned int), base);
            break;
        }
        case '%': put(&s, '%'); break;
        default: /* unsupported conversion: copy it literally, consume nothing */
            for (const char *q = start; q <= p && *q; q++) put(&s, *q);
            if (!*p) p--;
            break;
        }
    }
    va_end(ap);
    s.out[s.n] = '\0';
    return (int)s.n;
}
#endif
