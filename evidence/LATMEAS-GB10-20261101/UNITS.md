# Stall units: this kit and NAK at Mesa 01bab930

Mesa commit `01bab9302a6bf0a082c7ba8c49d74427cf10b0cf`. Paths are under `src/nouveau/compiler/nak/` unless noted. The conclusion is that our smallest passing stall S is the same field NAK writes, and the same kind of count (cycles from the producer issue to the consumer issue). NAK's published number is that count after it adds padding. The six fixed-latency pairs match the sm100 table before that padding. HMMA does not match either the bare table or NAK's padded total.

## What NAK writes into the instruction

`InstrDeps.delay` is a `u8` (`ir.rs:8722`). The minimum stored on an instruction is 1 (`MIN_INSTR_DELAY`, `ir.rs:8719`, applied at `calc_instr_deps.rs:1013`).

The encoder writes that delay into instruction bits 105..109 (`sm70_encode.rs:325`, `set_field(105..109, deps.delay)`). That range is four bits, so the field holds 0..15. Bit 105 is bit 9 of the control word, which is the stall field this repository's `cw()` shifts by 9. Yield on SM before 120 sits at bit 109 (`sm70_encode.rs:329-331`); SM120 and later do not use that bit for yield. The delay field itself is the same.

`max_instr_delay()` is 15 (`sm70.rs:300-301`). When `deps.delay` is larger, `calc_delays` keeps 15 on the instruction and pushes the remainder as following `NOP`s, each capped at 15 (`calc_instr_deps.rs:1031-1040`):

```text
delay = instr.deps.delay - 15
instr.deps = 15
while delay > 0:
    nop.delay = min(delay, 15)
    delay -= nop.delay
    emit nop after the instruction
```

The sum of those delay fields is the original delay. A NOP does not add a hidden extra cycle on top of its own delay field, and it does not drop out of the sum.

`exec_latency` of an ordinary ALU, FADD, HMMA, or NOP is 1 (`sm70.rs:183-199`, the default arm). `calc_delays` walks the block backwards. The consumer is given a cycle of `exec_latency`. The producer is then placed at `consumer_cycle + raw_latency` when that is the tighter bound (`calc_instr_deps.rs:811-812` and `838-846`). The delay stored on the producer is `producer_cycle - consumer_cycle` (`calc_instr_deps.rs:1011-1013`), so the `exec_latency` of 1 cancels. There is no second implicit +1 in that subtraction. The only +1 that survives into the field is the one inside `SM120Latency::raw`, below.

Instructions whose `exec_latency` is greater than 1 also get a following NOP of delay 2 (`calc_instr_deps.rs:1046-1053`). That arm does not apply to these pairs.

## Where the NAK number comes from

For a GPR raw on SM120, `sm70.rs:202-218` calls `SM120Latency::raw`. That function (`sm120_instr_latencies.rs:439-464`) reads the sm100 CSV and then adds

```text
max(latency_adjust(writer), latency_adjust(reader), 1)
```

`latency_adjust` (`sm120_instr_latencies.rs:431-436`) is 9 for `Hmma`, 7 for `Imma`, and 0 otherwise. The comment at `sm120_instr_latencies.rs:425-430` says the tables are SM100, the HMMA/IMMA adjusts were taken from CUDA SASS, and everything else still gets one cycle of padding because SM120 "has larger latencies."

So NAK's encoded delay for a coupled GPR pair is the CSV cell plus 1. For an HMMA write read by FADD (`FAdd` is `Fma`, line 60; the fma row of `latencies/sm100/reg_raw.csv` has column `hmma` = 19) it is 19 + 9 = 28, because 9 replaces the 1 rather than stacking on it. `Hmma` is also in `needs_scoreboards` (`sm120_instr_latencies.rs:413-421`). `raw()` does not return 0 for that. NAK still programs the delay of 28, and it also wants a scoreboard. This kit's HMMA sets no scoreboard (`SB_NONE`). The number 20 is a stall-only separation.

A predicate guard uses `paw_latency` (`sm70.rs:261-263`), which calls `raw(write, None)`. For a predicate that selects reader `RedirectedFp64` (`sm120_instr_latencies.rs:474-480`) and adds 1. `pred_raw.csv` row `redirected_fp64`, column `dualalu` is 13, so NAK's delay is 14. `ISetP` is `Dualalu` (line 49).

## This kit's S

S is the programmed stall on the producer, or, past 15, the producer's stall of 15 plus NOP stalls that sum to the rest (`gap_nops` in `tests/gpu_latmeas_test.c`). The consumer issues S cycles after the producer. That is the same sum `calc_delays` builds.

The smallest S at which every run matched, from `raw.json` in this directory:

| pair | CSV cell | NAK delay | measured S |
| --- | --- | --- | --- |
| ALU read of IADD3 | alu/alu = 4 | 5 | 4 |
| FMA read of FFMA | fma/fma = 4 | 5 | 4 |
| ALU read of FFMA and FMA read of IADD3 | 5 | 6 | 5 |
| guard read of ISETP | 13 | 14 | 13 |
| ALU read of FMNMX (`Dualalu`) | alu/dualalu = 4 | 5 | 4 |
| ALU read of IMAD.WIDE low | alu/imad_wide_write_dl = 3 | 4 | 3 |
| FMA read of HMMA | fma/hmma = 19 | 28 | 20 |

The six fixed-latency pairs equal the CSV cell. They are one less than the delay NAK writes. The units are the same; the difference is the padding at `sm120_instr_latencies.rs:458-463`.

## HMMA is not that pattern

Stall 20 in `sass/hmma_reader_20.pairs` is

```text
01c0 ... 000fde00... HMMA.16816.F32.BF16 R24, R16, R14, R20
01d0 ... 000fca00... NOP
01e0 ... FADD R10, R24, R5
```

Control `0x000fde00` is stall 15. Control `0x000fca00` is stall 5. The FADD issues 20 cycles after the HMMA. Stall 19 (`sass/hmma_reader_19.pairs`) is the same HMMA stall 15 plus a NOP of stall 4 (`0x000fc800`). That launch stored 0. Stall 20 stored `0x41800000`.

The split is the same one NAK uses (15 on the producer, the rest on NOPs). It does not turn a required separation of 19 into a reported 20: the failing program already summed to 19, and the passing program summed to 20. The FADD reads only `R24`, the first destination register. `R25` is stored later, after the second clock, so this run does not say whether d1..d3 were ready at the same cycle.

20 is the CSV cell plus the ordinary padding of 1, not plus the HMMA adjust of 9. NAK's 28 is that adjust, and it also asks for a scoreboard this kernel does not set. The six coupled pairs did not need their padding of 1. HMMA, measured with stalls only, needed one more cycle than the sm100 cell and did not need the extra eight.
