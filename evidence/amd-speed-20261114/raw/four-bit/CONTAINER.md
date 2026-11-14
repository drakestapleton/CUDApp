# Q4D8: the 4-bit weight layout and its arithmetic contract

Version `Q4D8-1`. Lane uxl-brief/FourBit, 2026-11-13. Reference implementation: `tools/q4format.py`
(Python, an outside instrument). This file is the contract shared by the conversion tool, the quality
harness and the gfx942 kernels. All integers are little-endian.

## 1. What is stored for one matrix

A linear layer `y = W x` with `W` of shape M x K (M outputs, K inputs) is stored as two tensors.

| Tensor | dtype | shape | bytes |
|---|---|---|---|
| `NAME.weight_q4` | U8 | M x (Kp/2) | M*Kp/2 |
| `NAME.weight_q4_scale` | BF16 | M x (nseg*64) | M*nseg*128 |

- `Kp = 512 * ceil(K/512)`: each row is padded with zero nibbles to a whole number of 512-column steps.
- `T = Kp/512` is the number of steps in a row. `SEG` (steps per segment, 1..16) is chosen per
  tensor and recorded in the shard metadata; `nseg = ceil(T/SEG)`.
- Column `c` (0 <= c < Kp) lives in dword `j = c div 8` of its row, at bits `[4e+3 : 4e]` with
  `e = c mod 8`. Equivalently: byte `c div 2`, low nibble for even `c`, high nibble for odd `c`.
- A nibble is a two's-complement signed 4-bit integer `q` in -8..7.
- The scale group of column `c` is `(seg, lane)` with `lane = j mod 64`, `step = j div 64`,
  `seg = step div SEG`. Its scale is entry `seg*64 + lane` of the row's scale vector.
  A group therefore holds 8 consecutive columns from each step of its segment: at most `8*SEG`
  (<= 128) weights.
- A scale is a BF16 value (the upper 16 bits of an IEEE binary32), finite and >= +0. A group whose
  weights are all zero has scale +0 and all-zero nibbles.
- The real weight is `w[c] = q[c] * scale[group(c)]`. Padding columns have `q = 0`.

Files are safetensors shards with the source shard names. Converted modules carry the two tensors above
in place of `NAME.weight` (F8_E4M3) and `NAME.weight_scale_inv` (BF16); every other tensor is copied
byte for byte. The safetensors `__metadata__` map carries: `q4_format = "Q4D8-1"`, `q4_seg` (JSON object: `NAME.weight_q4` -> SEG),
`q4_source_repo`, `q4_source_rev`, `q4_source_shard_sha256`, `q4_tool_sha256`, `q4_kept` (names kept
in their source form). No timestamps: the same inputs give the same bytes.

A reader refuses (nonzero exit, nothing written) when: the format string differs, `q4_seg` is outside
1..16, a `weight_q4` second dimension is not a multiple of 256, the scale tensor's shape disagrees with
M, Kp and that tensor's SEG, a scale is negative, infinite or NaN, or the file is truncated.

## 2. What is stored for one activation vector

The kernels multiply integers. A float vector `x` (binary32, length K) is first turned into `Xq`:

1. `m` = largest `|x[i]|`. If any `x[i]` is NaN or infinite: `xscale = NaN`, all digits zero.
   If the biased exponent of `m` is below 32 (`m < 2^-95`, which includes zero): `xscale = +0`, all
   digits zero.
2. Otherwise `E = floor(log2(m))`, `xq[i] = roundTiesToEven(x[i] * 2^(13-E))` (an integer with
   `|xq[i]| <= 16384`), and `xscale = 2^(E-13)` as binary32.
3. `xq[i]` for `K <= i < Kp` is 0.
4. Each `xq[i]` is written as four signed base-16 digits `d0..d3` in -8..7 with
   `xq = d0 + 16 d1 + 256 d2 + 4096 d3`. With `u = xq mod 65536`, `r = ((u + 0x8888) mod 65536) xor 0x8888`,
   digit `p` is nibble `p` of `r` read as a signed 4-bit integer.
5. Buffer `X`: for each dword index `j` (0 <= j < Kp/8), 16 bytes at offset `16j`: four u32 values
   `P0, P1, P2, P3`; nibble `e` of `Pp` is digit `p` of `xq[8j+e]`. After the last dword, at offset
   `2*Kp`, 16 bytes: `xscale` (binary32) then 12 zero bytes. Total `2*Kp + 16` bytes.

## 3. The product

For row `r`, lane `l`, segment `s`:

```
I[r,s,l]  = sum over steps t in segment s, e in 0..7 of  q[r, 8*(64t+l)+e] * xq[8*(64t+l)+e]     (exact integer, |I| <= 2^24)
A[r,l]    = fold over s ascending of  fma(float32(I[r,s,l]), scale[r,s,l], A)    starting from +0.0
R[r]      = butterfly over the 64 lane values: for d = 32,16,8,4,2,1: A[l] = A[l] + A[l xor d]; R = A[0]
y[r]      = R[r] * xscale        (every + and * a binary32 round-to-nearest-even; oracle: kernels/gate.py)
```

`I` is exact because `|q| <= 8`, `|xq| <= 16384` and a group has at most 128 weights, so `|I| <= 2^24`
and its conversion to binary32 is exact. Each plane sum is one `V_DOT8C_I32_I4` per 8 weights (section 3
of FORMAT.md). Results whose magnitude is below `2^-126` are outside the bit-exact contract.

The quality harness uses the same real-number definition (`w = q*scale`, `x' = xq*xscale`,
`y = W x'`) with float32 matrix products. It does not reproduce the kernel's summation order. The difference
is bounded by about (nseg + 6)·2^-24·Σ|terms|, relative to the sum of absolute products, not to |y|, so it is
not small relative to y under cancellation (review finding 14).

## 4. How the conversion picks `q` and `scale`

For each group with source weights `w` (real numbers `e4m3 * weight_scale_inv`, exact in binary64):

1. `a = max |w|`. If `a = 0`: scale +0, all `q = 0`.
2. For clip ratios `rho = 1.00, 0.97, 0.94, ..., 0.40` (21 values, in this order):
   `s = bf16(rho * a / 7)` (binary64 product and quotient, then rounded to 8 significant bits, ties to
   even; a result below 2^-126 is replaced by 2^-126), `q = clamp(roundTiesToEven(w / s), -8, 7)`,
   `err = sum (w - q s)^2` (binary64, summed in ascending step order, then ascending `e`).
3. Keep the first `rho` with the smallest `err`.

Source E4M3 NaN encodings (0x7F, 0xFF) are refused: the tool stops and names the tensor. The BF16 lm_head
source is converted the same way with `w` = its BF16 values.
