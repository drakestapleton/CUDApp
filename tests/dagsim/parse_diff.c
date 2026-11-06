/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#include "dagsim.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int oracle_ds_parse_text(const char *, ds_model *, char *, size_t);
int main(int argc, char **argv) {
    static ds_model a, b;
    int failures = 0;
    for (int i = 1; i < argc; i++) {
        FILE *f = fopen(argv[i], "rb"); if (!f) return 2;
        if (fseek(f, 0, SEEK_END)) return 2;
        long n = ftell(f); if (n < 0) return 2;
        rewind(f); char *s = malloc((size_t)n + 1); if (!s) return 2;
        if (fread(s, 1, (size_t)n, f) != (size_t)n) return 2;
        fclose(f); s[n] = 0;
        char ea[256] = {0}, eb[256] = {0};
        int ra = ds_parse_text(s, &a, ea, sizeof ea);
        int rb = oracle_ds_parse_text(s, &b, eb, sizeof eb);
        if (n >= 1048576) {
            if (ra != DS_PARSE_ERROR || strcmp(ea, "file size out of range")) failures++;
            free(s); continue; /* documented bounded-view refusal, not C grammar */
        }
        if (getenv("DS_PARSE_RED") && !rb) b.version ^= 1;
        if (ra != rb || strcmp(ea, eb) || (!ra && memcmp(&a, &b, sizeof a))) {
            fprintf(stderr, "%s: production %d '%s'; oracle %d '%s'\n", argv[i], ra, ea, rb, eb);
            if (!ra && !rb) {
                const unsigned char *pa = (const unsigned char *)&a, *pb = (const unsigned char *)&b;
                for (size_t k = 0; k < sizeof a; k++) if (pa[k] != pb[k]) {
                    fprintf(stderr, "first model byte %zu: %u != %u\n", k, pa[k], pb[k]); break;
                }
            }
            failures++;
        }
        free(s);
    }
    printf("parse differential: %d cases, %d failures\n", argc - 1, failures);
    return failures ? 1 : 0;
}
