#!/bin/sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# live.sh: read-only live tier for cudapp-smi. Builds cudapp-smi with the aien-dev/omega-mv#1048
# candidate omc (native/smi/util.om needs its devcall bound), runs it WITHOUT --rm against the real
# /proc and /sys, runs nvidia-smi read-only queries, and compares field by field.
# Usage: sh tests/smi/live.sh OMC OUTDIR
# Writes OUTDIR/{cudapp-smi.txt,nvidia-smi-gpu.csv,nvidia-smi-apps.csv,nvidia-smi-q.txt,compare.tsv}.
# The resource-manager path (--rm) opens /dev/nvidiactl and /dev/nvidia0 and is never run here.
# Last line: CUDAPP-SMI LIVE PASS (<n> fields) or CUDAPP-SMI LIVE FAIL (<n>).
set -u
omc=${1:?usage: live.sh OMC OUTDIR}
o=${2:?usage: live.sh OMC OUTDIR}
here=$(cd "$(dirname "$0")/../.." && pwd)
pin=390629fd22590aaefca852045f8b0ace86c62cefb3b1bc203ce400773c8abc2f
got=$(sha256sum "$omc") || exit 74
if [ "${got%% *}" != "$pin" ]; then echo "REFUSED: omc digest ${got%% *} is not the omega-mv#1048 candidate (e9b63587)" >&2; exit 69; fi
mkdir -p "$o" || exit 3
bin=$(mktemp -d)/cudapp-smi
cat "$here/native/smi/smi.om" "$here/native/nvrm/context.om" "$here/native/smi/util.om" "$here/native/smi/rm.om" | "$omc" > "$bin" || {
  echo 'CUDAPP-SMI LIVE FAIL (build)'; exit 1; }
chmod +x "$bin"
"$bin" > "$o/cudapp-smi.txt"; rc=$?
q='driver_version,count,pci.bus_id,pci.domain,pci.bus,pci.device,uuid,vbios_version,pci.device_id,pci.sub_device_id,pci.baseClass,pci.subClass,pcie.link.gen.current,pcie.link.gen.max,pcie.link.width.current,pcie.link.width.max,name,temperature.gpu,pstate,utilization.gpu,utilization.memory'
nvidia-smi --query-gpu="$q" --format=csv,noheader > "$o/nvidia-smi-gpu.csv" || { echo 'CUDAPP-SMI LIVE FAIL (nvidia-smi)'; exit 1; }
nvidia-smi --query-compute-apps=pid,process_name,used_memory --format=csv,noheader > "$o/nvidia-smi-apps.csv" || exit 1
nvidia-smi -q > "$o/nvidia-smi-q.txt" || exit 1
if [ "$rc" -ne 0 ]; then echo "CUDAPP-SMI LIVE FAIL (cudapp-smi rc=$rc)"; exit 1; fi
if [ "$(wc -l < "$o/nvidia-smi-gpu.csv")" -ne 1 ]; then echo 'CUDAPP-SMI LIVE FAIL (expects one GPU)'; exit 1; fi

ours() { sed -n "s/^$1=//p" "$o/cudapp-smi.txt"; }
theirs() { awk -F', ' -v i="$1" '{print $i}' "$o/nvidia-smi-gpu.csv"; }
qline() { awk -F': ' -v k="$1" 'index($1, k) && !s {sub(/[ \t]+$/, "", $2); print $2; s=1}' "$o/nvidia-smi-q.txt"; }
bad=0; n=0
printf 'field\tcudapp-smi\tnvidia-smi\tverdict\n' > "$o/compare.tsv"
row() { # field ours theirs verdict
  printf '%s\t%s\t%s\t%s\n' "$1" "$2" "$3" "$4" >> "$o/compare.tsv"
  case $4 in MATCH) n=$((n + 1)) ;; DIFF) bad=$((bad + 1)) ;; esac
}
same() { if [ "$2" = "$3" ]; then row "$1" "$2" "$3" MATCH; else row "$1" "$2" "$3" DIFF; fi; }
num() { if [ "$(printf '%d' "$2")" = "$(printf '%d' "$3")" ]; then row "$1" "$2" "$3" MATCH; else row "$1" "$2" "$3" DIFF; fi; }

same driver_version "$(ours driver_version)" "$(theirs 1)"
same count "$(ours count)" "$(theirs 2)"
i=3
for k in pci.bus_id pci.domain pci.bus pci.device uuid vbios_version pci.device_id; do
  same "$k" "$(ours "gpu.0.$k")" "$(theirs $i)"; i=$((i + 1))
done
num pci.sub_device_id "$(ours gpu.0.pci.sub_device_id)" "$(theirs 10)"
same pci.baseClass "$(ours gpu.0.pci.base_classcode)" "$(theirs 11)"
same pci.subClass "$(ours gpu.0.pci.sub_classcode)" "$(theirs 12)"
i=13
for k in pcie.link.gen.current pcie.link.gen.max pcie.link.width.current pcie.link.width.max; do
  same "$k" "$(ours "gpu.0.$k")" "$(theirs $i)"; i=$((i + 1))
done
same minor_number "$(ours gpu.0.minor_number)" "$(qline 'Minor Number')"
same gsp_firmware_version "$(ours gpu.0.gsp_firmware_version)" "$(qline 'GSP Firmware Version')"
# Without --rm these are [N/A] by design; the RM values are NOT_RUN on the live device.
row name "$(ours gpu.0.name)" "$(theirs 17)" RM_NOT_RUN
row temperature.gpu "$(ours gpu.0.temperature.gpu)" "$(theirs 18)" RM_NOT_RUN
row pstate "$(ours gpu.0.pstate)" "$(theirs 19)" RM_NOT_RUN
row utilization.gpu "$(ours gpu.0.utilization.gpu)" "$(theirs 20)" RM_NOT_RUN
row utilization.memory "$(ours gpu.0.utilization.memory)" "$(theirs 21)" RM_NOT_RUN
# Holders are every process with an open /dev/nvidia* node: a superset of nvidia-smi compute apps.
while IFS=', ' read -r pid pname mem; do
  [ -n "$pid" ] || continue
  k=$(grep -x "gpu\.0\.holder\.[0-9]*\.pid=$pid" "$o/cudapp-smi.txt" | sed 's/^gpu\.0\.holder\.\([0-9]*\)\.pid=.*/\1/')
  if [ -z "$k" ]; then row "compute_app.pid=$pid" '' "$pid" DIFF; continue; fi
  row "compute_app.pid=$pid" "holder.$k" "$pid" MATCH
  same "compute_app.$pid.process_name" "$(ours "gpu.0.holder.$k.process_name")" "$pname"
  row "compute_app.$pid.used_memory" '[rm]' "$mem" RM_NOT_RUN
done < "$o/nvidia-smi-apps.csv"
sed -i "s#$HOME#~#g" "$o/cudapp-smi.txt" "$o/nvidia-smi-apps.csv" "$o/nvidia-smi-q.txt" "$o/compare.tsv"
# Comparator negative control: an altered uuid and an altered sub-device id must both be DIFF.
u="$(ours gpu.0.uuid)x"; tu=$(theirs 7); ts=$(theirs 10)
nc=$(nb=$bad; o=$(mktemp -d); same uuid "$u" "$tu"; num pci.sub_device_id 0x10DF "$ts"; echo $((bad - nb)))
if [ "$nc" -ne 2 ]; then echo "CUDAPP-SMI LIVE FAIL (comparator negative control $nc/2)"; exit 1; fi
echo 'PASS comparator negative control (2/2 altered fields rejected)'
cat "$o/compare.tsv"
if [ "$bad" -eq 0 ]; then echo "CUDAPP-SMI LIVE PASS ($n fields)"; else echo "CUDAPP-SMI LIVE FAIL ($bad)"; exit 1; fi
