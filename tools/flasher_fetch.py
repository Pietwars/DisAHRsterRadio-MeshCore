#!/usr/bin/env python3
"""Web-Flasher (GitHub Pages): holt die Firmware aus dem GitHub-Release, prüft SHA-256 und schreibt die Manifeste.

Aufruf:  python3 tools/flasher_fetch.py <Pages-Ordner>/flasher        (nur im Pages-Workflow, braucht Internet)
         python3 tools/flasher_fetch.py --check                       (offline: Konfiguration prüfen)
         python3 tools/flasher_fetch.py --cfg tools/flasher-next --check   (vorbereitete Konfiguration prüfen)
Konfiguration: <Ordner>/flasher.json + <Ordner>/index.html (Standard docs/flasher). Zwei Formate:
  alt:  ein Gerät  {repo, tag, asset, member, sha256, out, manifest}            -> manifest.json
  neu:  {repo, tag, devices:[{id, label, asset, member, sha256, out, manifest}]} -> manifest-<id>.json
"""
import hashlib, io, json, re, sys, urllib.request, zipfile
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
args = sys.argv[1:]
CFGDIR = ROOT / 'docs/flasher'
if '--cfg' in args:
    i = args.index('--cfg'); CFGDIR = (ROOT / args[i + 1]).resolve(); del args[i:i + 2]
CFG = json.loads((CFGDIR / 'flasher.json').read_text(encoding='utf-8'))

def devices():
    if 'devices' in CFG:
        return [dict(d, repo=CFG['repo'], tag=CFG['tag'], mfile=f"manifest-{d['id']}.json") for d in CFG['devices']]
    return [dict(CFG, mfile='manifest.json')]

def check():
    html = (CFGDIR / 'index.html').read_text(encoding='utf-8')
    assert 'FLASHEN.md' in html, 'Link auf FLASHEN.md fehlt'
    outs = set()
    for d in devices():
        m = d['manifest']; parts = [p for b in m['builds'] for p in b['parts']]
        assert re.fullmatch(r'v\d+\.\d+\.\d+', d['tag']), 'tag'
        assert re.fullmatch(r'[0-9a-f]{64}', d['sha256']), 'sha256'
        assert m['version'] == d['tag'][1:], 'Manifest-Version passt nicht zum Tag'
        assert [b['chipFamily'] for b in m['builds']] == ['ESP32-S3'], 'nur ESP32-S3-Geräte'
        assert parts == [{'path': d['out'], 'offset': 0}], 'factory.bin muss an 0x0'
        assert d['out'] not in outs, 'Dateiname doppelt'; outs.add(d['out'])
        assert f'manifest="{d["mfile"]}"' in html and d['sha256'] in html, f'index.html passt nicht zu {d["mfile"]}'
        print('Flasher-Konfiguration OK:', d['tag'], d['asset'], d.get('member', ''))

def fetch(out):
    out = Path(out); out.mkdir(parents=True, exist_ok=True)
    cache = {}
    for d in devices():
        url = f"https://github.com/{d['repo']}/releases/download/{d['tag']}/{d['asset']}"
        if url not in cache: cache[url] = urllib.request.urlopen(url, timeout=120).read()
        data = cache[url]
        if d.get('member'):
            data = zipfile.ZipFile(io.BytesIO(data)).read(d['member'])
        got = hashlib.sha256(data).hexdigest()
        if got != d['sha256']: sys.exit(f'SHA-256 stimmt nicht ({d["asset"]}): {got} != {d["sha256"]}')
        (out / d['out']).write_bytes(data)
        (out / d['mfile']).write_text(json.dumps(d['manifest'], ensure_ascii=False, indent=1) + '\n', encoding='utf-8')
        print('Flasher-Firmware:', out / d['out'], len(data), 'Bytes, SHA-256 ok')
    (out / 'flasher.json').unlink(missing_ok=True)

if __name__ == '__main__':
    check()
    if args and args[0] != '--check': fetch(args[0])
