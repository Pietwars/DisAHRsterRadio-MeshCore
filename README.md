# DisAHRsterRadio-MeshCore

> **Aktueller Stand:** offenes WLAN `DisAHRsterRadio-XXXXXX` → `http://192.168.4.1`, MeshCore-App per Bluetooth mit PIN-Kopplung (Standard-PIN `123456`, in der App änderbar), Brett (Room-Server) mit dem öffentlichen Gast-Passwort `hello` (sendet die Weboberfläche automatisch) – einfach verbinden, nichts eingeben. Volle Einstellungen in der MeshCore-App über PIN-Bluetooth; in der Weboberfläche Knotenname, Funkprofil, Kanäle und Verlauf löschen ohne Anmeldung (jede Person im offenen WLAN kann das). [Zugang auf einen Blick](#zugang-auf-einen-blick) · [Zugang, Update und Abnahme](docs/OFFENER_ZUGANG.md)


**🔴 Live-Demo (simulierte Daten, kein Gerät nötig): https://pietwars.github.io/DisAHRsterRadio-MeshCore/**

**Version 0.9.1** · MeshCore 1.18.0 (Upstream `main`, Commit `81dbae59`) · Firmwarekennung `v1.18.0-dar0.9.1`

> **Release 0.9.1** ([Download](https://github.com/Pietwars/DisAHRsterRadio-MeshCore/releases/tag/v0.9.1)) – Firmware für **LILYGO T-Deck Plus** und **Heltec WiFi LoRa 32 V2**, beide auf echter Hardware getestet. Neu gegenüber 0.9.0-alpha:
> - **Chatverlauf-Fix (alle ESP32-Portal-Boards):** Der Chat in der Weboberfläche blieb leer, weil die Verlaufsdatei im SPIFFS nie angelegt wurde. Jetzt wird sie angelegt; „Gerät“ → **Verlauf-Speicher** zeigt „ok“.
> - **T-Deck-Display-Fix (SPI):** Display und LoRa teilen beim LILYGO T-Deck einen SPI-Bus; der Bildschirm bleibt nicht mehr bei „Loading...“ stehen. Nur `LILYGO_TDECK` betroffen.
> - **Kontakte automatisch nur für Repeater und Room-Server:** Chat-Kontakte (Companions) und Sensoren werden nicht mehr automatisch hinzugefügt (Werkseinstellung bei frischem Speicher, in der App über PIN-Bluetooth änderbar; gespeicherte Einstellungen bleiben beim reinen Firmware-Update erhalten).
> - **Weboberfläche ohne Admin:** Weboberfläche (offenes WLAN, **ohne Anmeldung**): Knotenname, Funkprofil (nur feste Deutschland-Profile), Kanäle anlegen/ändern und Nachrichtenverlauf löschen im Reiter „Gerät“. **Achtung: Jede Person im offenen WLAN kann Kanäle, Knotenname und Funkprofil ändern und den Verlauf löschen.** Einen Web-Admin-Bereich (Admin-Passwort) gibt es nicht mehr.
> - **Volle MeshCore-App über PIN-geschütztes Bluetooth:** MeshCore-App über PIN-gekoppeltes Bluetooth: **alle** Einstellungen und Funktionen wie bei Standard-MeshCore (Name, Standort, Kanäle, Funkparameter, Kontakte, GPS, Neustart, Werksreset, CLI), auch **privaten Schlüssel exportieren/importieren** und eine **eigene 6-stellige PIN** setzen (wird gespeichert, gilt nach dem Neustart; danach die alte Kopplung am Handy entfernen und mit der neuen PIN neu koppeln). Über das offene WLAN/Ethernet und Bluetooth ohne PIN bleibt die Companion-Schnittstelle eingeschränkt wie bisher.
> - **Bluetooth immer mit PIN** (statische Passkey-Kopplung mit MITM-Schutz und Bonding): Standard `123456`, eigene 6-stellige PIN per App oder USB-Befehl `set pin <6 Ziffern>`. WLAN bleibt offen.
>
> Auf echter Hardware getestet (0.9.1): **LILYGO T-Deck Plus** und **Heltec WiFi LoRa 32 V2**. Alle anderen Boards sind vorerst nicht im Release (der Code bleibt erhalten, siehe [Weitere Geräte](#weitere-geräte-im-code-derzeit-nicht-im-release)).

**DisAHRsterRadio mit der Weboberfläche MeshAhrtal auf MeshCore-Firmware.** Das Board ist ein MeshCore-Companion und spannt ein eigenes WLAN auf. Man verbindet sich mit dem Handy und nutzt im Browser Chat, Schnellmeldungen, Brett, Karte und Standort. Eine App ist nicht nötig, Internet auch nicht.

> ⚠️ **Kein zertifiziertes Notrufsystem.** In Lebensgefahr immer **112** wählen, wenn irgendwie möglich.

<!-- projektfamilie:start (in allen fünf Repositorys identisch; erzeugt mit DisasterMesh/tools/projektfamilie.py) -->
### Projektfamilie DisAHRsterRadio / HaldenFunk

| Repository | Inhalt | Funk | Version | Demo · Download · Flyer |
|---|---|---|---|---|
| [Pietwars/DisasterMesh](https://github.com/Pietwars/DisasterMesh) | Original-Repo: MeshCore 1.18 mit der großen Weboberfläche MeshAhrtal, Karten, Werkzeuge, Doku (Ahrtal) | MeshCore | 0.9.2 | [Live-Demo](https://pietwars.github.io/DisasterMesh/demo/) · [Release v0.9.2](https://github.com/Pietwars/DisasterMesh/releases/tag/v0.9.2) · Flyer [Druck](https://pietwars.github.io/DisasterMesh/flyer/DisasterMesh-Flyer.pdf) / [digital](https://pietwars.github.io/DisasterMesh/flyer/DisasterMesh-Flyer-digital.pdf) |
| [Pietwars/DisAHRsterRadio-MeshCore](https://github.com/Pietwars/DisAHRsterRadio-MeshCore) | MeshCore 1.18 mit kompakter MeshAhrtal-Oberfläche per WLAN plus Bluetooth für die MeshCore-App; Firmware für LILYGO T-Deck Plus und Heltec V2 (Ahrtal) | MeshCore | 0.9.1 | [Live-Demo](https://pietwars.github.io/DisAHRsterRadio-MeshCore/) · [Release v0.9.1](https://github.com/Pietwars/DisAHRsterRadio-MeshCore/releases/tag/v0.9.1) · Flyer [Druck](https://pietwars.github.io/DisAHRsterRadio-MeshCore/flyer/DisAHRsterRadio-MeshCore-Flyer.pdf) / [digital](https://pietwars.github.io/DisAHRsterRadio-MeshCore/flyer/DisAHRsterRadio-MeshCore-Flyer-digital.pdf) |
| [Pietwars/DisAHRsterRadio](https://github.com/Pietwars/DisAHRsterRadio) | Ohne MeshCore: modernisiertes disaster-radio (LoRaLayer2, RadioLib) mit kompakter MeshAhrtal-Oberfläche (Ahrtal) | LoRaLayer2 | 0.9.1 | [Live-Demo](https://pietwars.github.io/DisAHRsterRadio/) · [Release v0.9.1](https://github.com/Pietwars/DisAHRsterRadio/releases/tag/v0.9.1) · Flyer [Druck](https://pietwars.github.io/DisAHRsterRadio/flyer/DisAHRsterRadio-Flyer.pdf) / [digital](https://pietwars.github.io/DisAHRsterRadio/flyer/DisAHRsterRadio-Flyer-digital.pdf) |
| [Pietwars/HaldenFunk](https://github.com/Pietwars/HaldenFunk) | Regionalvariante von DisAHRsterRadio für Sangerhausen / Mansfelder Land, ohne MeshCore | LoRaLayer2 | 0.9.1 | [Live-Demo](https://pietwars.github.io/HaldenFunk/) · [Release v0.9.1](https://github.com/Pietwars/HaldenFunk/releases/tag/v0.9.1) · Flyer [Druck](https://pietwars.github.io/HaldenFunk/flyer/HaldenFunk-Flyer.pdf) / [digital](https://pietwars.github.io/HaldenFunk/flyer/HaldenFunk-Flyer-digital.pdf) |
| [Pietwars/HaldenFunk-MeshCore](https://github.com/Pietwars/HaldenFunk-MeshCore) | Regionalvariante von DisAHRsterRadio-MeshCore für das Mansfelder Land: MeshCore 1.18, WLAN + Bluetooth, Gerätematrix | MeshCore | 0.9.1 | [Live-Demo](https://pietwars.github.io/HaldenFunk-MeshCore/) · [Release v0.9.1](https://github.com/Pietwars/HaldenFunk-MeshCore/releases/tag/v0.9.1) · Flyer [Druck](https://pietwars.github.io/HaldenFunk-MeshCore/flyer/HaldenFunk-MeshCore-Flyer.pdf) / [digital](https://pietwars.github.io/HaldenFunk-MeshCore/flyer/HaldenFunk-MeshCore-Flyer-digital.pdf) |

Gleiche Spalte **Funk** = Geräte können miteinander funken (gleiches Funkprofil vorausgesetzt). MeshCore- und LoRaLayer2-Varianten verstehen sich **nicht**.

**Namen:** **DisAHRsterRadio** = Projekt und Marke (WLAN `DisAHRsterRadio-XXXXXX`) · **MeshAhrtal** = die Weboberfläche im Handy-Browser · **DisasterMesh** = das Original-Repository · **HaldenFunk** = Regionalvariante fürs Mansfelder Land (WLAN `HaldenFunk-XXXXXX`).

Firmware 0.9.1 auf echter Hardware getestet: **LILYGO T-Deck Plus** und **Heltec WiFi LoRa 32 V2** (868 MHz) mit DisAHRsterRadio-MeshCore. Die Releases enthalten nur Firmware für diese beiden Boards (soweit das jeweilige Repository sie unterstützt); weitere Boards bleiben im Code, sind aber vorerst nicht im Release. Druck-Flyer haben QR-Codes, digitale Flyer klickbare Links. Alle Demos sind simuliert.

> ⚠️ Prototyp, kein zertifiziertes Notrufsystem. **Im Notfall immer 112.**
<!-- projektfamilie:end -->

**Download:** [Release v0.9.1](https://github.com/Pietwars/DisAHRsterRadio-MeshCore/releases/tag/v0.9.1) (T-Deck Plus und Heltec V2, je Paket-ZIP und `factory.bin` an **0x0**) · **Flyer (A5):** [Druck-PDF mit QR-Codes](docs/flyer/DisAHRsterRadio-MeshCore-Flyer.pdf) · [Digital-PDF mit klickbaren Links](docs/flyer/DisAHRsterRadio-MeshCore-Flyer-digital.pdf)

**Web-Flasher (nur LILYGO T-Deck Plus):** [pietwars.github.io/DisAHRsterRadio-MeshCore/flasher](https://pietwars.github.io/DisAHRsterRadio-MeshCore/flasher/) – in Chrome/Edge am Computer, schreibt die `factory.bin` aus dem Release v0.9.1 an 0x0 (löscht Einstellungen). **Heltec V2 vorerst manuell** mit esptool bzw. esptool-js: [Anleitung Flashen](docs/FLASHEN.md) (Port finden unter Windows/macOS/Linux, Befehle, Update ohne Datenverlust).

## Zugang auf einen Blick

| Zugang | Aktueller Stand |
|---|---|
| WLAN `DisAHRsterRadio-XXXXXX` | **offen, ohne Passwort** – verbinden, `http://192.168.4.1` öffnen, nichts eingeben (höchstens 4 Handys je Board) |
| Bluetooth (MeshCore-App) | **PIN-Kopplung**: PIN bei der Kopplung eingeben – Standard **`123456`** (öffentlich bekannt, bitte ändern), eigene 6-stellige PIN in der App setzbar (gilt nach Neustart, dann neu koppeln); alte Kopplung vorher am Handy entfernen. Mit PIN-Kopplung hat die App alle Einstellungen wie Standard-MeshCore, auch Export/Import des privaten Schlüssels. Versionen vor 0.9.1 koppelten ohne PIN („Just Works“) |
| Schwarzes Brett (Room-Server) | öffentliches MeshCore-Standard-Gast-Passwort **`hello`**: damit lesen und schreiben. Die Weboberfläche sendet `hello` beim „Beitreten“ automatisch, niemand muss etwas eintippen. In der MeshCore-App `hello` eingeben; ohne Passwort nur lesen |
| Kontakte | automatisch nur **Repeater und Room-Server**; Chat-Kontakte und Sensoren manuell (Werkseinstellung 0.9.1, in der MeshCore-App über PIN-Bluetooth änderbar; dort Kontakte auch manuell hinzufügen, bearbeiten, löschen) |
| Weboberfläche | **keine Anmeldung mehr:** Knotenname, Funkprofil (feste Deutschland-Profile), Kanäle anlegen/ändern, Verlauf löschen – für jede Person im offenen WLAN. Das frühere Web-Admin-Passwort (serielle Anzeige, 10 s BOOT-Taste) ist entfernt. Repeater und Room-Server haben weiterhin ihr eigenes Admin-Passwort (bei jedem Gerät eigen, nur beim ersten Start über USB angezeigt, per USB-Befehl `password <neu>` änderbar). |

Außer dem öffentlichen Standard `hello` stehen keine Passwörter in Code, Doku, Flyern oder der Live-Demo;  Details: [Offener Zugang](docs/OFFENER_ZUGANG.md).

> ⚠️ **Flashen:** Die `factory.bin` immer an **0x0** flashen. Sie überschreibt das Dateisystem: Einstellungen, Knotenidentität, Kontakte und Verlauf sind danach gelöscht. Update ohne Datenverlust: nur `teile/firmware.bin` an `0x10000`.

## Funktionen im Überblick

| Chat | Schnellmeldung | Brett |
|---|---|---|
| <img src="docs/images/funktion-chat.jpg" width="230" alt="Chat"> | <img src="docs/images/funktion-quick.jpg" width="230" alt="Schnellmeldung"> | <img src="docs/images/funktion-board.jpg" width="230" alt="Brett"> |
| **Karte** | **Standort** | **Gerät** |
| <img src="docs/images/funktion-map.jpg" width="230" alt="Karte"> | <img src="docs/images/funktion-loc.jpg" width="230" alt="Standort"> | <img src="docs/images/funktion-info.jpg" width="230" alt="Gerät"> |

(Aufnahmen der [Live-Demo](https://pietwars.github.io/DisAHRsterRadio-MeshCore/) in Handygröße, Stand 0.9.1; alle Daten simuliert.)

- **Chat:** öffentlicher Kanal, eigene Kanäle und Direktnachrichten, Verlauf auf dem Board
- **Schnellmeldung:** Vorlagen wie Trinkwasser, Strom, Medikamente, Verletzt, Evakuierung mit Ortsangabe
- **Brett:** Schwarzes Brett über einen MeshCore-Room-Server; Beitritt automatisch mit dem öffentlichen Gast-Passwort `hello` (lesen und schreiben)
- **Karte:** Offline-Karten des Ahrtals direkt aus der Firmware, Knoten und geteilte Positionen
- **Standort:** Handy-GPS oder Ortsliste, als Chat-Position oder Knoten-Standort (Advert)
- **Gerät:** Status, Funkzähler, Kontakte, Bekanntgabe, Knotenname, Funkprofil (feste Deutschland-Profile), Kanäle, Verlauf löschen – alles ohne Anmeldung
- **WLAN + Bluetooth gleichzeitig:** Auf ESP32-Geräten läuft das WLAN-Portal und parallel die Bluetooth-Verbindung für die offizielle MeshCore-App (siehe unten)

## Ehrlicher Stand (Oktober 2026)

| Punkt | Stand |
|---|---|
| Basis | MeshCore `main` nach **v1.18.0** (meshcore-dev/MeshCore), Git-Historie erhalten |
| Firmware baut | ✅ LILYGO T-Deck Plus und Heltec V2 per GitHub Actions, veröffentlicht als [Release v0.9.1](https://github.com/Pietwars/DisAHRsterRadio-MeshCore/releases/tag/v0.9.1). Die übrigen Geräte der MeshCore-Konfigurator-Liste bleiben im Code, werden aber vorerst weder im CI gebaut noch veröffentlicht |
| Auf echter Hardware getestet | ✅ **LILYGO T-Deck Plus** und ✅ **Heltec WiFi LoRa 32 V2** mit 0.9.1 (Display, Chat und Verlauf, offenes WLAN, Bluetooth mit PIN-Kopplung). Andere Boards: nicht geprüft und nicht im Release |
| Kompatibel mit anderen MeshCore-Geräten/Apps | ✅ normales MeshCore-Funkprotokoll (gleiches Funkprofil vorausgesetzt) |

## Kartenbedienung und Speicheroptimierung

Die Navigation bleibt unten in zwei Reihen erreichbar (einschließlich iPhone-Safe-Area).
Die Kartenzeichenfläche ist auf ihren Container begrenzt; Zoomtasten sind 44 × 44 Pixel groß.
Im Browser werden maximal zwei Karten zwischengespeichert. Beim schnellen Wechsel gewinnt
immer die zuletzt gewählte Karte. Gleichzeitige Statusabfragen werden vermieden.

Die Weboberfläche wird beim Bauen gzip-komprimiert und direkt aus PROGMEM ausgeliefert.
Die genauen Roh- und gzip-Größen zeigt `python tools/embed_web.py` an (derzeit rund 19,6 kB
Ersparnis gegenüber derselben unkomprimierten Seite). `/api/state` überträgt Kontakte und
Kanäle stückweise, statt dafür einen großen Gesamt-String im Geräte-RAM anzulegen.
Unter „Gerät“ erscheinen freier RAM, bisheriges RAM-Minimum und größter freier Block.
Karten, Bluetooth, Kontaktlimit und Nachrichtenwarteschlange bleiben erhalten.
Die fünf Ahrtal-Karten sind mit `tools/shrink_maps.py` vereinfacht (zusammen ca. 190 KB gzip statt 290 KB; Orte und POIs vollständig). Neu erzeugen: `python3 tools/make_map.py <DisasterMesh/maps> /tmp/roh && python3 tools/shrink_maps.py /tmp/roh web/map && python3 tools/embed_web.py`.

Regressionstest: `node tests/web_regression.cjs` (Node 22); prüft JavaScript-Syntax,
gzip-Roundtrip, CSS-Selektoren, Kartencache und überholte Ladeantworten. Dies ersetzt
keinen Safari-/Hardwaretest. Vor Freigabe auf iPhone Hoch-/Querformat, Tastatur,
Karte → Chat/Brett und auf dem Board mehrere WLAN-Clients sowie BLE parallel prüfen.

## Boards

| PlatformIO-Env | Board | Release-Paket | Flash |
|---|---|---|---|
| `dar_LilyGo_TDeck_wifi` | LILYGO T-Deck / **T-Deck Plus** (ESP32-S3, SX1262) | `DisAHRsterRadio-MeshCore-tdeck-plus-v0.9.1-paket.zip` / `…-factory.bin` | `factory.bin` an **0x0** |
| `disaster_heltec_v2_sx1276_wifi` | **Heltec WiFi LoRa 32 V2** (ESP32, SX1276) | `DisAHRsterRadio-MeshCore-heltec-v2-v0.9.1-paket.zip` / `…-factory.bin` | `factory.bin` an **0x0** (Bootloader liegt im Image bei 0x1000) |

Beide Portal-Profile nutzen `huge_app.csv` (3 MB App, kein OTA, SPIFFS ab 0x310000). Die `factory.bin` enthält Bootloader, Partitionen, Firmware und Dateisystem und **löscht beim Flashen alle Einstellungen** (Knotenidentität, Kontakte, Verlauf, Bluetooth-PIN). Update ohne Datenverlust: nur `teile/firmware.bin` an `0x10000`.

**LILYGO T-Deck Plus:** Firmwarekennung `v1.18.0-dar0.9.1-wifi`. Display und LoRa teilen sich einen SPI-Bus (Fix gegen das Hängenbleiben bei „Loading...“).

**Heltec V2:** Firmwarekennung `v1.18.0-dar0.9.1-heltecv2-wifi`. WLAN-Fix: Brownout-Detektor früh aus, Startmarken `[DIAG …]` seriell, WLAN startet zuletzt und erst 2 s nach dem Funkmodul, WLAN-Sendeleistung 8,5 dBm. Der Bootfix für klassische ESP32 (PR #5) ist **nicht** enthalten; 0.9.1 lief bei Peters Test trotzdem stabil. Der klassische ESP32 hat wenig freien Heap – falls doch eine Boot-Schleife auftritt, bitte ein serielles Log mit **115200 Baud** (erste ca. 60 s nach RST) schicken.

Weitere Profile (T3-S3, T3 V1.6.1, Repeater, Room-Server, SD-Archiv, Heltec-Diagnose) sind in `platformio.local.ini` enthalten, werden aber vorerst nicht im CI gebaut und nicht veröffentlicht.

Alle WLAN-Portal-Boards: offenes WLAN **`DisAHRsterRadio-XXXXXX`** (höchstens 4 Handys, nichts eingeben). Auf dem OLED stehen WLAN-Name und Zugangshinweis. Bei einem fehlgeschlagenen AP-Start erfolgt alle 5 s ein neuer Versuch.

## WLAN-Portal und MeshCore-App gleichzeitig (Bluetooth)

Auf ESP32-Geräten mit Bluetooth sind Web-Portal und BLE-Companion beide am selben Companion angemeldet (MeshCore `MultiSerialInterface`): Jede Antwort und jede eingehende Nachricht geht an beide. Die offizielle MeshCore-App verbindet sich per Bluetooth, während Handys im WLAN die Weboberfläche nutzen.

- **Bluetooth-Kopplung:** Ab 0.9.1 mit PIN-Kopplung (statischer Passkey, MITM-Schutz, Bonding; Standard-PIN `123456`, eigene 6-stellige PIN in der App oder per USB `set pin <6 Ziffern>`, gilt nach Neustart, dann neu koppeln); vorher Just-Works ohne Zahlencode. Alte Kopplung bei Bedarf am Handy entfernen. Mit PIN-Kopplung stehen in der MeshCore-App alle Einstellungen und Befehle zur Verfügung (wie Standard-MeshCore, inkl. Schlüssel-Export/-Import und eigener PIN). Über die Companion-Schnittstelle im offenen WLAN/Ethernet bleiben diese Befehle gesperrt (die Weboberfläche hat ihre eigenen, festen Funktionen); Details unter [Offener Zugang](docs/OFFENER_ZUGANG.md).
- **Grenzen:** Ruft die App Nachrichten aus der Warteschlange ab, sieht das Portal sie im Verlauf mit, die App aber nicht umgekehrt die Abrufe des Portals. Gleichzeitige Befehle aus App und Portal können sich in seltenen Fällen kreuzen. WLAN + Bluetooth + Webserver brauchen viel RAM; auf dem Heltec V2 (klassischer ESP32) mit 0.9.1 auf Hardware geprüft, auf anderen klassischen ESP32 nicht.
- **Speicher:** Portal-Profile nutzen die Partition `huge_app.csv` (3 MB App, kein OTA, 896 KB SPIFFS); T-Deck Plus ca. 62 % von 3 MB. Nach dem Wechsel von 0.1.0/älter einmal komplett mit `factory.bin` flashen (Partitionstabelle ändert sich, Identität wird neu erzeugt).

## So wird die Oberfläche auf MeshCore abgebildet

| Reiter | MeshCore |
|---|---|
| **Chat** | Öffentlicher Kanal, weitere Kanäle (Slots 0–7) und Direktnachrichten an bekannte Chat-Kontakte |
| **Schnellmeldung** | Vorlagen (Trinkwasser, Strom, Medikamente …) mit Ort, gesendet an Kanal oder Kontakt |
| **Brett** | MeshCore-**Room-Server**: Portal tritt automatisch mit dem öffentlichen Gast-Passwort `hello` bei, Beiträge lesen und schreiben (ohne Passwort nur lesen) |
| **Karte** | Offline-OSM-Karten (Ahrtal, Heimersheim, Dernau, Altenahr, Sinzig) aus der Firmware. Angezeigt werden Knoten mit Standort in ihrer Bekanntgabe (Advert), Positionen aus Nachrichten und die eigene Position |
| **Standort** | GPS des Handys oder Ortsliste. „Position in Chat senden“ (Format `!DP1\|lat,lon\|zeit`, kompatibel zu DisasterMesh) oder als Knoten-Standort setzen und per Advert bekanntgeben |
| **Gerät** | Status, Funkzähler, Kontakte neu lesen, Advert senden; ohne Anmeldung: Knotenname (`CMD_SET_ADVERT_NAME`), Funkprofil (`CMD_SET_RADIO_PARAMS`, nur feste Deutschland-Profile), Kanal mit Schlüssel (`CMD_SET_CHANNEL`), Verlauf löschen |

Nachrichten sind bei MeshCore auf **100 Byte** begrenzt.

## Bauen und Flashen

```bash
pip install platformio
python tools/embed_web.py                       # nach Änderungen an web/
pio run -e disaster_heltec_v2_sx1276_wifi -t upload
pio run -e disaster_heltec_v2_sx1276_wifi -t uploadfs  # nur einmal! (Dateisystem, löscht Identität)
ENVS=disaster_heltec_v2_sx1276_wifi tools/package.sh   # dist/DisAHRsterRadio-MeshCore-0.9.1-heltec-v2.zip
tools/build_device.sh LilyGo_TDeck dist                # dist/DisAHRsterRadio-MeshCore-0.9.1-LilyGo_TDeck.zip
```

Version und MeshCore-Basis stehen nur in der Datei `VERSION`; `python3 tools/version.py` überträgt sie in Oberfläche, Demo, Firmwarekennung und README (`--check` läuft im CI). Bibliotheken sind auf feste Versionen gesetzt (`tools/pin_libs.py`, CI prüft mit `--check`).

**CI und Releases** ([Actions](https://github.com/Pietwars/DisAHRsterRadio-MeshCore/actions)): „DisAHRsterRadio-Build“ (Web-Regressionstest, Versions-, Bibliotheks- und Kartenprüfung, Build von T-Deck und Heltec V2), „Alle Geräte (MeshCore-Konfigurator-Liste)“ (vorerst nur `LilyGo_TDeck` und `Heltec_v2`), „Release“ (bei jedem Tag `v*`: T-Deck und Heltec V2 bauen und prüfen; ist das Release mit hardware-getesteten Paketen schon angelegt, lädt der Lauf nichts hoch), „Live-Demo (GitHub Pages)“.

Die Weboberfläche liegt in `web/index.html`. `tools/embed_web.py` bettet die gzip-komprimierte Seite, Hintergrundbild (WebP, 38 KB) und Karten (gzip, ca. 190 KB) in `examples/companion_radio/PortalPage.h` und `PortalAssets.h` ein. Demo ohne Gerät: `web/index.html?demo=1` lokal öffnen.

## Funkprofil

**Funkprofil (alle Geräte, alle Rollen):** MeshCore-Standard-Preset **EU/UK (Narrow)** – **869,618 MHz, Bandbreite 62,5 kHz, SF8, CR 4/8**, MeshCore-Standard-Syncword (privat, 0x12). Gesetzt über die Build-Flags `LORA_FREQ=869.618`, `LORA_BW=62.5`, `LORA_SF=8`, `LORA_CR=8` in `platformio.ini` (gilt für alle 105 Geräte der Matrix und die eigenen Profile). Teilband 869,4–869,65 MHz (SRD, ETSI EN 300 220 / BNetzA): max. 500 mW ERP (27 dBm), 10 % Sendezeit; die Firmware bleibt bei der bisherigen boardsicheren Sendeleistung (nicht erhöht). Alle Knoten eines Netzes müssen dasselbe Profil nutzen – Geräte mit älterem Stand (CR 4/5) einmal in der Weboberfläche auf „EU/UK Narrow“ stellen oder per CLI `set radio 869.618,62.5,8,8`.

## Weitere Geräte im Code (derzeit nicht im Release)

Der Code für alle 105 Geräte der [MeshCore-Konfigurator](https://meshcore.co.uk/configurator/)-Liste bleibt erhalten (`tools/devices.json`, `tools/devices.upstream.json`, Envs aus `tools/gen_devices.py` → `platformio.devices.ini`; Bau einzeln mit `tools/build_device.sh <Gerät> dist`). Ab 0.9.1 werden sie aber **vorerst weder im CI gebaut noch veröffentlicht** – im Release sind nur die beiden auf Hardware getesteten Boards:

| Gerät | Chip | Web-Oberfläche | Release 0.9.1 |
|---|---|---|---|
| LILYGO T-Deck / T-Deck Plus (`LilyGo_TDeck`) | esp32s3 | ja (WLAN-Portal + Bluetooth) | `DisAHRsterRadio-MeshCore-tdeck-plus-v0.9.1-paket.zip` |
| Heltec WiFi LoRa 32 V2 (`disaster_heltec_v2_sx1276_wifi`) | esp32 | ja (WLAN-Portal + Bluetooth) | `DisAHRsterRadio-MeshCore-heltec-v2-v0.9.1-paket.zip` |

## Grenzen / offen

- Auf Hardware getestet sind nur T-Deck Plus und Heltec V2 (0.9.1); alle anderen Boards nicht
- **Offenes WLAN:** Jede Person im WLAN kann mitlesen und senden sowie Kanäle, Knotenname und Funkprofil ändern und den Verlauf löschen
- Die Brett-Funktion braucht einen erreichbaren Room-Server (z. B. Profil `disaster_t3s3_sx1262_room` bzw. `disaster_ttgo_room` im Code, nicht im Release, nicht auf Hardware geprüft)
- Positionsfreigabe: Wer einen Standort als Knoten-Standort setzt, schaltet das Teilen der Position in Adverts ein (bei 0,0 wieder aus)
- Karten haben keine Navigation; Datenstand 16.10.2026

## Lizenz und Danksagung

- **MeshCore** © Scott Powell / rippleradios.com und Mitwirkende, **MIT-Lizenz** (`license.txt`). Original-README: [README.meshcore.md](README.meshcore.md).
- WLAN-Portal, Zustell-Warteschlange, Room-/Repeater-Anpassungen: aus [DisasterMesh](https://github.com/Pietwars/DisasterMesh) (MIT).
- Weboberfläche: aus [DisAHRsterRadio](https://github.com/Pietwars/DisAHRsterRadio), vom selben Autor, hier unter MIT.
- Kartendaten © OpenStreetMap-Mitwirkende, ODbL 1.0 (`web/map/ODbL-1.0.txt`).
