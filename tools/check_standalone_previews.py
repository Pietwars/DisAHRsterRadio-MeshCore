#!/usr/bin/env python3
"""Check that the release-candidate gallery contains current 320x240 layouts."""
import hashlib
import json
from pathlib import Path
import struct
import sys

ROOT = Path(__file__).resolve().parents[1]
GALLERY = ROOT / 'docs/images/standalone-test6'


def main():
    manifest = json.loads((GALLERY / 'manifest.json').read_text(encoding='utf-8'))
    if manifest['hardware_approved'] or manifest['kind'] != 'real-layout-demo-data':
        raise ValueError('Vorschau darf keine Hardwareabnahme behaupten')
    for name, digest in manifest['sources'].items():
        if hashlib.sha256((ROOT / name).read_text(encoding='utf-8').replace('\r\n', '\n').encode()).hexdigest() != digest:
            raise ValueError('Vorschauen nach Codeaenderung neu rendern: ' + name)
    for item in manifest['images']:
        image = (GALLERY / item['file']).read_bytes()
        if image[:8] != b'\x89PNG\r\n\x1a\n' or struct.unpack('>II', image[16:24]) != (320, 240):
            raise ValueError('Falsche Displayabmessungen: ' + item['file'])
        if hashlib.sha256(image).hexdigest() != item['sha256']:
            raise ValueError('Vorschaubild hat sich geaendert: ' + item['file'])
    if len(manifest['images']) != 19:
        raise ValueError('Die vollstaendige Galerie hat 19 unterschiedliche Ansichten')
    readme = (ROOT / 'README.md').read_text(encoding='utf-8')
    if manifest['version'] not in readme or 'Hardwareabnahme' not in readme:
        raise ValueError('README muss Version und Hardwarestatus nennen')
    print('19 aktuelle 320x240-Layouts, Quellen und Kandidatenkennzeichnung geprueft')


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, KeyError) as exc:
        sys.exit('FEHLER: ' + str(exc))
