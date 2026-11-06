# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Seeded Omega units behind the host tools (om/tools/*.om). Each test-tools-om-<unit> target runs the
# unit's differential against its C oracle (the default-OFF CUDAPP_MIGRATION_PENDING_C_TOOLS_<PART> build),
# a flipped-record negative control and recompiled .om mutants through the pinned omc and bridge
# (OMEGA_OMC, OMEGA_BRIDGE). Host only; run under bwrap/taskset (tests/tools-om/README.md).
.PHONY: test-tools-om test-tools-om-own30b test-tools-om-doctor test-tools-om-fmt
test-tools-om-own30b: tools/own30b_decode_profile.c src/cudapp_om_own30b_prof.c tests/tools-om/own30b_diff.c
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -DCUDAPP_MIGRATION_PENDING_C_TOOLS_OWN30B -c -o $(OUT_DIR)/own30b_decode_profile_c.o tools/own30b_decode_profile.c
	sh tests/tools-om/own30b.sh
test-tools-om-doctor: tools/doctor/cudapp_doctor.c src/cudapp_om_doctor.c om/tools/fmt.om om/tools/doctor.om tests/tools-om/doctor.sh
	sh tests/tools-om/doctor.sh
test-tools-om-fmt: om/tools/fmt.om tests/tools-om/fmt_diff.c tests/tools-om/fmt.sh
	sh tests/tools-om/fmt.sh
test-tools-om: test-tools-om-own30b test-tools-om-fmt test-tools-om-doctor
