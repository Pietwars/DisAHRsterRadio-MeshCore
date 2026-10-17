#!/usr/bin/env python3
"""Prüft die Gerätepakete (tools/build_device.sh) vor der Veröffentlichung.

Aufruf: verify_devices.py <Ordner> <Version>
Jedes Gerät aus tools/devices.json braucht DisAHRsterRadio-MeshCore-<Version>-<Gerät>.zip mit SHA256SUMS (alle Summen
stimmen, keine leere Datei), mindestens einem Flash-Abbild (factory.bin / .uf2 / .bin / .hex) und LIESMICH mit Version.
"""
import hashlib, json, sys, zipfile
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
d, version = sys.argv[1:3]
devices = [e['device'] for e in json.load(open(ROOT / 'tools' / 'devices.json'))]
errors = []
for dev in devices:
    name = f'DisAHRsterRadio-MeshCore-{version}-{dev}'
    z = Path(d) / f'{name}.zip'
    if not z.is_file(): errors.append(f'{z.name}: fehlt'); continue
    with zipfile.ZipFile(z) as zf:
        files = {n[len(name) + 1:]: n for n in zf.namelist() if n.startswith(name + '/') and not n.endswith('/')}
        if 'SHA256SUMS' not in files: errors.append(f'{z.name}: SHA256SUMS fehlt'); continue
        for line in filter(None, zf.read(files['SHA256SUMS']).decode().split('\n')):
            h, f = line.split(None, 1); f = f.lstrip('*').removeprefix('./')
            if f not in files: errors.append(f'{z.name}: {f} fehlt')
            elif zf.getinfo(files[f]).file_size == 0: errors.append(f'{z.name}: {f} ist leer')
            elif hashlib.sha256(zf.read(files[f])).hexdigest() != h: errors.append(f'{z.name}: Prüfsumme von {f} falsch')
        if not any(n.endswith(('.bin', '.uf2', '.hex')) and '/' not in n for n in files): errors.append(f'{z.name}: kein Flash-Abbild')
        docs = [n for n in files if n.upper().startswith(('LIESMICH', 'ANLEITUNG'))]
        if not docs: errors.append(f'{z.name}: LIESMICH/ANLEITUNG fehlt')
        elif not any(version in zf.read(files[n]).decode('utf-8', 'replace') for n in docs): errors.append(f'{z.name}: Anleitung nennt Version {version} nicht')
if errors:
    print('\n'.join('FEHLER: ' + e for e in errors)); sys.exit(1)
print(f'Alle {len(devices)} Gerätepakete vollständig, Prüfsummen stimmen.')
