/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef OMEGA_NO_POSIX_SESSION_H
#define OMEGA_NO_POSIX_SESSION_H
/* Default-OFF oracle. NO_POSIX never implies permission to select C decisions. */
#ifndef CUDAPP_MIGRATION_PENDING_C_NO_POSIX_SESSION
#define CUDAPP_MIGRATION_PENDING_C_NO_POSIX_SESSION 0
#endif
#ifdef OMEGA_GPU_NO_POSIX
#include "omega_native.h"
#define omega_decide omega2_decide
#define omega_emit omega2_emit
#define omega_scan omega2_scan
#define omega_book omega2_book
#define omega_streams_words omega2_streams_words
#endif
#endif
