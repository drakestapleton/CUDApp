# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Verification only. All CUDA/session transports are CPU stubs or WsSim.
Run inside the documented device-empty bwrap, never against a real driver.
--base optionally compares the original source at that exact Git revision;
otherwise the explicit default-OFF C migration oracle supplies the baseline.
"""
import argparse, os, resource, struct, subprocess, tempfile
from pathlib import Path

root=Path(__file__).resolve().parents[2]
lane=root/'om/standalone-oracles'
p=argparse.ArgumentParser()
p.add_argument('library',type=Path)
p.add_argument('--frame',type=Path,required=True)
p.add_argument('--base')
args=p.parse_args()
assert not list(Path('/dev').glob('nvidia*')), 'run inside the documented device-empty bwrap'
resource.setrlimit(resource.RLIMIT_CORE,(0,0))
cc=os.environ.get('CC','cc')
flags=['-O2','-std=gnu11','-Wall','-Wextra','-Werror','-Wno-unused-function','-ffp-contract=off','-I'+str(root/'src'),'-I'+str(root/'tests'),'-I'+os.environ.get('CUDA_INC','/usr/local/cuda/include')]
lib=str(args.library.resolve())
frame=str(args.frame.resolve())
attn=['omega_gpu_session_lock','omega_gpu_session_unlock','omega_gpu_session_open','omega_gpu_session_on_close','omega_gpu_decode_attn256_cache_view','omega_gpu_session_sync_to_gpu','omega_gpu_session_sync_to_cpu','omega_gpu_session_free','omega_gpu_pf_alloc_dev_locked','omega_gpu_pf_copy_va_locked','omega_gpu_session_launch']
norm=['omega_blackwell_codegen_dec_prog','omega_gpu_session_lock','omega_gpu_session_unlock','omega_gpu_session_open','omega_gpu_session_alloc_code','omega_gpu_session_alloc','omega_gpu_session_sync_to_gpu','omega_gpu_session_sync_to_cpu','omega_gpu_session_free','omega_gpu_session_launch']
checks=[
 ('vadd','compat/cuda/fixture/vadd_drv.c',[],['host-cuda.c'],None),
 ('probe','tools/interop/cubin_probe.c',[],['host-cuda.c'],None),
 ('dispatch','om/attention-mirror/dispatch-check.c',attn,[],3),
 ('cache','om/prefill-shape/cache-check.c',['omega_gpu_session_alloc_code'],[],14),
 ('scoped','om/prefill-shape/scoped-check.c',['omega_gpu_pf_open_locked','omega_gpu_pf_kernel_locked'],[],3),
 ('budget','om/q38-head/budget-check.c',[],[],3),
 ('bounds','om/q38-head/check.c',[],[],4),
 ('norm','om/q38-head/norm-check.c',norm,['host-norm.c'],None),
]

def invoke(cmd,env=None):
    return subprocess.run(cmd,env=env,text=True,capture_output=True)

with tempfile.TemporaryDirectory(prefix='standalone-family-') as directory:
    tmp=Path(directory)
    (tmp/'src').symlink_to(root/'src',target_is_directory=True)
    (tmp/'weights.f32').write_bytes(struct.pack('=f',1.0)*5120)
    (tmp/'module.txt').write_text('Host-only CPU transport input, never executable GPU code.\n')
    missing=invoke([cc,*flags,str(lane/'scalar-check.c'),str(lane/'oracle-c.c'),'-o',str(tmp/'missing-unit')])
    assert missing.returncode!=0 and 'omega_so_oracles' in missing.stderr,('strong-link negative control',missing.stderr)
    print('PASS missing Omega unit: link refused with undefined omega_so_oracles',flush=True)
    baseline='original@'+args.base if args.base else 'C-snapshot'
    print('BASELINE '+baseline,flush=True)
    for name,path,wraps,extras,redop in checks:
        original=tmp/path
        original.parent.mkdir(parents=True,exist_ok=True)
        if args.base:
            old=subprocess.run(['git','show',args.base+':'+path],cwd=root,capture_output=True,check=True)
            original.write_bytes(old.stdout)
        else:
            original=root/path
        runargs={'vadd':[str(root/'compat/cuda/fixture/vadd_u32.ptx')],
                 'probe':[str(tmp/'module.txt'),'host_gemm','17','7','19'],
                 'norm':['--chip',str(tmp/'weights.f32'),'100'],
                 'bounds':[frame]}.get(name,[])
        env=dict(os.environ,SPARK_WINDOW='HOST-STUB-NOT-A-GPU-WINDOW',SO_HOST_M='17',SO_HOST_K='7',SO_HOST_N='19')
        env.pop('SO_HOST_RED',None)
        env.pop('SO_RED_OP',None)
        outputs=[]
        for variant in ('before','after','c-oracle'):
            exe=tmp/(name+'-'+variant)
            source=original if variant=='before' else root/path
            defines=[] if args.base and variant=='before' else (['-DCUDAPP_MIGRATION_PENDING_C_STANDALONE_ORACLES=1'] if variant!='after' else [])
            wrapopts=['-Wl,--wrap='+f for f in wraps]
            sources=[str(source)]+[str(lane/e) for e in extras]
            if variant=='after':
                sources+=[str(lane/'red.c')]
                wrapopts+=['-Wl,--wrap=omega_so_oracles']
            command=[cc,*flags,*defines,*sources,lib,*wrapopts,'-lpthread','-lm','-o',str(exe)]
            subprocess.run(command,check=True)
            result=invoke([str(exe),*runargs],env)
            assert result.returncode==0,(name,variant,result.returncode,result.stdout,result.stderr)
            outputs.append((result.stdout,result.stderr))
            print(name+' '+variant+' rc=0\n'+result.stdout+result.stderr,flush=True)
            if variant=='after':
                poisoned=dict(env,**({'SO_HOST_RED':'1'} if redop is None else {'SO_RED_OP':str(redop)}))
                red=invoke([str(exe),*runargs],poisoned)
                assert red.returncode!=0,(name,'red control escaped',red.stdout,red.stderr)
                print(name+' RED rc='+str(red.returncode)+' (corrupted '+('transport result' if redop is None else 'Omega verdict op '+str(redop))+')',flush=True)
        assert outputs[0]==outputs[1]==outputs[2],(name,'before/after/C-oracle output drift',outputs)
        print('PASS '+name+' '+baseline+'/default/C-oracle outputs byte-identical; red rejected',flush=True)
print('PASS standalone family: eight host checks, identical before/after verdicts, eight red controls',flush=True)
