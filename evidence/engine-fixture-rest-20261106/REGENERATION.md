# Final generated-source check — 2026-11-06

Using the exact compiler/bridge/seed environment documented in README.md:

```sh
bwrap --dev-bind / / --dev /dev --proc /proc -- \
  taskset -c 10-14 nice -n 10 sh om/units.sh --check
```

Observed rc: 0. Final line:
`PASS portable Omega units regenerate byte for byte`.

Full text: `units-check.log`. Hardware and live-RM remain NOT_RUN.
