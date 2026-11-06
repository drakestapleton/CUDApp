/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#define CUDAPP_MIGRATION_PENDING_LEFTOVERS 1
#define omega_path_byte oracle_path_byte
#define omega_path_copy oracle_path_copy
#include "omega_path_check.h"
#undef omega_path_byte
#undef omega_path_copy
#undef CUDAPP_MIGRATION_PENDING_LEFTOVERS
#undef OMEGA_PATH_CHECK_H
#include "omega_path_check.h"
#ifdef LEFTOVERS_ORACLE
#define CUDAPP_MIGRATION_PENDING_LEFTOVERS 1
#endif
#include "omega_numeric_lifecycle.h"
#include "omega_gpu_q36_native.h"
#ifdef LEFTOVERS_ORACLE
#undef CUDAPP_MIGRATION_PENDING_LEFTOVERS
#endif
atomic_int omega_numeric_native_state;
static unsigned checks;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"FAIL line %d\n",__LINE__); return 1; } } while (0)
static int path(const char *s, size_t cap) {
    char a[64], b[64]; memset(a, 85, sizeof a); memset(b, 85, sizeof b);
    int ar = oracle_path_copy(a, cap, s), br = omega_path_copy(b, cap, s);
#ifdef LEFTOVERS_RED
    br ^= 1;
#endif
    CHECK(ar == br);
    /* Refusals promise an empty string, not equality of scratch bytes. */
    CHECK(cap == 0 || strcmp(a, b) == 0);
    return 0;
}
int main(void) {
    const char *cases[] = {NULL,"","/",".","..","...","a/../b","../","a/..","a/./b","a//b/","/..","/a/b","a b","a\nb","AZaz09._-/+,=@:~","..a","a..","a/.../b"};
    for (size_t i=0;i<sizeof cases/sizeof *cases;++i)
        for (size_t cap=0;cap<=32;++cap) CHECK(path(cases[i],cap)==0);
    CHECK(omega_path_copy(NULL, 1, "a") == -1);
    CHECK(oracle_path_copy(NULL, 1, "a") == -1);
    for (int c=-1;c<=256;++c) CHECK(oracle_path_byte(c)==omega_path_byte(c));
    char s[3]={0,'x',0};
    for (unsigned c=1;c<=255;++c) { s[0]=(char)c; CHECK(path(s,3)==0); }
    uint64_t r=99;
    CHECK(omega_leftovers(NULL, 0, 0, 0, 0, 0)==OMEGA_BAD_ARG);
    CHECK(omega_leftovers(&r,1,0,'a',UINT64_MAX,UINT64_MAX)==0 && r==4);
    CHECK(omega_leftovers(&r,1,0,'a',UINT32_MAX,UINT32_MAX)==0 && r==4);
    const int ord[]={INT_MIN,-1,0,1,INT_MAX};
    for (unsigned i=0;i<sizeof ord/sizeof *ord;++i) {
        CHECK(omega_leftovers(&r,2,(uint64_t)(int64_t)ord[i],0,0,0)==0);
        CHECK(r==(ord[i]<0?(uint64_t)(int64_t)ord[i]:0));
        CHECK(omega_q36_native_record_status(ord[i])==(ord[i]<0?ord[i]:0));
    }
    for (unsigned st=0;st<4;++st) {
        atomic_store(&omega_numeric_native_state,st);
        CHECK(omega_numeric_native_uncertain()==(st==2));
        CHECK(omega_numeric_native_acquire()==(st==0?0:-1));
        CHECK(atomic_load(&omega_numeric_native_state)==(st==0?1:(int)st));
        omega_numeric_native_release();
        CHECK(atomic_load(&omega_numeric_native_state)==(st<=1?0:(int)st));
        omega_numeric_native_poison(); omega_numeric_native_release();
        CHECK(omega_numeric_native_uncertain());
    }
    const uint64_t values[]={0,1,31,32,33,UINT32_MAX-1ULL,UINT32_MAX,UINT32_MAX+1ULL,INT64_MAX,1ULL<<63,UINT64_MAX};
    for (unsigned i=0;i<sizeof values/sizeof *values;++i) {
        for (unsigned p=0;p<2;++p) {
            CHECK(omega_leftovers(&r,5,values[i],p,0,0)==0);
            CHECK(r==(values[i]==0?1:(values[i]>>63||!p?UINT64_MAX:0)));
        }
        for (unsigned j=0;j<sizeof values/sizeof *values;++j) {
            CHECK(omega_leftovers(&r,6,values[i],values[j],0,0)==0);
            CHECK(r==(values[j]<values[i] && values[j]<32));
        }
    }
    printf("LEFTOVERS PASS checks=%u\n",checks);
    return 0;
}
