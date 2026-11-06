# Chip enumeration policy, 2026-11-06

IMPLEMENTED and EXERCISED host-only, stacked on #296 at 00f88e1. Not merged, deployed or chip-qualified. Qualified-target identities and the existing admission table are unchanged. Chip, live RM, VFIO and hardware tiers NOT_RUN; no GPU window was requested or used.

`om/admission/chip_enum.om` owns row identity, raw sysfs vendor filtering, display-class selection, output-capacity selection, dot-entry exclusion and PCI address masks. C retains table/input traversal, sysfs effects, ABI storage and snprintf effects. The normal call is strong; the old arithmetic is available only under default-OFF `CUDAPP_MIGRATION_PENDING_C_CHIP_ENUM_POLICY=1`. Library admission and no-node-before-admission ordering are unchanged.

## Acceptance

Command, rc 0:

```
bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10 env OMEGA_OMC=$HOME/work/cudastack/omega-mv/seed/out/omc OMEGA_BRIDGE=$HOME/work/cudastack/omega-bridge make -j3 test-chip-enum-om test-chip-admission test-doctor test-ampere-runtime test-om-units
```

Focused final line: `CHIP ENUM DIFFERENTIAL RED LINK SEVEN MUTANTS PASS`; measured 460712 scalar differential cases. Every 16-bit vendor and device value is tested against the qualified row and through the actual table API. Every 16-bit value is tested under each PCI mask. Vendor edges 0/1/0x10dd/0x10de/0x10df/0xffff/0x10000/UINT32_MAX/UINT64_MAX and display class boundaries 0x2ffff/0x30000/0x30001/0x3ffff/0x40000 and high-bit classes are covered. All class bytes are tested with zero, one and all low bits plus out-of-range upper bits. Mask boundaries are v-1/v/v+1 at 7/31/255/65535, zero and UINT32_MAX. Signed capacity limits include negative and zero capacities, equality, both output-pointer states and upper-bit values. Actual admit-card refusal text verifies masked domain/bus/slot/function fields; valid GB10 admission still selects sm_121.

Fixture scan compares the entire normal and C-oracle outputs across vendor/class/device edge combinations, missing/invalid fields, hidden entries, capacities -1 through 3, NULL output and absent sysfs. No real device nodes are opened. Existing admission tests report zero simulated device opens for pre-admission refusals; qualified paths use intercepted simulated opens only. Existing doctor fixtures: nine passed, zero failed. Ampere allocation, session and Linux reference simulations PASS; Linux reference reports seventeen passed, zero failed. Unit regeneration is byte-for-byte PASS.

Planted output difference is rejected; removing the Omega unit fails the link. Seven .om mutants killed: row-vendor, row-device, vendor, class, mask, capacity, dot. gcov reports the retained policy oracle with 100% blocks and every conditional branch taken. Raw acceptance and oracle receipts are included, with home prefix normalized to `~`.

During implementation an invalid bool-to-integer Omega cast was caught by the compiler and corrected to explicit integer comparison results. The first focused run then exposed a signed-capacity conversion trap for encoded negative values; the fixed unit compares sign bits and unsigned magnitudes instead. The successful command above includes negative capacities and UINT64_MAX, so this is exercised rather than assumed. The initial admission-family mutant invocation lacked OMEGA_MV/OMEGA_BRIDGE_DIR and returned NOT_RUN; its corrected invocation and result are recorded separately.

Compiler pin: omega-mv e5b32567; sha256 2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f. Bridge pins unchanged from om/units.sh. WolframLanguageEvaluator: `FromDigits["10de",16]` -> `4318`; `2^63` -> `9223372036854775808` (signed-boundary check). No public API, pins, compiler or harness guide changed. Raw sysfs parsing remains the existing C effect interface.

## Chip runbook (NOT_RUN)

Only after current operator authorization for an exclusive GPU window, enumerate the real sysfs tree and compare enumeration with the C policy oracle without opening device nodes. Then run the existing admission/hello-gpu hardware runbook at this exact head, verifying refusal before device-node opens for an unsupported selection and collecting a new supported-device receipt. No new device identity may be admitted without its own qualification. Host fixtures are not chip evidence.
