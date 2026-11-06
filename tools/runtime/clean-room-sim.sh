#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# clean-room-sim.sh <branch> [log-file]   (bar C2 without a GPU: hello-GPU on the simulator from an empty folder)
#
# Clones <branch> into a fresh temporary folder, takes the commands of the README section "Run your first kernel
# without a GPU" (its first bash block) from that clone, and runs only those, one by one, in a new empty folder with an
# empty environment: env -i with HOME set to a new empty directory, PATH=/usr/local/bin:/usr/bin:/bin, LANG=C.UTF-8
# and nothing else. No sudo, no GPU lock: the block must open no /dev/nvidia node, which the script checks. The whole
# block runs under strace (execve and openat, following every child); the check fails on a successful open of a
# /dev/nvidia* node or of a CUDA file (toolkit tree, CUDA or NVIDIA JIT user-space libraries, nvcc). Two red controls
# show the check can see one: a real traced read of a CUDA toolkit file, and a synthetic /dev/nvidia0 open line given to
# the same checker (the script never opens a device, not even as a control).
# CUDAPP_URL overrides the repository (a local path or file:// URL works; the clone line of the README is pointed at it).
# Exit code: that of the first README command that fails (0 when all pass), 3 setup failure, 4 the trace saw a device
# node or CUDA file opened, 5 a red control was not caught.
set -uo pipefail
branch=${1:?usage: clean-room-sim.sh <branch> [log-file]}
log=${2:-/dev/stdout}
url=${CUDAPP_URL:-https://github.com/aien-dev/CUDApp}
work=$(mktemp -d "${TMPDIR:-/tmp}/cudapp-cleanroom.XXXXXX") || exit 3
cleanup() { rm -rf "$work"; }
trap cleanup EXIT
CLEAN_PATH=/usr/local/bin:/usr/bin:/bin
# A successful execve/openat (no "= -1") of a device node or a CUDA file. The zero-CUDA gate's own red control builds a
# stub libcudart.so in a mktemp directory (/tmp/tmp.*) on purpose; those calls are counted apart and are not a CUDA use.
BAD_RE='"(/dev/nvidia[^"]*|/usr/local/cuda[^"]*|/opt/nvidia[^"]*|[^"]*/(libcuda|libcudart|libcublas|libcudnn|libnvrtc|libnvJitLink|libnvidia-ptxjitcompiler)[^"/]*|[^"]*/nvcc)"'
bad_hits() { grep -E '(execve|openat)\(' "$1" | grep -vE '= -1 ' | grep -E "$BAD_RE" | grep -vE '"/tmp/tmp\.[A-Za-z0-9]+/' || true; }
stub_hits() { grep -E '(execve|openat)\(' "$1" | grep -vE '= -1 ' | grep -E "$BAD_RE" | grep -cE '"/tmp/tmp\.[A-Za-z0-9]+/' || true; }
{
	echo "# clean-room-sim: branch=$branch url=$url"
	echo "# date: $(date -u +%FT%TZ)  host: $(uname -srm)  loadavg: $(cut -d' ' -f1-3 /proc/loadavg)"
	echo "# tools: $(git --version), $(make --version | sed -n 1p), $(cc --version | sed -n 1p), $(strace -V | sed -n 1p)"
	git clone -q --branch "$branch" "$url" "$work/src.readme" 2>&1 || { echo "FAIL: cannot clone $branch"; exit 3; }
	cmds=$(awk '/^## Run your first kernel without a GPU/{s=1;next} s&&/^```bash/{b=1;next} s&&b&&/^```/{exit} s&&b{print}' "$work/src.readme/README.md")
	[ -n "$cmds" ] || { echo "FAIL: no bash block under 'Run your first kernel without a GPU' in README.md"; exit 3; }
	echo "# README commands, verbatim:"; printf '%s\n' "$cmds" | sed 's/^/#   /'
	rm -rf "$work/src.readme"
	# the one change: the clone takes the branch under test from $url
	cmds=$(printf '%s\n' "$cmds" | sed -E "s#^git clone #git clone --branch $branch #; s#https://github.com/aien-dev/CUDApp([^A-Za-z0-9_.-]|\$)#$url\\1#")
	mkdir "$work/home" "$work/empty" || exit 3
	E() { env -i HOME="$work/home" PATH="$CLEAN_PATH" LANG=C.UTF-8 "$@"; }
	echo "# environment of every command (env -i, then exactly these):"
	E env | sort | sed 's/^/#   /'
	echo "# cwd: $work/empty, contents: [$(ls -A "$work/empty")]"
	rc=0; dir="$work/empty"; n=0; t0=$(date +%s)
	while IFS= read -r line; do
		[ -n "$line" ] || continue
		n=$((n+1))
		echo "\$ $line"
		(cd "$dir" && E strace -f -qq -e trace=execve,openat -o "$work/trace.$n" bash -c "$line") 2>&1 || { rc=$?; echo "# command exited $rc"; }
		[ "$rc" -eq 0 ] || break
		case $line in "cd "*) dir="$dir/${line#cd }"; echo "# commit: $(git -C "$dir" rev-parse HEAD)";; esac
	done <<<"$cmds"
	echo "# README block exit code: $rc  ($(( $(date +%s) - t0 )) s)"
	echo "# --- trace check: device nodes and CUDA files opened or executed during the README block ---"
	cat "$work"/trace.* > "$work/readme.trace" 2>/dev/null
	hits=$(bad_hits "$work/readme.trace")
	echo "#   traced calls: $(wc -l < "$work/readme.trace"), programs executed: $(grep -c 'execve(' "$work/readme.trace")"
	echo "#   /dev/nvidia* paths tried and refused or absent (= -1): $(grep -E '(execve|openat)\(' "$work/readme.trace" | grep -E '"/dev/nvidia' | grep -cE '= -1 ' || true)"
	if [ -n "$hits" ]; then
		echo "# TRACE FAIL: a device node or CUDA file was opened or executed:"; printf '%s\n' "$hits" | sed -n 1,10p | sed 's/^/#   /'
		[ "$rc" -eq 0 ] && rc=4
	else
		echo "# TRACE PASS: no /dev/nvidia node and no CUDA file was opened or executed"
		echo "#   (the zero-CUDA gate's own stub libcudart in /tmp/tmp.*: $(stub_hits "$work/readme.trace") successful calls, its red control)"
	fi
	echo "# --- red control 1: the same trace and check on a real read of a CUDA toolkit file ---"
	ctl=/usr/local/cuda/version.json
	if [ -r "$ctl" ]; then
		E strace -f -qq -e trace=execve,openat -o "$work/control.trace" cat "$ctl" > /dev/null 2>&1
		c=$(bad_hits "$work/control.trace")
		if [ -n "$c" ]; then echo "# TRACE RED CONTROL 1: caught as it must ($(printf '%s\n' "$c" | wc -l) uses, first: $(printf '%s\n' "$c" | sed -n 1p))"
		else echo "# TRACE RED CONTROL 1 NOT CAUGHT"; [ "$rc" -eq 0 ] && rc=5; fi
	else echo "# TRACE RED CONTROL 1 NOT_RUN: $ctl is absent on this machine"; fi
	echo "# --- red control 2: a synthetic successful /dev/nvidia0 open given to the same checker ---"
	printf '%s\n' '4242  openat(AT_FDCWD, "/dev/nvidia0", O_RDWR|O_CLOEXEC) = 3' > "$work/synthetic.trace"
	if [ -n "$(bad_hits "$work/synthetic.trace")" ]; then echo "# TRACE RED CONTROL 2: caught as it must"
	else echo "# TRACE RED CONTROL 2 NOT CAUGHT"; [ "$rc" -eq 0 ] && rc=5; fi
	echo "# clean-room-sim exit code: $rc"
	exit $rc
} > >(tee "$log") 2>&1
