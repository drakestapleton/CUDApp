# Conservative output-head screening

Experimental Omega-generated GPU primitive for Core issue 710 and Core PR 895.
This is one step toward an exact two-pass output head for the 27B model.
It is not connected to serving. No end-to-end speed or first-token improvement
is established by this primitive.

The compact head produces approximate scores. This kernel turns them into
conservative intervals. A later maximum reduction can discard a row only when
its upper bound is below the largest lower bound. The full-precision head must
still recompute every surviving row and choose the final token.

## Contract

Trusted caller requirements:

- `1 <= count <= 248320`, 256 threads per block, grid `(count + 255) / 256`.
- Four separate arrays of at least `count` binary32 values: score, coefficient,
  lower output, upper output. Inputs are finite; coefficients are nonnegative.
- `hn` is finite, positive, and at least the Euclidean norm of the exact hidden
  vector used by both heads. An unrepresentable norm requires a full-head fallback.
- The coefficient was rounded upward and proves
  `abs(full_score - compact_score) <= coefficient * hn` for the actual compact
  and full GPU reductions. This kernel does not establish that upstream premise.
- Binary32 round-to-nearest-even arithmetic, with subnormals retained.
- The caller validates input values before launching. This primitive does not
  admit untrusted buffers or detect invalid coefficients on the device.

Argument words start at constant-bank byte 896: score address, coefficient
address, lower address, upper address (four 64-bit addresses), `hn` (binary32),
then `count` (u32). No shared memory. The generated frame records register use;
the verification launch declares 64 registers, as the existing runtime requires.

For `e = 2^-20` and `tiny = 2^-100`, operations are separately rounded except
for the explicit fused multiply-add:

```
r = coefficient * hn
r = r * (1 + e)
r = fma(abs(score), e, r)
r = r + tiny
lower = score - r
upper = score + r
```

The relative margin is sixteen binary32 unit roundoffs. It covers rounding in
the two products, fused sum, addition and final endpoints. The score term
covers endpoint rounding when the radius is small. The positive normal `tiny`
covers absolute subnormal rounding errors. Overflow of the nonnegative radius
produces `[-infinity, +infinity]`, retaining the row. These intervals are wider
than the binary64 next-up reference in Core PR 895. Equal endpoints are retained.

For a finite intermediate result, let `u=2^-24`, `eta=2^-150`, `R=c*hn`
and `S=abs(score)`. The standard RN error bound gives
`r >= A*R + B*S + (1-u)*tiny - D*eta`, with
`A=(1-u)^4*(1+e)`, `B=e*(1-u)^2`, and
`D=(1+e)*(1-u)^3+(1-u)^2+(1-u)+1`.
All three exact inequalities hold: `A*(1-u)>1`, `B*(1-u)>u`, and
`((1-u)*tiny-D*eta)*(1-u)>eta`. Thus `r*(1-u)>R+u*S+eta`,
which covers the final endpoint error `u*(S+r)+eta`. Nonnegative intermediate
overflow takes the infinite-interval case above. This argument depends on
the stated RN/subnormal contract, not approximate reciprocal instructions.

Numerical reference: NVIDIA PTX ISA 9.4, physical page 108, section 6.5.2
(round-to-nearest-even). Machine instructions are emitted by the existing
checked Omega encoder, not copied from a vendor binary or listing.

## Verification

Build the existing runtime with `make -j1 build/libomega_gpu.a`. Set
`OMEGA_OMC` to the compiler pinned in the repository AGENTS.md and set
`Q38_GPU_OUT` to an external artifact directory. From the repository root:

```
sh om/q38-head/run.sh
```

`make test-q38-head-bounds` runs the same host check and is included in
`make test` when the pinned compiler can run on the host. On other hosts the
explicit target reports NOT_RUN and exits 2. Neither command opens the GPU.

The external C instrument runs the generated IR in the existing warp simulator
against a long-double reference. Cases include tails, zero coefficients,
subnormals, cancellation, extreme exponents, overflow and both fragment orders.
It then damages a fused arithmetic instruction and requires a numerical failure.
The initial generator stub refused with exit 66 before implementation.

With an authorized, idle chip window, `run.sh --chip` loads the exact machine
code from the Omega frame through the existing runtime and repeats the numerical
battery. The optional two directory arguments after `--chip` add eight captured
27B-state replays: compact-score/coefficient files, then hidden/full-score files.
Those current replay scores came from the external host compact-head reference;
they do not yet qualify a complete GPU compact-head/recheck pipeline.

The shell and C files are external verification instruments only. They are not
first-party runtime implementations or new production dependencies.

## Runtime regression found during qualification

The unmodified runtime refused this Spark before launching: the driver returned
raw RM SM version `10.04`, while class admission compared it only with `12.01`.
The existing diagnostic chip table already identifies PCI vendor/device
`10de:2e12` as GB10 with execution target SM121. The repaired Omega policy accepts
the observed raw version only for that exact PCI identity and the complete
Blackwell class set. Other devices, vendor IDs, raw versions and missing classes
remain refused. `nvrm.c` forwards identity fields from its existing CARD_INFO
result; it adds no driver query and changes no qualified-chip table.

The new regression assertion fails before the Omega policy fix and passes after
regenerating its binding with the pinned Bridge. Physical qualification uses the
patched runtime itself, including its normal class admission. No bypass or
environment override selects the target.

## Isolated two-pass head experiment

`pipeline.patch` derives an external verification instrument from the existing
`tests/gpu_exact_head_test.c`. It uses width 5120, this Omega-generated bound
kernel, and conservatively computes the hidden norm on the host. The existing
GPU kernels perform compact screening, threshold reduction, masked full-precision
recheck and final selection. Apply it outside the source tree:

```
patch -o /tmp/pipeline.c tests/gpu_exact_head_test.c om/q38-head/pipeline.patch
```

`pipeline-validation.json` records the source fingerprints and results. All eight
saved states reproduce the original full GPU logits bit for bit; every compact
interval contains the full score, every rechecked score matches, and every final
token matches. Zeroing the coefficients loses the true winner at position 97,
reproducing the earlier compact-head error. The recorded timing is provisional:
host suites were active in the shared correctness window. It is neither an
accepted speed result nor a whole-model/first-token measurement. Serving
integration and its complete exact-token gate remain untested.

## Fixed envelope and experimental graph integration

`envelope.om` calculates an upward-rounded binary32 bound from the folded final
RMSNorm weights: `72 * max(abs(weight)) * 1025/1024`. It uses integer arithmetic
on the weight encodings, refuses nonfinite weights and overflow, and conservatively
returns 1 for an all-subnormal or zero weight vector. `envelope-check.py` checks
all exponent ranges against exact rational arithmetic; five compiled source
mutants exercise tail scanning, upward rounding, nonfinite refusal, sign masking and slack.

The ideal normalized vector has norm at most `sqrt(5120) * max(abs(weight))`.
The fixed envelope adds room for the existing GPU normalization's rounding.
Its scope is finite input, folded finite weights, epsilon `1e-6f`, width 5120,
and output scale 1. The graph adapter refuses a different epsilon. The actual
normalizer is in `omega_blackwell_codegen_dec.c`: 20 products per lane, five
warp sums, seven CTA sums, the mean plus epsilon, reciprocal square root and
two output products. NVIDIA PTX ISA 9.4, physical page 211 (printed 199), gives
the reciprocal-square-root relative error as at most `2^-22.9`; this is a
numerical reference, not independent proof of every native instruction.
`norm-check.c` tests the actual emitted normalizer on the chip, including
underflow and overflow magnitudes, dense and sparse input, and adversarial
weights. The replay and stress receipts do not prove all possible model states.

`plan.om` owns the scratch layout and six operations: scale, compact screen,
intervals, two threshold reductions, and full-precision masked recheck.
`emit.om` produces the checked native bounds kernel. Both are compiled through
the pinned Omega Bridge. The C graph adapter only supplies resources and
dispatches those operations. The feature is opt-in through
`omega_gpu_q38_graph_set_exact_head`; the original head remains available.
Disable the feature for sampling, which requires all logits. Full serving
qualification is pending; the default path is unchanged.

The Omega plan also supplies an additional 2 GiB admission reserve for the
resident screen, coefficients and scratch. The Core opt-in adds that reserve
before loading. It is a conservative budget, not a measured allocation size.
