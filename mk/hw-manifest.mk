# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Hardware capability manifest (DRA-400/401): backend/hw/manifest.tsv checked and run by hwman
# (backend/hw/{core,cpu,tpm,main}.om, seeded Omega). OMC is the absolute path to the pinned seeded omc
# (aien-dev/omega-mv e5b32567 seed/out/omc).
#   make test-hw OMC=...   tests/hw/run.sh: manifest refusals, CPU lifecycle on processor HW_CPU
#                          (default 19) against Wolfram oracles, TPM lifecycle on /dev/tpmrm0 against
#                          tpm2_getcap. Not in HOST_TESTS: it touches a CPU and the TPM, so it runs
#                          only where the operator allots them. A missing device reports NOT_RUN.
OMC ?=
.PHONY: test-hw
test-hw:
	@test -n '$(OMC)' || { echo 'NOT_RUN: test-hw needs OMC=/absolute/path/to/pinned/omc'; exit 77; }
	sh tests/hw/run.sh '$(OMC)'
