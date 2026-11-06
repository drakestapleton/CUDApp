# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Developer bar D2 and D6 (docs/PAIN_POINTS.md): `make doctor` says, in sentences, whether this
# machine can run CUDA++ and what to do if not. It reads files under /proc, /sys and /dev only:
# it opens no device and needs no GPU window. `make hello-gpu` runs it first.
DOCTOR = $(OUT_DIR)/cudapp-doctor
# The one version CUDA++ has to match: the NVIDIA open kernel module whose headers it is built
# against. Taken from the folder name, so there is exactly one place it is written.
CUDAPP_NVIDIA_OPEN_ABI := $(patsubst nvidia-open-%,%,$(notdir $(NVIDIA_OPEN)))

.PHONY: doctor test-doctor
# The checklist itself is the seeded Omega unit om/tools/doctor.om (src/cudapp_om_doctor.c, from sh om/units.sh);
# cudapp_doctor.c probes and passes the facts in. src/omega_gpu_chip.c also holds the library admission, which calls
# the generated om/admission/chip_admit.om unit.
$(DOCTOR): tools/doctor/cudapp_doctor.c src/cudapp_om_doctor.c $(wildcard src/omega_gpu_chip.c src/omega_gpu_chip.h src/omega_chip_enum.h src/cudapp_om_chip_admit.c src/cudapp_om_chip_enum.c) | $(OUT_DIR)
	$(CC) $(CFLAGS) -DCUDAPP_NVIDIA_OPEN_ABI='"$(CUDAPP_NVIDIA_OPEN_ABI)"' -o $@ tools/doctor/cudapp_doctor.c src/cudapp_om_doctor.c \
		$(wildcard src/omega_gpu_chip.c src/cudapp_om_chip_admit.c src/cudapp_om_chip_enum.c)
doctor: $(DOCTOR)
	$(DOCTOR); rc=$$?; sh tools/multitarget/targets-doctor.sh '$(OMC)' $(OUT_DIR)/multitarget || rc=1; exit $$rc
test-doctor: $(DOCTOR)
	sh tests/doctor/run.sh $(DOCTOR)
