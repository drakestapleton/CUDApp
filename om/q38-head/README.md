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
