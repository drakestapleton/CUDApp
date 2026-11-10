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
/* Each vector goes through both unit paths: sha256_hash (one-shot op 3 below 4096 bytes) and init/update/final. */
static int vector(const uint8_t *p,size_t n,const char *want) { uint8_t d[32]; char h[65]; sha256_hash(p,n,d); hex(d,h); if(strcmp(h,want)) { fprintf(stderr,"SHA vector length %zu: %s\n",n,h);return 1;}
    sha256_ctx c; sha256_init(&c); sha256_update(&c,p,n); sha256_final(&c,d); hex(d,h); if(strcmp(h,want)) { fprintf(stderr,"SHA vector length %zu (streaming): %s\n",n,h);return 1;} return 0; }
static int unhex(char c) {
    if(c>='0'&&c<='9')return c-'0';
    if(c>='a'&&c<='f')return c-'a'+10;
    return -1;
}
static int cavp(const char *path) {
    FILE *f=fopen(path,"r");if(!f){perror(path);return 1;}
    char *line=NULL;size_t cap=0,n=0;unsigned count=0;
    uint8_t message[65536];int have_message=0,result=0;
    while(getline(&line,&cap,f)>=0) {
        if(!strncmp(line,"Len = ",6)) {
            unsigned long bits=strtoul(line+6,NULL,10);
            if(bits%8||bits/8>sizeof message){result=1;break;}
            n=bits/8;have_message=0;
        } else if(!strncmp(line,"Msg = ",6)) {
            if(strlen(line+6)<n*2){result=1;break;}
            for(size_t i=0;i<n;i++) {
                int a=unhex(line[6+i*2]),b=unhex(line[7+i*2]);
                if(a<0||b<0){result=1;break;}
                message[i]=(uint8_t)(a*16+b);
            }
            if(result)break;
            have_message=1;
        } else if(!strncmp(line,"MD = ",5)) {
            line[strcspn(line,"\r\n")]=0;
            if(!have_message||strlen(line+5)!=64||vector(message,n,line+5)){result=1;break;}
            count++;have_message=0;
        }
    }
    if(ferror(f)||!count||have_message)result=1;
    free(line);fclose(f);
    if(!result)printf("CAVP %s PASS (%u records)\n",path,count);
    return result;
}
/* Padding crosses a block at 56 bytes: lengths 55 and 56, the 64-byte edges, and two/three blocks.
 * WolframLanguageEvaluator: Mod[56, 64] -> 56; 128/64 -> 2; 192/64 -> 3. */
static int partition(void) {
    static const size_t lens[] = {0, 55, 56, 63, 64, 65, 128, 192};
    uint8_t buf[192], a[32], b[32];
    for (size_t i = 0; i < sizeof buf; i++) buf[i] = (uint8_t)(i * 17u + 3u);
    for (size_t k = 0; k < sizeof lens / sizeof lens[0]; k++) {
        sha256_hash(buf, lens[k], a);
        ref_sha256_hash(buf, lens[k], b);
        if (memcmp(a, b, 32)) { fprintf(stderr, "SHA partition length %zu\n", lens[k]); return 1; }
    }
    puts("CORE-UTIL SHA partition PASS: 0 55 56 63 64 65 128 192");
    return 0;
}
int main(int argc,char **argv) {
    uint8_t p[32768],a[32],b[32],c[32];
    if(vector((const uint8_t *)"",0,"e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855")) return 1;
    if(vector((const uint8_t *)"abc",3,"ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad")) return 1;
    const char *msg="abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    if(vector((const uint8_t *)msg,strlen(msg),"248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1")) return 1;
    if(partition()) return 1;
    if(argc>1&&!strcmp(argv[1],"--quick"))return 0;
    if(cavp("tests/vectors/core/SHA256ShortMsg.rsp")||
       cavp("tests/vectors/core/SHA256LongMsg.rsp"))return 1;
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
    puts("CORE-UTIL SHA PASS: FIPS/CAVP vectors, 10000 random and streaming cases");
    return 0;
}
