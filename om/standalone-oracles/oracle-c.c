/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Default-OFF migration oracle, linked only by this differential test. */
#define CUDAPP_MIGRATION_PENDING_C_STANDALONE_ORACLES 1
#include "oracle.h"
uint64_t standalone_c_oracle(unsigned op,uint64_t a,uint64_t b,uint64_t c,uint64_t d) {
    return so_call(op,a,b,c,d);
}
