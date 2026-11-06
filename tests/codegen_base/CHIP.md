# Base codegen/common replay chip qualification

**NOT_RUN. This assignment grants no GPU, live-RM or VFIO authority.**
No device, service, driver, boot target or GPU lock was changed by this lane.
Host byte equality, CPU simulation and the inherited freestanding C oracle
are not chip qualification.

1. Integrate this branch with #309 and the intended family/f64 branches. Keep
   the single `omega_gpu_om_ir_replay` entry. If #312 is present, use its
   `BW_IR_LAST` as `omega_cg_replay`'s maximum-op argument; do not allocate 79
   implicitly. Freeze source/compiler/bridge revisions and every generated
   unit `.ident` in a new receipt. The base pin is omega-mv e5b32567 and
   omc SHA-256 2c82480596bbdc50cda9626866209ef662d5834f278f839ff4ce0250c495029f.
2. First run the host differential, five seeded-source mutants and existing
   codegen tests from README.md in the exact device-hidden bwrap wrapper.
   Require full SASS and cubin equality and require the one-bit red check to
   fail because bytes differ, not because execution lacked a dependency.
3. Obtain fresh explicit exclusive-window authorization under the current
   Spark procedure. Confirm the exact allowed devices and service disposition
   with the operator. The present `/dev`-hidden wrapper intentionally cannot
   execute on hardware. Neither a free lock nor a previous receipt grants
   permission to bind devices or stop a service.
4. Only in that newly authorized environment, follow the existing
   `tests/regalloc_om/CHIP.md` chip sweep using `gpu_matmul_api_test` and its
   documented output option. Use a new receipt directory, not any historical
   evidence. Exercise programs with loop-carried intervals, each instruction
   family adopted in the integrated source, scalar/uniform pressure and
   different padding boundaries. Before launch, independently encode the same
   accepted IR with the default Omega path and the explicitly selected C
   codegen/encoder oracles and require byte-identical SASS and cubin images.
   Reject malformed opcode/operand/closure metadata before any launch.
5. Require expected/observed output equality from the existing chip reference
   and a real detected mutant mismatch. A missing-device refusal is NOT_RUN,
   not a successful negative control. Stop on a device fault or unexpected
   session state; do not clear alarms, reset a driver, or weaken any gate.
6. Record the exact commands and rc, source/unit identities, hardware state,
   input corpus, output and negative-control evidence. Report chip, live-RM
   and VFIO separately from the already exercised host/simulator tiers.
   Qualification does not authorize merge or deployment.
