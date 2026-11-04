# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# External verification of the Omega source, never a runtime dependency.
import pathlib
import subprocess
import sys
import tempfile

here = pathlib.Path(__file__).resolve().parent
source = (here / 'envelope.om').read_text()
driver = (here / 'envelope-driver.om').read_text()
mutants = {
    'omit_tail': ('for i ulong 5120', 'for i ulong 5119'),
    'round_down': ('div add sig 1 2', 'div sig 2'),
    'accept_nonfinite': ('if ge v 2139095040\nret 0', 'if ge v 2139095040\nret 1065353216'),
    'omit_margin': ('mul sig 73800', 'mul sig 73728'),
    'keep_sign_bit': ('and ulong get bits i 2147483647', 'ulong get bits i'),
}
with tempfile.TemporaryDirectory(prefix='q38-envelope-') as tmp:
    for name, (old, new) in mutants.items():
        assert source.count(old) == 1
        binary = pathlib.Path(tmp) / name
        result = subprocess.run([sys.argv[1]], input=(source.replace(old, new) + driver).encode(),
                                capture_output=True, check=True)
        binary.write_bytes(result.stdout)
        binary.chmod(0o700)
        result = subprocess.run([sys.executable, str(here / 'envelope-check.py'), str(binary)],
                                capture_output=True)
        assert result.returncode != 0, f'{name}: survived'
        assert b'AssertionError' in result.stderr, (name, result.stderr)
        print(f'{name}: caught by numerical oracle')
