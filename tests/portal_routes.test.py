#!/usr/bin/env python3
"""Prüft die Web-Portal-Grenzen (0.9.1): Funkprofil ohne Admin, Admin nur noch für Kanäle und Verlauf löschen."""
import re
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
src = (ROOT / 'examples/companion_radio/DisasterPortal.h').read_text(encoding='utf-8')
html = (ROOT / 'web/index.html').read_text(encoding='utf-8')
def handler(route):
    m = re.search(r'http\.on\("' + re.escape(route) + r'",HTTP_POST,\[this\]\(\)\{(.*?)\n    \}\);', src, re.S)
    assert m, f'{route} fehlt'
    return m.group(1)
# Entfernt: Knotenname (MeshCore-App per PIN-Bluetooth) und Admin-Passwort ändern
for gone in ('/api/name', '/api/admin/password'):
    assert f'"{gone}"' not in src, f'{gone} darf es nicht mehr geben'
# Funkprofil: ohne Adminanmeldung, aber nur feste Deutschland-Profile
radio = handler('/api/radio')
assert 'if(!room())return;' in radio and 'adminRoom' not in radio and 'adminAllowed' not in radio
assert 'disaster::dePreset(' in radio
# Verlauf löschen und Kanäle: weiterhin nur mit Adminanmeldung
assert 'if(!adminAllowed())return;' in handler('/api/history-wipe')
assert 'if(!adminRoom())return;' in handler('/api/channel')
assert '"/api/admin/login"' in src
# Weboberfläche: Funkprofil außerhalb des Admin-Bereichs, Admin-Bereich = Kanäle + Verlauf löschen
admin = html[html.index('<div id="admin">'):html.index('<p class="muted" id="ifb">')]
assert 'id="wipe"' in admin and 'id="setch"' in admin and 'id="cslot"' in admin and 'id="rpreset"' not in admin and 'id="setname"' not in admin
assert "act('channel'" in html
for gone in ('id="nname"', 'id="setname"', "act('name'", 'admin/password'):
    assert gone not in html, gone
assert 'id="rpreset"' in html and "act('radio'" in html and "act('history-wipe')" in html
assert set(re.findall(r'<option value="([a-z-]+)">[^<]*MHz', html)) == {'de-narrow', 'ahr-legacy', 'eu-legacy'}
print('portal_routes.test.py: ok')
