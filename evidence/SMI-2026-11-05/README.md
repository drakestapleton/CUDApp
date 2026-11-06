<!-- SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception -->
# SMI-2026-11-05: cudapp-smi host and live read-only receipts

Machine: DGX Spark GB10 (sm_121), driver 580.173.02, Linux aarch64.
Compiler: omega-mv `e5b32567` seeded omc, sha256
`2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
Paths under the user's home are written as `~`.

## Live read-only tier (`live/`)

Run 2026-11-06T02:51Z (2026-11-05 America/Chicago) from branch `feat/cudapp-smi` at `3aae7a2`; `host-run.log` was produced in the same session at the same commit:

```sh
taskset -c 10-14 nice -n 10 sh tests/smi/live.sh "$OMEGA_OMC" evidence/SMI-2026-11-05/live
```

rc 0. Last line `CUDAPP-SMI LIVE PASS (20 fields)`. The comparator negative
control (altered uuid, altered sub-device id) rejected 2/2 (`live/run.log`).

`cudapp-smi` ran without `--rm`: it read `/proc` and `/sys` only and opened
no device. `nvidia-smi` ran read-only queries (`--query-gpu`,
`--query-compute-apps`, `-q`). `live/compare.tsv` lists every field:

- MATCH (20): driver_version, count, pci.bus_id, pci.domain, pci.bus,
  pci.device, uuid, vbios_version, pci.device_id, pci.sub_device_id
  (compared numerically: 0x000010DE = 0x10DE), pci.baseClass, pci.subClass,
  pcie.link.gen.current/max, pcie.link.width.current/max, minor_number,
  gsp_firmware_version, compute-app pid 68451 found among the holders, and its
  process_name.
- RM_NOT_RUN (4): name, temperature.gpu, pstate and used_memory come only
  from the `--rm` path, which was not run.

## Not run

- `cudapp-smi --rm` on the live GPU: NOT_RUN. It opens `/dev/nvidiactl` and
  `/dev/nvidia0` and needs an authorized spark-window. Its logic is tested
  only against the host fake (`tests/smi/rmfake.om`); the fake replies are
  synthetic and are not a hardware record.
- Chip qualification: NOT_RUN.

## Files

| File | Content |
| --- | --- |
| `live/cudapp-smi.txt` | cudapp-smi report (real `/proc`, `/sys`) |
| `live/nvidia-smi-gpu.csv` | `nvidia-smi --query-gpu=...` |
| `live/nvidia-smi-apps.csv` | `nvidia-smi --query-compute-apps=pid,process_name,used_memory` |
| `live/nvidia-smi-q.txt` | `nvidia-smi -q` |
| `live/compare.tsv` | field-by-field verdicts |
| `live/run.log` | live.sh output |
| `host-run.log` | `tests/smi/run.sh` output (host tier) |
