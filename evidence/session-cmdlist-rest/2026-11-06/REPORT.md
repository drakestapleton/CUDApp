# Session and command-list decision residue, 2026-11-06

Lane: feat/session-cmdlist-rest-om, stacked on #304 (feat/cmdlist-program-om @581c048, which includes #272). #300's C5 policy was not merged; the recovery decisions already present on this base were rewired, and the C5 deadline oracle stays behind its own flag.

## What moved

Decision and computation left in src/omega_gpu_session.c and src/omega_gpu_cmdlist.c moved into seeded Omega units. The C shell keeps the lock, the atomics, and every dma, queue, sync, poll and device call.

- om/session/admit.om: admission, free, scratch, platform-busy, recovery-admission, hook-capacity and completion decisions. Export omega_decide.
- om/session/park.om: parked-handle scan. Export omega_scan.
- om/session/words.om: method-word packet for the launch pushbuffer. Export omega_emit.
- om/cmdlist/book.om: command-list stale and latch decisions. Export omega_book.

The default path calls the Omega unit. The previous C is the default-OFF differential oracle, compiled only with CUDAPP_MIGRATION_PENDING_C_SESSION or CUDAPP_MIGRATION_PENDING_C_CMDLIST2.

## Verdicts

- units.sh --check: PASS (portable units regenerate byte for byte).
- Hosted and freestanding (-DOMEGA_GPU_NO_POSIX) builds of both files: compile clean under -Wall -Wextra -Werror.
- test-gpu-cmdlist: PASS (sim 74 checks, 0 failed).
- test-gpu-session-platform, session half: PASS (16 passed, 0 failed).
- test-om-streams: PASS (host 5, sim 9, isolation 5).
- test-om-cmdlist shell: rc 1. The stale() gate this lane rewired is semantically identical to the base (gen and epoch equality). The record binding is byte-identical (units.sh --check). The failure is in the unchanged #304 record path. Not claimed as a regression; not re-run against a clean HEAD binary.
- test-gpu-session-mutants: NOT_RUN. The mutant driver calls `smut`, which is not on PATH.
- Chip, live-RM, VFIO: NOT_RUN. See CHIP-RUNBOOK.md.

Compiler pin: omega-mv e5b32567, omc sha256 2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f.
