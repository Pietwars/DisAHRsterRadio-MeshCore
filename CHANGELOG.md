# Änderungen

## Unveröffentlicht · 17.10.2026 · Verbesserungsliste

- **Zentrale Version:** Datei `VERSION` (Version + MeshCore-Basis) ist die einzige Quelle. `tools/version.py` schreibt Fußzeile „Weboberfläche DisAHRsterRadio-MeshCore … · MeshCore v…“ im Gerät-Reiter, Demo-Firmwarekennung, `FIRMWARE_VERSION` aller Profile (local + devices), README (Versionszeile, Gerätetabelle); `gen_devices.py`, `device_table.py`, `package.sh`, `build_device.sh` lesen `VERSION`. `--check` + `tests/version.test.py` laufen in CI.
- Projektfamilie: gemeinsame Tabelle mit allen fünf Repositorys.
- **Freier Speicher (Heap):** Gerät-Reiter zeigt freien RAM, Minimum und größten Block in KB; unter der Warnschwelle (`HEAP_WARN_BYTES`, Standard 40 KB, auch als `heapWarn` in `/api/state` → `diagnostics`) rot mit ⚠. Seriell alle 30 s `[HEAP] frei … | Minimum … | groesster Block …`, unter der Schwelle mit `WARNUNG: wenig freier Speicher`.
- **Karten verkleinert** mit `tools/shrink_maps.py` (aus HaldenFunk, Profil `ahrtal`; dieselben Dateien wie in DisAHRsterRadio): zusammen ca. 290 → 190 KB im Firmware-Abbild (≈100 KB weniger Flash). Orte und POIs unverändert, Vorher/Nachher-Screenshots verglichen.
- **Fehler behoben:** Dernau, Altenahr und Sinzig zeigten „Karte nicht verfügbar“ (fehlende bbox); die Grenzen werden jetzt beim Erzeugen (`tools/make_map.py`) ergänzt. Neuer Test `tests/maps.test.py` in der CI.

## Unveröffentlicht · 17.10.2026 · Funkprofil EU/UK (Narrow)

- Standard-Funkprofil vereinheitlicht auf das MeshCore-Preset **EU/UK (Narrow): 869,618 MHz, 62,5 kHz, SF8, CR 4/8**. Neu: `-D LORA_CR=8` global in `platformio.ini` – bisher bekamen nur die eigenen Profile CR 4/8, die übrigen Geräte der Matrix fielen auf CR 4/5 zurück. Sendeleistung unverändert (nicht erhöht), Syncword MeshCore-Standard.
- Doku (`docs/cli_commands.md` u. a.): Standardwerte `869.618,62.5,8,8`.

## 0.9.0-alpha-Nachtrag · 17.10.2026 · Vereinheitlichung

- README: Abschnitt „Projektfamilie DisAHRsterRadio“ (in allen drei Repositorys identisch) mit Versionen, Live-Demos, Release, Flyern und Namensregel (DisAHRsterRadio = Projekt/Marke, MeshAhrtal = Weboberfläche, DisasterMesh = Original-Repo).
- Vorschaubilder aller sechs Bereiche neu aus der Live-Demo aufgenommen (Handygröße, aktuelles Vollbild-Design) als `docs/images/funktion-*.jpg`; alte Bilder `darmc-*.png` und PNG-Fassungen entfernt.
- Flyer `docs/flyer/DisAHRsterRadio-MeshCore-Flyer.pdf` (Druck, QR-Codes zu Release und Demo) und `…-Flyer-digital.pdf` (klickbare Links zu Repository, Release, Live-Demo, Projektseite); in der Live-Demo unter `/flyer/` verlinkt.
- Boardtabelle mit Flash-Belegung der 3-MB-App-Partition, Build-Stand 105 Geräte.

## 0.9.0-alpha · 16.10.2026 · erste Vorabversion

[Release v0.9.0-alpha](https://github.com/Pietwars/DisAHRsterRadio-MeshCore/releases/tag/v0.9.0-alpha) – Vorabversion, auf keinem Board abgenommen.

- MeshCore auf **1.18.0** (Upstream `main`, Commit `81dbae59`), Firmwarekennung `v1.18.0-dar0.9.0-alpha`.
- Builds für alle 105 Geräte der MeshCore-Konfigurator-Liste (202 Builds): ESP32-Geräte mit WLAN-Portal (36), nRF52/RP2040/STM32 als Standard-Companion, Repeater und Room-Server ohne Web-Oberfläche; Ausnahmen mit Begründung in der Gerätetabelle (ThinkNode M7, M5Stack Unit C6L, ThinkNode M8 Repeater/Room).
- WLAN-Portal und Bluetooth für die offizielle MeshCore-App gleichzeitig; zufällige Bluetooth-PIN pro Gerät (seriell/OLED).
- Partition `huge_app.csv` (3 MB App); `factory.bin` einmalig an Adresse 0x0 flashen.
- Heltec V2: gestaffelter WLAN-Start gegen Brownout (ungeprüft).
- Weboberfläche MeshAhrtal (kompakt): Chat, Schnellmeldung, Brett, Karte, Standort, Gerät; Hintergrundbild in voller Breite.
- Live-Demo auf GitHub Pages: https://pietwars.github.io/DisAHRsterRadio-MeshCore/

**Im Notfall immer 112.**
