date -u +%FT%TZ; uname -a; id -u; cat /opt/rocm/.info/version
rocminfo 2>/dev/null | grep -m3 -E 'Marketing Name|Name: +gfx'
python3 -c 'import torch,transformers,safetensors;print("torch",torch.__version__,"transformers",transformers.__version__,"safetensors",safetensors.__version__)'
cat /sys/class/kfd/kfd/version 2>&1; ls -l /dev/kfd /dev/dri
for n in /sys/class/kfd/kfd/topology/nodes/*; do echo "node $n gpu_id $(cat $n/gpu_id)"; grep -E 'gfx_target_version|device_id|vendor_id|simd_count|drm_render_minor|local_mem_size|fw_version' $n/properties; done
df -h /tmp /workspace; nproc; free -g | head -2; ls ~/.cache/huggingface/hub 2>&1; which git qemu-aarch64 qemu-aarch64-static
sha256sum launch-x86 vadd.hsaco
echo "--- small example"; date -u +%FT%TZ
./launch-x86 vadd vadd.hsaco vadd 10000000000; echo "VADD_RC=$?"
echo ENV_DONE
