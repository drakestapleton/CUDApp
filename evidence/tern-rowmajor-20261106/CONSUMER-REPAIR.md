# Published consumer repair addendum

This addendum records evidence received after the original README was committed.
The README's statement that the API-rest consumer repair was then uncommitted
remains an accurate collection-time statement; the old receipt is unchanged.

The consumer repair is now published in draft
[CUDApp #333](https://github.com/aien-dev/CUDApp/pull/333), head
`5dad87a569f4fcca3410c4558691b4fdcb9589ca`, branch
`feat/api-views-rest-om`.

Fresh GitHub PR metadata confirmed that head and draft state. The committed
consumer `tests/api_rest_families.sh` compiles the external algebra oracle
separately with function/data sections, then pre-includes the current
`tests/bw_warp_sim_k.h` only into the external tern test translation unit.
The link uses section GC. This is the consumer correction diagnosed here.
No generator, packing rule or expected output was changed to accommodate the
stale simulator.

The API-rest lane's filed receipt is:

`evidence/api-views-rest-2026-11-06/continuation/families-final.txt`

TernSimFix read that receipt, rather than rerunning the lane's family suite.
It records the required bwrap wrapper at line 1, tern host PASS with 56 checks
at line 355, tern sim PASS with 34 checks at line 360, and final
`PASS api-rest family host/sim; chip NOT_RUN`, rc 0. These are the other lane's
filed execution results, distinct from TernSimFix's independent reproductions
and regression executions in this directory.

No merge, deployment or chip qualification is implied. Integrator4 owns adoption
of both draft PRs and the standalone tern target's canonical dependency wiring.
