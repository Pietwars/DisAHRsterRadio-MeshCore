#!/usr/bin/env python3
"""Package the verified Standalone test build for human acceptance. Never tags or publishes."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import sys
import zipfile

from check_standalone import check

ROOT = Path(__file__).resolve().parents[1]


def verify(path, release=False):
    with zipfile.ZipFile(path) as archive:
        names = archive.namelist()
        sums = next(name for name in names if name.endswith('/SHA256SUMS'))
        prefix = sums[:-len('SHA256SUMS')]
        listed = set()
        for line in archive.read(sums).decode().splitlines():
            digest, name = line.split('  ', 1)
            if hashlib.sha256(archive.read(prefix + name)).hexdigest() != digest:
                raise ValueError('Pruefsumme falsch: ' + name)
            listed.add(prefix + name)
        if listed != set(names) - {sums}:
            raise ValueError('Pruefsummenliste ist nicht vollstaendig')
        candidate = json.loads(archive.read(prefix + 'candidate.json'))
        if candidate['hardware_status'] != 'pending' or candidate['publication_approved'] != release:
            raise ValueError('Ein Testpaket darf keine Abnahme oder Freigabe behaupten')
        for name, digest in candidate['sha256'].items():
            if hashlib.sha256(archive.read(prefix + name)).hexdigest() != digest:
                raise ValueError('Manifest passt nicht zu ' + name)
        return candidate


def build_factory(files, partitions):
    boot = files['boot_app0.bin']
    if not 0 < len(boot) <= 8192 or not any(p['type'] == 1 and p['subtype'] == 0 and
                                          p['offset'] <= 0xe000 and 0xe000+len(boot) <= p['offset']+p['size']
                                          for p in partitions):
        raise ValueError('boot_app0 passt nicht zur OTA-Datenpartition')
    chunks = [(0, files['bootloader.bin']), (0x8000, files['partitions.bin']),
              (0xe000, boot), (0x10000, files['firmware.bin'])]
    for (offset, data), (next_offset, _) in zip(chunks, chunks[1:]):
        if offset + len(data) > next_offset:
            raise ValueError('Factory-Bestandteile ueberlappen')
    # ESP32-S3 factory image: validated components at their physical flash offsets.
    factory = bytearray(b'\xff' * (0x10000 + len(files['firmware.bin'])))
    for offset, data in chunks:
        factory[offset:offset+len(data)] = data
    for offset, data in chunks:
        if factory[offset:offset+len(data)] != data:
            raise ValueError('Factory-Bestandteile stimmen nicht ueberein')
    return bytes(factory)

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-dir', type=Path)
    parser.add_argument('--build-log', type=Path)
    parser.add_argument('--out', type=Path)
    parser.add_argument('--boot-app0', type=Path)
    parser.add_argument('--check', type=Path)
    parser.add_argument('--release', action='store_true', help='Publication authorized; does not assert hardware acceptance')
    args = parser.parse_args()
    if args.check:
        print(json.dumps(verify(args.check, release=args.release), indent=2))
        return
    if not all((args.build_dir, args.build_log, args.out, args.boot_app0)):
        parser.error('--build-dir, --build-log, --boot-app0 und --out sind erforderlich')
    problems, result = check(args.build_dir, args.build_log.read_text(errors='replace'))
    if problems:
        raise ValueError('Standalone-Pruefung fehlgeschlagen: ' + ', '.join(problems))
    config = (ROOT / 'platformio.standalone.ini').read_text(encoding='utf-8')
    firmware = re.search(r'FIRMWARE_VERSION=\x27"([^"\x27]+)', config).group(1)
    suffix = re.search(r'-standalone-(test\d+)$', firmware).group(1)
    version = dict(line.split('=', 1) for line in (ROOT / 'VERSION').read_text().splitlines() if '=' in line)['STANDALONE']
    name = f'DisAHRsterRadio-MeshCore-{version}-LilyGo_TDeck_standalone-{suffix}'
    files = {part: (args.build_dir / part).read_bytes() for part in ('firmware.bin', 'bootloader.bin', 'partitions.bin')}
    if firmware.encode() not in files['firmware.bin']:
        raise ValueError('Firmwarekennung passt nicht zum gebauten Abbild')
    files['boot_app0.bin'] = args.boot_app0.read_bytes()
    files['factory.bin'] = build_factory(files, result['partitions'])
    manifest = dict(schema=1, version=version, firmware=firmware, environment='LilyGo_TDeck_standalone',
                    source_commit=os.environ.get('GITHUB_SHA', 'local'), hardware_status='pending',
                    publication_approved=args.release, factory_offsets={'bootloader.bin': 0, 'partitions.bin': 0x8000,
                                                                'boot_app0.bin': 0xe000, 'firmware.bin': 0x10000},
                    sha256={part: hashlib.sha256(data).hexdigest() for part, data in files.items()})
    files['candidate.json'] = (json.dumps(manifest, indent=2) + '\n').encode()
    files['standalone-report.json'] = (json.dumps(result, indent=2) + '\n').encode()
    files['standalone-report.md'] = (args.build_dir / 'standalone-report.md').read_bytes()
    files['BEDIENUNG.md'] = (ROOT / 'docs/STANDALONE_TOUCH.md').read_bytes()
    guide = f'''# {name}: Testpaket fuer die Abnahme

Firmware: `{firmware}`. Noch kein Release, Hardwareabnahme OFFEN.

Nur LILYGO T-Deck Plus, MeshCore. Antenne vor dem Einschalten anschrauben.
Identitaet, Kontakte und Verlauf vor dem Flashen sichern.

Auf einer passenden vorhandenen MeshCore-Installation ist `firmware.bin` ein
Update an **0x10000**. Nicht loeschen/Erase nicht aktivieren.
`factory.bin` ist die vollstaendige Neuinstallation an **0x0**: vorher gesamten
Flash loeschen. Das loescht Identitaet, Kontakte, Einstellungen und Verlauf.
Neuinstallation nur nach Sicherung und separat abnehmen. Nicht als Update ueber
LoRaLayer2/Meshtastic oder unbekannte Partitionen flashen.

Zuerst SHA256SUMS pruefen (z. B. `sha256sum -c SHA256SUMS`). `candidate.json`
nennt die zu testenden Bytes, Factory-Offsets und den CI-Commit.

Nach dem Start ggf. Touch kalibrieren. Bedienung: BEDIENUNG.md.
Bitte Wischen/Abbrechen von Sende-Gesten, Tastatur/Trackball, Empfang/Versand,
GPS-Freigabe und Kalibrierung nach Neustart pruefen. Ergebnis und Firmware-SHA256
anhand docs/RELEASE-0.9.5.md dokumentieren. Erst danach ausdrueckliche Freigabe.

Vorschauen: echte Layouts mit simulierten Daten, keine Hardwarefotos.
Kein zertifiziertes Notrufsystem. In Lebensgefahr 112.
'''
    files['ANLEITUNG.md'] = guide.encode()
    if args.release:
        files['ANLEITUNG.md'] = guide.replace('Testpaket fuer die Abnahme', 'Release 0.9.5: Standalone (experimentell)').replace('Noch kein Release, Hardwareabnahme OFFEN.', 'Veroeffentlichung vom Projektbetreiber am 21.10.2026 beauftragt. Vollstaendige Hardwareabnahme OFFEN.').replace('Erst danach ausdrueckliche Freigabe.', 'Die Veroeffentlichungsfreigabe ersetzt diese Hardwarepruefungen nicht.').encode()
    for png in sorted((args.build_dir / 'touch-preview').glob('*.png')):
        files['vorschau/' + png.name] = png.read_bytes()
    files['SHA256SUMS'] = ''.join(hashlib.sha256(data).hexdigest() + '  ' + path + '\n'
                                for path, data in sorted(files.items())).encode()
    args.out.mkdir(parents=True, exist_ok=True)
    output = args.out / (name + '.zip')
    with zipfile.ZipFile(output, 'x', compression=zipfile.ZIP_DEFLATED) as archive:
        for path, data in sorted(files.items()):
            archive.writestr(name + '/' + path, data)
    verify(output, release=args.release)
    print(output)


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, KeyError, StopIteration, zipfile.BadZipFile) as exc:
        sys.exit('FEHLER: ' + str(exc))
