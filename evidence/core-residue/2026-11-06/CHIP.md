# Core residue chip runbook

Hardware verdict: **NOT_RUN**. This lane has no GPU window, live-RM session or VFIO authorization. Host differentials and killed mutants are not chip qualification.

## Device-free preflight

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 \
  make -j3 test-core-util test-core-cubin-message test-core-residue test-core-residue-mutants test-om-cubin
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 sh om/units.sh --check
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 sh om/core/native.sh --check
```

Pins: omc `e5b32567`, sha256 `2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`. `OMEGA_OMC=~/work/cudastack/omega-mv/seed/out/omc`. `OMEGA_BRIDGE=~/work/cudastack/omega-bridge`. At most three compile jobs. Cores 10–14.

The residue gate writes cubin images only under `/tmp` and unlinks them. It must not commit a cubin. A missing `/dev/nvidia*` node is not a reason to leave the namespace.

## Authorized chip acceptance

After a fresh operator window names the device and the interruption:

1. Pack sm_86 and sm_121 images on the host with the default Omega path. Confirm ELF magic and that the byte-identical C differential still agrees. Do not load an sm_86 image on an sm_121 device.
2. Run the platform's existing sm_121 vector route. Compare outputs with the host oracle, including one refusal. An sm_86 execution check needs a separately authorized sm_86 machine.
3. Record the platform command, expected versus observed, the negative control, the return code and device logs.

If no authorized device is available, the hardware tier stays **NOT_RUN**. Host PASS is EXERCISED, not QUALIFIED, MERGED or DEPLOYED.
