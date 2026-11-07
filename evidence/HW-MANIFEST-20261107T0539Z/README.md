# HW-MANIFEST-20261107T0539Z: hardware capability manifest gate

Status: CHIP_TESTED on two devices of one DGX Spark (`spark-b87b`, Linux 7.0.0-1019-nvidia aarch64):
the Cortex-X925 at processor 19 (`midr_el1=0x410fd851`) and the Nuvoton NPCT75x TPM 2.0 at
`/dev/tpmrm0` (manufacturer `0x4e544300`, firmware `0x00070002.0x00040001`, revision 1.59).
One run, one machine: EXERCISED, not QUALIFIED. The GPU was not opened. Every other register row is
unchanged by this run (`backend/hw/manifest.tsv`).

Source: CUDApp `b774723f537cec16a359fe082b8100317a5b5490` plus the files of this change; digests in
`outputs.sha256`. Device identity: `identity.txt` (hostname, `lscpu`, per-processor MIDR,
`nvidia-smi -L`, `tpm2_getcap properties-fixed`, tpm2-tools version).

## Command

```
HW_OUT=/tmp/cudapp-hw-20261107T0539Z flock -o <cpu-pool lock> taskset -c 19 nice -n 10 \
  sh tests/hw/run.sh <abs>/omc                                   -> run.log, exit 0
```

## Result (run.log)

`HW pass=71 fail=0 not_run=2`. Per-check output: `logs/<check>.log`.

CPU oracles were computed in the Wolfram Language, not by the code under test:

```
ss[n_]:=Mod[Sum[i^2,{i,0,n-1}],2^64]
fnv[n_]:=Fold[Mod[BitXor[#1,#2]*1099511628211,2^64]&,14695981039346656037,Mod[Range[0,n-1],251]]
{ss[1],ss[1000],ss[16777216],fnv[1],fnv[1000],fnv[1000000]}
-> {0,332833500,6148773953750958080,12638153115695167455,10432030352173600877,4529795169249947525}
```

TPM properties were compared with `tpm2_getcap properties-fixed` (tpm2-tools, an independent ESAPI
client); two 32-byte `TPM2_GetRandom` results were required to differ.

Negative controls (each exits with its class): 12 mutated manifests (exit 3) plus an absent one (74);
unknown target 4; unsupported operation and fault kind 5; not admitted, GPU without a spark-window lease,
and runner/route authority mismatch 6; stale generation replay, wrong-target handle and corrupt handle 7;
MIDR and TPM manufacturer mismatch 8; handle locked by another process 9; absent node 10; zero budget 11
(CPU and TPM); wrong oracle value 12; invalid argument and generation 14. TPM `badsize` was refused by
the kernel (`-22`), `badcc` by the TPM (`rc=0x143`, TPM_RC_COMMAND_CODE).

NOT_RUN: the unpinned-process control (needs a CPU set wider than the one allotted processor) and the
Cortex-A725 row (no A725 processor allotted).
