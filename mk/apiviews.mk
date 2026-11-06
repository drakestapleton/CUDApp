# Host-data apiviews gate. Compile is unwrapped. Every test and mutant runs
# under the lane bwrap rule.
ROOT := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/..)
CC ?= cc
OUT ?= $(ROOT)/build/apiviews
OMC ?= $(HOME)/work/cudastack/omega-mv/seed/out/omc
BRIDGE ?= /tmp/bridge-71
BWRAP = bwrap --dev-bind / / --dev /dev --proc /proc -- taskset -c 10-14 nice -n 10
AV_C = $(ROOT)/src/cudapp_om_av_bf16.c $(ROOT)/src/cudapp_om_av_e4m3.c \
	$(ROOT)/src/cudapp_om_av_ewarg.c $(ROOT)/src/cudapp_om_av_fp8am.c \
	$(ROOT)/src/cudapp_om_av_fp8en.c $(ROOT)/src/cudapp_om_av_fp8q.c \
	$(ROOT)/src/cudapp_om_av_fp8sc.c $(ROOT)/src/cudapp_om_av_gdnch.c \
	$(ROOT)/src/cudapp_om_av_gdnsc.c $(ROOT)/src/cudapp_om_av_gdnsl.c \
	$(ROOT)/src/cudapp_om_av_rope.c $(ROOT)/src/cudapp_om_av_scan.c \
	$(ROOT)/src/cudapp_om_av_widen.c
CFLAGS = -std=c11 -Wall -Wextra -Wno-unused-function -I$(ROOT)/src -ffp-contract=off -O0 --coverage \
	-DCUDAPP_MIGRATION_PENDING_APIVIEWS_DIFF

.PHONY: test red mutants bench check

$(OUT)/apiviews_host_test: $(ROOT)/tests/apiviews_host_test.c $(AV_C) $(ROOT)/src/omega_gpu_apiviews.h
	mkdir -p $(OUT)
	$(CC) $(CFLAGS) -o $@ $(ROOT)/tests/apiviews_host_test.c $(AV_C) -lm

$(OUT)/apiviews_host_selftest: $(ROOT)/tests/apiviews_host_test.c $(AV_C) $(ROOT)/src/omega_gpu_apiviews.h
	mkdir -p $(OUT)
	$(CC) $(CFLAGS) -DCUDAPP_MIGRATION_PENDING_APIVIEWS_DIFF_SELFTEST -o $@ \
		$(ROOT)/tests/apiviews_host_test.c $(AV_C) -lm

test: $(OUT)/apiviews_host_test
	$(BWRAP) $(OUT)/apiviews_host_test

bench: $(OUT)/apiviews_host_test
	$(BWRAP) $(OUT)/apiviews_host_test --bench

red: $(OUT)/apiviews_host_selftest
	$(BWRAP) $(OUT)/apiviews_host_selftest; rc=$$?; \
	if [ $$rc -eq 0 ]; then echo "RED CHECK BROKEN: selftest passed"; exit 1; fi; \
	echo "RED CHECK: selftest rc $$rc"

check:
	OMEGA_OMC=$(OMC) OMEGA_BRIDGE=$(BRIDGE) sh $(ROOT)/om/units.sh --check

# Five killed mutants. Each sed site is unique in its unit. A mutant that
# still prints PASS is a failure of the gate.
mutants: $(OUT)/apiviews_host_test
	sh $(ROOT)/tests/apiviews_mutants.sh $(OUT) $(OMC) $(BRIDGE) $(ROOT)
