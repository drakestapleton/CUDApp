/* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
/* Do not count oracle reads in the production read-once fixture observer. */
#define fopen fopen64
/* Immutable algorithm source is the PENDING branch in the production file. */
#define CUDAPP_MIGRATION_PENDING_C_ARTIFACT 1
#define omega_native_artifact_export artifact_c_export
#define omega_native_artifact_admit artifact_c_admit
#define omega_native_artifact_snapshot artifact_c_snapshot
#define omega_native_artifact_snapshot_free artifact_c_snapshot_free
#define omega_native_artifact_rc_name artifact_c_rc_name
#include "../../src/omega_native_artifact.c"

/* Expose the legacy output codec only to the independent host differential. */
size_t artifact_c_json_escape(const char *input, unsigned char *out, size_t cap) {
    FILE *f = tmpfile();
    if (!f) abort();
    json_str(f, input);
    if (fflush(f) || fseek(f, 0, SEEK_SET)) abort();
    size_t n = fread(out, 1, cap, f);
    if (ferror(f) || fclose(f)) abort();
    return n;
}
