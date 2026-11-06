/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "sha256.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void ref_sha256_hash(const uint8_t *, size_t, uint8_t *);
static uint64_t rng = 0x4f4d454741ULL;
static uint32_t random32(void) { rng ^= rng << 13; rng ^= rng >> 7; rng ^= rng << 17; return (uint32_t)rng; }
static void hex(const uint8_t *d, char *h) { for(unsigned i=0;i<32;i++) sprintf(h+2*i,"%02x",d[i]); }
static int vector(const uint8_t *p,size_t n,const char *want) { uint8_t d[32]; char h[65]; sha256_hash(p,n,d); hex(d,h); if(strcmp(h,want)) { fprintf(stderr,"SHA vector length %zu: %s\n",n,h);return 1;} return 0; }
int main(int argc,char **argv) {
    uint8_t p[32768],a[32],b[32],c[32];
    if(vector((const uint8_t *)"",0,"e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855")) return 1;
    if(vector((const uint8_t *)"abc",3,"ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad")) return 1;
    const char *msg="abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    if(vector((const uint8_t *)msg,strlen(msg),"248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1")) return 1;
    sha256_ctx ctx; sha256_init(&ctx); memset(p,'a',1000);
    for(unsigned i=0;i<1000;i++) sha256_update(&ctx,p,1000);
    sha256_final(&ctx,a); char h[65]; hex(a,h);
    if(strcmp(h,"cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0")) return 1;
    for(unsigned t=0;t<10000;t++) {
        size_t n=t<512?t:random32()%sizeof p;
        for(size_t j=0;j<n;j++) p[j]=(uint8_t)random32();
        sha256_hash(p,n,a); ref_sha256_hash(p,n,b);
        sha256_init(&ctx);
        for(size_t j=0;j<n;) {size_t k=1+random32()%257;if(k>n-j)k=n-j;sha256_update(&ctx,p+j,k);j+=k;}
        sha256_final(&ctx,c);
        if(argc>1 && !strcmp(argv[1],"--flip") && t==0) a[0]^=1;
        if(memcmp(a,b,32)||memcmp(a,c,32)) {fprintf(stderr,"SHA differential case %u length %zu\n",t,n);return 1;}
    }
    puts("CORE-UTIL SHA PASS: FIPS vectors, 10000 random and streaming cases");
    return 0;
}
