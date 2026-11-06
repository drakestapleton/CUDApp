/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* HOST_ONLY. Compares the C oracle and the Omega policy on the same inputs.
 * No device node is opened. NVRM_POLICY_MUTANT=1 flips one input so the
 * comparison must fail: that is the red check. */
#include "nvrm_policy.h"
#include <stdio.h>
#include <stdint.h>

static int fails;

static void __attribute__((unused)) eq(const char *name, uint64_t c, uint64_t o) {
    if (c != o) {
        printf("DIFF %s c=%llu o=%llu\n", name, (unsigned long long)c, (unsigned long long)o);
        fails++;
    }
}

static void __attribute__((unused)) eq4(const char *name, const uint64_t *c, const uint64_t *o) {
    for (int i = 0; i < 4; i++) {
        if (c[i] != o[i]) {
            printf("DIFF %s[%d] c=%llu o=%llu\n", name, i,
                   (unsigned long long)c[i], (unsigned long long)o[i]);
            fails++;
        }
    }
}

int main(void) {
#ifdef NVRM_POLICY_MUTANT
    /* Red check: one input differs, so the two sides must disagree. */
    if (nvrm_c_nrstat(0) == nvrm_om_nrstat(0x51)) {
        printf("RED CHECK FAILED: a flipped input still compared equal\n");
        return 0;
    }
    printf("NVRM policy red check caught the flipped input\n");
    return 1;
#else
    eq("nrstat0", nvrm_c_nrstat(0), nvrm_om_nrstat(0));
    eq("nrstat_nomem", nvrm_c_nrstat(0x51), nvrm_om_nrstat(0x51));
    eq("nrstat_other", nvrm_c_nrstat(1), nvrm_om_nrstat(1));

    eq("hnext_root", nvrm_c_hnext(0x41, 5, 0), nvrm_om_hnext(0x41, 5, 0));
    eq("hnext_adv", nvrm_c_hnext(0x42, 7, 3), nvrm_om_hnext(0x42, 7, 3));
    eq("hnext_handle_full", nvrm_c_hnext(0x42, 4294967295ull, 0), nvrm_om_hnext(0x42, 4294967295ull, 0));
    eq("hnext_acct_full", nvrm_c_hnext(0x41, 0, 18446744073709551615ull), nvrm_om_hnext(0x41, 0, 18446744073709551615ull));

    eq("vunits0", nvrm_c_vunits(0), nvrm_om_vunits(0));
    eq("vunits1", nvrm_c_vunits(1), nvrm_om_vunits(1));
    eq("vunits_4g", nvrm_c_vunits(4294967296ull), nvrm_om_vunits(4294967296ull));
    eq("vunits_over", nvrm_c_vunits(18446744073709551615ull), nvrm_om_vunits(18446744073709551615ull));

    uint64_t co[4] = {0}, oo[4] = {0};
    uint64_t cd = nvrm_c_vcover(0x100000000ull, 0x100000000ull, 0x100000100ull, co);
    uint64_t od = nvrm_om_vcover(0x100000000ull, 0x100000000ull, 0x100000100ull, oo);
    eq("vcover_covered", cd, od);
    eq4("vcover_covered_out", co, oo);
    cd = nvrm_c_vcover(0x100000000ull, 0x100000000ull, 0x200000000ull, co);
    od = nvrm_om_vcover(0x100000000ull, 0x100000000ull, 0x200000000ull, oo);
    eq("vcover_grow", cd, od);
    eq4("vcover_grow_out", co, oo);
    eq("vcover_noslot", nvrm_c_vcover(0, 1, 2, co), nvrm_om_vcover(0, 1, 2, oo));
    eq("vcover_bad", nvrm_c_vcover(0x100ull, 0x50ull, 0x200ull, co), nvrm_om_vcover(0x100ull, 0x50ull, 0x200ull, oo));
    cd = nvrm_c_vcover(0x100000000ull, 0x100000000ull, 0x200000001ull, co);
    od = nvrm_om_vcover(0x100000000ull, 0x100000000ull, 0x200000001ull, oo);
    eq("vcover_cas", cd, od);
    eq4("vcover_cas_out", co, oo);
    eq("vcover_units0", nvrm_c_vcover(1, 18446744069414584322ull, 2, co), nvrm_om_vcover(1, 18446744069414584322ull, 2, oo));
    eq("vcover_endv", nvrm_c_vcover(0x100ull, 0x200ull, 0x50ull, co), nvrm_om_vcover(0x100ull, 0x200ull, 0x50ull, oo));
    eq("vcover_want0", nvrm_c_vcover(1, 2, 18446744069414584322ull, co), nvrm_om_vcover(1, 2, 18446744069414584322ull, oo));

    eq("palign_ok", nvrm_c_palign(0x1000, 0x100), nvrm_om_palign(0x1000, 0x100));
    eq("palign_bad", nvrm_c_palign(0x1000, 3), nvrm_om_palign(0x1000, 3));
    eq("palign_zero", nvrm_c_palign(0x1000, 0), nvrm_om_palign(0x1000, 0));
    eq("palign_over", nvrm_c_palign(18446744073709551605ull, 256), nvrm_om_palign(18446744073709551605ull, 256));

    eq("pspan_ok", nvrm_c_pspan(0, 4, 64, 0x1000), nvrm_om_pspan(0, 4, 64, 0x1000));
    eq("pspan_misalign", nvrm_c_pspan(1, 4, 64, 0x1000), nvrm_om_pspan(1, 4, 64, 0x1000));
    eq("pspan_zero", nvrm_c_pspan(0, 0, 64, 0x1000), nvrm_om_pspan(0, 0, 64, 0x1000));
    eq("pspan_big", nvrm_c_pspan(0, 2097152ull, 64, 0x1000), nvrm_om_pspan(0, 2097152ull, 64, 0x1000));
    eq("pspan_vaover", nvrm_c_pspan(0, 4, 64, 18446744073709551615ull), nvrm_om_pspan(0, 4, 64, 18446744073709551615ull));
    eq("pspan_offover", nvrm_c_pspan(18446744073709551600ull, 4, 18446744073709551615ull, 0), nvrm_om_pspan(18446744073709551600ull, 4, 18446744073709551615ull, 0));
    eq("pspan_spanbig", nvrm_c_pspan(0, 4, 8, 0), nvrm_om_pspan(0, 4, 8, 0));
    eq("pspan_vaoverflow", nvrm_c_pspan(0x1000, 4, 18446744073709551615ull, 18446744073709547520ull), nvrm_om_pspan(0x1000, 4, 18446744073709551615ull, 18446744073709547520ull));

    eq("pfull_room", nvrm_c_pfull(3, 1, 8), nvrm_om_pfull(3, 1, 8));
    eq("pfull_full", nvrm_c_pfull(8, 1, 8), nvrm_om_pfull(8, 1, 8));
    eq("pspan_limit", nvrm_c_pspan(0, 2097151ull, 8388604ull, 0x1000), nvrm_om_pspan(0, 2097151ull, 8388604ull, 0x1000));
    eq("pfull_wrap", nvrm_c_pfull(1, 4294967295ull, 4), nvrm_om_pfull(1, 4294967295ull, 4));

    eq("chadm_clean", nvrm_c_chadm(0, 0, 0, 0, 1, 3), nvrm_om_chadm(0, 0, 0, 0, 1, 3));
    eq("chadm_dirty", nvrm_c_chadm(1, 0, 0, 0, 0, 3), nvrm_om_chadm(1, 0, 0, 0, 0, 3));
    eq("chadm_extra", nvrm_c_chadm(0, 0, 0, 0, 4, 3), nvrm_om_chadm(0, 0, 0, 0, 4, 3));
    eq("chadm_chgroup", nvrm_c_chadm(0, 1, 0, 0, 1, 3), nvrm_om_chadm(0, 1, 0, 0, 1, 3));
    eq("chadm_regd", nvrm_c_chadm(0, 0, 1, 0, 1, 3), nvrm_om_chadm(0, 0, 1, 0, 1, 3));
    eq("chadm_nextra", nvrm_c_chadm(0, 0, 0, 1, 1, 3), nvrm_om_chadm(0, 0, 0, 1, 1, 3));

    eq("retry_ok", nvrm_c_retry(1, 0, 0, 3), nvrm_om_retry(1, 0, 0, 3));
    eq("retry_other", nvrm_c_retry(0, 0, 0, 3), nvrm_om_retry(0, 0, 0, 3));
    eq("retry_budget", nvrm_c_retry(0, 1, 3, 3), nvrm_om_retry(0, 1, 3, 3));
    eq("retry_again", nvrm_c_retry(0, 1, 1, 3), nvrm_om_retry(0, 1, 1, 3));
    eq("retry_loop", nvrm_c_retry(1, 1, 5, 1), nvrm_om_retry(1, 1, 5, 1));
    eq("retry_nonmem", nvrm_c_retry(1, 0, 5, 0), nvrm_om_retry(1, 0, 5, 0));

    eq("mthd", nvrm_c_mthd(1, 0x005c, 5), nvrm_om_mthd(1, 0x005c, 5));
    eq("gpentry", nvrm_c_gpentry(0x1000, 16), nvrm_om_gpentry(0x1000, 16));

    if (fails) {
        printf("NVRM POLICY DIFFERENTIAL FAIL %d\n", fails);
        return 1;
    }
    printf("NVRM POLICY DIFFERENTIAL PASS\n");
    return 0;
#endif
}
