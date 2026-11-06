# CUDA++ fixed latencies vs NAK

Mesa commit `01bab9302a6bf0a082c7ba8c49d74427cf10b0cf`.
Clock: nak_private.h:135 NAK_SV_CLOCK_LO = 0x50; from_nir.rs:3522 SM>=50 uses CS2R; sm70_encode.rs:4278 opcode 0x805, index bits 72..80.
Priority 1 pairs match NAK on paper. They are measured because that match is circular.
Rows below are the ones that differ. A blank NAK stall means NAK calls the op decoupled.

| id | pair | CUDA++ | NAK | kernel |
|---|---|---|---|---|
| pred_attention | Predicate write -> guard, attention emitter | 13 | 13 + 1 = 14 | tests/gpu_latmeas_test.c (pred) |
| hmma_reader | HMMA -> non-HMMA reader (FADD of d0) | 15 + NOP 14 = 29 | 19 + 9 = 28 | tests/gpu_latmeas_test.c (hmma_reader) |
| imad_wide_lo | IMAD.WIDE low half -> IADD3 | 6 (blanket K_FIXED; IMAD.WIDE is not in fixed_pair_class) | 3 + 1 = 4 | tests/gpu_latmeas_test.c (imad_wide_lo) |
| fmnmx_alu | FMNMX (dualalu) -> IADD3 (alu) | 6 (blanket K_FIXED; FMNMX is not in fixed_pair_class) | 4 + 1 = 5 | tests/gpu_latmeas_test.c (fmnmx_alu) |
| qmma_reader | QMMA (IMMA) -> next reader | 15 + NOP 15 = 30 | 19 + 7 = 26 | not generated (patched QMMA placeholder; see REPRODUCE.md) |
| numeric_min | Fixed-producer minimum in the numeric checker | 5 (any fixed producer; mixed pairs need 6) | 5 + 1 = 6 for mixed ALU/FMA | tests/gpu_latmeas_test.c (mixed) |
| bssy | BSSY stall | 1, no scoreboard | decoupled (1 & sb) | not generated (NAK class is decoupled, not a fixed GPR RAW) |
| bsync | BSYNC stall | 5, no scoreboard | decoupled (1 & sb) | not generated (NAK class is decoupled, not a fixed GPR RAW) |
| bra | BRA stall | 5 | decoupled (1 & sb) | not generated (NAK class is decoupled, not a fixed GPR RAW) |
| exit_op | EXIT stall | 5 | decoupled (1 & sb) | not generated (NAK class is decoupled, not a fixed GPR RAW) |
| bar | BAR.SYNC stall | 6 | decoupled AGU (1 & sb) | not generated (NAK class is decoupled, not a fixed GPR RAW) |

