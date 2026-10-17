#!/usr/bin/env python3
"""Bibliotheksversionen festlegen (Verbesserungsliste 6).

Ersetzt in platformio.ini-Dateien Bereichsangaben wie `owner/Name @ ^1.2.3` oder `~1.4.0` durch die
höchste dazu passende Version aus der PlatformIO-Registry (also genau das, was ein Build heute holen würde).
  python3 tools/pin_libs.py [--dir DIR]          festlegen (Standard: Repository-Wurzel bzw. PIODIR)
  python3 tools/pin_libs.py --check [--dir DIR]  CI: Fehler, wenn noch ^/~-Bereiche übrig sind
Plattformen sind bereits fest (z. B. platformio/espressif32@6.11.0, pioarduino-ZIP-URL, Git-Commits).
"""
import json, re, sys, urllib.parse, urllib.request
from pathlib import Path
LINE = re.compile(r'^(\s+;?\s*)([A-Za-z0-9_.-]+)/([^@;\n]+?)\s*@\s*([\^~])\s*(\d+(?:\.\d+){0,2})(\s*(?:;.*)?)$', re.M)
def ver(s): return tuple(int(x) for x in (s.split('.') + ['0', '0'])[:3])
def ok(v, op, base):
    b = ver(base)
    if v < b: return False
    if op == '~': return v[:2] == b[:2]
    if b[0] > 0: return v[0] == b[0]
    if b[1] > 0: return v[:2] == b[:2]
    return v == b
CACHE = {}
def best(owner, name, op, base):
    key = (owner, name)
    if key not in CACHE:
        url = f'https://api.registry.platformio.org/v3/packages/{urllib.parse.quote(owner)}/library/{urllib.parse.quote(name)}'
        CACHE[key] = [v['name'] for v in json.load(urllib.request.urlopen(url, timeout=30))['versions']]
    cands = [v for v in CACHE[key] if re.fullmatch(r'\d+(\.\d+){0,2}', v) and ok(ver(v), op, base)]
    if not cands: sys.exit(f'keine passende Version für {owner}/{name} @ {op}{base}')
    return max(cands, key=ver)
def main():
    a = sys.argv[1:]; check = '--check' in a
    root = Path(a[a.index('--dir') + 1]) if '--dir' in a else Path(__file__).resolve().parents[1]
    files = [p for p in root.rglob('platformio.ini') if '.pio' not in p.parts]
    left = 0
    for p in sorted(files):
        t = p.read_text(encoding='utf-8'); changed = False
        def rep(m):
            nonlocal changed, left
            pre, owner, name, op, base, post = m.groups()
            if owner in ('platformio',) and name.startswith(('toolchain', 'framework', 'tool-')): return m.group(0)  # Werkzeug-Pakete, keine Bibliothek
            if pre.strip().startswith(';'): return m.group(0)  # auskommentiert
            if check: left += 1; print(f'{p.relative_to(root)}: {owner}/{name} @ {op}{base}'); return m.group(0)
            v = best(owner, name.strip(), op, base); changed = True
            return f'{pre}{owner}/{name.strip()} @ {v}{post}'
        t2 = LINE.sub(rep, t)
        if changed: p.write_text(t2, encoding='utf-8'); print('festgelegt:', p.relative_to(root))
    if check:
        if left: sys.exit(f'{left} Bibliotheken ohne feste Version (python3 tools/pin_libs.py ausführen)')
        print('Bibliotheksversionen fest:', len(files), 'platformio.ini')
main()
