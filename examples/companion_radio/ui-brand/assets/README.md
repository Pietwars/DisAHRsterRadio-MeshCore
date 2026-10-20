# T-Deck Standalone Bootmotiv

Quelle: Pietwars/DisAHRsterRadio, `docs/images/disahrsterradio-header.jpg`,
Commit `50d21caba97a4cbbe97f75428214fd676e51dbe6`, Artwork © Pietwars.
Die unveränderte Quelle liegt hier als `tdeck-splash-source.jpg`.

`python3 tools/gen_tdeck_splash.py` erzeugt RGB565 little endian, 320 × 213,
136320 Bytes. `--check` vergleicht die Datei und prüft Rot/Grün/Blau/Weiß.
Die test4-Datei hatte bei allen Pixeln ein Null-Highbyte (verlorene Rot-/Grünbits).
Test5 kodiert mit breiten Integern neu; Motiv und vollständiger Bildausschnitt bleiben gleich.
Firmware und Hostvorschau nutzen denselben Renderer, (0,13), 640-Byte-Zeilenpuffer.
