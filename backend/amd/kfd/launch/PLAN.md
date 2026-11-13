# OMPLAN01: one decode plan for the native Omega launcher

This is the native launcher's binary plan contract. First-party plan emitters
record the model's dispatch list and checkpoint byte ranges. Production execution
loads weights directly; external controls may embed deliberately altered vectors.

CLI: `omega-launch plan PLAN CODE_0 .. CODE_{n_code-1} WEIGHT_0 .. WEIGHT_{n_file-1}`; argc = 3 + n_code + n_file.
Weight arguments follow the order chosen by the emitter; the model fixture lists that order in `PLAN.json` `files`. The reader validates each file independently.
All plan, file and ELF bounds are validated before /dev/kfd is opened. Little-endian throughout.

## Layout (absolute file offsets)
Header, 64 bytes: `"OMPLAN01"`, then u64 n_code, n_file, n_buf, n_disp, n_prompt, n_gen, tok_buf.
- n_prompt >= 1, n_gen >= 1. Steps executed = n_prompt + n_gen - 1. Step i < n_prompt: host writes prompt[i] into the token cell before the step. The token after step n_prompt-1 is generated token 0; step n_prompt-1+j yields generated token j.
- tok_buf: buffer index of the int32 token cell (`argmax_final` writes it, `embed5120_step` reads it). Its flags are 2 (GTT).

Code table, n_code x 16: u64 name_offset (relative to the string pool), u64 name_length. Name = kernel symbol; code object i is CLI argument CODE_i. Group segment, private segment and kernarg size come from the validated kernel descriptor (`symbol.kd`), not from the plan.

Buffer table, n_buf x 48: u64 bytes, kind, file_index, file_offset, file_length, flags.
- kind 0: zero-filled; file fields 0.
- kind 1: raw copy of file_length bytes at file_offset of WEIGHT_file_index; bytes >= file_length.
- kind 2: BF16 -> F32 widen (1-D norm/bias vectors, exactly as Card.__init__ widens); file_length even; bytes >= 2 * file_length.
- kind 3: bytes from the plan data pool; file_offset relative to the data pool, file_index 0; bytes = file_length (RoPE cos/sin table, computed by the harness exactly as Card does).
- Kinds 0 to 3 only. The CONTROLS plan ships its 161 mutated norm vectors (checkpoint vector widened, minus 1.0, as Card does in mode `controls`) as kind 3 data; `PLAN.json` `control_vectors` names each source range so the checker can prove it.
- flags exactly 1 (VRAM) or 2 (GTT). Any bytes beyond the filled length are zero.

Prompt ids: n_prompt x i32, then zero padding to the next multiple of 8.

Dispatch table, n_disp entries in order, each:
- u32 code_index, gx, gy, gz (=1), block_x (=64), kernarg_bytes (= descriptor kernarg size), n_reloc, pad 0;
- kernarg bytes; scalars are u32 at natural alignment, pointer slots are 8-aligned and zero in the plan;
- n_reloc x 16: u32 kernarg_offset, u32 buffer_index, u64 byte_offset (at least 1 byte inside the buffer; slot 8-aligned and zero; no duplicate slot);
- zero padding to the next multiple of 8.
The dispatch table ends exactly at the string pool.

String pool, then data pool immediately after it, then a 32-byte trailer immediately after the data pool:
u64 string_pool_offset, string_pool_length, data_pool_offset, data_pool_length (absolute).

## Execution
Packets in table order, barrier bit on every packet (same strict order as the HIP stream), completion signal on the last packet of each step; per-step timeout 10 s. The dispatch list is identical every step (device step counter `step_advance`), so packets can be built once.
Output per step: `OMEGA_STEP i tok=T us=U` (us = launcher wall clock from first packet write to signal). Final line `OMEGA_PLAN ids=[...]` with the n_gen generated tokens.

## Model fixture gate
- OMEGA native: 24 generated ids equal golden exactly.
- CONTROLS: the controls plan (kind 3 mutated norms) must not produce the golden ids.
- Beside it, same session: real2 HIP graph arm and AMD library reference timings.
`PLAN.json` beside each plan records sha256, buffers, device bytes, dispatches per step, kernel counts and code object sha256s.
