# Rung 5: one 27B decode step across 2, 4 and 8 MI300X cards (design, model, host-gated code)

State, 2026-11-13: **PROPOSED** (the design) and **IMPLEMENTED + EXERCISED on the host** (the code in `code/`,
gate `MCGATE PASS cases=203` on Linux AArch64 natively and on x86-64 under QEMU, which is EMULATED, not native).
Nothing here has run on an MI300X. The multi-card numbers are **MODELED**. Measured inputs are named with
their source. No GPU, account, or paid service was touched.

Pins: model `Qwen/Qwen3.8-27B-FP8` rev `017b9c7af6b5689d5dd426a76e0bc077eb5ca20a` (`sources/config.json`, sha256
`74227dd6…70bc`); single-card launcher and plan: CUDApp `dc536882`, receipt
`evidence/amd-qwen38-native-refusal-20261113` (26.726 ms/token median, 1,126 dispatches per step, 18 objects);
compiler omega-mv `2ee0a70d`, omc sha256 `26540e6de62402d5784b30b24d7c6faae488137c1f8a4b3e784fd0b5d5528991`
(the newest build: https://interplane.io/drakestapleton/omega-mv showed `main` at `2ee0a7…` when read at 23:35Z, HTTP 200;
the MCP server and the launcher's `COMPILER` file name the same omc; a trivial compile ran and exited 7 as expected);
kernel sources Linux **v6.18** (`sources/linux-v6.18/`, per-file sha256 in `SHA256SUMS`); ROCr/ROCT from the
`rocm-systems` checkout at `rocm-7.2.2` (`671d39a7`) that AmdLaunchGap already holds.

## 1. What one decode step reads (and a correction)

From `config.json` and the measured plan (`real2.plan.json` `by_kernel`): 64 layers, 48 gated delta-net (GDN)
and 16 full attention (every 4th). hidden 5120, MLP 17408, vocabulary 248320. Attention: 24 query heads ×256
with an output gate (`q_proj` 12288 rows), 4 kv heads ×256. GDN: 16 key heads ×128, 48 value heads ×128,
conv 4. FP8 E4M3 with 128×128 block scales, except `lm_head`, `embed_tokens` and GDN `in_proj_a/b`, which are
BF16 (`modules_to_not_convert` lists `lm_head`; the plan has 97 `bf16gemv2` = 48×2 + 1 and 400 `fp8gemv_b128f`
= 48×3 + 16×4 + 64×3 dispatches).

Weight bytes read per step, computed in Wolfram and recomputed by `mc_plan` (they agree exactly):

| Part | Bytes |
|---|---|
| FP8 matrices | 24,326,963,200 |
| their block scales | 5,939,200 |
| BF16 `in_proj_a/b` | 47,185,920 |
| BF16 `lm_head` | 2,542,796,800 |
| **total** | **26,922,885,120** |

**Correction to the rung's brief:** the stated 24.38 GB per step leaves out the BF16 `lm_head` (26.92 GB minus
2.54 GB = 24.380 GB, Wolfram). The plan the card ran reads `lm_head` on every step, so the single-card 8-bit
weight floor is 26.92 GB / 5.3 TB/s = **5.08 ms**, not 4.6 ms (MODELED). At 8 cards it is 0.639 ms, not 0.58 ms. Today's
26.726 ms is therefore 19.0 % of peak bandwidth (Wolfram: 26,922,885,120 / (5.3e12 × 26.726e-3) = 0.1901).
Running `lm_head` in 4 bits too brings the 4-bit total to 12.85 GB (2.42 ms on one card). With `lm_head` left
in BF16 it is 14.76 GB (2.78 ms).

These are weight floors. Two further terms add to them (Wolfram Q7):
- The GDN recurrent state is read and written every step: 48 layers × 48×128×128 f32 × 2 = 301,989,888 bytes,
  0.057 ms on one card, 1.1 % of the weights.
- The KV cache adds 65,536 bytes per context position (16 layers × 4 kv heads × 256 × k,v × BF16), which is
  0.012 ms per 1,000 positions on one card.
Both split across cards with their heads.

## 2. The split, layer by layer

Two modes. Both put every card through the whole layer sequence on its own share. The final hidden state is
held, identical, on every card.

**exact** splits only by output rows. Each card computes whole dot products for its rows with the same kernel
and the same reduction order that one card uses, so (given the same kernels) the result is bit-identical to one
card. Then each card gathers the full vector. **fast** is the Megatron pairing: column split followed by row
split, with an all-reduce. It needs half the exchanges, but it changes the summation order of `out_proj`,
`o_proj` and `down_proj`, so output tokens can change and would have to be measured and explained with logit
margins. Recommendation: build **exact** first and keep it as the correctness reference. Switch to fast only
with a measured logit-margin report.

Units are heads or 128-row FP8 scale blocks. Card c of N takes units [⌊cU/N⌋, ⌊(c+1)U/N⌋). Rows of one card,
for N = 2 / 4 / 8 (from `mc_plan`; every range ends on a head or a 128 boundary):

| Tensor (m × k) | Split | N=2 | N=4 | N=8 |
|---|---|---|---|---|
| GDN `in_proj_qkv` (10240×5120) | key heads; q, k and v segments | 8 kh: q 1024, k 1024, v 3072 | 4 kh: 512/512/1536 | 2 kh: 256/256/768 |
| GDN `in_proj_z` (6144×5120) | value heads of the card's key heads | 3072 | 1536 | 768 |
| GDN `in_proj_a`, `in_proj_b` (48×5120, BF16) | same value heads | 24 | 12 | 6 |
| GDN conv1d, `A_log`, `dt_bias`, state | follow the channels/heads | local | local | local |
| GDN `out_proj` (5120×6144) | exact: output rows / fast: input columns = its value heads | 2560 / 3072 cols | 1280 / 1536 | 640 / 768 |
| attn `q_proj` (12288×5120; `[24][512]`, Q then gate in each head: CUDApp `PROVENANCE.md:911`) | query heads | 12 heads, 6144 | 6 heads, 3072 | 3 heads, 1536 |
| attn `k_proj`, `v_proj` (1024×5120) | kv heads its query heads use | 2 kv heads | 1 | 1, **shared by 2 cards (replicated)** |
| attn `o_proj` (5120×6144) | exact: output rows / fast: input columns = its heads | 2560 / 3072 cols | 1280 / 1536 | 640 / 768 |
| MLP `gate_proj`, `up_proj` (17408×5120) | output rows (136 blocks) | 8704 | 4352 | 2176 |
| MLP `down_proj` (5120×17408) | exact: output rows / fast: input columns = its MLP rows | 2560 / 8704 cols | 1280 / 4352 | 640 / 2176 |
| `lm_head` (248320×5120, BF16) | vocabulary rows (64-row argmax units) | 124160 | 62080 | 31040 |
| `embed_tokens`, all norms | replicated (one 10 KiB row per step) | all | all | all |

The GDN core needs no exchange. Value head v uses key head ⌊v·16/48⌋. A card that holds key heads
[k0,k1) also holds value heads [3k0,3k1), so the conv, q/k norm, delta-rule state, and RMS gate all stay local.
Attention is the same: query head q uses kv head ⌊q·4/24⌋. At N=8 each kv head (256 rows of k and v, and its KV
cache) is computed on two cards. That costs 167,813,120 extra bytes over the model (`mc_plan`:
sum 27,090,698,240 − 26,922,885,120), 21 MB per card.

Per card per step: **13.46 GB (N=2), 6.73 GB (N=4), 3.39 GB (N=8)**, the same in both modes and balanced
exactly (max = min). N=3 and N=16 also pass the check, unevenly (16 cards: 1.55 to 1.87 GB).

**Kernel variants needed (owner: AmdKernelSpeed).** `fp8gemv_b128f` and `bf16gemv2` already take m and k and use
one workgroup per row, so a row slice is the same kernel with a sliced weight pointer. Fast mode's column split
needs a k-offset into the input vector. These kernels hard-code the full head counts and need a head count and a
head offset: `attn256_prep/part/merge_step` and `attn256_gate` (grid 24), the `gdn*` kernels (48 value heads,
16 key heads), and `argmax_part/final` (vocabulary offset, plus emitting (value, index) for the exchange). New:
one exchange kernel per vector size (section 4). Where a `residual5120` follows the exchange, the exchange
kernel does the residual add and takes its place:
- fast mode: both all-reduces are followed by a residual, so only the head exchange adds a dispatch (+1);
- exact mode: the gathers of the attention/GDN output and of the MLP activation are not followed by one, so it
  adds 2 per layer plus the head (+129).

## 3. Exchanges per step

| Mode | After each layer | Per step | Bytes each card sends per step, N=8 |
|---|---|---|---|
| exact | gather GDN/attention output (6144 f32), gather `out_proj`/`o_proj` rows (5120), gather MLP activation (17408), gather `down_proj` rows (5120) | 4×64 + 1 = **257** | 7,569,464 |
| fast | all-reduce after `out_proj`/`o_proj` (5120 f32), all-reduce after `down_proj` (5120) | 2×64 + 1 = **129** | 18,350,136 |

The "+1" is the head. Each card sends its local first maximum (f32 value, u32 index) of its vocabulary rows,
8 bytes. Every card then picks the largest value, with ties going to the lowest card. That is the same "first
maximum" rule as one card's scan, so the token is identical. At N=1 there are no exchanges.

## 4. Exchange mechanism through KFD

One process opens `/dev/kfd` once and binds every card's render node with ACQUIRE_VM. One KFD process holds at
most `MAX_GPU_INSTANCE` = 64 KFD nodes, where each partition counts (v6.18 `amdgpu.h:123`, `kfd_priv.h:930`).
The steps:

1. **Topology.** Read `/sys/class/kfd/kfd/topology/nodes/N` for N = 0, 1, ... until `gpu_id` is absent.
   - Nodes are numbered in list order, so the numbers are contiguous (`kfd_topology.c:820-834`).
   - Skip CPU nodes (`gpu_id` 0) and nodes the tenant may not read (the notebook VF returned EPERM for other
     tenants' nodes).
   - Choose N nodes with `gfx_target_version` 90402 and one `device_id`. 90402 is the gfx 9.4.2 number KFD gives
     IP 9.4.3/9.4.4, which covers MI300A, MI300X and MI325X; `device_id` tells them apart.
   - Require each ordered pair to share a nonzero `hive_id` and to have a direct `io_links` entry of type 11,
     XGMI (`kfd_crat.h:254`; field list `kfd_topology.c:263-280`).
   - Require the pair to be **two physical cards**. A partitioned card (CPX/DPX) shows each partition as its own
     node in the same hive, with XGMI links to all (`kfd_crat.c:2357-2372`). Partitions share the PCI domain and
     bus/device, and only the partition number is OR'ed into `location_id` (`kfd_topology.c:2093-2097`).
     `unique_id` is per partition (`:532-535`), so it cannot be used. The card key is therefore (`domain`,
     `location_id >> 3`), and two selected nodes of one card are refused (`reason=partition`).
   - Require a **whole card**: `num_xcc` 8. The measured notebook VF, a whole card in SPX mode, shows 8. A CPX or
     DPX SR-IOV VF has its own PCI address, so it passes the partition key, but it has fewer XCCs and is refused
     (`reason=partial_card`).
   - `node_to` is a proximity domain (`:1326-1327`). Nodes take proximity domains in increasing order
     (`:1870`, `:2200-2201`) and the sysfs index is the list position (`:827-831`), so `node_to` is the sysfs
     node number. H1 prints every link to confirm it.
2. **One receive buffer per card.** ALLOC_MEMORY_OF_GPU on card c, VRAM with flags
   `VRAM | EXT_COHERENT | PUBLIC | WRITABLE` = `0xa1000001` (`kfd_ioctl.h:407-421`). Then **MAP_MEMORY_TO_GPU
   with all N gpu_ids** (`kfd_ioctl.h:467-472`). The kernel maps that one buffer into the VM of every listed
   card (`kfd_chardev.c:1240-1320`) at the same virtual address, because the caller chose the VA. Another card's VRAM is
   accepted only inside one XGMI hive, or when `amdgpu_device_is_peer_accessible(owner, accessor)` holds
   (`amdgpu_amdkfd_gpuvm.c:880-902`). That function lives in upstream `amdgpu_device.c`, which is not in
   `sources/`. For PCIe it is the owner's BAR that matters, and PCIe P2P also needs `CONFIG_HSA_AMD_P2P`
   (`Kconfig`). Otherwise the call returns EINVAL. Step 1 checks the XGMI rule before any ioctl.
   - `PUBLIC` is requested only so that the host can check the buffer through the CPU. On a small-BAR card
     ALLOC itself returns EINVAL (`kfd_chardev.c:1105-1110`), and `mc_kfd` prints that hint.
   - On a map failure `mc_kfd` prints `n_success` and the gpu_id of the first card not mapped
     (`kfd_chardev.c:1294-1324`). A failure before that loop (`:1251-1290`) leaves 0. ROCr does the same thing: `hsa_amd_agents_allow_access` →
   `hsaKmtMapMemoryToGPUNodes` → this ioctl (`libhsakmt/src/memory.c:540-570`, `fmm.c:3368`).
3. **Coherence.** On gfx 9.4.3, an EXT_COHERENT buffer is MTYPE_CC where it is local and MTYPE_UC on the peers
   (`gmc_v9_0.c:1158-1196`). "Local" also requires the buffer's memory partition to be the VM's own
   (`KFD_XCP_MEM_ID == vm->mem_id`, true in NPS1/SPX). A peer's store therefore goes uncached across XGMI to the owner's HBM. The owner's
   load (with system-scope `sc0 sc1` acquire semantics in the kernel) sees it. Without EXT_COHERENT, the owner's
   default local MTYPE_RW (`amdgpu_mtype_local` 0; 1 gives NC, 2 gives CC) lets it cache a flag line that a peer
   has since written. [INFERENCE from the source; H2 below
   tests it.]
4. **Protocol (push, double-buffered, generation-stamped).** For exchange number g (counting through the whole
   run) with parity p = g & 1:
   - The sender writes its slice (gather) or its whole partial (all-reduce, slot [sender]) into parity buffer p
     of **every** receiver.
   - The sender issues a system-scope release, then stores g into the receiver's flag word [p][sender].
   - The receiver waits until all N flag words of parity p equal g, issues an acquire, and reads its own local
     buffer. All-reduce slots are summed in card order 0..N−1, so every card holds the same bits.
   - Double buffering is enough. A sender cannot send g+2 before every receiver has sent g+1, and each receiver
     sends g+1 only after it has read g.
   - Flags compare for equality, so a stale value (g−2, same parity) never satisfies a wait.
   - Remote stores only: every read is local VRAM.
   - Buffer size: two parities, each max(intermediate_size × 4, N × hidden_size × 4, N × 8) + 64·N bytes. The
     first term is the gather, the second the all-reduce slots, the third the head pairs; the 64·N is flags.
     Both sizes come from the model file. The total is rounded up to 2 MiB by choice (`kfdres` needs only a
     4096 multiple). That gives 2 MiB for N ≤ 8 and 4 MiB at N = 64.
5. **Where the wait runs.** Three options, cheapest first:
   - (a) **In the exchange kernel.** One workgroup stores its part to the peers, sets the flags, then spins on
     the N local flags and does the sum or copy into `h` (it also does the residual add). No host and no extra
     packet.
   - (b) **AQL barrier-AND packets.** Use `dep_signal[5]` (`hsa.h`, `hsa_barrier_and_packet_t`) on signals
     placed in peer-mapped memory. ROCr chains dependencies this way (`amd_blit_kernel.cpp:649-706`). That is
     one CP-side wait per exchange, plus one barrier packet per five signals.
   - (c) **Host-mediated.** Wait for each card's completion signal, then ring the next doorbells. KFD events
     (CREATE_EVENT / SET_EVENT / WAIT_EVENTS, `kfd_ioctl.h:282-363`, `libhsakmt/src/events.c:122-461`; ROCr's interrupt
     signal carries `event_mailbox_ptr`, `interrupt_signal.cpp:105-109`) only make host waits sleep. They do
     not take the host out of the path.
   - The design uses (a). (c) is the fallback. Its per-exchange cost is bounded below by the measured
     single-packet host round trip on the MI300X notebook: 29.271 µs and 44.080 µs (`vadd-lds` / `vadd`
     `dispatch_ns`, same receipt). An N-card host exchange itself is unmeasured.
6. **Launcher shape.** One AQL queue per card. Each card's step is its own packet list: the same 1,126
   dispatches on its shards, with exchange kernels in place of the residual adds that follow an exchange. Each
   card gets one doorbell per step. The host waits only on card 0's completion signal and reads the token there
   (all cards hold the same token). The existing launcher keeps one device in globals (`dev ulong 14`,
   `launch ...`). Several devices need those tables indexed by card. That is AmdLaunchGap's file set. `mc_kfd`
   does the multi-node part standalone, so nothing in their files changed.

## 5. Latency model (MODELED, Wolfram; queries and results in `model/WOLFRAM.md`)

ms/token = (per-card bytes) / (e × 5.3 TB/s) + S × t_x + (per-link transfer)

- **e** is the fraction of peak bandwidth: today's measured value is 0.190.
- **S** is the number of exchanges per step: 257 exact, 129 fast.
- **t_x** is one exchange's synchronisation latency. Nobody has measured it on XGMI. 2 µs and 10 µs are
  scenarios [INFERENCE]; 29.271 µs is the measured host round trip above.
- **Per-link transfer** is the bytes sent per peer over a 64 GB/s XGMI link. AMD's own RCCL blog puts usable
  bandwidth at 45 to 48 GB/s per link. At 48 GB/s the term grows by a third: 22.5 µs exact and 54.6 µs fast at
  N=8.

| e | t_x | mode | N=1 | N=2 | N=4 | N=8 |
|---|---|---|---|---|---|---|
| 0.190 (today) | 2 µs | exact | 26.726 | 13.945 | 7.229 | **3.892** |
| 0.190 | 29.3 µs (host) | exact | 26.726 | 20.953 | 14.238 | 10.901 |
| 0.8 | 2 µs | exact | 6.350 | 3.756 | 2.135 | 1.330 |
| 0.8 | 10 µs | exact | 6.350 | 5.812 | 4.191 | 3.386 |
| 0.8 | 2 µs | fast | 6.350 | 3.474 | 1.886 | 1.098 |
| 1.0 | 2 µs | fast | 5.080 | 2.839 | 1.569 | **0.938** |
| 1.0 | 29.3 µs (host) | exact | 5.080 | 10.130 | 8.826 | 8.178 |

Full grid: 3 values of e × 3 of t_x × 2 modes × 4 card counts, in `model/WOLFRAM.md`.

What the model says:

1. **The exchange count, not the bytes, dominates at 8 cards.** 257 exchanges at 2 µs cost 0.514 ms, about
   80 % of the 0.639 ms floor of reading the weights (Wolfram Q7: 0.804). In exact mode, a host-mediated exchange
   at 29 µs makes 2, 4 or 8 cards **slower than one card** once kernels reach 80 % of peak. Fast mode at
   e = 0.8: N=2 is 6.99 ms, slower than one card, but N=4 is 5.40 ms and N=8 is 4.62 ms, both faster. Multi-card is worth building only with a
   device-side exchange (5a) whose t_x is a few µs. That number has to be measured first (H2).
2. **It matters what today's 26.7 ms is made of.** If the 21.6 ms gap above the floor scales with bytes, 8 cards
   give 3.9 ms. If it is a fixed cost per dispatch (19.2 µs × 1,126), 8 cards give 25.3 ms in exact mode, which
   adds 129 exchange dispatches, and 22.6 ms in fast mode, which adds 1 (Wolfram Q7). AmdLaunchGap's measurement decides
   which, before any money is spent.
3. **0.1 ms per token cannot come from cards alone.** With k tokens per pass (rung 3, speculative decoding), the
   weight read and the exchange syncs are paid once per pass, but every exchange carries k vectors. So the
   time per token is (fixed per pass)/k + (link transfer per token) (Wolfram Q8; 8 cards, 4-bit including
   `lm_head`, e = 1, t_x = 2 µs):

   | Exchange | fixed per pass | link per token at 64 / 48 GB/s | k for 0.1 ms at 64 / 48 GB/s |
   |---|---|---|---|
   | fast, one-shot all-reduce | 0.563 ms | 0.041 / 0.055 ms | **9.5 / 12.4** |
   | exact, gathers | 0.819 ms | 0.017 / 0.023 ms | 9.9 / 10.6 |
   | fast, two-phase all-reduce (reduce-scatter + all-gather: 257 syncs, 1/4 the bytes at N=8) | 0.819 ms | 0.010 / 0.014 ms | 9.1 / 9.5 |

   This assumes every drafted token is accepted [INFERENCE]. At k = 6 the time is 0.13 to 0.15 ms per token. The
   link term is the floor that k cannot amortise, so the larger k gets, the more the lighter exchanges (exact
   gathers or the two-phase all-reduce) pay off.

## 6. Built and gated (host only)

Seeded Omega, composed as `cat mc_common.om <unit>.om | omc`, both AArch64 and `target x86linux`:

| Unit | Does | Gate (all in `code/gate.sh`) |
|---|---|---|
| `mc_topo` | Multi-node KFD topology scan, card selection, XGMI/hive/device/physical-card/whole-card checks, skips with errno | 8/4/1/64 cards select; refusals: too_few, mixed_device, hive, no_xgmi (including one missing direction), partition (CPX fixture), partial_card (VF fixture, num_xcc 1), malformed node (errno 22), unreadable nodes (errno 13), absent topology; CLI 64 |
| `mc_kfd` | One `/dev/kfd`; per-card render + ACQUIRE_VM; per-card EXT_COHERENT VRAM receive buffer sized from the model and mapped into all N cards; CPU map; CPU write/read check; unmap/free; on failure, the failing peer and the small-BAR hint | `records` mode is byte-identical to an **encoding oracle**, a C program compiled against v6.18 `kfd_ioctl.h` (struct members, request macros and flag macros by name; it recomputes the buffer size from the model file), for N=1,2,4,8,64. The values and their order are design choices shared with `mc_kfd`, so the oracle does not check them. Five source mutants must differ: n_devices offset, flags, gpu_id offset, size offset, and all-reduce slots dropped from the size (caught at N=64). A wrong record length is compile error 9. Model-file refusal → 65; `/dev/kfd` absent → 77; 65 cards → refusal at the KFD limit. The run path (reservation VA, mmap) is never reached without a device |
| `mc_plan` | Per-tensor slices for N cards in exact/fast, exchange counts, per-card bytes, and a check of the split against the model (bounds, 128/head alignment, disjoint unless declared replicated, full cover, GQA for GDN and attention, fast-mode MLP locality) | N=1,2,3,4,8,16 × 2 modes PASS; byte totals equal Wolfram; overlap, drop, misalign, gqa and kv mutants each FAIL at N=2,4,8; malformed model file → 65 |
| `mc_sim` | CPU simulation of the protocol of section 4: one forked process per card, one shared mapping, parity buffers, generation flags, `cpufence`, a 5 s stall alarm (a waiting card sleeps 20 µs between polls after 256 spins), a toy f32 model with the design's exchange pattern (2 gathers or 1 all-reduce per layer, plus the head), not the real 4-exchange layer | exact N=1..8: every card's tokens **and final hidden bits** equal one card's. fast: every card's tokens and bits equal an in-process **fast reference**: the same column split, with the N partials summed in card order. 30–37 of 64 hidden words differ from one card, the reduction-order effect of section 2, observed and reported. Mutants (the race-sensitive ones are made deterministic, with card N-1 slowed, and must be caught on 20 of 20 runs): a dropped card → stall; a wait that accepts any nonzero flag → mismatch; no wait → mismatch; one data buffer for both parities → mismatch; card-relative sum order → mismatch. Also, 10 of 10 unmutated runs pass for fast N=4, exact N=8 and fast N=8. Not exercised: the fences. Every `mapset`/`mapget` already carries DSB SY on this CPU, so removing `cpufence` is not observable here |

Plus `model-spec.sh` (config.json → model file, gated for byte-identical regeneration), `fixtures/gen.py`
(topology trees from the measured MI300X node, synthetic peers), `oracle/kfd_records.c`, and a wrong-compiler
control (one appended byte → `MCGATE NOT_RUN`, exit 3). Evidence: `evidence/GATE-*` (verdict, results.tsv, per-case
logs, binaries' sha256, SHA256SUMS). The newest folder is the one for the code as it stands. Older folders are
kept unchanged as history: 20261113T233413Z, 178 cases, before review 1.

Reproduce (Linux, as a normal user, with the pinned omc; on the Mac `code/vm.sh` runs it in the local Lima
AArch64 VM): `sh code/gate.sh /path/to/omega-mv/seed/out/omc NEW_EVIDENCE_DIR` → `MCGATE PASS cases=203`.

## 7. Compiler gaps found (omc `26540e6d`)

- **Run-time pools are AArch64-only.** `global T *` and `enlarge` are error 25 under `target x86linux`, and the
  MI300X hosts are x86-64. The units therefore keep growable tables in owned anonymous mappings (`mcset`/`mcget`
  in `mc_common.om`; ids 0–23: topology 0–11 and 22–23, kfd 12–15, plan 16–20, sim 21). They double in size, with no
  cap. Next compiler: pools on x86linux.
- **The KFD device-id span must be a fixed array.** A pool or a mapping handle as the `kfdctl`
  MAP/UNMAP span is error 7. 256 bytes is used, which is the kernel's own 64-device boundary, with a clean
  refusal above it. Next compiler: accept a mapping-handle span there, as GET_PROCESS_APERTURES_NEW already does.
- **Expression depth hits the 16-register pool quickly** (error 12). Not a blocker; noted for the compiler lane.

## 8. Hardware steps, in order, once a machine exists (all NOT_RUN)

- **H1. `mc_kfd run /sys/class/kfd/kfd/topology 8 < model.txt`** (x86 build from the gate). Expect
  `MCKFD ok cards=8`. Record:
  - the topology lines (link types, `node_to` numbering, hive, location);
  - `kfd_version`;
  - on any refusal, its step and errno, plus `n_success` and the first unmapped gpu_id for a map, or the
    small-BAR hint for an alloc;
  - the kernel's `Failed to map peer` line from dmesg, if it can be read. A failure before the per-card loop
    leaves n_success 0 and names card 0 without that card being at fault.
- **H2. Peer write and t_x.** Two pieces:
  - A two-card ping-pong kernel (new, small) measures one flag round trip over XGMI with EXT_COHERENT buffers.
  - The existing `vadd` object with its output pointer set to a peer's receive buffer proves that a kernel's
    stores reach the peer (read back on the CPU).
  - This needs the per-card launcher tables (section 4.6).
- **H3. One layer exact at N=2.** Run one layer, compare against one card bit for bit, then the full 64 layers
  against the 24-id golden and the 256-id lighthouse.
- **H4.** N = 2, 4, 8 timing beside the single-card receipt and AMD's own stack in the same session (Drake's
  standing ask).

## 9. Limits

- Everything above section 6 is design and model. `mc_kfd` has never opened a real `/dev/kfd`.
- The fixtures' peers, links, locations and hive are synthetic. The measured VF shows `io_links_count 8`, but
  the contents of its io_links were not captured.
- QEMU runs of the x86 build are EMULATED.
- The simulation proves the protocol's logic and the bit-identity argument on a CPU. It does not prove XGMI
  ordering, MTYPE behaviour or latency.
- t_x is unmeasured.
- Fast mode's effect on real tokens is unmeasured.
