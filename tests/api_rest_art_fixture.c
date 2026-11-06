/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "../src/omega_native_artifact.c"
#ifndef REST
#define REST(x) om_##x
#endif
int REST(cubin)(const uint8_t *b,size_t n,const char *f,const uint8_t *t,size_t tn) { return cubin_text_matches(b,n,f,t,tn); }
int REST(layout)(const uint8_t *b,size_t n) { return manifest_layout_ok(b,n); }
int REST(version)(const char *h,const char *n) { return semver_ge(h,n); }
size_t REST(json)(unsigned char c,unsigned char out[32]) {
    FILE *f=tmpfile(); if(!f) abort(); char s[2]={(char)c,0}; json_str(f,s);
    rewind(f);size_t n=fread(out,1,32,f);fclose(f);return n;
}
