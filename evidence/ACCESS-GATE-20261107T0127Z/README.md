# ACCESS-GATE-20261107T0127Z: host gate for the cloud-window receipt tools

Status: host EXERCISED on the Spark's CPU (Linux 7.0.0-1019-nvidia aarch64), cores 10-11, nice 10.
No GPU was opened and no network was used. This receipt is about the receipt tools only: no AMD,
Intel or Google TPU device was reached, and every hardware row in `docs/access/` stays NOT_RUN.

Source: CUDApp `cce2886ed46af10b59b314b3ba17a189410ce600` (branch `hive/t_e308e9c5-access-packets`),
clean tree (`git status --porcelain` empty, `env.txt`). Compiler: omega-mv `e5b32567` `seed/out/omc`,
sha256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f` (the gate refuses any other).

## Command

```
ACCESS_OUT=<out> sh tests/access/run.sh <abs>/omc      -> run.log, exit 0
```

`run.log` has the home directory replaced by `<home>` and the output directory by `<out>`; nothing
else was changed. sha256: `run.log` `38c68612…3384`, `env.txt` `d6173744…3d10`.

## Result

`ACCESS GATE PASS pass=52 fail=0`.

| Group | Cases | Outcome |
|---|---|---|
| SYNTHETIC fixtures accepted with exact counts (AMD, Intel, TPU-blocked) and one non-synthetic window id | 4 | all ACCEPT, the fixtures as `ACCEPT-SYNTHETIC` |
| One-line mutants refused, each for its own reason and line | 40 | all REFUSE as expected |
| Empty input; input of 65536 bytes | 2 | REFUSE exit 1; exit 2 |
| Checker mutant (`if ne get dcls i ulong 1` replaced by a false condition) accepts the CPU-pass receipt the real checker refuses | 1 | ACCEPT, so the gate notices the rule's removal |
| Collector end to end on this host: accept with claim NONE; CPU-pass refused; ACCESS_TESTED without an accelerator refused; an edited log fails SHA256SUMS; no accelerator DEVICE line on this host | 5 | as expected |

Binaries: `rcheck` sha256 `1b4d14fc28e6b757a6c7cb1c13dcf6ead09a5d7f2424fb2c19973db81cd6ddd7` from source
`45d131ab68d88f52f5880122d9338ee69dba30c5abc82710a8eeb768bcdf9e93`.

## Limits

- The fixtures are synthetic by construction (`window=SYNTHETIC-…`, `model=SYNTHETIC…`); they test
  the checker, not any machine.
- The collector self-test ran on an NVIDIA GB10 host with no AMD, Intel or TPU accelerator, so only
  its no-accelerator paths ran. Device detection on a real MI300X, Intel GPU or TPU VM is NOT_RUN.
- `collect.sh` on x86-64 provider VMs is NOT_RUN.
