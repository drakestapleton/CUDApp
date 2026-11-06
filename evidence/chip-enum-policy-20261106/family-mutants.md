# Admission family negative controls

Command rc 0, final line `chip admission mutants: 8 of 8 red`:

```
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 env OMEGA_OMC=$HOME/work/cudastack/omega-mv/seed/out/omc OMEGA_BRIDGE=$HOME/work/cudastack/omega-bridge make -j3 OMEGA_MV=$HOME/work/cudastack/omega-mv OMEGA_BRIDGE_DIR=$HOME/work/cudastack/omega-bridge test-chip-admission-mutants
```

Five existing admission .om mutants and three C caller ordering mutants were rejected. In particular, removing pre-sysfs or card-info refusal, or removing the target consistency guard, fails the simulated-driver gate. These are additional to the seven new enumeration .om mutants. All device opens in this gate are intercepted simulated operations; the outer bwrap also hides the host device tree.

The prior invocation without OMEGA_MV/OMEGA_BRIDGE_DIR returned make rc 2, final `NOT_RUN: set OMEGA_MV and OMEGA_BRIDGE_DIR`. It did not exercise mutants and is not counted as PASS. family-mutants.log records the corrected run; initial-prerequisites.log records that earlier refusal.
