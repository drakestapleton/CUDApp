# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# tools/artifact_export.c: the command line, the v1 contract checks, the kernel choice, the launch shape,
# the manifest texts, every message and the exit status are om/tools/artexp.om (src/cudapp_om_artexp.c in
# the library). CUDAPP_MIGRATION_PENDING_C_TOOLS_ARTEXP=1 builds the former C: the oracle of
# tests/tools-om/artexp.sh.
CUDAPP_MIGRATION_PENDING_C_TOOLS_ARTEXP ?= 0
ifeq ($(CUDAPP_MIGRATION_PENDING_C_TOOLS_ARTEXP),1)
CFLAGS += -DCUDAPP_MIGRATION_PENDING_C_TOOLS_ARTEXP
endif
