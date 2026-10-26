#!/usr/bin/env python3
"""Baut die Live-Demo (GitHub Pages): Weboberflaeche im Demo-Modus, Karten entpackt, Hinweisbanner."""
import gzip,hashlib,shutil,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
SRC=ROOT/'web'; OUT=Path(sys.argv[1] if len(sys.argv)>1 else ROOT/'_site')
if OUT.exists(): shutil.rmtree(OUT)
shutil.copytree(SRC,OUT)
for gz in (OUT/'map').glob('*.json.gz'):
    (OUT/'map'/gz.name[:-3]).write_bytes(gzip.decompress(gz.read_bytes()))
html=(OUT/'index.html').read_text(encoding='utf-8')
force='<script>if(!/demo/.test(location.search))location.replace("?demo=1"+location.hash)</script>'
banner=('<div style="background:#f0d5a0;color:#3b2a1f;text-align:center;'
        'font:600 13px/16px system-ui,sans-serif;padding:6px 8px;height:28px;box-sizing:border-box;white-space:nowrap;overflow:hidden">Demo – simulierte Daten · '
        '<a style="color:#3b2a1f" href="standalone/">T-Deck 0.9.6 Vorschau</a> · '
        '<a style="color:#3b2a1f" href="https://github.com/Pietwars/DisAHRsterRadio-MeshCore">Quellcode &amp; Firmware</a> · '
        '<a style="color:#3b2a1f" href="flasher/">Flasher</a> · '
        '<a style="color:#3b2a1f" href="flyer/DisAHRsterRadio-MeshCore-Flyer-digital.pdf">Flyer</a></div>')
html=html.replace('<head>','<head>'+force,1)
i=html.index('<body'); j=html.index('>',i)+1
html=html[:j]+banner+html[j:]
(OUT/'index.html').write_text(html,encoding='utf-8')
# A new shell or map set gets its own atomic service-worker cache. Scope is also
# part of the runtime cache name, so other deployments on the host are untouched.
assets = ['index.html', 'app.webmanifest', 'app-icon.png', 'bg.webp']
assets += [str(p.relative_to(OUT)) for p in sorted((OUT/'map').glob('*.json*'))]
fingerprint = hashlib.sha256()
for name in assets + ['sw.js']:
    fingerprint.update(name.encode())
    fingerprint.update((OUT/name).read_bytes())
sw = (OUT/'sw.js').read_text(encoding='utf-8')
assert "const VERSION = 'development';" in sw
(OUT/'sw.js').write_text(sw.replace("const VERSION = 'development';", "const VERSION = '" + fingerprint.hexdigest()[:20] + "';"), encoding='utf-8')

if (ROOT/'docs/flyer').is_dir(): shutil.copytree(ROOT/'docs/flyer',OUT/'flyer')
if (ROOT/'docs/flasher').is_dir(): shutil.copytree(ROOT/'docs/flasher',OUT/'flasher')  # Firmware holt tools/flasher_fetch.py im Pages-Workflow
if (ROOT/'docs/standalone').is_dir():
    shutil.copytree(ROOT/'docs/standalone',OUT/'standalone')
    shutil.copytree(ROOT/'docs/images/standalone-test6',OUT/'images/standalone-test6')
(OUT/'.nojekyll').write_text('')
print('Demo gebaut:',OUT)
