# Änderungen

## 0.9.2 · DisAHRsterRadio-MeshCore v0.9.2

Geräte: **LILYGO T-Deck Plus** (`DisAHRsterRadio-MeshCore-0.9.2-LilyGo_TDeck.zip`) und **LILYGO T-Watch S3** (`DisAHRsterRadio-MeshCore-0.9.2-LilyGo_TWatchS3.zip`). Firmwarekennung `v1.18.0-dar0.9.2-wifi`.

- **Minimal-Anzeige (neue Oberfläche `examples/companion_radio/ui-brand/`):** Das Display zeigt nur den Schriftzug DisAHRsterRadio mit Logo (`logo/disahrsterradio-display.png`, Ausschnitt mit Ahrtal aus dem Titelbild `docs/flyer/disahrsterradio-header.jpg`, Bild 320×180 auf dem T-Deck bzw. 240×140 auf der T-Watch, erzeugt mit `tools/gen_brand_logo.py`) und einen Knopf **„Advert senden“**. Alle übrigen Display-Seiten, die Nachrichtenvorschau, die Nachrichten-/Tastaturbedienung der Testversionen und der WLAN/PIN-Wechselhinweis sind entfernt. Nachrichten und Einstellungen nur noch über MeshCore-App (Bluetooth mit PIN) und Weboberfläche.
- **Advert:** jeder Advert ist ein **Flood-Advert** (`MyMesh::floodAdvert()`, wie „Advert senden → Flood“ in der App); danach 2 s „Advert gesendet“, höchstens einer je 10 s („Bitte N s warten“). Display geht nach 30 s ohne Bedienung aus; neue Nachrichten wecken es nicht.
- **T-Deck Plus:** Trackball kurz = Display an/aus, 2,5 s halten = Flood-Advert, kein Abschalten per Trackball; Tastatur (auch Enter) ohne Funktion.
- **LILYGO T-Watch S3 neu** (Profil `dar_LilyGo_TWatchS3_wifi`, ESP32-S3, SX1262, AXP2101, Touch): Seitentaste kurz = Display an/aus, ca. 2 s = Flood-Advert (Tasten-Interrupt des AXP2101), ca. 6 s = harte Abschaltung durch den AXP2101 (vorher 4 s); volle Auflösung 240×240, Texte in glatter Schrift (FreeSans, LovyanGFX); Knopf „Advert senden“ antippen = Flood-Advert, Antippen bei dunklem Display weckt nur. Abfrage alle 20 ms, 300 ms Entprellung.
- **Bluetooth-PIN nur während der Kopplung** auf dem Display (Passkey-Abfrage bis Kopplung fertig/abgebrochen, höchstens 60 s; Verbindung ohne Kopplung nach 2,5 s, höchstens 60 s). Die PIN steht in keiner Anleitung mehr.
- **Schnellmeldung (Weboberfläche):** „Senden“-Knopf auf schmalen Handys wieder im Bild.
- **Nur noch T-Deck Plus und T-Watch S3 in CI und Release** (`build.yml`, `devices.yml`, `release.yml`; Release-Titel „DisAHRsterRadio-MeshCore v<Version>“). Heltec V2 ist nicht mehr Teil des Releases.
- Doku (README, `docs/DETAILS.md`, `docs/FLASHEN.md`, `docs/OFFENER_ZUGANG.md`, ANLEITUNG-Vorlagen) auf 0.9.2 aktualisiert: Anzeige, Tastentabellen, Geräte; Hinweise auf frühere Schwester-Repositorys entfernt.
- **Web-Flasher für LILYGO T-Deck Plus** auf GitHub Pages (`/flasher/`, ESP Web Tools 10.4.0): schreibt die unveränderte `factory.bin` aus dem Release v0.9.1 an 0x0; der Pages-Workflow lädt sie aus dem Release und prüft die SHA-256 (`tools/flasher_fetch.py`, Konfiguration `docs/flasher/flasher.json`). Neue Anleitung `docs/FLASHEN.md` (Port finden, esptool-Befehle, Update an 0x10000). Firmware unverändert. Bis zum Release 0.9.2 installiert der Web-Flasher die getestete 0.9.1-Datei.

## 0.9.1 · 18.10.2026 · Release

- **Flyer 0.9.1 (nachgereicht):** Druck-PDF (QR-Code zum Release v0.9.1 und zur Demo) und Digital-PDF (klickbare Links) neu erzeugt, mit Vorschau der aktuellen Weboberfläche; Inhalt und Boards entsprechen dem Release.

GitHub-Release `v0.9.1` mit den auf Hardware getesteten Paketen für **LILYGO T-Deck Plus** und **Heltec WiFi LoRa 32 V2**. Firmwarekennung `v1.18.0-dar0.9.1` (T-Deck: `v1.18.0-dar0.9.1-wifi`, Heltec V2: `v1.18.0-dar0.9.1-heltecv2-wifi`).

- **Chatverlauf-Fix (alle ESP32-Portal-Boards):** Der Web-Chat blieb leer (weder eigene noch empfangene Nachrichten). Ursache: `SPIFFS.open(fehlendeDatei,"r+")` liefert unter arduino-esp32 ein gültiges Verzeichnis-Handle, daher wurde `/dr-history.bin` nie angelegt. Neu `SlotFile.h` (Öffnen per `SPIFFS.exists()`, Auffüllen bis zum Slot), genutzt von `DisasterPortal` und `DeliveryOutbox`; Weboberfläche zeigt „Verlauf-Speicher“. Hosttest `tests/slot_file.test.cpp`. Alte, nie gespeicherte Nachrichten erscheinen nicht nachträglich.
- **T-Deck-Display-Fix (SPI):** Display und LoRa nutzen beim LILYGO T-Deck eine gemeinsame `SPIClass` (FSPI) statt zweier Controller auf denselben Leitungen – kein Hängenbleiben bei „Loading...“ mehr. Nur unter `LILYGO_TDECK`; Heltec V4 TFT unverändert.
- **Bluetooth mit PIN-Kopplung** (statischer Passkey, MITM-Schutz, Bonding; PIN wird auf dem Display angezeigt, eigene PIN siehe unten) statt PIN-freier Kopplung. WLAN bleibt offen, Brett-Gast-Passwort bleibt `hello`.
- **Kontakt-Einstellungen der MeshCore-App:** „Kontakte automatisch hinzufügen“ u. a. wurden über Bluetooth abgelehnt (Befehle 38 `CMD_SET_OTHER_PARAMS` und 58 `CMD_SET_AUTOADD_CONFIG` nicht freigegeben, App wartete endlos). Jetzt nur über **PIN-geschütztes Bluetooth** erlaubt (GATT-Schreibzugriff mit MITM-Verschlüsselung); offenes WLAN/Ethernet bleiben gesperrt. Funkparameter, Schlüssel, Neustart/Werksreset usw. bleiben USB/Admin vorbehalten. Test in `tests/open_access.test.cpp`.
- **Kontaktverwaltung über PIN-geschütztes Bluetooth:** zusätzlich 9 `CMD_ADD_UPDATE_CONTACT` (entdeckten Kontakt übernehmen, Name/Flags/Pfad bearbeiten) und 15 `CMD_REMOVE_CONTACT` nach derselben Regel (nur Bluetooth mit aktiver PIN, nicht WLAN/Ethernet). Teilen (16), Exportieren (17), Importieren (18), Pfad zurücksetzen (13) waren schon offen. Kanäle (32), Name (8), Standort (14), Funk (11/12/21), Schlüssel (23/24), Neustart/Reset (19/51) bleiben gesperrt.
- **Weboberfläche ohne Admin-Bereich:** Der Web-Admin des Companions ist entfernt (`/api/admin/login`, `/api/admin/logout`, `/api/admin/password`, `AdminSession`-Nutzung, serielle Anzeige des Admin-Passworts, 10-s-BOOT-Taste zum Neuerzeugen; alte Einträge `admin-salt`/`admin-hash` werden beim Start gelöscht). Ohne Anmeldung im Reiter Gerät: Knotenname (`/api/name`, 1–24 UTF-8-Bytes), Funkprofil (`/api/radio`, nur `dePreset`: de-narrow, ahr-legacy, eu-legacy), Kanäle anlegen/ändern (`/api/channel`), Verlauf löschen (`/api/history-wipe`, mit Rückfrage). **Jede Person im offenen WLAN kann Kanäle, Name und Funkprofil ändern und den Verlauf löschen.** Keine Kanäle vorbelegt (nur der MeshCore-Standardkanal „Public“). Repeater/Room-Server-Admin unverändert. Test `tests/portal_routes.test.py`.
- **Volle MeshCore-App über PIN-geschütztes Bluetooth (ersetzt die Einzelfreigaben 9/15/38/58):** Kommt ein Befehl über Bluetooth und ist die BLE-PIN aktiv, sind **alle** Companion-Befehle erlaubt wie bei Standard-MeshCore – Name, Standort, Kanäle, Funk, Custom-Vars/GPS, Neustart, Werksreset, Flood-Scope, Pfad-Hash, CLI, Signieren, Rohpakete, Fernanfragen und **privater Schlüssel Export (23) / Import (24)** (`ENABLE_PRIVATE_KEY_EXPORT/IMPORT` in allen Portal-Builds wieder aktiv; die Export-Antwort geht nur an die anfragende USB- bzw. PIN-Bluetooth-Verbindung, nie an WLAN/Ethernet/Portal). **Eigene Bluetooth-PIN:** `CMD_SET_DEVICE_PIN` (37) und USB-Befehl `set pin` nehmen jede 6-stellige PIN (100000–999999) an, speichern sie in den Prefs; aktiv nach Neustart, danach am Handy neu koppeln. Ungültig/0 → Standard-PIN (wird auf dem Display angezeigt; Bluetooth bleibt immer mit PIN). Offenes WLAN/Ethernet und Bluetooth ohne PIN unverändert eingeschränkt. Test `tests/open_access.test.cpp`.
- **Kontakte automatisch nur Repeater und Room-Server (Werkseinstellung, alle Portal-Builds):** unter `DISASTER_PORTAL` startet ein Gerät mit leerem Speicher mit `manual_add_contacts = 1` und `autoadd_config = 0x0C` (Repeater + Room-Server) statt „alle automatisch“. Chat-Kontakte (Companions), Sensoren und unbekannte Typen bleiben manuell. Eine gespeicherte `/prefs.json` (`man_add`, `auto_add`) hat Vorrang; die factory-Datei überschreibt das SPIFFS und damit auch die Einstellungen. Logik in `AutoAddPolicy.h`, Hosttest `tests/autoadd.test.cpp`.
- Flashen: `factory.bin` immer an **0x0** (klassischer ESP32: Bootloader im Image bei 0x1000; S3/C3 bei 0x0).
- Hardware: **LILYGO T-Deck Plus und Heltec V2 getestet** (von Peter). Der Bootfix für klassische ESP32 (PR #5) ist **nicht** enthalten; der Heltec V2 lief im Test trotzdem stabil.
- **Nur T-Deck Plus und Heltec V2 im CI und im Release:** `build.yml`, `devices.yml` und `release.yml` bauen nur noch `dar_LilyGo_TDeck_wifi`/`LilyGo_TDeck` und `disaster_heltec_v2_sx1276_wifi`; die übrigen Geräte bleiben im Code (`tools/devices.json`, `platformio.local.ini`), werden aber vorerst nicht gebaut oder veröffentlicht. `tools/verify_devices.py` nimmt optional eine Geräteliste. Ist das Release (mit den hardware-getesteten Paketen) beim Tag schon angelegt, lädt `release.yml` nichts hoch.
- README-Bilder aus der Live-Demo neu aufgenommen (Stand 0.9.1).

## Unveröffentlicht – offener Zugang

- **Offenes WLAN:** `softAP(ssid, nullptr, 1, false, 4)`, gespeichertes `ap-pass` wird beim Start gelöscht; seriell/OLED „offen“.
- **Bluetooth ohne PIN:** Just Works (ESP32/nRF52/RP2040), gespeicherte BLE-PIN wird auf 0 gesetzt, keine Zufalls-PIN mehr, App kann keine PIN setzen; nRF52 ohne Bluetooth-DFU.
- Offene Bluetooth-/Netzwerk-Companions: Nachrichten, Kontakte, Uhrzeit-Abgleich, Status; Betreiberbefehle (Funk, Name, Kanäle, Schlüssel, Reset, CLI) nur USB/Admin-Bereich. Längen-/Pfadprüfungen für Companion-Befehle.
- Doku, Flash-Anleitung, LIESMICH und Flyer ohne Zugangsdaten-Hinweise; Hosttest `tests/open_access.test.cpp`. Hardwareabnahme steht aus.
- **Room-Server mit öffentlichem Gast-Passwort `hello`** (MeshCore-Standard): mit `hello` lesen und schreiben, ohne Passwort nur lesen (`allow.read.only` an). Beim Start wird das Gast-Passwort auf `hello` gesetzt (ersetzt leere/zufällige Werte älterer Versionen); `set guest.password` und `set allow.read.only` werden abgelehnt. Admin-Login unverändert. Portal: kein Eingabefeld, „Beitreten“ meldet automatisch mit `hello` an – Aushängen im Portal wieder möglich. Hosttest `tests/room_access.test.cpp`.

## Unveröffentlicht · 17.10.2026 · Feste Versionen

- **Bibliotheken festgelegt (Verbesserungsliste 6):** alle `^`/`~`-Bereiche in `platformio.ini` und allen `variants/*/platformio.ini` (105 Geräte) auf die heute aufgelöste Version gesetzt (z. B. Crypto 0.4.0, RTClib 2.1.4, Melopero RV3028 1.2.0, Adafruit SSD1306 2.5.17, base64 1.4.0) mit `tools/pin_libs.py`; CI prüft mit `--check`. Plattformen waren schon fest.

## Unveröffentlicht · 17.10.2026 · Verbesserungsliste

- **Zentrale Version:** Datei `VERSION` (Version + MeshCore-Basis) ist die einzige Quelle. `tools/version.py` schreibt Fußzeile „Weboberfläche DisAHRsterRadio-MeshCore … · MeshCore v…“ im Gerät-Reiter, Demo-Firmwarekennung, `FIRMWARE_VERSION` aller Profile (local + devices), README (Versionszeile, Gerätetabelle); `gen_devices.py`, `device_table.py`, `package.sh`, `build_device.sh` lesen `VERSION`. `--check` + `tests/version.test.py` laufen in CI.
- Projektfamilie: gemeinsame Tabelle mit allen fünf Repositorys.
- **Automatische Releases:** Workflow `release.yml` baut bei jedem Tag `v*` die drei Haupt-Boards (`tools/package.sh`, jetzt mit `ENVS`-Auswahl) und alle Geräte aus `tools/devices.json` (`tools/build_device.sh`), prüft jedes Paket (`tools/verify_release.py`, `tools/verify_devices.py`: Datei fehlt, leer, Prüfsumme falsch, Version fehlt → Abbruch) und veröffentlicht eine Vorabversion mit Text aus diesem CHANGELOG. Probelauf ohne Veröffentlichung bei PRs, die die Paketierung ändern.
- CI: PlatformIO `6.2.0` und esptool `5.6.0` fest; „Alle Geräte“ läuft bei Pull Requests nur noch, wenn sich Geräte-Builds ändern können (vorher 105 Jobs pro PR).
- **Freier Speicher (Heap):** Gerät-Reiter zeigt freien RAM, Minimum und größten Block in KB; unter der Warnschwelle (`HEAP_WARN_BYTES`, Standard 40 KB, auch als `heapWarn` in `/api/state` → `diagnostics`) rot mit ⚠. Seriell alle 30 s `[HEAP] frei … | Minimum … | groesster Block …`, unter der Schwelle mit `WARNUNG: wenig freier Speicher`.
- **Karten verkleinert** mit `tools/shrink_maps.py` (aus HaldenFunk, Profil `ahrtal`; dieselben Dateien wie in DisAHRsterRadio): zusammen ca. 290 → 190 KB im Firmware-Abbild (≈100 KB weniger Flash). Orte und POIs unverändert, Vorher/Nachher-Screenshots verglichen.
- **Fehler behoben:** Dernau, Altenahr und Sinzig zeigten „Karte nicht verfügbar“ (fehlende bbox); die Grenzen werden jetzt beim Erzeugen (`tools/make_map.py`) ergänzt. Neuer Test `tests/maps.test.py` in der CI.

## Unveröffentlicht · 17.10.2026 · Funkprofil EU/UK (Narrow)

- Standard-Funkprofil vereinheitlicht auf das MeshCore-Preset **EU/UK (Narrow): 869,618 MHz, 62,5 kHz, SF8, CR 4/8**. Neu: `-D LORA_CR=8` global in `platformio.ini` – bisher bekamen nur die eigenen Profile CR 4/8, die übrigen Geräte der Matrix fielen auf CR 4/5 zurück. Sendeleistung unverändert (nicht erhöht), Syncword MeshCore-Standard.
- Doku (`docs/cli_commands.md` u. a.): Standardwerte `869.618,62.5,8,8`.

## Unveröffentlicht · Schlanke TTGO-/T3-V1.6.1-Profile

- **T3 V1.6.1 / TLora V2.1-1.6 wurde ohne Optimierung gebaut (-O0):** Die Upstream-Variante `LilyGo_TLora_V2_1_1_6` entfernt mit `build_unflags = -Os` die Optimierung, dabei fällt auch das `-Os` aus `build_flags` weg. `disaster_ttgo_wifi` (= `disaster_tlora_v2_1_sx1276_wifi`, `_wifi_sd`), `disaster_ttgo_repeater` und `disaster_ttgo_room` übernehmen dieses `build_unflags` nicht mehr → `-Os` wie bei allen anderen Boards.
- Die ~15 Sensorbibliotheken aus `sensor_base` (INA3221/219/226/260, AHTX0, BME280/680, BMP280/085, SHTC3, SHT4x, LPS22HB, MLX90614, VL53L0X, MicroNMEA, BSEC) werden für diese Profile nicht mehr gebaut (`[dar_classic_slim]`: eigene `lib_deps`, `lib_ignore`). `ENV_INCLUDE_*` waren schon aus; der Sensorcode fällt beim Kompilieren weg, Batterie-Telemetrie bleibt.
- Flash: **T3 V1.6.1 86,8 % → 71,8 %** (2 729 045 → 2 258 481 Bytes, 3-MB-Partition), SD-Profil 72,8 %, TTGO-Repeater 44,4 % → 29,5 %, TTGO-Room 43,4 % → 29,3 % (1,9-MB-Partition). Heltec V2, T-Beam, Meshadventurer, T3-S3 unverändert (dort keine Sensorbibliotheken bzw. schon `-Os`).
- Unabhängig vom Bootschleifen-Fix (PR #5, NimBLE); auf Hardware noch nicht geprüft.

## 0.9.0-alpha-Nachtrag · 17.10.2026 · Vereinheitlichung

- README: Abschnitt „Projektfamilie DisAHRsterRadio“ (in allen drei Repositorys identisch) mit Versionen, Live-Demos, Release, Flyern und Namensregel (DisAHRsterRadio = Projekt/Marke, MeshAhrtal = Weboberfläche).
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
