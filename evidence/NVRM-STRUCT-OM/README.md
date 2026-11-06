# NVRM struct codecs: host evidence

## State and scope

IMPLEMENTED and EXERCISED on AArch64 Linux. HOST_ONLY PASS. Chip, live-RM,
VFIO and deployment are NOT_RUN. This is not hardware qualification.

Base: CUDApp #324, `feat/nvrm-m16-rest-om`,
`196e84eeb753089cf0c92340a21788ea83f4eebb`; initial WIP `932e48fe`.
The first completion commit, `df36ab94f834`, registers the unit before the C
cutover. This receipt accompanies the subsequent implementation commit.

The default (`CUDAPP_MIGRATION_PENDING_C_NVRM_STRUCT=0`) calls the Omega
parameter encoder/decoder. Setting the flag to 1 compiles the old C parameter
fills as the migration oracle. The existing independent envelope/transport flag
is unchanged. Raw ioctls, allocation ownership and pointer acquisition stay C.

## Measured layouts

`tests/nvrm_struct_layout.c` is a host `sizeof`/`offsetof`/field-width probe;
`tests/nvrm_struct_layout.txt` is its actual stdout, not a hand-entered table.
The gate recompiles it against the read-only vendor source
`~/workspace/_ext/open-gpu-kernel-modules-580.173.02`, runs it, and compares
its entire output with the committed measurement. Includes are the vendor
SDK, common, UVM and nvalloc/unix header directories. Compile command:

```sh
cc -std=gnu11 -Wall -Wextra -Werror \
  -I"$V/src/common/sdk/nvidia/inc" -I"$V/kernel-open/common/inc" \
  -I"$V/kernel-open/nvidia-uvm" \
  -I"$V/src/nvidia/arch/nvalloc/unix/include" \
  tests/nvrm_struct_layout.c -o "$TMP/probe"
```

The source/output cover RM request and reply fields, card fields, channel,
context share and scheduling, VASPACE, GID and every UVM parameter block used
by this client. The differential independently derives its schema with vendor
`offsetof` and `sizeof(field)` and compares that schema to Omega modes 2–5.
Examples directly from the probe: VASPACE size 56, `vaBase` at 40; GPU info V2
size 564 with its list at 4; GR info is a distinct 8-byte structure; GID size
268, length at 8 and byte data at 12. The card table contains 64 measured
72-byte records. Channel entries have their measured field width, not a width
inferred from the next offset.

The codec projects 26 layouts: ID, device, subdevice, virtual allocation,
GPU info V2, GR-get, classlist, VASPACE, GID, memory, channel, channel group,
context share, submit token, schedule, cards, GR info, and the UVM initialize,
MM initialize, range/free, register GPU, UUID-only unregister, register VASPACE,
register channel, unregister channel and external-map blocks.

## Independent controls and partitions

`tests/nvrm_struct_test.c` uses vendor field descriptors as a C byte oracle.
For every projected field of every layout it varies zero, one, the field's
maximum, maximum minus one, the high bit and a mixed-bit value. The oracle
masks to the actual vendor field width. It compares every byte of the whole
block, including zeroed reserved fields and padding, then decodes the field.
A combined nonzero-fields case catches overlap. Every GID data byte is a
separate field. Every card field is decoded at all 64 indexes, and both GPU
list fields at all 70 indexes; one-past-end indexes and short buffers are
refused. UVM command sizes/status and heap status/total/free have independent
checks. The red control corrupts the C expected block and must fail the byte
comparison.

`tests/nvrm_struct_check.py` changes real Omega semantic sites, regenerates
and compiles each mutant, and requires the independent oracle to reject it.
Compile failure does not count as a kill. All 12 were killed: GPU block size,
card table size, device mode offset, VASPACE base offset, GID data offset,
memory limit offset, channel engine offset, reversed card bound, GID byte
width, zero fill, UVM status width and heap free offset. Existing wire and
policy red controls and the six existing policy mutants also passed.

## Toolchain and shared implementation

Seeded omc SHA-256:
`2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f`.
Bridge source used: draft omega-bridge #76,
`741c1ebb2b0fbae311a77ad2118b13d5ec7a17ee`.
Portable converter SHA-256:
`d84fd6ebd258170a023ed5235880cd3e1ca8664703a2542542a7f8d103120538`;
portable frontend:
`a3a455bd77381e132215924295763e8728889d7d578278ae376de78324be8629`.
The bridge is draft / host-EXERCISED, not QUALIFIED. This evidence does not
claim its native-v2 path, hardware behavior or unrelated lowering is qualified.

`om/units.sh` applies `bridge-seeded --runtime-views c|h` only to these
composites; ordinary units share the single portable converter pin:

- `nvrm_struct=native/nvrm/uvm_fields.om,native/nvrm/struct.om`
- `nvrm_uvm_wire=native/nvrm/uvm_fields.om,native/nvrm/uvm_wire.om`
- `nvrm_uvm_map_wire=native/nvrm/uvm_fields.om,native/nvrm/uvm_map_wire.om`

`uvm_fields.om` holds the existing UVM byte bodies shared by both callers.
The older fixed-size exports keep their ABI; the struct entry passes exact-size
runtime views. No parallel UVM encoder is retained. Existing envelope and heap
encoders remain the implementation for their existing requests. Generated C
is converter output, not a handwritten substitute for the Omega computation.
Host build dependency: GCC `13.3.0` (Ubuntu `13.3.0-6ubuntu2~24.04.1`).
Host call/runtime dependency: glibc `2.39`. These existing dependencies compile
and call the bridge-generated C. Their Omega replacement belongs to the seeded
compiler/native-linker work; no host-runtime replacement is claimed here.

## Reproduction and observed results

Every probe execution, test, mutant, differential and unit check ran with the
following isolation prefix. It hides host GPU devices; no live ioctl was made.
Build parallelism was at most `-j3`. The full `make test` was not run.

```sh
export OMEGA_OMC="$HOME/work/cudastack/omega-mv/seed/out/omc"
export OMEGA_SEED="$HOME/work/cudastack/omega-bridge/build/bootstrap/omc"
export OMEGA_BRIDGE=/tmp/bridge-pin-merge # checkout at the exact revision above
bwrap --dev-bind / / --dev /dev --proc /proc -- \
  taskset -c 10-14 nice -n 10 \
  make -j3 test-nvrm-struct test-nvrm-wire test-nvrm-native \
    test-nvrm-differential test-nvrm-boundary test-nvrm-policy test-streams-submit
bwrap --dev-bind / / --dev /dev --proc /proc -- \
  taskset -c 10-14 nice -n 10 sh om/units.sh --check
```

- `host.log`: exit 0; all named targets PASS; struct red killed, 12 struct
  mutants killed, existing policy six-mutant gate PASS, native fake-RM request
  bytes/replies PASS, C/Omega lifecycle output equal, submit image equal.
- `units-check.log`: exit 0, generated units regenerate byte for byte.
- `crumb.log`: compile and verify results for this change.

Home-directory prefixes in logs are replaced with `~` for publication; no
semantic output is changed. Initial compilation caught an invalid bare return
in the new close error path; it was fixed to retain fault state while continuing
cleanup before the successful host run recorded here.

## Chip runbook — NOT_RUN

Only the operator may authorize the exclusive GPU window. This document does
not grant permission to open devices, alter a driver, touch the serving model,
claim the GPU lock or run live checks. Do not run a live command under the host
sandbox and misreport the inevitable refusal as chip evidence.

In an authorized window, first pin this PR and bridge revisions and rerun the
isolated host gates above. Record kernel/open-module version and device IDs;
require the 580.173.02 interface and the documented chip admission procedure.
Then use the repository's documented `make hello-gpu` entrypoint (which invokes
its doctor) with bounded build parallelism. Record allocation, channel creation,
GPU UUID/SM decoding, launch result and teardown, process exit and fresh driver
fault diagnostics. Follow the maintainer's existing exclusive-window runbook;
do not modify the lock/watchdog/service policy. A C-oracle live comparison
requires separate explicit authorization and an isolated output directory.

Expected: correct hello output and clean teardown on the intended chip, with no
new driver fault. A failed admission or unavailable GPU is NOT_RUN, not PASS.
Preserve actual logs and negative-control results in a new chip receipt; never
edit this host receipt into a chip qualification. No chip step was executed here.

Harness guide changes are tracked by `aien-dev/omega-harness#194`, consolidated
by Integrator4 because that repository is outside this lane's write scope.
