# First attention-core device exercise

This supplements, and does not overwrite, the earlier immutable attention and preparation receipts. Main supplied the MI300X notebook log from `~/handoffs/hive/amdserve/evidence/AMD-MI300X-NOTEBOOK-20261113T000755Z/amdserve-probe6.txt`. The unmodified log and exact object digests are filed here. This lane did not operate the GPU.

| Scope | Evidence | Status |
|---|---|---|
| attn256_part + corrected attn256_merge, deterministic T=64,65,128,129,1000; random T=64,65,1000 | Filed log reports zero output/partial errors under the notebook float64 checks and zero guard corruption; see each printed worst error and threshold in the diagnostic source | EXERCISED on MI300X |
| Repeated T=129 | Six output runs identical; four part-only runs identical | EXERCISED on MI300X |
| Old no-NOP merge versus corrected merge | 384 different outputs at T=129; the only instruction change is the required RCP-consumer wait | Hardware evidence of the scheduling defect and repair |
| New prep and sigmoid-gate kernels; complete token path | Not exercised by this diagnostic | NOT_RUN |

The diagnostic is not the full interval/negative-control hardware gate and explicitly prints no RESULT verdict. No blanket hardware qualification, speed claim, merge or deployment follows from it.

The two `simple equal-scores` rows are retained verbatim despite their reported errors. Main's explanation: “I fed non-BF16-exact V; the counts 3840/4608 and worst 1.367e-2 reproduce exactly from BF16 truncation alone.” Those rows are excluded from the successful exercise claim; the next notebook reference must decode the actual uploaded BF16 bytes, not the unquantized inputs.

The CDNA3 Table11 static scheduling gate in `tests/amd/run.sh` remains mandatory for every discovered `.isa` object, including future objects. Functional execution alone cannot detect a missing issue delay. No committed object changed for this receipt.
