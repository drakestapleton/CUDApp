# Wolfram queries behind DESIGN.md (2026-11-13, `WolframLanguageEvaluator`, hosted agenttools.wolfram.com)

Every number in DESIGN.md sections 1, 3 and 5 comes from one of these evaluations, quoted as sent and as
returned. Inputs marked MEASURED come from the CUDApp `dc536882` receipt
`evidence/amd-qwen38-native-refusal-20261113` (26.726 ms/token median, `vadd`/`vadd-lds` `dispatch_ns` 44080 and
29271). Per-card bytes and exchange bytes come from `mc_plan` output (gate logs). 5.3 TB/s is AMD's MI300X peak
(see ACCESS.md, hardware facts). Everything returned is MODELED.

## Q1. Bytes per step and the one-card floor

```
h=5120; ff=17408; v=248320; fp8[m_,k_]:=m k + Ceiling[m/128] Ceiling[k/128] 4; bf[m_,k_]:=2 m k;
gdn = fp8[10240,h]+fp8[6144,h]+fp8[h,6144]+2 bf[48,h];
att = fp8[12288,h]+2 fp8[1024,h]+fp8[h,6144];
mlp = fp8[ff,h]+fp8[ff,h]+fp8[h,ff];
nomat = {gdn, att, mlp};
totalNoScale = 48 (10240 h + 6144 h + h 6144 + 2*2*48 h) + 16 (12288 h + 2*1024 h + h 6144) + 64 (3 ff h) + 2 v h;
total = 48 gdn + 16 att + 64 mlp + bf[v,h];
{"gdn"->gdn, "att"->att, "mlp"->mlp, "lmhead"->bf[v,h], "total_bytes"->total, "total_GB"->N[total/10^9,6], "no_scales_GB"->N[totalNoScale/10^9,6], "floor_ms_1card_5.3TBs"->N[total/(5.3 10^12) 1000,5]}
```
→ `{"gdn" -> 116354560, "att" -> 104883200, "mlp" -> 267452160, "lmhead" -> 2542796800, "total_bytes" -> 26922885120, "total_GB" -> 26.92288512, "no_scales_GB" -> 26.91694592, "floor_ms_1card_5.3TBs" -> 5.079789645283019}`

## Q2. ms/token grid (section 5 table)

```
bw = 5.3*^12;
bytes = <|1 -> 26922885120, 2 -> 13461442560, 4 -> 6730721280, 8 -> 3386337280|>;
sent = <|"exact" -> <|1 -> 0, 2 -> 4325384, 4 -> 6488088, 8 -> 7569464|>, "fast" -> <|1 -> 0, 2 -> 2621448, 4 -> 7864344, 8 -> 18350136|>|>;
syncs = <|"exact" -> 257, "fast" -> 129|>;
linkBW = 64*^9;
effToday = N[26922885120/(bw*26.726*^-3)];
ms[n_, e_, tx_, mode_] := 1000*(bytes[n]/(e*bw) + If[n == 1, 0, syncs[mode]*tx + (sent[mode][n]/Max[n - 1, 1])/linkBW]);
tab = Table[{mode, n, e, tx*10^6, NumberForm[ms[n, e, tx, mode], {5, 3}]}, {mode, {"exact", "fast"}}, {e, {effToday, 0.8, 1.0}}, {tx, {2*^-6, 10*^-6, 29.271*^-6}}, {n, {1, 2, 4, 8}}];
{"effToday" -> effToday, "xfer_us_exact8" -> N[10^6 (sent["exact"][8]/7)/linkBW], "xfer_us_fast8" -> N[10^6 (sent["fast"][8]/7)/linkBW], "rows" -> Flatten[tab, 3]}
```
→ effToday 0.1900692077109563; xfer_us_exact8 16.896125; xfer_us_fast8 40.960125. Rows {mode, N, e, t_x µs, ms}:

| mode | e | t_x µs | N=1 | N=2 | N=4 | N=8 |
|---|---|---|---|---|---|---|
| exact | 0.1901 | 2 | 26.726 | 13.945 | 7.229 | 3.892 |
| exact | 0.1901 | 10 | 26.726 | 16.001 | 9.285 | 5.948 |
| exact | 0.1901 | 29.271 | 26.726 | 20.953 | 14.238 | 10.901 |
| exact | 0.8 | 2 | 6.350 | 3.756 | 2.135 | 1.330 |
| exact | 0.8 | 10 | 6.350 | 5.812 | 4.191 | 3.386 |
| exact | 0.8 | 29.271 | 6.350 | 10.765 | 9.144 | 8.338 |
| exact | 1.0 | 2 | 5.080 | 3.121 | 1.818 | 1.170 |
| exact | 1.0 | 10 | 5.080 | 5.177 | 3.874 | 3.226 |
| exact | 1.0 | 29.271 | 5.080 | 10.130 | 8.826 | 8.178 |
| fast | 0.1901 | 2 | 26.726 | 13.662 | 6.980 | 3.661 |
| fast | 0.1901 | 10 | 26.726 | 14.694 | 8.012 | 4.693 |
| fast | 0.1901 | 29.271 | 26.726 | 17.180 | 10.498 | 7.178 |
| fast | 0.8 | 2 | 6.350 | 3.474 | 1.886 | 1.098 |
| fast | 0.8 | 10 | 6.350 | 4.506 | 2.918 | 2.130 |
| fast | 0.8 | 29.271 | 6.350 | 6.992 | 5.404 | 4.616 |
| fast | 1.0 | 2 | 5.080 | 2.839 | 1.569 | 0.938 |
| fast | 1.0 | 10 | 5.080 | 3.871 | 2.601 | 1.970 |
| fast | 1.0 | 29.271 | 5.080 | 6.357 | 5.087 | 4.456 |

(Values copied from the returned `NumberForm[...]` rows, rounded to 3 decimals.)

## Q3. 4-bit floors, fixed-overhead bound, tokens per pass

```
h=5120; ff=17408; v=248320; bw=5.3*^12;
fp8w = 48 (10240 h + 6144 h + h 6144) + 16 (12288 h + 2*1024 h + h 6144) + 64 (3 ff h);
scales[m_,k_] := Ceiling[m/128] Ceiling[k/128] 4;
sc = 48 (scales[10240,h]+scales[6144,h]+scales[h,6144]) + 16 (scales[12288,h]+2 scales[1024,h]+scales[h,6144]) + 64 (2 scales[ff,h]+scales[h,ff]);
bfab = 48*2*(2*48 h); lmh = 2 v h;
rep8 = 167813120;
tot8bit = fp8w + sc + bfab + lmh;
tot4a = fp8w/2 + sc + bfab + lmh;
tot4b = fp8w/2 + sc + bfab + lmh/4;
percard[t_, n_, r_] := t/n + If[n == 8, r/8, 0];
floor[t_] := 1000 t/bw;
gap = 26.726 - floor[tot8bit];
fixedBound[n_] := gap + floor[percard[tot8bit, n, rep8]] + If[n > 1, 257*2*^-6*1000, 0];
pass[n_, t_, r_, e_, tx_, s_] := floor[percard[t, n, r]]/e + If[n > 1, s tx 1000, 0];
best = pass[8, tot4b, rep8/2, 1.0, 2*^-6, 129];
{... "breakeven..." -> Solve[...]}
```
→ `"tot8bit" -> 26922885120, "check_vs_mc_plan" -> True, "fp8w" -> 24326963200, "scales" -> 5939200, "bf16_ab" -> 47185920, "lm_head" -> 2542796800, "tot4a_GB" -> 14.75940352, "tot4b_GB" -> 12.85230592,`
`"floor_ms" -> {{1, 5.0798, 2.7848, 2.4250}, {2, 2.5399, 1.3924, 1.2125}, {4, 1.2699, 0.6962, 0.6062}, {8, 0.6389, 0.3501, 0.3051}}` (columns: N, 8-bit, 4-bit body + BF16 lm_head, all 4-bit),
`"today_gap_ms" -> 21.64621035471698, "if_gap_fixed_ms" -> {{1, 26.726}, {2, 24.7001}, {4, 23.4302}, {8, 22.7991}},`
`"best_pass_ms_8card_4bit_e1_tx2us_fast" -> 0.5630993509433962, "tokens_per_pass_for_0.1ms" -> 5.630993509433963, "sync_only_ms_fast_2us" -> 129/500, "sync_only_ms_exact_2us" -> 257/500`.
The break-even `Solve` returned no solution. At equal exchange counts the t_x terms cancel, so N=8 is never
slower than N=4 at the same t_x. That result is not used.

## Q4. Transfer at usable link bandwidth, prices, the lm_head share

```
{"exact8_us_48GBs" -> N[10^6 (7569464/7)/48*^9], "fast8_us_48GBs" -> N[10^6 (18350136/7)/48*^9], "lm_head_share" -> N[2542796800/26922885120], "floor_1card_GB_excl_lmhead" -> N[(26922885120 - 2542796800)/10^9], "vadd_us" -> {29271/1000., 44080/1000.}, "hourly_8x" -> {"tensorwave" -> 8*1.71, "hotaisle" -> 8*3.39, "oracle" -> 8*6, "do_devcloud" -> 8*2.59, "do_devcloud_reserved" -> 8*1.91, "vultr_mi300x_machine" -> 31.92, "azure_vm" -> 48.0, "tensorwave_mi325x" -> 8*2.25, "tensorwave_mi355x" -> 8*2.95}, "hotaisle_month_min_usd" -> 8*3.39*24*30, "overgpu_mi355x_per_hour_30d" -> N[19900/(24*30)]}
```
→ `{"exact8_us_48GBs" -> 22.528166666666667, "fast8_us_48GBs" -> 54.6135, "lm_head_share" -> 0.09444741113986524, "floor_1card_GB_excl_lmhead" -> 24.38008832, "vadd_us" -> {29.271, 44.08}, "hourly_8x" -> {"tensorwave" -> 13.68, "hotaisle" -> 27.12, "oracle" -> 48, "do_devcloud" -> 20.72, "do_devcloud_reserved" -> 15.28, "vultr_mi300x_machine" -> 31.92, "azure_vm" -> 48., "tensorwave_mi325x" -> 18., "tensorwave_mi355x" -> 23.6}, "hotaisle_month_min_usd" -> 19526.4, "overgpu_mi355x_per_hour_30d" -> 27.63888888888889}`

## Q5. Replication and buffer size

```
{"kv_dup_per_card_MB" -> N[167813120/8/10^6], "kv_dup_check" -> (27090698240 - 26922885120 == 167813120), "rx_bytes_8" -> With[{need = 2 (17408*4 + 8*64)}, {need, Ceiling[need/2097152]*2097152}], "largest_gather_bytes" -> 17408*4, "xfer_ratio_64_48" -> N[64/48]}
```
→ `{"kv_dup_per_card_MB" -> 20.97664, "kv_dup_check" -> True, "rx_bytes_8" -> {140288, 2097152}, "largest_gather_bytes" -> 69632, "xfer_ratio_64_48" -> 1.3333333333333333}`

## Q6. Independent check of mc_plan's exchange bytes (sent per card per step)

```
exact[n_] := Module[{p = n - 1, out = 5120/n, gate = 17408/n, v = 6144/n, a = 6144/n}, (out*2*64 + gate*64 + v*48 + a*16)*4*p + 8 p];
fast[n_] := 2*64*5120*4*(n - 1) + 8 (n - 1);
{"exact" -> Table[{n, exact[n]}, {n, {2, 4, 8}}], "fast" -> Table[{n, fast[n]}, {n, {2, 4, 8}}]}
```
→ `{"exact" -> {{2, 4325384}, {4, 6488088}, {8, 7569464}}, "fast" -> {{2, 2621448}, {4, 7864344}, {8, 18350136}}}`.
These equal `mc_plan`'s `exchange_bytes_sent` for N = 2, 4, 8 in both modes (gate logs `a64-plan-*-{2,4,8}.out`).

## Q7. Corrections after review 1: link term in the best pass, dispatch-bound scenario, state and KV traffic

```
bw = 5.3*^12; h=5120; ff=17408; v=248320;
fp8w = 48 (10240 h + 6144 h + h 6144) + 16 (12288 h + 2*1024 h + h 6144) + 64 (3 ff h);
scales[m_,k_] := Ceiling[m/128] Ceiling[k/128] 4;
sc = 48 (scales[10240,h]+scales[6144,h]+scales[h,6144]) + 16 (scales[12288,h]+2 scales[1024,h]+scales[h,6144]) + 64 (2 scales[ff,h]+scales[h,ff]);
bfab = 48*2*(2*48 h); lmh = 2 v h; rep8 = 167813120;
tot4b = fp8w/2 + sc + bfab + lmh/4;
weights8 = 1000 (tot4b/8 + rep8/2/8)/bw;
syncFast = 129*2*^-6*1000;
link[bwl_] := 1000 (18350136/7)/bwl;
pass64 = weights8 + syncFast + link[64*^9]; pass48 = weights8 + syncFast + link[48*^9];
gap = 26.726 - 1000*26922885120/bw;
perDisp = gap/1126;
floor8 = 1000 (26922885120/8 + rep8/8)/bw;
fixedExact = gap + floor8 + 257*2*^-6*1000 + 129*perDisp;
fixedFast = gap + floor8 + 129*2*^-6*1000 + 1*perDisp;
gdnState = 48*(48*128*128*4)*2;
kvPerPos = 16*4*256*2*2;
{"pass_ms_64" -> pass64, "tokens_64" -> pass64/0.1, "pass_ms_48" -> pass48, "tokens_48" -> pass48/0.1,
 "perDispatch_us" -> 1000 perDisp, "fixed_exact8_ms" -> fixedExact, "fixed_fast8_ms" -> fixedFast,
 "gdn_state_bytes" -> gdnState, "gdn_state_ms_1card" -> 1000 gdnState/bw, "gdn_share_of_weights" -> N[gdnState/26922885120],
 "kv_bytes_per_position" -> kvPerPos, "kv_ms_per_1000_positions_1card" -> 1000*1000 kvPerPos/bw,
 "exact8_vs_floor_ratio" -> (257*2*^-6*1000)/floor8, "host_fast_e08" -> {6.3497, 6.9918, 5.4044, 4.6156}}
```
→ `{"pass_ms_64" -> 0.6040594759433963, "tokens_64" -> 6.040594759433962, "pass_ms_48" -> 0.6177128509433962, "tokens_48" -> 6.177128509433962, "perDispatch_us" -> 19.223987881631423, "fixed_exact8_ms" -> 25.279036353711586, "fixed_fast8_ms" -> 22.562365904862762, "gdn_state_bytes" -> 301989888, "gdn_state_ms_1card" -> 0.0569792241509434, "gdn_share_of_weights" -> 0.011216847178672655, "kv_bytes_per_position" -> 65536, "kv_ms_per_1000_positions_1card" -> 0.012365283018867924, "exact8_vs_floor_ratio" -> 0.8044680062111238, "host_fast_e08" -> {6.3497, 6.9918, 5.4044, 4.6156}}` (the last entry only echoes Q2's fast, e = 0.8, t_x = 29.271 µs row for DESIGN.md §5 point 1)

Q3's `best_pass` (0.563 ms, 5.6 tokens) left out the per-link transfer term of the model's own formula. Review 1,
finding 3, caught it. Q7 replaces it, and DESIGN.md now quotes Q7. Q3's other values and the old `if_gap_fixed_ms`
(22.8 ms, which counted neither the added exchange dispatches nor fast mode's 129 exchanges) are kept above as
history.

## Q8. Tokens per pass with the link term growing with k (review 2, finding 3)

```
bw = 5.3*^12; h=5120; ff=17408; v=248320;
fp8w = 48 (10240 h + 6144 h + h 6144) + 16 (12288 h + 2*1024 h + h 6144) + 64 (3 ff h);
scales[m_,k_] := Ceiling[m/128] Ceiling[k/128] 4;
sc = 48 (scales[10240,h]+scales[6144,h]+scales[h,6144]) + 16 (scales[12288,h]+2 scales[1024,h]+scales[h,6144]) + 64 (2 scales[ff,h]+scales[h,ff]);
bfab = 48*2*(2*48 h); lmh = 2 v h; rep8 = 167813120;
tot4b = fp8w/2 + sc + bfab + lmh/4;
w8 = 1000 (tot4b/8 + rep8/2/8)/bw;
tx = 2*^-3; (* ms *)
linkMs[bytesPerCard_, bwl_] := 1000 (bytesPerCard/7)/bwl;
cases = <|
 "fast_oneshot" -> {w8 + 129 tx, 18350136},
 "exact_gather" -> {w8 + 257 tx, 7569464},
 "fast_two_phase" -> {w8 + 257 tx, 7*(128*2*(5120*4)/8) + 7*8}
|>;
kmin[{fixed_, bytes_}, bwl_] := fixed/(0.1 - linkMs[bytes, bwl]);
perTok[{fixed_, bytes_}, bwl_, k_] := fixed/k + linkMs[bytes, bwl];
{"w8_ms" -> w8, "rows" -> KeyValueMap[{#1, "fixed_ms" -> #2[[1]], "link_ms_per_token_64" -> linkMs[#2[[2]], 64*^9], "link_ms_per_token_48" -> linkMs[#2[[2]], 48*^9], "kmin_64" -> kmin[#2, 64*^9], "kmin_48" -> kmin[#2, 48*^9], "ms_per_token_k6_64" -> perTok[#2, 64*^9, 6], "ms_per_token_k16_48" -> perTok[#2, 48*^9, 16]} &, cases], "devcloud_5min_usd" -> 20.72*5/60}
```
→ `{"w8_ms" -> 0.30509935094339624, "rows" -> {{"fast_oneshot", "fixed_ms" -> 0.5630993509433962, "link_ms_per_token_64" -> 327681/8000000, "link_ms_per_token_48" -> 109227/2000000, "kmin_64" -> 9.53761082562245, "kmin_48" -> 12.406758638436457, "ms_per_token_k6_64" -> 0.13481001682389937, "ms_per_token_k16_48" -> 0.08980720943396227}, {"exact_gather", "fixed_ms" -> 0.8190993509433963, "link_ms_per_token_64" -> 135169/8000000, "link_ms_per_token_48" -> 135169/6000000, "kmin_64" -> 9.85633162043763, "kmin_48" -> 10.572866494834418, "ms_per_token_k6_64" -> 0.15341268349056605, "ms_per_token_k16_48" -> 0.07372187610062893}, {"fast_two_phase", "fixed_ms" -> 0.8190993509433963, "link_ms_per_token_64" -> 81921/8000000, "link_ms_per_token_48" -> 27307/2000000, "kmin_64" -> 9.125451109901793, "kmin_48" -> 9.486190533992648, "ms_per_token_k6_64" -> 0.14675668349056603, "ms_per_token_k16_48" -> 0.06484720943396227}}, "devcloud_5min_usd" -> 1.7266666666666666}`

This replaces Q7's `tokens_64`/`tokens_48` (6.04/6.18), which divided a one-token pass by 0.1 ms. Q7 is kept as
history.
