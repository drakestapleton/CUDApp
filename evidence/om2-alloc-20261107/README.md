# bw_alloc.om composes into an OM-2 unit (2026-11-07, Hive NG lane ng-gpu)

Omega NG builds one OM-2 unit (`om2` first line, omega-mv `seed/ng/ng.units`, Hive NG D11) and consumes
`om/components/bw_alloc.om` and `om/encoder/bw_enc_sm121.om` by pin (D10). In OM-2 `at` is a keyword, and
`bc_touch` used it as a parameter name. The parameter is renamed `ix`; nothing else changes.

| check (omc e5b32567, seed/out/omc 5f90edea) | result |
|---|---|
| `om2` + bw_alloc.om (base) + bw_enc_sm121.om + empty main | `omc: line 8: error 16`, exit 16 (`om2-before.txt`) |
| same with this change | exit 0 (`om2-after.txt`) |
| `make test-om-units` | PASS, units regenerate byte for byte (`test-om-units.log`) |
| generated C of the units that include bw_alloc.om (compose, q38_emit, validate) | identical: only the `src=` hash in the `.ident` files changes, every `c=` hash is unchanged |
