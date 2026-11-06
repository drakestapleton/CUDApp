# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
import os, pathlib, subprocess, tempfile
root=pathlib.Path(__file__).resolve().parents[2]
source=(root/'om/graph/headscale.om').read_text()
cc=os.environ.get('CC','cc')
bridge=pathlib.Path(os.environ['OMEGA_BRIDGE'])
env=dict(os.environ,OMEGA_SEEDED_OMC=os.environ['OMEGA_OMC'])
test=root/'tests/test_graph_headscale.c'
mutants=[('identity','528482304','1065353216'),('zero','528482304','0'),('negative','528482304','2675965952'),('half','528482304','520093696'),('skip','len x','0'),('add','mul get x i','add get x i')]
with tempfile.TemporaryDirectory() as td:
    d=pathlib.Path(td)
    subprocess.run([cc,'-O2','-ffp-contract=off','-I'+str(root/'src'),str(test),str(root/'src/cudapp_om_graph_headscale.c'),'-o',str(d/'green')],check=True)
    subprocess.run([str(d/'green')],check=True)
    p=subprocess.run([cc,'-I'+str(root/'src'),str(test),'-o',str(d/'missing')],capture_output=True)
    if p.returncode==0: raise SystemExit('FAIL strong link negative control')
    print('PASS missing binding refused')
    for name,old,new in mutants:
        assert source.count(old)==1
        om=d/(name+'.om'); om.write_text(source.replace(old,new))
        with (d/'mut.c').open('w') as out:
            subprocess.run(['sh',str(bridge/'tools/bridge-seeded'),'--runtime-views','c',str(om)],env=env,stdout=out,check=True)
        subprocess.run([cc,'-O2','-ffp-contract=off','-I'+str(root/'src'),str(test),str(d/'mut.c'),'-o',str(d/'mut')],check=True)
        p=subprocess.run([str(d/'mut')],capture_output=True)
        if p.returncode==0: raise SystemExit('FAIL survived '+name)
        print('KILLED '+name)
print('PASS graph head scaling red control and six mutants')
