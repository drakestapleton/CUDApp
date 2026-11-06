/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
/* Control surface of the test transport (tests/nvml/transport.c). fake_get indexes the
 * tests/nvml/rmfake.om state: 8 opens, 9 closes, 10 allocs, 11 frees, 12 controls,
 * 13 violations, 14 open-fd mask, 15 live objects, 16 root handle. */
#ifndef CUDAPP_NVML_TEST_TRANSPORT_H
#define CUDAPP_NVML_TEST_TRANSPORT_H

#include <stdint.h>

void fake_reset(int fault);
uint64_t fake_get(unsigned i);
uint64_t fake_trapped(void);

#endif
