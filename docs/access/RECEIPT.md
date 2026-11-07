# ACCESS1: the receipt for one cloud window

One window (one instance, one plan file, one sitting) gives one receipt: a directory with
`receipt.txt`, `plan.tsv`, `meta`, `receipt.body`, `rows/*.log` and `SHA256SUMS`. It is written by
`tools/access/collect.sh` and checked by `tools/access/rcheck.om`. Receipts are never edited after
`close`. A correction is a new file next to the receipt, as everywhere else in `evidence/`.

## Flow

```
# 1. On the Spark: freeze what goes to the VM.
git rev-parse HEAD                         # ACCESS_REV
# copy tools/access/collect.sh, the plan file and any bundle files to the VM (scp)

# 2. On the provider VM.
ACCESS_WINDOW=amd-w0-20261120 ACCESS_REPO=aien-dev/CUDApp ACCESS_REV=<40 hex> \
  sh collect.sh run amd-devcloud amd-mi300x.tsv run-amd-w0

# 3. Copy run-amd-w0/ back to the Spark (scp -r). Destroy the instance. In the provider console,
#    check that the instance, its volumes and its IP are gone, and read the final charge.

# 4. On the Spark.
RCHECK=<binary built from tools/access/rcheck.om> \
  sh tools/access/collect.sh close run-amd-w0 ACCESS_TESTED 3.89 yes drake 2026-11-20T18:05:00Z
```

`close` verifies `SHA256SUMS` first (a copied file that changed makes it exit 1), writes
`receipt.txt`, then runs the checker. Commit the whole directory under
`evidence/ACCESS-<provider>-<window>/` with a README in the usual receipt form.

## Lines

`receipt.txt` is ASCII, one record per line, `TAG key=value ...`. Values have no spaces; the
collector turns any other byte into `_`. A key counts only after a space, so a value that contains
`exit=0` is not an exit field. Order: header first, END last; the others in any order.

| Line | Keys | Rules rcheck enforces |
|---|---|---|
| `ACCESS1` | `provider` `window` `claim` | first line, once; provider `amd-devcloud`, `intel-cloud` or `google-trc`; claim `ACCESS_TESTED` or `NONE`; a window that starts with `SYNTHETIC` is reported `ACCEPT-SYNTHETIC` |
| `SRC` | `repo` `rev` | at least one; rev is 40 lowercase hex |
| `WHEN` | `start` / `end` | recorded, not checked |
| `HOST` | `os` `kernel` `arch` | recorded, not checked |
| `DEVICE` | `id` `vendor` `class` `model`, plus `arch` `pci` `count` `driver` `kmod` when known | id `d0`..`d7`, each once; vendor `amd` `intel` `google` `other`; class `gpu` `tpu` `cpu`. The provider's accelerator is vendor amd + class gpu (AMD), intel + gpu (Intel), google + tpu (TRC). The collector writes `d0` only for that accelerator and `d7` for the host CPU |
| `TOOL` | `name` `path` `sha256` | recorded, not checked |
| `ROW` | `id` `kind` `want` `verdict` `dev`, and for PASS or FAIL `exit` `log` | kind `inventory` `reference` `candidate` `control`; verdict `PASS` `FAIL` `NOT_RUN` `MISSING_IMPLEMENTATION` `BLOCKED_ACCESS` |
| `COST` | `charged_usd` | at least one; `DIGITS` or `DIGITS.DIGITS` |
| `TEARDOWN` | `destroyed` `by` `verified_at` | exactly one; destroyed `yes` or `no` |
| `END` | `rows` | last non-empty line; equals the number of ROW lines |

Row rules:

- A PASS or FAIL row names a declared device, a decimal exit and a log.
- An inventory, reference or candidate row may PASS only on the provider's accelerator (`d0` as
  above) and only with exit 0. A PASS on the host CPU, on another vendor's device or on the wrong
  class is refused. This is the rule that keeps a CPU result from being reported as a GPU result.
- A control row is a planted-wrong check. It passes only when it exited with its nonzero `want`.
- Any candidate PASS needs at least one control PASS in the same receipt.
- `claim=ACCESS_TESTED` needs at least one inventory PASS and one reference PASS on the
  accelerator, and `destroyed=yes`.
- NOT_RUN, MISSING_IMPLEMENTATION and BLOCKED_ACCESS rows are counted and printed. They are never
  counted as PASS.

## Output

Accept, exit 0, stdout:

```
RCHECK ACCEPT provider=amd-devcloud claim=ACCESS_TESTED rows=11 pass=6 fail=0 not_run=1 missing=4 blocked=0
```

Refuse, exit 1, stderr: `RCHECK REFUSE line=<n> reason=<reason>`. Input over 65535 bytes or
unreadable: exit 2. The full reason list is the `text r...` table at the top of `rcheck.om`; each
reason has a refusing case in `tests/access/run.sh`.

The example line above shows the format only. No real receipt exists yet.
