# Tools used

Observed with `cc --version && python3 --version && ldd --version && bwrap --version && gcov --version`, rc 0:

| Tool | Observed version | Role |
| --- | --- | --- |
| cc/gcov | Ubuntu 13.3.0-6ubuntu2~24.04.1, GCC 13.3.0 | Existing CUDApp build and generated checked-C compilation; C-oracle branch coverage |
| Python | 3.12.3 | Verification-only command runner; standard library only |
| glibc | Ubuntu 2.39-0ubuntu8.9, glibc 2.39 | Existing hosted C ABI/runtime and untouched C verification oracle |
| bubblewrap | 0.9.0 | Required device-hiding verification boundary |

The seeded compiler and converter source/SHA pins are in README.md and `om/omega-bridge.lock`. Their implementation is seeded Omega. The build still uses the existing hosted C bridge and C resource/launch callers; this lane does not claim native/freestanding variable-view support. Replacing that build path requires a separate native runtime-extents ABI extension with dynamic bounds and writable-overlap checks; it is not silently substituted here. Verification oracles (compiler, gcov, bubblewrap, Python runner, original C loop implementation) are not production Omega implementations or chip evidence.
