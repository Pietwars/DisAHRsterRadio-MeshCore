# Web-Flasher 0.9.2 (vorbereitet, noch nicht aktiv)

Konfiguration für LILYGO T-Deck Plus und LILYGO T-Watch S3. Aktiv wird sie erst **nach** dem Release v0.9.2
(die Firmware wird aus den Release-ZIPs geladen und per SHA-256 geprüft):

    cp tools/flasher-next/flasher.json tools/flasher-next/index.html docs/flasher/
    python3 tools/flasher_fetch.py --check

Die SHA-256-Werte gehören zu den hardware-getesteten `…-factory.bin` in
`DisAHRsterRadio-MeshCore-0.9.2-LilyGo_TDeck.zip` und `…-LilyGo_TWatchS3.zip`. Werden andere Pakete
veröffentlicht, die Werte hier und in `index.html` anpassen. Prüfen: `python3 tools/flasher_fetch.py --cfg tools/flasher-next --check`.
