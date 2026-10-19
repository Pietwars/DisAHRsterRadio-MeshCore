#!/usr/bin/env python3
"""Zentrale Versionsquelle ist die Datei VERSION im Repository-Wurzelordner.

  python3 tools/version.py          schreibt alle abgeleiteten Stellen (UI-Fußzeile, Demo, Firmwarekennung, README …)
  python3 tools/version.py --check  prüft nur und bricht mit Fehler ab, wenn eine Stelle abweicht (läuft in CI)
  python3 tools/version.py --get    gibt die Version aus (für Skripte/Workflows)

Neue Version: nur VERSION ändern, dann dieses Skript ohne Argumente ausführen.
"""
import re, sys
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]

def load():
    d = {}
    for line in (ROOT / 'VERSION').read_text(encoding='utf-8').splitlines():
        line = line.strip()
        if line and not line.startswith('#'):
            k, v = line.split('=', 1); d[k.strip()] = v.strip()
    if not re.fullmatch(r'\d+\.\d+\.\d+(-(alpha|beta|rc|dev|test)[0-9.]*)?', d.get('VERSION', '')):
        sys.exit('VERSION: ungültige oder fehlende Zeile VERSION=x.y.z[-alpha|-beta|-rc|-dev|-test]')
    return d

def run(rules, argv):
    vals = load()
    if '--get' in argv:
        print(vals[argv[argv.index('--get') + 1]] if len(argv) > argv.index('--get') + 1 else vals['VERSION']); return 0
    check = '--check' in argv
    bad = 0
    for path, pattern, repl, *opt in rules:
        p = ROOT / path
        if not p.exists():
            print(f'FEHLT: {path}'); bad += 1; continue
        text = p.read_text(encoding='utf-8')
        r = repl
        for k, v in vals.items(): r = r.replace('@' + k + '@', v)
        new, n = re.subn(pattern, r, text, flags=re.M)
        if n < (opt[0] if opt else 1):
            print(f'MUSTER NICHT GEFUNDEN: {path}: {pattern}'); bad += 1; continue
        if new != text:
            if check:
                old_lines = set(text.splitlines())
                for line in new.splitlines():
                    if line not in old_lines: print(f'ABWEICHUNG {path}: erwartet …{line.strip()[:160]}')
                bad += 1
            else:
                p.write_text(new, encoding='utf-8', newline='\n'); print(f'aktualisiert: {path}')
    if check:
        print('Versionen stimmen überein:' if not bad else 'Versionsprüfung fehlgeschlagen:', vals.get('VERSION'))
        if bad: print('Behebung: python3 tools/version.py ausführen und Ergebnis committen.')
    return 1 if bad else 0

ROW = r'^(\| \[Pietwars/DisAHRsterRadio-MeshCore\]\(https://github\.com/Pietwars/DisAHRsterRadio-MeshCore\) \| [^|]+ \| [^|]+ \| )[^|]+( \|)'
FW = r"(FIRMWARE_VERSION='\"v)[0-9.]+-dar[^\"']*?((?:-(?:relay|room|s3-wifi|s3-repeater|s3-room|heltecv2-wifi|wifi))?\"')"
ZIP = r'(DisAHRsterRadio-MeshCore-)\d+\.\d+\.\d+(?:-(?:alpha|beta|rc|dev|test)[0-9.]*)?(-(?:[A-Za-z0-9_]|-(?!(?:alpha|beta|rc|dev|test)))+\.zip|-<board>\.zip)'
RULES = [
    # Weboberfläche: Fußzeile im Gerät-Reiter, simulierte Demo-Firmwarekennung (PortalPage.h erzeugt tools/embed_web.py)
    ('web/index.html', r'(<p class="muted" id="uiver">Weboberfläche DisAHRsterRadio-MeshCore )[^ ]+( · MeshCore v)[0-9.]+(</p>)', r'\g<1>@VERSION@\g<2>@MESHCORE@\g<3>'),
    ('web/index.html', r"(firmware:'v)[0-9.]+-dar[^']+(')", r'\g<1>@MESHCORE@-dar@VERSION@\g<2>'),
    # Firmwarekennung aller Profile
    ('platformio.local.ini', FW, r'\g<1>@MESHCORE@-dar@VERSION@\g<2>', 10),
    ('platformio.devices.ini', FW, r'\g<1>@MESHCORE@-dar@VERSION@\g<2>', 100),
    # README
    ('README.md', r'^(\*\*Version )[^*]+(\*\* · MeshCore )[0-9.]+( .*· Firmwarekennung `v)[0-9.]+-dar[^`]+(`)', r'\g<1>@VERSION@\g<2>@MESHCORE@\g<3>@MESHCORE@-dar@VERSION@\g<4>'),
    ('README.md', r'(in Handygröße, Stand )[^;]+(; alle Daten simuliert)', r'\g<1>@VERSION@\g<2>'),
    ('README.md', ROW, r'\g<1>@VERSION@\g<2>'),
    ('docs/DETAILS.md', ZIP, r'\g<1>@VERSION@\g<2>', 2),  # Build-Beispiele in docs/DETAILS.md (README ab 0.9.1 kurz)
]

if __name__ == '__main__':
    sys.exit(run(RULES, sys.argv[1:]))
