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
run(['node', 'tests/offline_shell.test.cjs'])
run(['node', 'tests/flasher_integrity.test.mjs'])
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
    with tempfile.TemporaryDirectory(prefix='dar-host-') as host_tmp:
        run(['bash', '-euo', 'pipefail', '-c', block.replace('/tmp/', host_tmp + '/')])
with tempfile.TemporaryDirectory(prefix='dar096-') as tmp:
    # Each companion frontend must select the same actual dispatcher and BLE policy.
    for mode in ('DISASTER_STANDALONE','DISASTER_PORTAL'):
        client_binary=Path(tmp)/('local-companion-'+mode)
        run(['g++','-std=c++11','-Wall','-Wextra','-Werror','-Wno-misleading-indentation',
             '-fsanitize=address,undefined','-D'+mode,'-DLORA_TX_POWER=22',
             '-Itests/companion_fakes','-Isrc','tests/local_companion.test.cpp','-o',str(client_binary)])
        run([str(client_binary)])
        radio_binary=Path(tmp)/('radio-policy-'+mode)
        run(['g++','-std=c++11','-Wall','-Wextra','-Werror','-fsanitize=address,undefined',
             '-D'+mode,'tests/dar_radio_policy.test.cpp','-o',str(radio_binary)])
        run([str(radio_binary)])
        access_binary=Path(tmp)/('access-'+mode)
        run(['g++','-std=c++17','-Wall','-Wextra','-Werror','-fsanitize=address,undefined',
             '-D'+mode,'-Itests/open_access_fakes','-Isrc','tests/open_access.test.cpp','-o',str(access_binary)])
        run([str(access_binary)])
        tx_binary=Path(tmp)/('regional-tx-'+mode)
        run(['g++','-std=c++11','-Wall','-Wextra','-Wno-unused-parameter','-Wno-reorder',
             '-Wno-sign-compare','-fsanitize=address,undefined','-D'+mode,
             '-Isrc','-Itest/mocks','tests/dispatcher.test.cpp','src/Dispatcher.cpp',
             'src/Packet.cpp','src/helpers/StaticPoolPacketManager.cpp','-o',str(tx_binary)])
        run([str(tx_binary)])
    outbox_binary=Path(tmp)/'delivery-outbox'
    run(['g++','-std=c++11','-Wall','-Wextra','-Werror','-Wno-misleading-indentation',
         '-fsanitize=address,undefined','-Itests/companion_fakes',
         'tests/delivery_outbox.test.cpp','-o',str(outbox_binary)])
    run([str(outbox_binary)])
    sd_binary=Path(tmp)/'archive-sd'
    run(['g++','-std=c++11','-Wall','-Wextra','-Werror',
         '-fsanitize=address,undefined','-Itests/sd_fakes','tests/archive_sd.test.cpp','-o',str(sd_binary)])
    run([str(sd_binary)])
    profile_binary=Path(tmp)/'dar-radio-policy'
    run(['g++','-std=c++11','-Wall','-Wextra','-Werror',
         '-fsanitize=address,undefined','tests/dar_radio_policy.test.cpp','-o',str(profile_binary)])
    run([str(profile_binary)])
    editor_binary=Path(tmp)/'radio-profile-editor'
    run(['g++','-std=c++11','-Wall','-Wextra','-Werror',
         '-fsanitize=address,undefined','tests/radio_profile_editor.test.cpp','-o',str(editor_binary)])
    run([str(editor_binary)])
    profile_view_binary=Path(tmp)/'radio-profile-view'
    run(['g++','-std=c++11','-Wall','-Wextra','-Werror','-Wno-unused-parameter',
         '-fsanitize=address,undefined','-Itests/ui_fakes','-Isrc',
         'tests/radio_profile_view.test.cpp','-o',str(profile_view_binary)])
    run([str(profile_view_binary)])
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
