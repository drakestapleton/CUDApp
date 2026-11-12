# Parked attention investigation

Intent: single-token decode attention for the full-attention layers of the 27B model served on 2026-11-01. Parked before implementation at Main’s priority change to tuned GEMV.

Observed source: /home/drakestapleton/handoffs/Optimize27-seal-tune-20261101/workers-default-short-20261101T165016Z/server.log lines 1-8 identifies /home/drakestapleton/models/qwen3.8-27b-fp8-017b9c7a. That directory is absent from the current models listing. The independent historical /home/drakestapleton/handoffs/Q27/evidence/manifest-27b.sha256sumfmt records config.json SHA256 74227dd615bf1ea975aa676bdf355a0379858c12f394b5365cd9dfa5fc2c70bc (51350 bytes in rel1/x/share/aienos/model.manifest). The config contents were not recovered, so no attention dimensions are claimed.

XERJ map *handoff* returned no indexed datasets; direct scoped file search was used. An initial overbroad find timed out; subsequent bounded directories identified the serving evidence above.

Manual observation: AMD_Instinct_MI300_CDNA3_ISA.txt V_EXP_F32 opcode32 says “1ULP accuracy, denormals are flushed.” V_RCP_F32 opcode34 says “1ULP accuracy. Accuracy converges to < 0.5ULP when using the Newton-Raphson method and 2 FMA operations. Denormals are flushed.” No approximation or kernel has been implemented.

Resume: recover the digest-matching config and model q/k norm / gate evidence before choosing dimensions and implementing the attention contract. No builds or gates were run for this slice; no production files were edited.
