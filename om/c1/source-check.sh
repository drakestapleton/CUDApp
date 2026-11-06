#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
set -eu
lang=${1:-}
read -r want < om/c1/omega-mv.lock
[ -n "$lang" ] && [ -d "$lang/.git" -o -f "$lang/.git" ] || { echo "NOT_RUN C1: set OMEGA_C1_SOURCE to omega-mv source pin $want" >&2; exit 3; }
have=$(git -C "$lang" rev-parse HEAD)
[ "$have" = "$want" ] || { echo "NOT_RUN C1: source pin $have != $want" >&2; exit 3; }
git -C "$lang" diff --quiet HEAD -- . || { echo 'NOT_RUN C1: tracked language checkout files differ from pin' >&2; exit 3; }
compiler=3ad9f10f64309e7acf8a725082b28e23a2a4aeb07e98322fb4dc0fcd8fc93194
[ -x "$lang/seed/out/omc" ] || { echo "NOT_RUN C1: compiler artifact absent; required sha256 $compiler" >&2; exit 3; }
have_compiler=$(sha256sum "$lang/seed/out/omc")
have_compiler=${have_compiler%% *}
[ "$have_compiler" = "$compiler" ] || { echo "NOT_RUN C1: compiler sha256 $have_compiler != $compiler" >&2; exit 3; }
echo "C1 SOURCE PASS source=$have compiler_sha256=$have_compiler"
