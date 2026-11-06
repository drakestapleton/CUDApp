#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Omega GPU Engine mutation sweep, a thin adapter over tools/mutation_runner.sh.
# For each rule marked MUT:<name> in src/omega_gpu_engine.c the runner copies the
# needed files to a scratch directory, breaks that one protection, rebuilds
# tests/test_omega_gpu_engine.c there and requires it to FAIL. A mutant that does
# not apply, does not build or is not caught fails the sweep (exit nonzero).
# Host only, no device, no physics headers. Shell only (no Python).
# Two runs share the test:
#   oracle  the MUT:<name> rows on the C oracle, built with
#           -DCUDAPP_MIGRATION_PENDING_C_ENGINE (the reference of make test-om-eng-diff)
#   om      the MUT:OM_<name> rows on the shell of the default build, where the
#           decisions come from om/engine/eng.om (src/cudapp_om_eng.c). The decisions
#           themselves are mutated in eng.om by tests/om_eng_mutants.sh.
# Each mutant names what it guards; the MUT:<name> comment in the source states it.
# Set GPU_ENGINE_MUT_OUT=path.json to also keep the JSON receipt of the oracle run
# (path.om.json for the om run).
set -u
cd "$(dirname "$0")/.." || exit 2
unset ONLY

FILE=src/omega_gpu_engine.c
BUILD_OM='gcc -std=gnu11 -O2 -Wall -Wextra -Werror -Isrc -DOMEGA_NUMERIC_CPU_ONLY -o .gpu_engine_t tests/test_omega_gpu_engine.c src/omega_gpu_engine.c src/cudapp_om_eng.c'
BUILD_C="$BUILD_OM -DCUDAPP_MIGRATION_PENDING_C_ENGINE"
COPY="src/omega_gpu_engine.c src/omega_gpu_engine.h src/omega_eng_dec.h src/cudapp_om_eng.c tests/test_omega_gpu_engine.c"
# name|file|sed expression (applied only to the line carrying MUT:<name>).
# In an expression, & in the replacement is written \& .
ROWS="VALIDATE_SKIPPED|$FILE|s/if (!job || !job_valid(job))/if (0 \\&\\& (!job || !job_valid(job)))/
VALID_PROGRAM|$FILE|s/return 0;/(void)0;/
VALID_LAYOUT|$FILE|s/return 0;/(void)0;/
VALID_COUNT_ZERO|$FILE|s/return 0;/(void)0;/
VALID_ALIGN|$FILE|s/return 0;/(void)0;/
VALID_POISON_COUNT|$FILE|s/return 0;/(void)0;/
VALID_FLAGS|$FILE|s/return 0;/(void)0;/
VALID_TIMEOUT_MARKER1|$FILE|s/return 0;/(void)0;/
VALID_TIMEOUT_MARKER2|$FILE|s/return 0;/(void)0;/
BLOCK_CHECK_SKIPPED|$FILE|s/if (omega_gpu_engine_is_blocked())/if (0)/
BACKEND_CHECK_SKIPPED|$FILE|s/if (!backend_complete(b))/if (0 \\&\\& !backend_complete(b))/
OPEN_NOT_TRACKED|$FILE|s/run.opened = 1;/(void)0;/
ALLOC_NOT_TRACKED|$FILE|s/run.allocated\\[role\\] = 1;/(void)0;/
CLEANUP_SKIPPED_CHANNEL|$FILE|s/return leave(b, &run, result);/return result->failure;/
CLEANUP_SKIPPED_ALLOC|$FILE|s/return leave(b, &run, result);/return result->failure;/
CLEANUP_SKIPPED_COPY_IN|$FILE|s/return leave(b, &run, result);/return result->failure;/
CLEANUP_SKIPPED_SUBMIT|$FILE|s/return leave(b, &run, result);/return result->failure;/
FREE_UNALLOCATED|$FILE|s/if (!run->allocated\\[role\\]) continue;/(void)0;/
FREE_SKIPPED|$FILE|s/rc = b->free_buf(b->ctx, role);/rc = 0;/
CLOSE_SKIPPED|$FILE|s/rc = b->close(b->ctx);/rc = 0;/
CLEANUP_OVERWRITES_FIRST|$FILE|s/if (result->failure == OMEGA_GPU_ENGINE_OK)/if (1)/
CLEANUP_FLAG_NOT_SET|$FILE|s/result->cleanup_failed = 1;/(void)0;/
POISON_FILL_SKIPPED|$FILE|s/ERRNO_CLEAR(); rc = b->fill_poison.*/rc = 0; e = 0;/
STATE_PREPARED_SKIPPED|$FILE|s/result->last_state = OMEGA_GPU_ENGINE_STATE_PREPARED;/(void)0;/
STATE_SUBMITTED_SKIPPED|$FILE|s/result->last_state = OMEGA_GPU_ENGINE_STATE_SUBMITTED;/(void)0;/
STATE_GPU_COMPLETE_SKIPPED|$FILE|s/result->last_state = OMEGA_GPU_ENGINE_STATE_GPU_COMPLETE;/(void)0;/
STATE_OUTPUT_VISIBLE_SKIPPED|$FILE|s/result->last_state = OMEGA_GPU_ENGINE_STATE_OUTPUT_VISIBLE;/(void)0;/
STATE_OUTPUT_PRODUCED_SKIPPED|$FILE|s/result->last_state = OMEGA_GPU_ENGINE_STATE_OUTPUT_PRODUCED;/(void)0;/
STATE_SUCCESS_SKIPPED|$FILE|s/if (rc == OMEGA_GPU_ENGINE_OK) result->last_state = OMEGA_GPU_ENGINE_STATE_SUCCESS;/(void)0;/
SUCCESS_AFTER_CLEANUP_FAIL|$FILE|s/if (rc == OMEGA_GPU_ENGINE_OK) result/if (1) result/
MARKER1_WAIT_SKIPPED|$FILE|s/rc = timed_call(.*\$/rc = 0;/
MARKER1_TIMEOUT_WRONG|$FILE|s/job->timeouts.marker_ms, &waited/job->timeouts.release_semaphore_ms, \\&waited/
MARKER1_RESULT_IGNORED|$FILE|s/if (rc != 0) return uncertain/if (0 \\&\\& rc != 0) return uncertain/
UNCERTAIN_FREES_MARKER1|$FILE|s/return uncertain(\\(.*\\));/{ (void)uncertain(\\1); return leave(b, \\&run, result); }/
MARKER2_WAIT_SKIPPED|$FILE|s/if (!(job->flags & OMEGA_GPU_ENGINE_FLAG_NO_C3)) {/if (0) {/
C3_FLAG_IGNORED|$FILE|s/if (!(job->flags & OMEGA_GPU_ENGINE_FLAG_NO_C3)) {/if (1) {/
MARKER2_RESULT_IGNORED|$FILE|s/if (rc != 0) return uncertain/if (0 \\&\\& rc != 0) return uncertain/
UNCERTAIN_FREES_MARKER2|$FILE|s/return uncertain(\\(.*\\));/{ (void)uncertain(\\1); return leave(b, \\&run, result); }/
RELEASE_WAIT_SKIPPED|$FILE|s/rc = timed_call(.*\$/rc = 0;/
RELEASE_RESULT_IGNORED|$FILE|s/if (rc != 0) return uncertain/if (0 \\&\\& rc != 0) return uncertain/
UNCERTAIN_FREES_RELEASE|$FILE|s/return uncertain(\\(.*\\));/{ (void)uncertain(\\1); return leave(b, \\&run, result); }/
BARRIER_SKIPPED|$FILE|s/rc = timed_call(.*\$/rc = 0;/
BARRIER_RESULT_IGNORED|$FILE|s/if (rc != 0) {/if (0 \\&\\& rc != 0) {/
RETAINED_FLAG_NOT_SET|$FILE|s/result->retained = 1;/(void)0;/
NO_BLOCK|$FILE|s/atomic_store(&g_blocked, 1);/(void)0;/
WAITED_NOT_A_DIFFERENCE|$FILE|s/t1 >= t0 ? t1 - t0 : 0u;/t1 >= t0 ? t1 : 0u;/
POISON_INDEX_FIXED|$FILE|s/w == job->poison\\[i\\]/w == job->poison[0]/
UNCHANGED_COUNT_SATURATES|$FILE|s/unchanged++;/unchanged = 1u;/
UNCHANGED_CHECK_SKIPPED|$FILE|s/if (unchanged != 0u) {/if (0) {/"

OM_ROWS="OM_CLEANUP_FAIL_SKIPPED|$FILE|s/(void)fail(b, result, OMEGA_GPU_ENGINE_CLEANUP, step, err);/{ (void)b; (void)step; (void)err; }/
OM_CLEANUP_FLAG_NOT_SET|$FILE|s/result->cleanup_failed = 1;/(void)0;/
OM_FREE_UNALLOCATED|$FILE|s/if (!run->allocated\\[role\\]) continue;/(void)0;/
OM_FREE_SKIPPED|$FILE|s/rc = b->free_buf(b->ctx, role);/rc = 0;/
OM_CLOSE_SKIPPED|$FILE|s/rc = b->close(b->ctx);/rc = 0;/
OM_RETAINED_NOT_SET|$FILE|s/result->retained = 1;/(void)0;/
OM_NO_BLOCK|$FILE|s/atomic_store(&g_blocked, 1);/(void)0;/
OM_UNWIND_SKIPPED|$FILE|s/return leave_om(b, run, result);/return ((void)run, result->failure);/
OM_POISON_INDEX_FIXED|$FILE|s/job->poison\\[i\\]/job->poison[0]/
OM_WAITED_NOT_A_DIFFERENCE|$FILE|s/in\\[1\\] = t0;/in[1] = ((void)t0, 0);/
OM_OPEN_NOT_TRACKED|$FILE|s/run.opened = 1;/(void)0;/
OM_ALLOC_NOT_TRACKED|$FILE|s/run.allocated\\[role\\] = 1;/(void)0;/
OM_COPY_IN_CHAIN|$FILE|s/if (rc == 0) rc = copy_in_role/rc = copy_in_role/
OM_POISON_FILL_SKIPPED|$FILE|s/rc = b->fill_poison(.*);/rc = 0;/
OM_MARKER1_WAIT_SKIPPED|$FILE|s/rc = timed_om(.*);/rc = 0;/
OM_MARKER1_LIMIT_WRONG|$FILE|s/job->timeouts.marker_ms, waited/job->timeouts.release_semaphore_ms, waited/
OM_C3_FLAG_IGNORED|$FILE|s/eng_phase(13, 0, job->flags, 0, 0, w)/eng_phase(13, 0, 0, 0, 0, w)/
OM_MARKER2_WAIT_SKIPPED|$FILE|s/if (w\\[0\\] != 4) {/if (0) {/
OM_RELEASE_WAIT_SKIPPED|$FILE|s/rc = timed_om(.*);/rc = 0;/
OM_BARRIER_SKIPPED|$FILE|s/rc = timed_om(.*);/rc = 0;/
OM_UNCHANGED_NOT_REPORTED|$FILE|s/= waited;/= 0u;/"

OUT=${GPU_ENGINE_MUT_OUT:-}
sweep() { # build rows json
    set -- -k marker -m - -t ./.gpu_engine_t -b "$1" -c "$COPY" -f '\[FAIL\]' -B ${3:+-o "$3"}
    printf '%s\n' "$ROWS_IN" | tools/mutation_runner.sh "$@"
}
echo "oracle run (CUDAPP_MIGRATION_PENDING_C_ENGINE)"
ROWS_IN=$ROWS
sweep "$BUILD_C" rows "$OUT"
rc_c=$?
echo "om run (default build, decisions from om/engine/eng.om)"
ROWS_IN=$OM_ROWS
sweep "$BUILD_OM" rows "${OUT:+${OUT%.json}.om.json}"
rc_om=$?
[ "$rc_c" = 0 ] && [ "$rc_om" = 0 ]
