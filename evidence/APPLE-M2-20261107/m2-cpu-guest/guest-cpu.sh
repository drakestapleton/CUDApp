#!/bin/sh
# apple195: run CUDApp tests/multitarget/run.sh and the shared own-model job inside the Lima omega-llm
# guest (Linux AArch64 on the Apple M2 CPU via Virtualization.framework). Runs on the Mac. Read-only use
# of the VM: everything stays in the guest's /tmp/apple195 (tmpfs) and is removed afterwards.
set -u
export PATH=/opt/homebrew/bin:/usr/bin:/bin:/usr/sbin:/sbin
S=$HOME/work/hive/apple195/stage
VM=omega-llm
limactl shell $VM -- sh -c 'rm -rf /tmp/apple195 && mkdir -p /tmp/apple195' || exit 1
tar cf - -C "$S" guest model | limactl shell $VM -- tar xf - -C /tmp/apple195 || exit 1
limactl shell $VM -- sh -c '
set -u
G=/tmp/apple195/guest
cd $G || exit 1
chmod 755 omc spirv/bin/spirv-val
{
  echo "DATE $(date -u +%Y-%m-%dT%H:%M:%SZ)"
  echo "GUEST $(uname -srm); $(. /etc/os-release; echo $PRETTY_NAME)"
  echo "GUEST_CPUS $(nproc) MIDR_EL1 $(cat /sys/devices/system/cpu/cpu0/regs/identification/midr_el1 2>/dev/null) (implementer 0x61 = Apple)"
  echo "GUEST_FEATURES $(grep -m1 ^Features /proc/cpuinfo)"
  echo "VIRT $(cat /sys/class/dmi/id/sys_vendor 2>/dev/null) / $(cat /sys/class/dmi/id/product_name 2>/dev/null)"
  echo "OMC $(sha256sum omc)"
} > identity.txt
cat identity.txt
export SPIRV_VAL=$G/spirv/bin/spirv-val TMPDIR=/tmp/apple195 MT_OUT=/tmp/apple195/run CUDAPP_MIGRATION_PENDING_SPIRV_HOST=0
nice -n 10 sh CUDApp/tests/multitarget/run.sh $G/omc > gate.log 2>&1
echo "EXIT $?" >> gate.log
tail -2 gate.log
M=/tmp/apple195/model
cd $M && sha256sum -c SHA256SUMS > inputs.check 2>&1; echo "model inputs: $(grep -c ": OK" inputs.check)/8 OK"
chmod 755 ollm ollm-mut-b
nice -n 10 sh model-job.sh $M 1 > model-guest.txt 2>&1
echo "MODEL rc=$?"
'
mkdir -p "$S/guest-out"
limactl shell $VM -- sh -c 'cd /tmp/apple195 && tar cf - guest/identity.txt guest/gate.log run/vadd.ref run/fsum16.ref run/gemm16.ref run/doctor.txt run/*.spv model/model-guest.txt model/job8.out model/files.txt model/extra.out model/inputs.check' | tar xf - -C "$S/guest-out"
limactl shell $VM -- sh -c 'rm -rf /tmp/apple195; ls /tmp/apple195 2>&1 | head -1'
