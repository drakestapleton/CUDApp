# D13: my own disassembler, in plain text a person can read

Labels: D13

**The pain.** PTX "is only partially documented, constantly shifting between hardware generations, and effectively a black box for developers." (Modular). CUDA++'s host tests still call NVIDIA's `nvdisasm` as a cross-check when it is installed.

**What is left.** A disassembler built from CUDA++'s own encoder tables, printing each instruction with its control word (stall, barriers) in words, and a timing view like the motor's. Omega bar C6, ledger L386.

**Done when.** It agrees with `nvdisasm` on every library kernel, and nothing in the tree calls `nvdisasm`.

**Status 2026-11-07 (CUDA++ C6, DRA-475, host only).** The decoder is seeded Omega (`om/disasm/sm121_dis.om`, through
the generated binding `src/cudapp_om_sm121_dis.c`); the hand-written C decoder is the migration-pending differential
oracle (`CUDAPP_MIGRATION_PENDING_C_DISASM`). On the whole corpus of 3,191 kernels (1,237,248 words) its text equals
nvdisasm 13.0.85's (`evidence/C6-DISASM-OM/`). Still open: the control word printed in words and the timing view.
