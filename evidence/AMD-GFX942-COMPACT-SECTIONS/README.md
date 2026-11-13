# Code-only notebook sections

Operator change: the relay cell has an approximately60KB limit; each generated section should be code-only and below15KB, with identical fixtures, numerical checks, guards, controls and exact timing grids.

IMPLEMENTED and EXERCISED on the host. The generator derives the fixture's actual function dependency closure, including globals used inside comprehensions, and specializes the existing runtime's torch branch by kernel name. It embeds neither memory images nor interval lists. The generated code constructs the complete fixtures and computes the same Fraction/Decimal/manual-ULP oracle in the cell. No second maintained numerical implementation was added.

The acceptance run executes each generated fixture factory and compares every case to the canonical host fixture. Missing-case controls are CAUGHT. The three existing kernel suites run against the previously gated executor and unchanged instruction bytes, checking every output, cache word, input/guard byte and wrong-object control. Complete logs and exact artifact hashes are filed here; emitted source byte counts are generated in sizes.txt. No kernel source or object is changed by this commit, and no new compiler run was needed.

The new sections replace the exported mlpsilu.py, rmsnorm5120.py and gdnconv4.py files in the requested notebook sections directory. Historical receipts remain immutable. The numerical fixture JSON remains in the acceptance workspace as evidence, not embedded in a notebook cell. Device execution of the sections remains NOT_RUN. The unrelated pending GDN Q/K and recurrence tables are uncommitted and are not part of this acceptance claim.
