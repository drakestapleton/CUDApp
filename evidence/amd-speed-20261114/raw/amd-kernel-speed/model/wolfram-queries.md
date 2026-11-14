# Wolfram queries behind RANKING.md (WolframLanguageEvaluator, 2026-11-13)

1. Floors per class and total:
   `bw=53*10^11; rows={...classes.tsv weight bytes...}; tot=Total[n*b]` -> totalBytes 26919925760, floorMs 5.0792312754716981132,
   withoutLmHeadGB 24.37712896, floorWithoutLmHeadMs 4.5994582943396226415, baselineFractionOfBandwidth 0.19008 (26.721 ms),
   baselineTBperS 1.0074, gdnStateBytesPerStep 305154240, gdnStateFloorMs 0.05758; per-dispatch floors (us): 17408x5120 and
   5120x17408 16.8188, 10240x5120 9.8934, 6144x5120 and 5120x6144 5.9361, 12288x5120 11.8721, 1024x5120 0.98934,
   48x5120 0.092740, lm_head 479.773.
2. Earlier measured kernel times against floors: fp8 17408x5120 51.91 percent, gap 1.99439 ms/step; fp8 5120x17408 45.83,
   1.27239; lm_head 69.88, 0.206827; rmsnorm5120 0.0648, 2.30760; mlpsilu 1.159, 0.215077; gdnconv4 2.061, 0.141032;
   sumPriorMs 9.8533; remainder of 26.721 ms 16.8677.
3. `26919925760+305154240+16*4096*286` -> 27243823296 B, 5.14034 ms; `10^3*16.8677/1126` -> 14.98 us; `1126-545` -> 581.
