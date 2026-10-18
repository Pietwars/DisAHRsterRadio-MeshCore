# Web-Flasher 0.9.2 (vorbereitet, geht mit dem Release-PR live)

Konfiguration für LILYGO T-Deck Plus und LILYGO T-Watch S3, je zwei Wege:

| Knopf | Manifest | schreibt | Löschen |
|---|---|---|---|
| **Update (Einstellungen bleiben)** – Standard, oben, grün | `manifest-<gerät>-update.json` | nur `…-firmware.bin` an **0x10000** | nur wenn im Fenster „Erase device“ angehakt (`new_install_prompt_erase: true`, Kästchen ist leer voreingestellt) |
| **Neuinstallation (alles löschen)** – unten, orange, mit Warnung | `manifest-<gerät>-neu.json` | `…-factory.bin` an **0x0** | immer (`new_install_prompt_erase: false` → ESP Web Tools löscht bei Firmware ohne Improv immer) |

`new_install_improv_wait_time: 0` (Firmware hat kein Improv; der Flasher wartet nicht darauf). Bootloader, Partitionstabelle
und `spiffs.bin` schreibt das Update bewusst nicht: Die Partitionstabelle ist seit 0.9.0 gleich (nvs 0x9000, otadata 0xe000,
app0 0x10000/3 MB, spiffs 0x310000, coredump 0x3f0000); in spiffs und nvs liegen Identität, Kontakte, Kanäle, Einstellungen und Bluetooth-Kopplungen.

Der Pages-Workflow (`tools/flasher_fetch.py`) lädt beide ZIPs aus dem Release, entpackt `factory.bin` und `firmware.bin`
und prüft beide gegen die SHA-256-Werte in `flasher.json` (auch auf der Seite angezeigt). Stimmt eine nicht, bricht er ab.

Neu schreiben nach einem Neubau (Werte aus den getesteten Paketen):

    python3 tools/flasher-next/aktualisieren.py <…-LilyGo_TDeck.zip> <…-LilyGo_TWatchS3.zip>
    python3 tools/flasher_fetch.py --cfg tools/flasher-next --check

Aktivieren (im Release-PR): `cp tools/flasher-next/flasher.json tools/flasher-next/index.html docs/flasher/`.
Wichtig: Das Release v0.9.2 muss existieren, **bevor** der PR gemergt wird – sonst findet der Pages-Lauf die ZIPs nicht
(dann nach dem Release `gh workflow run pages.yml` erneut starten).
