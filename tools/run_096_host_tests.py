#!/usr/bin/env python3
"""Run the 0.9.6 host gates; these do not replace target builds or hardware tests."""
from pathlib import Path
import os
import shlex
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
os.chdir(ROOT)
def run(args):
    print('+', shlex.join(map(str,args)), flush=True)
    subprocess.run(args, check=True)

run([sys.executable, 'tools/check_096_static.py'])
run([sys.executable, 'tests/portal_routes.test.py'])
run(['node', 'tests/web_regression.cjs'])
# Reuse every existing host compile/run block, rather than maintaining a second list.
# Only literal run blocks from this checked-in workflow are executed.
workflow = (ROOT / '.github/workflows/open-access.yml').read_text().splitlines()
blocks=[]
for i,line in enumerate(workflow):
    if line.strip() == 'run: |':
        body=[]
        for following in workflow[i+1:]:
            if following.strip() and not following.startswith('          '): break
            body.append(following[10:])
        blocks.append('\n'.join(body))
assert blocks, 'No existing regression commands found'
for block in blocks:
    run(['bash', '-euo', 'pipefail', '-c', block])
with tempfile.TemporaryDirectory(prefix='dar096-') as tmp:
    profile_binary=Path(tmp)/'dar-radio-policy'
    run(['g++','-std=c++11','-Wall','-Wextra','-Werror',
         '-fsanitize=address,undefined','tests/dar_radio_policy.test.cpp','-o',str(profile_binary)])
    run([str(profile_binary)])
    window_binary=Path(tmp)/'tx-window'
    run(['g++','-std=c++11','-Wall','-Wextra','-Werror',
         '-fsanitize=address,undefined','-Isrc','tests/tx_window.test.cpp','-o',str(window_binary)])
    run([str(window_binary)])
    store_binary=Path(tmp)/'tx-window-store'
    run(['g++','-std=c++11','-Wall','-Wextra','-Werror',
         '-fsanitize=address,undefined','-Itests/tx_window_fakes','-Isrc',
         'tests/tx_window_store.test.cpp','-o',str(store_binary)])
    run([str(store_binary)])
    binary=Path(tmp)/'dispatcher'
    run(['g++','-std=c++11','-Wall','-Wextra','-Wno-unused-parameter',
         '-Wno-reorder','-Wno-sign-compare','-fsanitize=address,undefined',
         '-Isrc','-Itest/mocks','tests/dispatcher.test.cpp','src/Dispatcher.cpp',
         'src/Packet.cpp','src/helpers/StaticPoolPacketManager.cpp','-o',str(binary)])
    run([str(binary)])
print('0.9.6 HOST GATES PASSED; target builds and hardware acceptance remain separate.')
