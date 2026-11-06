# Seeded Omega any-shape attention

IMPLEMENTED: `emit.om` and `kernel.om` select all instructions, tails, loops,
masking and online softmax; `plan.om` admits shapes and selects GQA offsets and
launch geometry. The C API is an allocation/copy/session-launch shell. This is a
new entry, not a replacement of the tuned attention generators. No existing
attn/pattn/attn256/pattn256 generator is changed.

The original attention branch stacks on CodegenA's generic replay prerequisite
`22ebf8f` (`feat/omega-codegen-a`, CUDApp #275). The DNN follow-on includes
the separately authorized replay admission repair `a8c6a22`: the opcode bound
uses `BW_IR_LAST`, with a compiled reverting mutant and boundary regression.
Compiler: omega-mv `e5b32567`, omc SHA-256
`2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
Generated C uses the pinned `om/units.sh` bridge frontend and stays generated.
This does not adopt an outside production implementation. The existing host IR
replay/register allocator and session remain migration-pending dependencies.

## Contract

Contiguous Q `[q_heads][seq_q][head_dim]`, K/V
`[kv_heads][seq_kv][head_dim]`; output `[q_heads][seq_q][head_dim]` in f32.
Inputs are f32, bf16 or IEEE binary16. Native half widening uses the seeded
sm_121 IR conversion; head dimension is any integer 1..256, not just multiples
of 32. Sequence lengths are independently any integer 1..65535; Q and KV head
counts are positive, Q heads at most 65535, KV heads divide Q heads, and each
Q/KV tensor has at most 1073741824 elements. GQA groups are contiguous.
Causal mode is top-left aligned (`key <= query`), not bottom-right aligned.
No batch dimension or strides are implied. Scale may be any finite f32,
including zero or negative. Input values and intermediate dots must be finite;
there is no input-value scan. The admission limits describe geometry, not a
universal numerical error bound.

One 32-lane warp owns a query row. Each lane holds up to eight Q components
and output numerators. Component tails predicate memory operations, never
remove lanes from the shuffle. The KV loop visits each admitted visible key,
reduces dot products across all lanes, updates f32 running maximum/sum and
output numerators, then normalizes once. No attention score matrix is stored.
Both query tails and key tails are exact loop bounds rather than padded rows.

## Numerical reference and tolerance

`tests/gpu_attn_any_ref.h` materializes naive scores in double, computes a
double maximum/exp/sum, then a weighted V sum. It shares no production
admission, recurrence, grouping or instruction helpers. The test fixtures use
exactly representable dyadics: |Q|, |K|, |V| <= 1/8, head_dim <= 256,
seq_kv <= 513, and |scale| <= 1/4. Simulator EX2/RCP use the host floating
oracle; their passing does not qualify the chip's approximations.

For binary16 fixtures the independent reference decodes integer fields directly
to double, without using the production conversion instruction:
`WolframLanguageEvaluator: {FullSimplify[e-15-10],1-15-10,N[2^-24],N[2^-10]}`
returned `{e-25,-24,5.960464477539063*^-8,0.0009765625}`.
Normal values use `ldexp(1024+fraction,exponent-25)`; subnormals use
`ldexp(fraction,-24)`. Storage decoding introduces no approximate half arithmetic.

WolframLanguageEvaluator query:

```wolfram
FullSimplify[(s Exp[m-t] o + Exp[x-t] v)/(s Exp[m-t]+Exp[x-t]) -
 (Exp[m] s o + Exp[x] v)/(Exp[m] s+Exp[x]),
 Assumptions -> Element[{s,m,t,x,o,v},Reals] && s>0]
```

Result: `0`. This verifies rescaling the prior sum/numerator to the new maximum
preserves the softmax-weighted value in exact arithmetic.

The absolute comparison tolerance is `2^-10 = 0.0009765625`. It is a
conservative **modeled test envelope**, not a proven universal error bound.
Using f32 unit roundoff `u=2^-24`, gamma(n)=n*u/(1-n*u), a provisional
transcendental allowance `2^-20` and safety factor 16:

```wolfram
N[16 ((256 2^-24)/(1-256 2^-24)+
       (513 2^-24)/(1-513 2^-24)+2^-20)]
N[2^-10]
N[256 (1/8)^2 (1/4)]
```

Wolfram results: `0.0007486530236091783`, `0.0009765625`, and `1.`
respectively. The last is the fixture's maximum absolute score envelope.
The chosen tolerance rounds up the modeled accumulation/transcendental
allowance. Actual errors are checked element by element against the independent
reference; NaNs fail. No empirical error or speed is inferred from these models.

## Host acceptance

```sh
export OMEGA_OMC=$HOME/work/cudastack/omega-mv/seed/out/omc
export OMEGA_BRIDGE=$HOME/work/cudastack/omega-bridge
taskset -c 10-14 nice -n 10 sh om/units.sh
taskset -c 10-14 nice -n 10 make -j3 test-gpu-attn-any test-gpu-attn-any-mutants
```

The mixed-factor sweep covers sequence lengths 1, 2, 7, 64, 65, 513 independently
for Q/KV, head dimensions 64, 80, 96, 128, 256, GQA groups 1, 4, 8,
both causal modes and all three dtypes. It is a documented mixed-factor sweep, not
an exhaustive Cartesian product. Additional cases cover dimensions 1,3,31,33,255,
eight additional f16 head dimensions, negative and zero scale, rectangular wrong-mask control, distinct KV-head grouping and a
513-by-513 case. Allocation ranges guard all global memory accesses.
Refusal controls cover dimension 257, non-dividing/zero heads, zero KV length,
over-limit sequences/tensors, invalid causal flags, nonfinite scale and invalid dtype 3.
The execution API is also checked to refuse an invalid shape before opening a session.
Source mutants change mask, GQA mapping, normalization, component tails,
dimension admission and native half widening in isolated copies. All six must generate successfully
and then fail behavior checks; a compile error does not count as a kill.

Chip execution and qualification: **NOT_RUN**. See [CHIP.md](CHIP.md). A passing
host reference/simulator is EXERCISED only. ROADMAP's status remains PARTIAL.
The native half encoder seam is documented in `../dnn-layers/README.md`.
The compiler pin and seeded language are unchanged. The new experimental API
and lane-local commands are documented here and in `docs/runtime/API.md`.
