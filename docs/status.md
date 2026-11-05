# Encoding status

Host golden words are not a chip result. A row marked NOT QUALIFIED has no GB10 receipt. Do not read it as qualified, exercised, or merged. Status words in `README.md` and `ROADMAP.md` stay PASS, PARTIAL, MISSING, and BUILDING. This page is the list of encodings this tree still owes a chip receipt for.

| Encoding | Where | Host evidence | Chip |
| --- | --- | --- | --- |
| `ATOMG.E.ADD.F32.FTZ.RN.STRONG.SYS` | `om/encoder/bw_enc_sm121.om` `enc_e` opcode 62; `src/omega_blackwell_codegen.c` `BW_IR_ATOMG_ADD_F32_STRONG_SYS` | Golden words in `omega_blackwell_verify_codegen_fixtures_atomf32` | NOT QUALIFIED (pending a GB10 receipt). `tests/gpu_atomf32_probe_test.c` is NOT RUN. BLOCKED_OPERATOR. No authorized GPU window. |
| `ATOMG.E.ADD.F32.FTZ.RN.STRONG.GPU` | `om/encoder/bw_enc_sm121.om` `enc_e` opcode 63; `src/omega_blackwell_codegen.c` `BW_IR_ATOMG_ADD_F32_STRONG_GPU` | Same host fixture, including the RZ destination form | NOT QUALIFIED (pending a GB10 receipt). Same probe, NOT RUN. BLOCKED_OPERATOR. |
| Destination presence slot 15 (`put o 15 pres d`) | `om/encoder/bw_enc_sm121.om` line that writes operand slot 15 from the destination vreg | The sm_121 encoder sets that presence bit so a negative destination vreg can encode RZ. Portable units regenerate from the pinned tools. The SM86 offline oracle does not execute this field on a GB10. | NOT QUALIFIED (pending a GB10 receipt). No sm_121 chip run covers this field. |
