#!/bin/bash
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Host test of `cudapp swap` on a fake machine root (no root rights, no GPU, nothing outside a temp dir touched).
# Every system command the tool runs (ldconfig, depmod, update-initramfs, lsinitramfs, mokutil, fuser, make, modinfo,
# uname) is a fake on PATH that logs its call, so the test sees exactly what the tool would do.
#   bash tests/swap/test_swap_cli.sh [path/to/cudapp]
set -u
TOOL=${1:-$(cd "$(dirname "$0")/../.." && pwd)/swap/cudapp}
PASS=0; FAIL=0
ok()   { PASS=$((PASS+1)); echo "ok   $*"; }
bad()  { FAIL=$((FAIL+1)); echo "FAIL $*"; }
check(){ local name=$1; shift; if "$@"; then ok "$name"; else bad "$name"; fi; }

KVER=7.0.0-1019-nvidia
mkroot() { # $1 = dir; builds a fake GB10 machine with NVIDIA's stack installed
    local R=$1
    mkdir -p "$R"/usr/lib/aarch64-linux-gnu "$R"/etc/ld.so.conf.d "$R"/etc/modprobe.d "$R"/etc/modules-load.d \
             "$R"/lib/modules/$KVER/kernel/drivers/video "$R"/boot "$R"/sys/bus/pci/devices/000f:01:00.0 \
             "$R"/sys/module/nvidia "$R"/dev "$R"/fakebin "$R"/driver-src "$R"/build
    echo nvidia-libcuda-580 > "$R"/usr/lib/aarch64-linux-gnu/libcuda.so.580.173.02
    ln -s libcuda.so.580.173.02 "$R"/usr/lib/aarch64-linux-gnu/libcuda.so.1
    echo "/usr/lib/aarch64-linux-gnu" > "$R"/etc/ld.so.conf.d/aarch64-linux-gnu.conf
    echo "original-cache" > "$R"/etc/ld.so.cache
    echo "nvidia.ko" > "$R"/lib/modules/$KVER/kernel/drivers/video/nvidia.ko
    echo "dep-original" > "$R"/lib/modules/$KVER/modules.dep
    echo "alias-original" > "$R"/lib/modules/$KVER/modules.alias
    echo "initrd-original" > "$R"/boot/initrd.img-$KVER
    echo 0x10de > "$R"/sys/bus/pci/devices/000f:01:00.0/vendor
    echo 0x2e12 > "$R"/sys/bus/pci/devices/000f:01:00.0/device
    echo 580.173.02 > "$R"/sys/module/nvidia/version
    touch "$R"/dev/nvidia0 "$R"/dev/nvidiactl
    echo "our-libcuda" > "$R"/build/libcuda.so.1
    echo "obj-m := aien_gpu.o" > "$R"/driver-src/Kbuild
    echo "/* aien_gpu */" > "$R"/driver-src/aien_gpu.c
    local F="$R"/fakebin
    for c in ldconfig depmod update-initramfs; do
        printf '#!/bin/bash\necho "%s $*" >> "%s/calls.log"\n' "$c" "$R" > "$F/$c"
    done
    # ldconfig also rewrites the cache (as the real one does), so the restore must bring the old bytes back
    printf '#!/bin/bash\necho "ldconfig $*" >> "%s/calls.log"\necho "cache-with-cudapp" > "%s/etc/ld.so.cache"\n' "$R" "$R" > "$F/ldconfig"
    printf '#!/bin/bash\necho "depmod $*" >> "%s/calls.log"\necho "dep-new" > "%s/lib/modules/%s/modules.dep"\n' "$R" "$R" $KVER > "$F/depmod"
    printf '#!/bin/bash\necho "update-initramfs $*" >> "%s/calls.log"\necho "initrd-new" > "%s/boot/initrd.img-%s"\n' "$R" "$R" $KVER > "$F/update-initramfs"
    printf '#!/bin/bash\ncat "%s/lsinitramfs.out" 2>/dev/null\n' "$R" > "$F/lsinitramfs"
    printf '#!/bin/bash\ncat "%s/sb.out" 2>/dev/null || echo "SecureBoot disabled"\n' "$R" > "$F/mokutil"
    printf '#!/bin/bash\ncat "%s/fuser.out" 2>/dev/null; exit 0\n' "$R" > "$F/fuser"
    printf '#!/bin/bash\necho %s\n' $KVER > "$F/uname"
    # make: "builds" aien_gpu.ko in M=dir
    printf '#!/bin/bash\necho "make $*" >> "%s/calls.log"\nfor a in "$@"; do case $a in M=*) echo "aien_gpu.ko built" > "${a#M=}/aien_gpu.ko";; esac; done\n' "$R" > "$F/make"
    printf '#!/bin/bash\necho "vermagic:       %s SMP preempt mod_unload modversions aarch64"\n' $KVER > "$F/modinfo"
    mkdir -p "$R"/lib/modules/$KVER/build
    chmod +x "$F"/*
}
tree_hash() { # hashes of every file and link under the fake root, minus the tool's own state and the fake tools
    (cd "$1" && find . \( -path ./var/lib/cudapp-swap -o -path ./fakebin -o -name calls.log -o -path ./driver-src -o -path ./build \) -prune -o \
        \( -type f -o -type l \) -print | LC_ALL=C sort | while read -r f; do
        if [ -L "$f" ]; then echo "L $f $(readlink "$f")"; else echo "F $f $(sha256sum < "$f" | cut -c1-16)"; fi; done)
}
run() { # run the tool against root $R
    PATH="$R/fakebin:$PATH" CUDAPP_ROOT="$R" CUDAPP_LIB="$R/build/libcuda.so.1" CUDAPP_DRIVER_SRC="$R/driver-src" \
        CUDAPP_KDIR="$R/lib/modules/$KVER/build" bash "$TOOL" "$@"
}

T=$(mktemp -d); trap 'rm -rf "$T"' EXIT

# 1. status names the GPU, the driver, Secure Boot, the holders and which libcuda.so.1 the loader finds
R=$T/r1; mkroot "$R"; echo "/dev/nvidia0: 108356" > "$R/fuser.out"
out=$(run swap status 2>&1); rc=$?
check "status exits 0" [ $rc -eq 0 ]
check "status names GB10" grep -q "GB10 (10de:2e12)" <<<"$out"
check "status names driver 580.173.02" grep -q "580.173.02" <<<"$out"
check "status names Secure Boot" grep -q "Secure Boot: disabled" <<<"$out"
check "status lists holder pid" grep -q "108356" <<<"$out"
check "status says NVIDIA libcuda is in use" grep -q "libcuda.so.1: NVIDIA" <<<"$out"

# 2. user without --yes is a dry run: prints the plan, changes nothing
R=$T/r2; mkroot "$R"; before=$(tree_hash "$R")
out=$(run swap user 2>&1); rc=$?
check "dry run exits 0" [ $rc -eq 0 ]
check "dry run says DRY RUN" grep -q "DRY RUN" <<<"$out"
check "dry run lists the ld.so.conf.d file" grep -q "etc/ld.so.conf.d/00000-cudapp-swap.conf" <<<"$out"
check "dry run changes nothing" [ "$before" = "$(tree_hash "$R")" ]
check "dry run runs no ldconfig" [ ! -s "$R/calls.log" ]

# 3. user --yes installs our lib first in the loader's search, keeps a dated backup; back restores every hash
out=$(run swap user --yes 2>&1); rc=$?
check "user --yes exits 0" [ $rc -eq 0 ]
check "our libcuda installed" cmp -s "$R/usr/lib/cudapp/libcuda.so.1" "$R/build/libcuda.so.1"
check "ld.so.conf.d entry points at /usr/lib/cudapp" grep -qx "/usr/lib/cudapp" "$R/etc/ld.so.conf.d/00000-cudapp-swap.conf"
check "ldconfig ran" grep -q "^ldconfig" "$R/calls.log"
check "NVIDIA libcuda untouched" [ "$(cat "$R/usr/lib/aarch64-linux-gnu/libcuda.so.580.173.02")" = nvidia-libcuda-580 ]
check "backup manifest written" bash -c "ls >/dev/null '$R'/var/lib/cudapp-swap/backup-*-user/manifest.tsv"
check "status now says user level swapped" bash -c "PATH='$R/fakebin:$PATH' CUDAPP_ROOT='$R' bash '$TOOL' swap status | grep -q 'user level: SWAPPED'"
out=$(run swap back --yes 2>&1); rc=$?
check "back exits 0" [ $rc -eq 0 ]
check "back reports PASS" grep -q "BACK PASS" <<<"$out"
check "back restores every hash" [ "$before" = "$(tree_hash "$R")" ]

# 4. kernel refuses with Secure Boot on (an unsigned module would be refused at boot)
R=$T/r4; mkroot "$R"; echo "SecureBoot enabled" > "$R/sb.out"
out=$(run swap kernel --yes 2>&1); rc=$?
check "kernel refuses under Secure Boot (exit 3)" [ $rc -eq 3 ]
check "refusal names Secure Boot" grep -q "Secure Boot is enabled" <<<"$out"

# 5. kernel --yes refuses while processes hold /dev/nvidia*, naming them
R=$T/r5; mkroot "$R"; echo "/dev/nvidia0: 108356 1816" > "$R/fuser.out"; before=$(tree_hash "$R")
out=$(run swap kernel --yes 2>&1); rc=$?
check "kernel refuses with holders (exit 3)" [ $rc -eq 3 ]
check "refusal names the holder" grep -q "108356" <<<"$out"
check "refusal changed nothing" [ "$before" = "$(tree_hash "$R")" ]

# 6. kernel refuses on a machine without a GB10
R=$T/r6; mkroot "$R"; echo 0x2b85 > "$R/sys/bus/pci/devices/000f:01:00.0/device"
out=$(run swap kernel 2>&1); rc=$?
check "kernel refuses without GB10 (exit 3)" [ $rc -eq 3 ]
check "refusal names the device found" grep -q "10de:2b85" <<<"$out"

# 7. kernel --yes stages the switch (module, hold-back, autoload, initramfs when NVIDIA is in it); back restores hashes
R=$T/r7; mkroot "$R"; echo "usr/lib/modules/$KVER/kernel/drivers/video/nvidia.ko" > "$R/lsinitramfs.out"; before=$(tree_hash "$R")
out=$(run swap kernel 2>&1)
check "kernel dry run names the reboot" grep -q "reboot" <<<"$out"
check "kernel dry run changed nothing" [ "$before" = "$(tree_hash "$R")" ]
out=$(run swap kernel --yes 2>&1); rc=$?
check "kernel --yes exits 0" [ $rc -eq 0 ]
MC="$R/etc/modprobe.d/cudapp-swap-kernel.conf"
for m in nvidia nvidia_uvm nvidia_modeset nvidia_drm; do
    check "hold-back blacklists $m" grep -qx "blacklist $m" "$MC"
    check "hold-back stops explicit modprobe $m" grep -qx "install $m /bin/false" "$MC"
done
check "aien_gpu installed for the running kernel" [ -f "$R/lib/modules/$KVER/updates/cudapp/aien_gpu.ko" ]
check "aien_gpu loads at boot" grep -qx "aien_gpu" "$R/etc/modules-load.d/cudapp-aien-gpu.conf"
check "depmod ran for $KVER" grep -q "^depmod -a $KVER" "$R/calls.log"
check "initramfs rebuilt (NVIDIA was in it)" grep -q "^update-initramfs -u -k $KVER" "$R/calls.log"
check "nvidia.ko untouched" [ "$(cat "$R/lib/modules/$KVER/kernel/drivers/video/nvidia.ko")" = nvidia.ko ]
check "never reboots by itself" bash -c "! grep -q reboot '$R/calls.log'"
out=$(run swap back --yes 2>&1); rc=$?
check "kernel back exits 0" [ $rc -eq 0 ]
check "kernel back restores every hash (initrd, modules.dep, conf files)" [ "$before" = "$(tree_hash "$R")" ]

# 8. negative control for the back check: a backup copy damaged on disk must make back FAIL, not PASS
R=$T/r8; mkroot "$R"; run swap user --yes > /dev/null 2>&1
b=$(ls -d "$R"/var/lib/cudapp-swap/backup-*-user | head -1)
echo "damaged" > "$b/files/etc/ld.so.cache"
out=$(run swap back --yes 2>&1); rc=$?
check "back with a damaged backup exits 1" [ $rc -eq 1 ]
check "back names the file that does not match" grep -q "MISMATCH.*etc/ld.so.cache" <<<"$out"

# 9. all = user + kernel, and one back undoes both
R=$T/r9; mkroot "$R"; before=$(tree_hash "$R")
run swap all --yes > /dev/null 2>&1; rc=$?
check "all --yes exits 0" [ $rc -eq 0 ]
check "all staged the user level" [ -f "$R/etc/ld.so.conf.d/00000-cudapp-swap.conf" ]
check "all staged the kernel level" [ -f "$R/etc/modprobe.d/cudapp-swap-kernel.conf" ]
run swap back --yes > /dev/null 2>&1
check "one back undoes all" [ "$before" = "$(tree_hash "$R")" ]

# 10. unknown level is refused with usage, exit 2
out=$(run swap sideways 2>&1); rc=$?
check "unknown level exits 2" [ $rc -eq 2 ]

# 11. escalation (audit C8): sudo without -E, the sources passed as explicit absolute arguments.
#     Fake id (uid 1000) and fake sudo (records argv, runs nothing); no CUDAPP_ROOT, so this is the real-system path.
S=$T/esc; mkdir -p "$S/bin" "$S/lib"; echo "lib" > "$S/lib/libcuda.so.1"
printf '#!/bin/bash\necho 1000\n' > "$S/bin/id"
printf '#!/bin/bash\nprintf "%%s\\n" "$@" > "%s/sudo.argv"\n' "$S" > "$S/bin/sudo"
chmod +x "$S/bin"/*
env -u CUDAPP_ROOT PATH="$S/bin:$PATH" CUDAPP_LIB="$S/lib/../lib/libcuda.so.1" bash "$TOOL" swap user --yes > /dev/null 2>&1
check "escalation calls sudo" [ -s "$S/sudo.argv" ]
check "escalation drops -E (no caller environment reaches root)" bash -c "! grep -qx -- '-E' '$S/sudo.argv'"
check "escalation re-runs this script by absolute path" [ "$(sed -n 1p "$S/sudo.argv")" = "$(readlink -f "$TOOL")" ]
check "escalation passes --lib as a resolved absolute path" grep -qx -- "$S/lib/libcuda.so.1" "$S/sudo.argv"
check "user escalation does not pass --kdir or --driver-src" bash -c "! grep -qx -- --kdir '$S/sudo.argv' && ! grep -qx -- --driver-src '$S/sudo.argv'"
: > "$S/sudo.argv"
env -u CUDAPP_ROOT -u CUDAPP_DRIVER_SRC -u CUDAPP_KDIR PATH="$S/bin:$PATH" \
    bash "$TOOL" swap kernel --yes > /dev/null 2>&1
check "kernel escalation calls sudo" [ -s "$S/sudo.argv" ]
check "kernel escalation does not pass --driver-src or --kdir" bash -c "! grep -qx -- --driver-src '$S/sudo.argv' && ! grep -qx -- --kdir '$S/sudo.argv'"
: > "$S/sudo.argv"
out=$(env -u CUDAPP_ROOT -u CUDAPP_DRIVER_SRC -u CUDAPP_KDIR PATH="$S/bin:$PATH" \
    bash "$TOOL" swap kernel --yes --driver-src "$S/lib" 2>&1); rc=$?
check "caller-selected driver source is refused before sudo (exit 3)" [ $rc -eq 3 ]
check "refused driver source never reaches sudo" [ ! -s "$S/sudo.argv" ]
check "driver-source refusal names the delegated swap" grep -q "delegated kernel swap" <<<"$out"
: > "$S/sudo.argv"
out=$(env -u CUDAPP_ROOT PATH="$S/bin:$PATH" CUDAPP_DRIVER_SRC="$S/lib" \
    bash "$TOOL" swap kernel --yes 2>&1); rc=$?
check "CUDAPP_DRIVER_SRC is refused before sudo (exit 3)" [ $rc -eq 3 ]
check "refused CUDAPP_DRIVER_SRC never reaches sudo" [ ! -s "$S/sudo.argv" ]
: > "$S/sudo.argv"
out=$(env -u CUDAPP_ROOT -u CUDAPP_DRIVER_SRC PATH="$S/bin:$PATH" \
    bash "$TOOL" swap all --yes --kdir "$S" 2>&1); rc=$?
check "caller-selected kdir is refused before sudo (exit 3)" [ $rc -eq 3 ]
check "refused kdir never reaches sudo" [ ! -s "$S/sudo.argv" ]
# 12. as root on the real system (fake id prints 0): sources outside the allowed places are refused before any level runs
printf '#!/bin/bash\necho 0\n' > "$S/bin/id"
out=$(env -u CUDAPP_ROOT PATH="$S/bin:$PATH" bash "$TOOL" swap user --lib "$S/lib/libcuda.so.1" 2>&1); rc=$?
check "root: --lib outside the repository refused (exit 3)" [ $rc -eq 3 ]
check "root: refusal names the repository" grep -q "outside the CUDA++ repository" <<<"$out"
REPO=$(cd "$(dirname "$(readlink -f "$TOOL")")/.." && pwd -P)
out=$(env -u CUDAPP_ROOT PATH="$S/bin:$PATH" CUDAPP_LIB="$S/lib/libcuda.so.1" bash "$TOOL" swap all 2>&1); rc=$?
check "root: CUDAPP_LIB outside the repository refused for 'all' (exit 3)" [ $rc -eq 3 ]
out=$(env -u CUDAPP_ROOT PATH="$S/bin:$PATH" bash "$TOOL" swap kernel --kdir "$S" 2>&1); rc=$?
check "root: --kdir outside /lib/modules and /usr/src refused (exit 3)" [ $rc -eq 3 ]
check "root: refusal names /lib/modules" grep -q "not under /lib/modules" <<<"$out"
# positive control: a regular file inside the repository passes the check (dry run, nothing changed)
out=$(env -u CUDAPP_ROOT PATH="$S/bin:$PATH" bash "$TOOL" swap user --lib "$REPO/LICENSE" 2>&1); rc=$?
check "root: --lib inside the repository accepted (dry run exits 0)" [ $rc -eq 0 ]
check "root: accepted dry run changed nothing" grep -q "DRY RUN" <<<"$out"
# 13. the installed library is pinned: a copy that differs from the planned sha256 is rolled back
R=$T/r13; mkroot "$R"; before=$(tree_hash "$R")
printf '#!/bin/bash\nfor a; do d=$a; done\necho tampered > "$d"\n' > "$R/fakebin/install"; chmod +x "$R/fakebin/install"
out=$(run swap user --yes 2>&1); rc=$?
check "differing installed library fails (exit 1)" [ $rc -eq 1 ]
check "failure names the sha256 mismatch" grep -q "sha256 .* != .* planned" <<<"$out"
check "differing installed library rolled back" [ "$before" = "$(tree_hash "$R")" ]
rm "$R/fakebin/install"; out=$(run swap user --yes 2>&1)
check "installed sha256 recorded next to the backup" bash -c "grep -q \"\$(sha256sum < '$R/build/libcuda.so.1' | cut -d' ' -f1)  /usr/lib/cudapp/libcuda.so.1\" '$R'/var/lib/cudapp-swap/backup-*-user/installed.sha256"

# 14. Delegated kernel build (SUDO_UID set, fake machine root). Root must build /usr/src/aien-gpu
#     and the running kernel's build dir, never the caller's tree. Ownership in the fixture is
#     the test uid, which stands in for uid 0.
lock_tree() { # $1 = trusted file or directory; drop group/other write on it and on ancestors inside $2
    local p=$1 stop=$2
    chmod -R u+rwX,go-w "$p"
    while :; do
        chmod u+rwx,go-w "$p"
        [ "$p" = "$stop" ] && break
        p=$(dirname "$p")
    done
}
stage_make() { # record the Kbuild the module build actually consumed
    local R=$1
    printf '#!/bin/bash\necho "make $*" >> "%s/calls.log"\nfor a in "$@"; do case $a in M=*) d=${a#M=}; echo "KBUILD:$(head -1 "$d/Kbuild")" >> "%s/calls.log"; echo built > "$d/aien_gpu.ko";; esac; done\n' "$R" "$R" > "$R/fakebin/make"
    chmod +x "$R/fakebin/make"
}
delegated() { # $1 = fake root; remaining args are the tool's
    local R=$1; shift
    env -u CUDAPP_DRIVER_SRC -u CUDAPP_KDIR PATH="$R/fakebin:$PATH" CUDAPP_ROOT="$R" \
        CUDAPP_LIB="$R/build/libcuda.so.1" SUDO_UID=1000 SUDO_USER=fixture \
        bash "$TOOL" "$@"
}
trusted_src() { # $1 = fake root. A locked tree whose Kbuild is not the attacker's.
    local R=$1
    mkdir -p "$R/usr/src/aien-gpu" "$R/attacker"
    printf 'trusted-marker\nobj-m := aien_gpu.o\n' > "$R/usr/src/aien-gpu/Kbuild"
    echo '/* trusted */' > "$R/usr/src/aien-gpu/aien_gpu.c"
    printf 'attacker-marker\nobj-m := aien_gpu.o\n' > "$R/attacker/Kbuild"
    echo '/* attacker */' > "$R/attacker/aien_gpu.c"
    lock_tree "$R/usr/src/aien-gpu" "$R"
}

R=$T/r14a; mkroot "$R"; trusted_src "$R"; stage_make "$R"
out=$(delegated "$R" swap kernel --yes --driver-src "$R/attacker" 2>&1); rc=$?
check "delegated kernel swap refuses --driver-src (exit 3)" [ $rc -eq 3 ]
check "delegated --driver-src refusal names the delegated swap" grep -q "delegated kernel swap" <<<"$out"
check "delegated --driver-src runs no make" bash -c "! grep -q '^make ' '$R/calls.log' 2>/dev/null"
check "delegated --driver-src does not install a module" [ ! -e "$R/lib/modules/$KVER/updates/cudapp/aien_gpu.ko" ]

R=$T/r14b; mkroot "$R"; trusted_src "$R"; stage_make "$R"
out=$(PATH="$R/fakebin:$PATH" CUDAPP_ROOT="$R" CUDAPP_LIB="$R/build/libcuda.so.1" \
    CUDAPP_DRIVER_SRC="$R/attacker" SUDO_UID=1000 bash "$TOOL" swap kernel --yes 2>&1); rc=$?
check "delegated kernel swap refuses CUDAPP_DRIVER_SRC (exit 3)" [ $rc -eq 3 ]
check "delegated CUDAPP_DRIVER_SRC refusal names the delegated swap" grep -q "delegated kernel swap" <<<"$out"
check "delegated CUDAPP_DRIVER_SRC runs no make" bash -c "! grep -q '^make ' '$R/calls.log' 2>/dev/null"

R=$T/r14c; mkroot "$R"; trusted_src "$R"; stage_make "$R"
out=$(delegated "$R" swap kernel --yes --kdir "$R/attacker" 2>&1); rc=$?
check "delegated kernel swap refuses --kdir (exit 3)" [ $rc -eq 3 ]
check "delegated --kdir refusal names the delegated swap" grep -q "delegated kernel swap" <<<"$out"
check "delegated --kdir runs no make" bash -c "! grep -q '^make ' '$R/calls.log' 2>/dev/null"

R=$T/r14d; mkroot "$R"; trusted_src "$R"; stage_make "$R"
chmod o+w "$R/usr/src/aien-gpu/Kbuild"
out=$(delegated "$R" swap kernel --yes 2>&1); rc=$?
check "delegated kernel swap refuses a group-or-other-writable source file (exit 3)" [ $rc -eq 3 ]
check "writable source refusal names the file mode" grep -q "writable" <<<"$out"
check "writable source file runs no make" bash -c "! grep -q '^make ' '$R/calls.log' 2>/dev/null"

R=$T/r14e; mkroot "$R"; trusted_src "$R"; stage_make "$R"
rm "$R/usr/src/aien-gpu/aien_gpu.c"
ln -s "$R/attacker/aien_gpu.c" "$R/usr/src/aien-gpu/aien_gpu.c"
out=$(delegated "$R" swap kernel --yes 2>&1); rc=$?
check "delegated kernel swap refuses a symlink in the source (exit 3)" [ $rc -eq 3 ]
check "source symlink refusal names the symlink" grep -q "symlink" <<<"$out"
check "source symlink runs no make" bash -c "! grep -q '^make ' '$R/calls.log' 2>/dev/null"

R=$T/r14f; mkroot "$R"; trusted_src "$R"; stage_make "$R"
rm -rf "$R/usr/src/aien-gpu"
ln -s "$R/attacker" "$R/usr/src/aien-gpu"
out=$(delegated "$R" swap kernel --yes 2>&1); rc=$?
check "delegated kernel swap refuses a symlinked source directory (exit 3)" [ $rc -eq 3 ]
check "symlinked source directory refusal names the symlink" grep -q "symlink" <<<"$out"

R=$T/r14g; mkroot "$R"; trusted_src "$R"; stage_make "$R"
chmod o+w "$R/usr/src"
out=$(delegated "$R" swap kernel --yes 2>&1); rc=$?
check "delegated kernel swap refuses a writable ancestor (exit 3)" [ $rc -eq 3 ]
check "writable ancestor refusal names the mode" grep -q "writable" <<<"$out"
check "writable ancestor runs no make" bash -c "! grep -q '^make ' '$R/calls.log' 2>/dev/null"

R=$T/r14h; mkroot "$R"; trusted_src "$R"; stage_make "$R"
rm -rf "$R/lib/modules/$KVER/build"
mkdir -p "$R/attacker-kdir"
chmod 777 "$R/attacker-kdir"
echo 'attacker-makefile' > "$R/attacker-kdir/Makefile"
ln -s "$R/attacker-kdir" "$R/lib/modules/$KVER/build"
out=$(delegated "$R" swap kernel --yes 2>&1); rc=$?
check "delegated kernel swap refuses a kdir symlink to a writable tree (exit 3)" [ $rc -eq 3 ]
check "writable kdir refusal names the kdir" grep -Eq "writable|symlink" <<<"$out"
check "writable kdir symlink runs no make" bash -c "! grep -q '^make ' '$R/calls.log' 2>/dev/null"

R=$T/r14i; mkroot "$R"; trusted_src "$R"; stage_make "$R"
rm -rf "$R/lib/modules/$KVER/build"
mkdir -p "$R/usr/src/linux-headers"
echo 'trusted-makefile' > "$R/usr/src/linux-headers/Makefile"
lock_tree "$R/usr/src/linux-headers" "$R"
ln -s "$R/usr/src/linux-headers" "$R/lib/modules/$KVER/build"
# The symlink itself is mode 777 on Linux; its owner is the fixture uid. Parent stays locked.
chown -h "$(id -u):$(id -g)" "$R/lib/modules/$KVER/build" 2>/dev/null || true
out=$(delegated "$R" swap kernel --yes 2>&1); rc=$?
check "delegated kernel swap accepts a locked kdir symlink (exit 0)" [ $rc -eq 0 ]
check "delegated kdir symlink build consumed the trusted module source" grep -qx "KBUILD:trusted-marker" "$R/calls.log"
check "delegated kdir symlink build did not consume the attacker source" bash -c "! grep -q 'KBUILD:attacker-marker' '$R/calls.log'"

R=$T/r14j; mkroot "$R"; trusted_src "$R"; stage_make "$R"
out=$(delegated "$R" swap kernel --yes 2>&1); rc=$?
check "delegated kernel swap accepts a locked /usr/src/aien-gpu (exit 0)" [ $rc -eq 0 ]
check "delegated build consumed the trusted Kbuild" grep -qx "KBUILD:trusted-marker" "$R/calls.log"
check "delegated build did not consume the attacker Kbuild" bash -c "! grep -q 'KBUILD:attacker-marker' '$R/calls.log'"
check "delegated build installed aien_gpu.ko" [ -f "$R/lib/modules/$KVER/updates/cudapp/aien_gpu.ko" ]

# Ubuntu's /lib is a symlink to usr/lib. The kernel build dir is reached through that link.
R=$T/r14k; mkroot "$R"; trusted_src "$R"; stage_make "$R"
rm -rf "$R/lib"
mkdir -p "$R/usr/lib/modules/$KVER" "$R/usr/src/linux-headers"
echo 'trusted-makefile' > "$R/usr/src/linux-headers/Makefile"
ln -s usr/lib "$R/lib"
ln -s "$R/usr/src/linux-headers" "$R/usr/lib/modules/$KVER/build"
lock_tree "$R/usr/lib/modules/$KVER" "$R"
lock_tree "$R/usr/src/linux-headers" "$R"
out=$(delegated "$R" swap kernel --yes 2>&1); rc=$?
check "delegated kernel swap follows a /lib -> usr/lib symlink (exit 0)" [ $rc -eq 0 ]
check "usr-merge build consumed the trusted module source" grep -qx "KBUILD:trusted-marker" "$R/calls.log"
check "usr-merge build did not consume the attacker source" bash -c "! grep -q 'KBUILD:attacker-marker' '$R/calls.log'"
check "usr-merge build installed aien_gpu.ko through the /lib symlink" [ -f "$R/usr/lib/modules/$KVER/updates/cudapp/aien_gpu.ko" ]

R=$T/r14l; mkroot "$R"; trusted_src "$R"; stage_make "$R"
rm -rf "$R/lib"
mkdir -p "$T/escape/modules/$KVER/build"
echo 'attacker-makefile' > "$T/escape/modules/$KVER/build/Makefile"
ln -s "$T/escape" "$R/lib"
out=$(delegated "$R" swap kernel --yes 2>&1); rc=$?
check "delegated kernel swap refuses a /lib symlink that leaves the machine root (exit 3)" [ $rc -eq 3 ]
check "escaping /lib symlink names the machine root" grep -q "outside the machine root" <<<"$out"
check "escaping /lib symlink runs no make" bash -c "! grep -q '^make ' '$R/calls.log' 2>/dev/null"
check "escaping /lib symlink did not consume the attacker makefile" bash -c "! grep -q 'attacker-makefile' '$R/calls.log' 2>/dev/null"

# A real delegated root (sudo sets SUDO_UID) must accept this machine's compiler path and still
# refuse before make, because /usr/src/aien-gpu is not installed here. Nothing is written.
if sudo -n true 2>/dev/null; then
    out=$(sudo -n env -u CUDAPP_ROOT -u CUDAPP_DRIVER_SRC -u CUDAPP_KDIR -u CUDAPP_LIB bash "$TOOL" swap kernel --yes 2>&1); rc=$?
    check "delegated root refuses before any build on this machine (exit 3)" [ $rc -eq 3 ]
    check "delegated root reached the trusted source check" grep -q "trusted aien_gpu source not found" <<<"$out"
    check "delegated root did not report a compiler-path failure" bash -c "! grep -Eq 'make PATH|not on the trusted PATH|does not resolve' <<<"$out""
fi

echo "swap cli: $PASS passed, $FAIL failed"
[ $FAIL -eq 0 ]
