# Native formatter identifier repair

BridgeHost reported that real freestanding aggregate generation with pinned
omega-mv e5b32567 / compiler SHA-256
`2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`
refused `native.om` line 2341 with error 16. The exported `omnum` function used
`out`, an OM-2 reserved word, as its output-view parameter. BridgeHost's minimal
`om2` reproduction refused `out` and accepted `buf`; its probe log was
`/tmp/bh/freestanding-aggregate-probe.log`. These are BridgeHost-reported results,
not a new independent native qualification by this lane.

Repair: rename that parameter and its two body references from `out` to `buf`
in `om/core/fmtnum.om`, then regenerate the composed native source and checked
portable units. Parameter order, type, extent and computation are unchanged.
No public machine-slot ABI, compiler pin or bridge pin changed.

Observed locally, with every command prefixed by:

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10
```

- `env OMEGA_OMC=$HOME/work/cudastack/omega-mv/seed/out/omc OMEGA_BRIDGE=$HOME/work/cudastack/omega-bridge sh om/units.sh`: rc 0, wrote checked portable units. Log: `native-fmt-regenerate.log`.
- `sh om/core/native.sh`: rc 0, `CORE-UTIL standalone native inputs write PASS`.
- `sh om/core/native.sh --check`: rc 0, `CORE-UTIL standalone native inputs --check PASS`.
- `sh om/units.sh --check`: rc 0, `PASS portable Omega units regenerate byte for byte`; log: `native-fmt-check.log`.
- `crumb compile .` and `crumb verify .`: rc 0; all 40 crumbs current.

The changed source and native composition were handed back to BridgeHost for its
aggregate continuation. Native execution, the integrated full gate and all
hardware tiers remain NOT_RUN by this repair lane. No behavioral suite was
rerun for this identifier-only change; prior host evidence describes the
implementation before this source-name correction.
