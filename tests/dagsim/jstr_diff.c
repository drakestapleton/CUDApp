/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#define main dagsim_unused_main
#include "../../tools/dagsim/dagsim_cli.c"
#undef main
#include "../../src/cudapp_native.h"
int main(int argc, char **argv) {
    if (argc > 1 && !strcmp(argv[1], "--stream")) {
        char s[2] = {0, 0};
        jstr(s);
        for (unsigned c = 1; c < 256; ++c) { s[0] = (char)c; jstr(s); }
        return 0;
    }
    for (unsigned c = 0; c < 256; ++c) {
        unsigned char b[6] = {0}; uint64_t n = 0; char want[8]; int len;
        if (c == '"' || c == '\\') len = snprintf(want, sizeof want, "\\%c", c);
        else if (c < 32) len = snprintf(want, sizeof want, "\\u%04x", c);
        else { want[0] = (char)c; len = 1; }
        if (omega_dag_jstr(&n, c, b, sizeof b)) return 2;
        if (argc > 1) b[0] ^= 1;
        if (n != (uint64_t)len || memcmp(b, want, n)) {
            fprintf(stderr, "byte %u differs\n", c); return 1;
        }
    }
    puts("JSON BYTE DIFFERENTIAL PASS all 256 bytes"); return 0;
}
