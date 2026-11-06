# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# C6 (CUDApp#10): the sm_121 disassembler in seeded Omega (om/disasm/sm121_dis.om), linked through its generated
# binding src/cudapp_om_sm121_dis.c (om/units.sh). The C decoder in src/omega_sm121_disasm.c is migration-pending:
# CUDAPP_DISASM=c builds it alone, CUDAPP_DISASM=diff runs both on every word and aborts on a difference.
#   make test-om-disasm          (in `make test`; no Omega tool, no NVIDIA tool) tests/sm121_dis_diff_test.c:
#       1. nine golden words print their text, and each with one flipped bit (the corrupted word) does not;
#       2. .om vs C on 25,928 words: every row of the C table with random operands, random words, IMAD immediates;
#       3. red checks: one planted byte in one .om text must FAIL step 2; corrupted words held to the golden text
#          must FAIL step 1.
#   make test-om-disasm-fimm     (pinned OMEGA_OMC and OMEGA_BRIDGE) the float-immediate printer, .om vs C, over a
#       binary32 sweep through the test-only entry om/disasm/fimm_probe.om; red check: one planted byte must FAIL.
#   make test-om-disasm-mutants  (pinned OMEGA_OMC and OMEGA_BRIDGE) four mutant .om decoders, each compiled by the
#       pinned omc into its own binding, must each fail the differential or the float sweep.
# make test-sm121dis (mk/sm121dis.mk) is the nvdisasm oracle and the .om vs C comparison over the kernel corpus.
CARRIED_HOST_TESTS += test-om-disasm
OM_DIS_DIR = $(OUT_DIR)/om-disasm
OM_DIS_BUILD = gcc -std=gnu11 -O2 -Wall -Wextra -Werror -Isrc
OM_DIS_DEPS = tests/sm121_dis_diff_test.c src/omega_sm121_disasm.c src/omega_sm121_disasm.h src/cudapp_native.h
OM_DIS_PIN_CHECK = [ -x "$(OMEGA_OMC)" ] && [ -f "$(OMEGA_BRIDGE)/src/toc.om" ] && [ -f "$(OMEGA_BRIDGE)/src/freest.om" ] || \
	{ echo "NOT_RUN: set pinned OMEGA_OMC and OMEGA_BRIDGE"; exit 3; }
# om/units.sh generates both units of a directory: the decoder and the float probe (the decoder with fimm_probe.om
# appended; the bridge exports the last function of a unit).
OM_DIS_UNITS = sm121_dis=$$(pwd)/$$d/sm121_dis.om sm121_dis_fimm=$$(pwd)/$$d/sm121_dis.om,$$(pwd)/om/disasm/fimm_probe.om

.PHONY: test-om-disasm test-om-disasm-fimm test-om-disasm-mutants
$(OM_DIS_DIR)/diff: $(OM_DIS_DEPS) src/cudapp_om_sm121_dis.c
	@mkdir -p $(OM_DIS_DIR)
	$(OM_DIS_BUILD) -o $@ tests/sm121_dis_diff_test.c src/cudapp_om_sm121_dis.c

test-om-disasm: $(OM_DIS_DIR)/diff
	$(OM_DIS_DIR)/diff
	@if $(OM_DIS_DIR)/diff --plant > $(OM_DIS_DIR)/plant.log 2>&1; then \
		echo "RED CHECK BROKEN: a planted .om text difference passed"; exit 1; \
	elif ! grep -q '^FAIL .* words, 1 differ' $(OM_DIS_DIR)/plant.log; then \
		echo "RED CHECK BROKEN: the planted run did not report one difference"; tail -3 $(OM_DIS_DIR)/plant.log; exit 1; \
	else echo "red check: the planted .om text difference FAILS, as it must"; fi
	@if $(OM_DIS_DIR)/diff --corrupt > $(OM_DIS_DIR)/corrupt.log 2>&1; then \
		echo "RED CHECK BROKEN: corrupted words passed as golden"; exit 1; \
	elif ! grep -q '^FAIL corrupted words read as golden: 9 of 9 differ' $(OM_DIS_DIR)/corrupt.log; then \
		echo "RED CHECK BROKEN: not every corrupted word failed"; tail -3 $(OM_DIS_DIR)/corrupt.log; exit 1; \
	else echo "red check: every corrupted word FAILS the golden text, as it must (9 of 9)"; fi

test-om-disasm-fimm: tests/sm121_dis_fimm_test.c $(OM_DIS_DEPS) src/cudapp_om_sm121_dis.c om/disasm/sm121_dis.om om/disasm/fimm_probe.om
	@$(OM_DIS_PIN_CHECK)
	@d=$(OM_DIS_DIR)/fimm; rm -rf $$d; mkdir -p $$d; cp om/disasm/sm121_dis.om $$d/sm121_dis.om; \
	OMEGA_UNITS="$(OM_DIS_UNITS)" OMEGA_UNITS_DEST=$$d OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" \
		sh om/units.sh > /dev/null || exit 1; \
	cmp -s $$d/cudapp_om_sm121_dis.c src/cudapp_om_sm121_dis.c || { echo "FAIL: committed binding differs from its regeneration"; exit 1; }; \
	$(OM_DIS_BUILD) -o $$d/fimm tests/sm121_dis_fimm_test.c src/cudapp_om_sm121_dis.c $$d/cudapp_om_sm121_dis_fimm.c || exit 1; \
	$$d/fimm || exit 1; \
	if $$d/fimm --plant > $$d/plant.log 2>&1; then echo "RED CHECK BROKEN: a planted float text difference passed"; exit 1; \
	elif ! grep -q '^FAIL .* floats, 1 differ' $$d/plant.log; then echo "RED CHECK BROKEN: no single planted difference"; exit 1; \
	else echo "red check: the planted float text difference FAILS, as it must"; fi

# Each mutant is one edit of the .om source; each target line occurs exactly once in sm121_dis.om.
#   1 field a/A read one bit higher; 2 a power-of-two IMAD immediate of 2^31 (Rc = RZ) printed as IMAD.SHL instead
#   of plain IMAD; 3 the never-true guard @!PT dropped like @PT; 4 the float printer switches to exponent form one
#   decade late.
OM_DIS_MUTANTS = 's/^ret fenc 0 24 8$$/ret fenc 0 25 8/' \
	's/^if or eq n 16 eq n 31$$/if eq n 16/' \
	's/^if and eq g 7 eq gn 0$$/if eq g 7/' \
	's/^if or lt x -4 ge x long n$$/if or lt x -5 ge x long n/'
test-om-disasm-mutants: tests/sm121_dis_fimm_test.c $(OM_DIS_DEPS) om/disasm/sm121_dis.om om/disasm/fimm_probe.om
	@$(OM_DIS_PIN_CHECK)
	@n=0; for m in $(OM_DIS_MUTANTS); do n=$$((n+1)); d=$(OM_DIS_DIR)/mutant$$n; rm -rf $$d; mkdir -p $$d; \
	sed "$$m" om/disasm/sm121_dis.om > $$d/sm121_dis.om; \
	if cmp -s $$d/sm121_dis.om om/disasm/sm121_dis.om; then echo "MUTANT $$n BROKEN: '$$m' changed nothing"; exit 1; fi; \
	OMEGA_UNITS="$(OM_DIS_UNITS)" OMEGA_UNITS_DEST=$$d OMEGA_OMC="$(OMEGA_OMC)" OMEGA_BRIDGE="$(OMEGA_BRIDGE)" \
		sh om/units.sh > /dev/null || exit 1; \
	$(OM_DIS_BUILD) -o $$d/diff tests/sm121_dis_diff_test.c $$d/cudapp_om_sm121_dis.c || exit 1; \
	$(OM_DIS_BUILD) -o $$d/fimm tests/sm121_dis_fimm_test.c $$d/cudapp_om_sm121_dis.c $$d/cudapp_om_sm121_dis_fimm.c || exit 1; \
	w=0; $$d/diff > $$d/diff.log 2>&1 || w=1; \
	f=0; $$d/fimm > $$d/fimm.log 2>&1 || f=1; \
	if [ $$w = 0 ] && [ $$f = 0 ]; then echo "MUTANT $$n SURVIVED: '$$m'"; exit 1; fi; \
	echo "mutant $$n red ('$$m'): words $$( [ $$w = 1 ] && echo FAIL || echo pass ), floats $$( [ $$f = 1 ] && echo FAIL || echo pass )"; \
	done
