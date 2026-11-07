#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Fetch the pinned host tools for the portable-application reproduction (docs/repro/README.md step 2)
# without root: three .deb files by fixed URL, each checked against its sha256 before use, then
# unpacked into DIR/root. Nothing is installed system-wide; removing DIR undoes this step.
#   spirv-tools 2025.4-1~ppa2 (spirv-val, the independent SPIR-V validator)  Apache-2.0
#   libvulkan-dev 1.4.328.1-1~1 (vulkan/vulkan.h headers for the launcher)   Apache-2.0
#   opencl-c-headers 3.0~2023.12.14-1 (CL/ headers for the launcher)         Apache-2.0
# The first two come from Canonical's public Launchpad PPA canonical-nvidia/vulkan-packages-nv-desktop
# (the source DGX OS configures), the third from Ubuntu noble.
#
# usage: fetch-tools.sh /absolute/DIR
# Exit: 0 FETCH PASS; 1 a digest differs (the file is kept as DIR/debs/*.rejected); 64 usage;
# 74 download or unpack failure; 77 NOT_RUN (not Linux AArch64).
set -u
LC_ALL=C
export LC_ALL
if [ "$(uname -s)" != Linux ] || [ "$(uname -m)" != aarch64 ]; then
  echo 'NOT_RUN: the pinned tools are arm64 packages; the producer needs Linux AArch64' >&2; exit 77
fi
[ "$#" -eq 1 ] || { echo 'usage: fetch-tools.sh /absolute/DIR' >&2; exit 64; }
D=$1
case $D in /*) ;; *) echo 'usage: DIR must be absolute' >&2; exit 64 ;; esac
mkdir -p "$D/debs" "$D/root" || exit 74
PPA=https://snapshot.ppa.launchpadcontent.net/canonical-nvidia/vulkan-packages-nv-desktop/ubuntu/pool/main
UB=http://ports.ubuntu.com/ubuntu-ports/pool/universe
rc=0
while read -r sum name url; do
  f=$D/debs/$name
  if [ ! -f "$f" ]; then
    curl -fsSL --retry 2 -o "$f.part" "$url" && mv "$f.part" "$f" || { echo "FETCH FAIL download $name" >&2; exit 74; }
  fi
  got=$(sha256sum < "$f"); got=${got%% *}
  if [ "$got" != "$sum" ]; then
    echo "FETCH FAIL $name sha256=$got want=$sum" >&2; mv "$f" "$f.rejected"; rc=1; continue
  fi
  echo "FETCH OK $name sha256=$got"
  if command -v dpkg-deb > /dev/null 2>&1; then
    dpkg-deb -x "$f" "$D/root" || exit 74
  else
    # no dpkg-deb: a .deb is an ar archive holding data.tar.*
    (cd "$D/debs" && ar x "$name" && tar -xf data.tar.* -C "$D/root" && rm -f data.tar.* control.tar.* debian-binary) || exit 74
  fi
done <<EOF
45defd68f509fce4853e371fea3e58702316b2922807b5e8b14570a046fe02f8 spirv-tools_2025.4-1~ppa2_arm64.deb $PPA/s/spirv-tools/spirv-tools_2025.4-1%7eppa2_arm64.deb
ad1170ca01e61f66d2d29cc705e08232097aa6ec9040ea84a3f29461b472431f libvulkan-dev_1.4.328.1-1~1_arm64.deb $PPA/v/vulkan-loader/libvulkan-dev_1.4.328.1-1%7e1_arm64.deb
38f4696aa506640cb6d1dfd343f7ec15e4a7d26229d6e262351546b3e0f2f27b opencl-c-headers_3.0~2023.12.14-1_all.deb $UB/k/khronos-opencl-headers/opencl-c-headers_3.0%7e2023.12.14-1_all.deb
EOF
[ "$rc" -eq 0 ] || exit 1
[ -x "$D/root/usr/bin/spirv-val" ] && [ -f "$D/root/usr/include/vulkan/vulkan.h" ] && [ -f "$D/root/usr/include/CL/cl.h" ] \
  || { echo 'FETCH FAIL unpacked tree lacks spirv-val, vulkan/vulkan.h or CL/cl.h' >&2; exit 74; }
echo "FETCH PASS root=$D/root"
