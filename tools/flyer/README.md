# Flyer-Generator (A5, Druck + digital)

`flyer.py` erzeugt aus `tools/flyer/flyer.json` und der Datei `VERSION` eines Repositorys:
`docs/flyer/<Name>-Flyer.pdf` (Druck, QR-Codes), `<Name>-Flyer-digital.pdf` (alle Links klickbar), `<Name>-Flyer-vorschau.jpg`, QR-PNGs.

    python3 -m venv /tmp/flyervenv && /tmp/flyervenv/bin/pip install qrcode pillow
    cd <Repository> && /tmp/flyervenv/bin/python tools/flyer/flyer.py

Chrome/Chromium headless wird über `$CHROME`, `google-chrome`, `chromium` oder Playwright-Browser gefunden
(z. B. `PLAYWRIGHT_BROWSERS_PATH=… npx playwright install chromium`). Vorschau braucht `pdftoppm` (poppler-utils).
Platzhalter in flyer.json: `@VERSION@`, `@RELEASE@` (Release-Seite des Tags v@VERSION@).
Eine Kopie liegt in jedem Repository unter `tools/flyer/` (Generator + Konfiguration); Master: /workspace/flyer/build.
Der Workflow `release.yml` hängt `docs/flyer/*-Flyer.pdf` und `*-Flyer-digital.pdf` an jedes Release an.

Für den kompakten 0.9.6-Pre-Release-Flyer mit eingebetteten Schriften und aktuellem Standalone-Layout: `python3 tools/flyer/prerelease.py` (reportlab, qrcode, pillow; DejaVu Sans unter /usr/share/fonts/truetype/dejavu). Beide A5-PDFs werden unter docs/flyer/ ersetzt; die digitale Fassung verlinkt QR-Code und Downloadzeile.
