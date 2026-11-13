> Note added when this folder was packaged for the brief, 13 November 2026: this test was run against an earlier draft of the brief (sha256 `5f94d683…`). The three corrections it lists are in the brief you received. Run B below is the Level 2 block as the brief now prints it. Run A records how the earlier draft failed. Lane, lock and board lines are my own job-scheduling notes.

# Level 2 finishing test: public rebuild of the launcher and kernels

Lane `uxl-brief/PublicRebuild`, Drake session on the Mac, 2026-11-13 18:46Z to 18:55Z.
Brief tested: `../BRIEF.md`, sha256 `5f94d6836642c58c2d895a0c56607f1aa216adb1fe35b99bf6623fb129d10bd4` (not edited).

## Outcome

- **Level 2 rebuilds the card's launcher byte for byte from the public source.** `launch-x86` sha256 is `08a504911ec6e87a9746632a14d22245155597e3a8ed384c93c0e5260bb691e1`, the value the brief prints.
- **All 18 model kernel objects were rebuilt byte-identical** to the pinned list (`model-object-source-check.json`: `required` 18, `all_rebuilt` true, `controls_caught` missing and changed). I rehashed the 18 `.hsaco` files myself: 18 of 18 equal.
- **The brief's Level 2 block fails if pasted after Level 1 in the same shell** (`cd CUDApp` exits 2). Corrected commands are below, proved by run B.
- **`qemu-x86_64` is required, not optional.** Without it on `PATH` the gate exits 1 and prints no PASS line.
- **`sh seed/challenge/run.sh` is the wrong command for this brief** and is NOT_RUN. `sh seed/omega-from-seed.sh` is the command that rebuilds the brief's compiler; it was run and passed.

## Where and how

- Machine: the Spark (`ssh spark`), Linux 7.0.0-1019-nvidia aarch64, Ubuntu 24.04.5, 4096-byte pages, glibc 2.39. Full record: `logs/env.txt`.
- Scratch: `~/work/hive/uxl-brief-20261113/PublicRebuild/` on the Spark. `B/` (the rebuild tree, 719 MB) is kept; the throwaway control clones were deleted.
- Every job ran through `logs/pool.sh`: hive locks `pool-10-11`, `pool-10`, `pool-11`, cores 10 and 11, `nice -n 10`, and a clean environment (`env -i` with `HOME`, system `PATH`, `LANG`, `TERM`). No lane variable was set.
- Clones were fresh `git clone` over the two https URLs in the brief. No GPU, no port 18091, no pushes.
- Tools found on the system `PATH`: git 2.43.0, Python 3.12.3, cc = gcc 13.3.0, qemu-x86_64 8.2.2 (package `qemu-user`).

## Run B: the Level 2 commands from the directory that holds both clones (PASS)

Scripts `logs/runB-clone.sh` and `logs/runB-gate.sh`; output `logs/runB-clone.log`, `logs/runB-gate.log`, `gate-stdout.log`.

| Command | Exit | Result |
|---|---|---|
| `git clone https://www.aienos.com/rad/z4RxD8vXigmWg61nM8miQ8DVPAXEP.git CUDApp` | 0 | 118 MB on disk |
| `git checkout dc536882c06a692991be97dc65cd2d037d82b129` (inside CUDApp, then `cd ..`) | 0 | HEAD `dc536882…` |
| `git clone https://www.aienos.com/rad/z6do9WDBhuSnvopDkgFdFr1yWA62.git omega-mv` | 0 | 595 MB on disk |
| `git -C omega-mv checkout 2ee0a70d1830581b706f288e722f3329d51f3583` | 0 | HEAD `2ee0a70d…` |
| `sha256sum omega-mv/seed/out/omc` | 0 | `26540e6de62402d5784b30b24d7c6faae488137c1f8a4b3e784fd0b5d5528991` (expected, equal) |
| `cd CUDApp` | 0 | |
| `python3 -I tests/amd-kfd-launch/check.py "$(realpath ../omega-mv/seed/out/omc)"` | 0 | 283 stdout lines, 0 stderr bytes |
| `sha256sum build/amd-kfd-launch/gate/launch-x86` | 0 | `08a504911ec6e87a9746632a14d22245155597e3a8ed384c93c0e5260bb691e1` (expected, equal) |

Wall time, measured by `/usr/bin/time -v`:

- Both clones, both checkouts and the compiler hash: 13.49 s.
- `cd`, `check.py` and the launcher hash: 20.69 s (user 4.95 s, system 12.71 s, peak RSS 181,120 kB). The machine's load average was about 25 at the time.

Final gate line (stdout line 282 of 283; full text in `gate-stdout.log`):

```
AMD-KFD-LAUNCH CPU PASS {"binaries": {"a64": "2d6bfba8…9a47", "x86": "08a50491…91e1"}, "checks": 274, "compiler": "26540e6d…8991", "device": "NOT_RUN", "failures": 0, "objects": {…}, …}
```

Line 283 is `NOT_RUN every AMD device row: KFD queues, GPU mappings, completion, parity, timing, Qwen inference`. This level proves the build, not the card run.

`summary.json`: `checks` 274, `failures` 0, `qemu` `/usr/bin/qemu-x86_64`, `device` `NOT_RUN`, `orchestration` "native CPU syscall simulation only", `publication` "native instruction-step observation only". In the log, 274 lines start with `PASS ` and 0 with `FAIL `.

| File | Bytes | sha256 |
|---|---:|---|
| `launch-x86` | 888,401 | `08a504911ec6e87a9746632a14d22245155597e3a8ed384c93c0e5260bb691e1` |
| `launch-a64` | 130,036 | `2d6bfba83dcf836f879e318df9df2cc75bd46909f6173c7b3f7b7c36b9e89a47` |
| `vadd.hsaco` | 2,608 | `10eb12e019ff2117ddc0190650899d4254589375c62e5ebb970b1bbb7e67e55f` |

All three are byte-identical (`cmp`) to the lane build in `~/work/hive/amdserve/Integrate/suite-launcher-dc536882/launcher-out/` on the Spark.

The 18 kernel objects: `tests/amd-kfd-launch/qwen38-objects.json` (authority "objects exercised by the native full-model card gate") lists 18. Each rebuilt object's sha256 equals its entry there, and each hash also appears in the card run's `evidence/amd-qwen38-native-refusal-20261113/native/MANIFEST.json`. The launcher hash appears in that folder's `README.md`, `host/qualification.json` and `native/MANIFEST.json`.

## Run A: the brief's Level 1 and Level 2 blocks pasted in order into one shell (Level 2 FAILS)

Script `logs/runA-literal.sh` is cut from the brief's two fenced blocks by `awk`; output `logs/runA-literal.log`.

| Command | Exit | Output |
|---|---|---|
| Level 1: clone, `cd CUDApp`, checkout | 0, 0, 0 | |
| `python3 -I evidence/amd-qwen38-native-refusal-20261113/verify.py` | 0 | `EVIDENCE PASS 65 hashed files; raw steps/IDs; independent graph; reference coding24; calculations` (as the brief prints) |
| Level 2: `git clone … omega-mv` | 0 | lands in `CUDApp/omega-mv`, because Level 1 left the shell inside `CUDApp` |
| `git -C omega-mv checkout …`, `sha256sum omega-mv/seed/out/omc` | 0, 0 | hash equal |
| `cd CUDApp` | **2** | `cd: can't cd to CUDApp` |
| `python3 -I tests/amd-kfd-launch/check.py "$(realpath ../omega-mv/seed/out/omc)"` | **64** | `realpath: ../omega-mv/seed/out/omc: No such file or directory` |
| `sha256sum build/amd-kfd-launch/gate/launch-x86` | **1** | `No such file or directory` |

Read on its own, the Level 2 block also never clones or checks out `CUDApp`; it relies on Level 1 having been done.

## Corrections for the brief

1. **Level 2 block.** Make it start from the directory that holds `CUDApp`, and say that `CUDApp` must be at the pinned commit. Run B is exactly this sequence:

   ```sh
   cd ..        # leave CUDApp if you have just done Level 1
   git clone https://www.aienos.com/rad/z6do9WDBhuSnvopDkgFdFr1yWA62.git omega-mv
   git -C omega-mv checkout 2ee0a70d1830581b706f288e722f3329d51f3583
   sha256sum omega-mv/seed/out/omc

   cd CUDApp    # the Level 1 clone, at dc536882c06a692991be97dc65cd2d037d82b129
   python3 -I tests/amd-kfd-launch/check.py "$(realpath ../omega-mv/seed/out/omc)"
   sha256sum build/amd-kfd-launch/gate/launch-x86
   ```

2. **"plus `qemu-x86_64` if you want the x86 build exercised" is wrong.** With `qemu-x86_64` hidden from `PATH`, the same command exits 1 after 208 `PASS` lines with `FAIL native plan CPU contracts exit 1 expected 0`; the cause is `tests/amd-kfd-launch/plan-check.py` line 7, `qemu=shutil.which('qemu-x86_64');assert qemu`. No `AMD-KFD-LAUNCH CPU PASS` line and no `summary.json` are written. `launch-x86` is still built with the right hash before the failure. With the same `PATH` plus `qemu-x86_64`, the gate passes 274 of 274 (`logs/runC-controls.log`, `logs/noqemu-gate-stdout.log`, `logs/runD-more.log`).

3. **"run `sh seed/challenge/run.sh`" does not rebuild the brief's compiler.** At `omega-mv` `2ee0a70d`, `seed/challenge/expected.txt` pins commit `c945f974674cd5ba0276759d161888bc6735fd56` (committed 2026-11-06) and expects omc `3ad9f10f64309e7acf8a725082b28e23a2a4aeb07e98322fb4dc0fcd8fc93194`, 625,676 bytes. The brief's compiler is `26540e6d…8991`, 911,580 bytes. `CHALLENGE.md` says so itself: "The pin is fixed. Later commits change `seed/omc/omc.om`, so the omc hash of `main` moves". Its "Run it" block also tells the reader to clone from GitHub.

   The command that does what the brief's sentence says, run in `omega-mv` at `2ee0a70d` (`logs/runE-from-seed.sh`, `logs/runE-from-seed.log`):

   ```sh
   sh seed/omega-from-seed.sh
   ```

   | | |
   |---|---|
   | Exit code | 0, last line `RC=0` |
   | Wall time | 0.83 s in the clean run; 0.89 s in the earlier run with the checked-in `omc` moved aside first |
   | Result | `PASS omc fixpoint 26540e6de62402d5784b30b24d7c6faae488137c1f8a4b3e784fd0b5d5528991`; `seed/out/omc` sha256 is that value |
   | `git status --short --untracked-files=all` before | 0 lines |
   | `git status --short --untracked-files=all` after | 0 lines; `git diff --quiet` exits 0 |
   | Tracked files | It rewrites `seed/out/omc` and `seed/out/omc.sha256` in place with the same bytes (the mtime of `seed/out/omc` changed, the hashes did not). `seed/omc/size-history.tsv` is unchanged. |
   | Other output | `build/` (3.3 MB, git-ignored) |
   | Needs | AArch64 Linux, `sh`, `sed`, coreutils and `python3` (it calls `seed/omc/tests/size-record.py`) |

   In the moved-aside run the committed binary was absent, so the pass cannot have come from it; the seed-built file was byte-identical (`cmp`) to the checked-in one.

## Negative controls

| Control | Exit | Output | Evidence |
|---|---|---|---|
| `check.py` with a copy of `omc` whose last byte is XORed with 1 (sha256 `93a01e5877b9cc4eee7a96730d704688ed8f74ad3a1b4cb580285a9971d89f56`, 1 byte differs) | 69 | `REFUSED compiler digest` | No `build` directory existed afterwards (`ls -d build` exit 2) and `git status --short --ignored` printed nothing: it refused before building. |
| `check.py` with a relative compiler path | 64 | none | No `build` directory. This is why the brief's `$(realpath …)` matters. |
| The gate's own control | | `CONTROL wrong compiler refused before execution/output` (line in `gate-stdout.log`) | |
| Level 1 `verify.py` after one byte of `native/sample1.stdout` was changed in a throwaway clone | 1 | `AssertionError: native/sample1.stdout` | The same clone printed `EVIDENCE PASS 65 hashed files; …` with exit 0 before the change. |

## Prerequisites the brief does not list

Found by reading `tests/amd-kfd-launch/check.py`, `plan-check.py`, `trace.c`, `publication.c` and `tests/amd-kfd/layout-oracle.c`. "Seen" is what the Spark has; nothing older or newer was tried.

| Need | Why | Seen |
|---|---|---|
| AArch64 Linux host | `omc` is a static AArch64 ELF. Its two segments are aligned to 0x1000. | 4096-byte pages. Kernels with 16 KiB or 64 KiB pages were not tried. |
| `git`, https access to `www.aienos.com`, about 720 MB of disk | Full clones: 118 MB and 595 MB, plus 7.4 MB of build output. | git 2.43.0 |
| Python 3.9 or later, standard library only | `str.removeprefix` is used. [INFERENCE from the source; only 3.12.3 was run.] | 3.12.3 |
| `qemu-x86_64` on `PATH` under exactly that name | Required, see correction 2. It is the user-mode emulator, not a system emulator. | 8.2.2, Ubuntu package `qemu-user` |
| `cc` that compiles three C files cleanly under `-Wall -Wextra -Werror` | A compiler with extra warnings would fail the gate. [INFERENCE; only gcc 13.3.0 was run.] | gcc 13.3.0 |
| C library headers | `<elf.h>`, `<sys/ptrace.h>`, `<sys/uio.h>`, `<sys/mman.h>` | `libc6-dev` 2.39 |
| Linux kernel headers that include `<linux/kfd_ioctl.h>` and `<asm/ptrace.h>` | `layout-oracle.c` prints the KFD structure layout and the gate requires it to equal `backend/amd/kfd/requests.tsv`. | `linux-libc-dev` 6.8.0-139; the header says KFD 1.14 |
| ptrace allowed on one's own child process | `trace.c` and `publication.c` fork a child with `PTRACE_TRACEME` and use `PTRACE_SYSCALL`, `PTRACE_SINGLESTEP`, `PTRACE_GETREGSET`/`SETREGSET` (`NT_PRSTATUS`, `NT_ARM_SYSTEM_CALL`) and `PEEKDATA`/`POKEDATA`. A container or policy that blocks ptrace would fail it. [INFERENCE; not tried.] | `kernel.yama.ptrace_scope` = 1, not root |
| `memfd_create` system call | `trace.c` turns the launcher's device opens into `memfd_create`. | Linux 7.0 |
| No `/dev/kfd` on the build host | The gate takes its "absent KFD" path (exit 77 expected) when the device is missing; with a device present it takes a branch I did not run. | absent |
| `sha256sum`, `realpath` | Used by the brief's commands. | coreutils |

Other facts:

- No root was needed and no environment variable is required. `AMD_KFD_LAUNCH_OUT` moves the output folder.
- Each check has a 120 s limit (90 s inside `plan-check.py`).
- `check.py` refuses a compiler path that is not absolute (exit 64).

## Not run

- `sh seed/challenge/run.sh`: NOT_RUN. Its documentation gives no run time, and Main confirmed it stays NOT_RUN because it checks a different compiler (correction 3). What it would do: `git archive` the pinned commit `c945f974` into `build/seed/challenge/`, rebuild seed0, hex1, asm2, om4 and three omc stages there, compare each with `expected.txt`, and end `CHALLENGE PASS pin=… omc=…` (exit 0), `CHALLENGE FAIL` (exit 1) or `CHALLENGE NOT_RUN` (exit 3, wrong architecture or pin missing).
- Any AMD device step. The gate itself reports `device: NOT_RUN`.
- The gate on any other host, compiler, Python, qemu or kernel-header version than the ones listed.
- Level 3 and the interplane.io link in Level 1.

## Files here

- `launch-x86`, `launch-a64`, `vadd.hsaco`, `summary.json`, `model-object-source-check.json`: from `B/CUDApp/build/amd-kfd-launch/gate/` of run B.
- `gate-stdout.log`: full stdout of the run B gate (stderr was empty, `logs/gate-stderr.log`).
- `TRANSCRIPT.txt`: every script followed by its output, in run order.
- `logs/`: the same scripts and logs as separate files, the environment record and the BOARD claim line. `logs/runC-controls.log` has the shell trace of one symlink loop removed; `logs/runC-controls.NOTE.txt` gives the sha256 of the untrimmed file on the Spark.
- `SHA256SUMS`: sha256 of every file in this folder.
