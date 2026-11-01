# Reusing a normalized command seal

The native command-list path hashes the driver bank, arguments, and normalized QMD bytes when recording each launch. The two QMD fields that submission changes are zeroed exactly as in `mcc_seal`. In a decode graph many of these images are unchanged from the previous token.

`native_mcreseal` owns the reuse decision in Omega. Each command-list slot has an optional, private 2048-byte memo. A hit requires the argument count and every normalized image byte to match. A miss computes the original SHA256, then replaces the memo. Invalid argument counts clear the output and refuse. Allocation failure uses the original uncached function. The memo stays across successful list resets and is freed with the list.

The memo supplies a digest only. Native lease acquisition, revision and epoch checks, recording ownership, completion tracking, and submit-time intent checks still run. It does not admit an artifact or authorize a launch. The cache is caller-owned internal state, never an external serialized credential.

`make test-seal-reuse` compares the result with Python's independent SHA256. It changes every byte of both input arrays, tests every argument count in both directions, includes all-zero images, and checks invalid counts and repeated hits. On a host that runs the pinned Omega compiler, five source mutants prove the oracle catches missing comparisons, a missing count check, a skipped byte, incorrect normalization, and a dirty refusal output. Python is an external verification tool only.

The full-model speed result remains pending. Initial shared-machine diagnostic data is recorded separately; no default-serving or first-token improvement is asserted here.
