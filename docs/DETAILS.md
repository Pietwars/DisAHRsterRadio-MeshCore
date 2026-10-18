# DisAHRsterRadio-MeshCore – Details

> Ausführliche Fassung der früheren README (Stand Oktober 2026). Die kurze Übersicht steht in der [README](../README.md); Versionsangaben gelten dort.

> **Aktueller Stand (0.9.2):** offenes WLAN `DisAHRsterRadio-XXXXXX` → `http://192.168.4.1`, MeshCore-App per Bluetooth mit PIN-Kopplung (die PIN zeigt das Display nur während der Kopplung an; eigene PIN in der App setzbar), Brett (Room-Server) mit dem öffentlichen Gast-Passwort `hello` (sendet die Weboberfläche automatisch). Volle Einstellungen in der MeshCore-App über PIN-Bluetooth; in der Weboberfläche Knotenname, Funkprofil, Kanäle und Verlauf löschen ohne Anmeldung (jede Person im offenen WLAN kann das). Das Display am Gerät zeigt nur Logo und den Knopf „Advert senden“. [Zugang auf einen Blick](#zugang-auf-einen-blick) · [Zugang, Update und Abnahme](OFFENER_ZUGANG.md)

**🔴 Live-Demo (simulierte Daten, kein Gerät nötig): https://pietwars.github.io/DisAHRsterRadio-MeshCore/**

> **Version 0.9.2** – Firmware für **LILYGO T-Deck Plus** und **LILYGO T-Watch S3**. Neu gegenüber 0.9.1:
> - **Minimal-Anzeige:** Das Display zeigt nur den Schriftzug DisAHRsterRadio mit Logo und einen Knopf **„Advert senden“** (Flood-Advert, höchstens einer je 10 s). Alle übrigen Display-Seiten und die Nachrichtenbedienung am Gerät sind entfernt; Nachrichten und Einstellungen nur über MeshCore-App und Weboberfläche. Details: [Anzeige und Tasten](#anzeige-und-tasten).
> - **Bluetooth-PIN nur während der Kopplung** auf dem Display (statt dauerhaft bzw. im Wechselhinweis).
> - **LILYGO T-Watch S3** als zweites Gerät (Seitentaste über den Power-Chip AXP2101, Touch-Knopf).
> - **Schnellmeldung:** „Senden“-Knopf auf schmalen Handys wieder im Bild (Weboberfläche).

**Download:** [Releases](https://github.com/Pietwars/DisAHRsterRadio-MeshCore/releases) (je Gerät ein Paket-ZIP mit `factory.bin` an **0x0**) · **Anleitung:** [Flashen](FLASHEN.md)

## Zugang auf einen Blick

| Zugang | Aktueller Stand |
|---|---|
| WLAN `DisAHRsterRadio-XXXXXX` | **offen, ohne Passwort** – verbinden, `http://192.168.4.1` öffnen, nichts eingeben (höchstens 4 Handys je Board) |
| Bluetooth (MeshCore-App) | **PIN-Kopplung**: PIN bei der Kopplung eingeben – das Display zeigt sie nur während der Kopplung an (bitte in der App eine eigene PIN setzen), eigene 6-stellige PIN in der App setzbar (gilt nach Neustart, dann neu koppeln); alte Kopplung vorher am Handy entfernen. Mit PIN-Kopplung hat die App alle Einstellungen wie Standard-MeshCore, auch Export/Import des privaten Schlüssels. Versionen vor 0.9.1 koppelten ohne PIN („Just Works“) |
| Schwarzes Brett (Room-Server) | öffentliches MeshCore-Standard-Gast-Passwort **`hello`**: damit lesen und schreiben. Die Weboberfläche sendet `hello` beim „Beitreten“ automatisch, niemand muss etwas eintippen. In der MeshCore-App `hello` eingeben; ohne Passwort nur lesen |
| Kontakte | automatisch nur **Repeater und Room-Server**; Chat-Kontakte und Sensoren manuell (Werkseinstellung seit 0.9.1, in der MeshCore-App über PIN-Bluetooth änderbar; dort Kontakte auch manuell hinzufügen, bearbeiten, löschen) |
| Weboberfläche | **keine Anmeldung mehr:** Knotenname, Funkprofil (feste Deutschland-Profile), Kanäle anlegen/ändern, Verlauf löschen – für jede Person im offenen WLAN. Das frühere Web-Admin-Passwort (serielle Anzeige, 10 s BOOT-Taste) ist entfernt. Repeater und Room-Server haben weiterhin ihr eigenes Admin-Passwort (bei jedem Gerät eigen, nur beim ersten Start über USB angezeigt, per USB-Befehl `password <neu>` änderbar). |

Außer dem öffentlichen Standard `hello` stehen keine Passwörter in Code, Doku, Flyern oder der Live-Demo;  Details: [Offener Zugang](OFFENER_ZUGANG.md).

> ⚠️ **Flashen:** Die `factory.bin` immer an **0x0** flashen. Sie überschreibt das Dateisystem: Einstellungen, Knotenidentität, Kontakte und Verlauf sind danach gelöscht. Update ohne Datenverlust: nur `…-firmware.bin` an `0x10000`.

## Funktionen im Überblick

| Chat | Schnellmeldung | Brett |
|---|---|---|
| <img src="images/funktion-chat.jpg" width="230" alt="Chat"> | <img src="images/funktion-quick.jpg" width="230" alt="Schnellmeldung"> | <img src="images/funktion-board.jpg" width="230" alt="Brett"> |
| **Karte** | **Standort** | **Gerät** |
| <img src="images/funktion-map.jpg" width="230" alt="Karte"> | <img src="images/funktion-loc.jpg" width="230" alt="Standort"> | <img src="images/funktion-info.jpg" width="230" alt="Gerät"> |

(Aufnahmen der [Live-Demo](https://pietwars.github.io/DisAHRsterRadio-MeshCore/) in Handygröße; alle Daten simuliert.)

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
| Firmware baut | ✅ LILYGO T-Deck Plus und LILYGO T-Watch S3 per GitHub Actions. Weitere Geräte der MeshCore-Konfigurator-Liste bleiben im Code, werden aber weder im CI gebaut noch veröffentlicht |
| Auf echter Hardware getestet | ✅ LILYGO T-Deck Plus und LILYGO T-Watch S3 mit 0.9.2 (von Peter, 18.10.2026); T-Deck Plus vorher mit 0.9.1 (Weboberfläche, Bluetooth mit PIN) |
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
Die fünf Ahrtal-Karten sind mit `tools/shrink_maps.py` vereinfacht (zusammen ca. 190 KB gzip statt 290 KB; Orte und POIs vollständig). Neu erzeugen: `python3 tools/make_map.py <GeoJSON-Ordner> /tmp/roh && python3 tools/shrink_maps.py /tmp/roh web/map && python3 tools/embed_web.py`.

Regressionstest: `node tests/web_regression.cjs` (Node 22); prüft JavaScript-Syntax,
gzip-Roundtrip, CSS-Selektoren, Kartencache und überholte Ladeantworten. Dies ersetzt
keinen Safari-/Hardwaretest. Vor Freigabe auf iPhone Hoch-/Querformat, Tastatur,
Karte → Chat/Brett und auf dem Board mehrere WLAN-Clients sowie BLE parallel prüfen.

## Boards

| PlatformIO-Env | Board | Release-Paket | Flash |
|---|---|---|---|
| `dar_LilyGo_TDeck_wifi` | **LILYGO T-Deck Plus** (ESP32-S3, SX1262) | `DisAHRsterRadio-MeshCore-0.9.2-LilyGo_TDeck.zip` | `factory.bin` an **0x0**, Update `firmware.bin` an **0x10000** |
| `dar_LilyGo_TWatchS3_wifi` | **LILYGO T-Watch S3** (ESP32-S3, SX1262, AXP2101) | `DisAHRsterRadio-MeshCore-0.9.2-LilyGo_TWatchS3.zip` | `factory.bin` an **0x0**, Update `firmware.bin` an **0x10000** |

Beide nutzen `huge_app.csv` (3 MB App, kein OTA, SPIFFS ab 0x310000). Die `factory.bin` enthält Bootloader, Partitionen, Firmware und Dateisystem und **löscht beim Flashen alle Einstellungen** (Knotenidentität, Kontakte, Verlauf, Bluetooth-PIN). Firmwarekennung `v1.18.0-dar0.9.2-wifi`.

**T-Deck Plus:** Display und LoRa teilen sich einen SPI-Bus (Fix gegen das Hängenbleiben bei „Loading...“). **T-Watch S3:** Display 240×240 mit Touch, Seitentaste über die Tasten-Interrupts des AXP2101.

Offenes WLAN **`DisAHRsterRadio-XXXXXX`** (höchstens 4 Handys, nichts eingeben). Bei einem fehlgeschlagenen AP-Start erfolgt alle 5 s ein neuer Versuch.

## Anzeige und Tasten

Oberfläche: `examples/companion_radio/ui-brand/` (für beide Geräte). Bild: `logo/disahrsterradio-display.png` (Ausschnitt unten rechts aus dem Titelbild `docs/flyer/disahrsterradio-header.jpg` mit Ahrtal, Berg und Lichtern; der Schriftzug wird oben rechts wieder eingesetzt), erzeugt mit `python3 tools/gen_brand_logo.py` zusammen mit `ui-brand/BrandLogo.h` (320×180 T-Deck, 240×140 T-Watch) und dem Flyer-Kopfbild `docs/flyer/disahrsterradio-hero.jpg`. Die T-Watch nutzt die volle Auflösung 240×240 und für Texte glatte Schriften (FreeSans), der T-Deck die eingebaute Pixelschrift.

- Angezeigt werden nur Schriftzug + Logo und der Knopf **„Advert senden“**. Keine weiteren Seiten, keine Nachrichtenvorschau, keine Tastatur-/Touch-Nachrichten, kein WLAN/PIN-Wechselhinweis.
- Jeder Advert ist ein **Flood-Advert** (wie „Advert senden → Flood“ in der App). Danach 2 s **„Advert gesendet“**; höchstens ein Advert je **10 s**, sonst „Bitte N s warten“.
- Display geht nach **30 s** ohne Bedienung aus; neue Nachrichten wecken es nicht.
- **Bluetooth-PIN:** nur während einer laufenden Kopplung (Passkey-Abfrage bis Kopplung fertig/abgebrochen, höchstens 60 s; bei einer Verbindung ohne Kopplung nach 2,5 s, ebenfalls höchstens 60 s). Das Display geht dafür an.

| T-Deck Plus | Wirkung |
|---|---|
| Trackball kurz klicken | Display an/aus |
| Trackball **2,5 s** halten | Flood-Advert (löst bei 2,5 s aus, Display geht an) |
| Tastatur (auch Enter) | keine Funktion |
| Ein/Aus | Schiebeschalter; kein Abschalten per Trackball |

| T-Watch S3 | Wirkung |
|---|---|
| Seitentaste kurz | Display an/aus (bei ausgeschalteter Uhr: einschalten) |
| Seitentaste ca. **2 s** halten | Flood-Advert (Tasten-Interrupt des AXP2101 bei 2 s) |
| Seitentaste ca. **6 s** halten | harte Abschaltung durch den AXP2101 (Sicherheitsfunktion; vorher 4 s) – 2 s schaltet nicht aus |
| Knopf „Advert senden“ antippen | Flood-Advert |
| Antippen bei dunklem Display | nur aufwecken |

Tastenereignisse der Uhr werden alle 20 ms abgefragt und mit 300 ms entprellt.

## WLAN-Portal und MeshCore-App gleichzeitig (Bluetooth)

Auf ESP32-Geräten mit Bluetooth sind Web-Portal und BLE-Companion beide am selben Companion angemeldet (MeshCore `MultiSerialInterface`): Jede Antwort und jede eingehende Nachricht geht an beide. Die offizielle MeshCore-App verbindet sich per Bluetooth, während Handys im WLAN die Weboberfläche nutzen.

- **Bluetooth-Kopplung:** Ab 0.9.1 mit PIN-Kopplung (statischer Passkey, MITM-Schutz, Bonding; Standard-PIN ab Werk, auf dem Display nur während der Kopplung angezeigt, eigene 6-stellige PIN in der App oder per USB `set pin <6 Ziffern>`, gilt nach Neustart, dann neu koppeln); vorher Just-Works ohne Zahlencode. Alte Kopplung bei Bedarf am Handy entfernen. Mit PIN-Kopplung stehen in der MeshCore-App alle Einstellungen und Befehle zur Verfügung (wie Standard-MeshCore, inkl. Schlüssel-Export/-Import und eigener PIN). Über die Companion-Schnittstelle im offenen WLAN/Ethernet bleiben diese Befehle gesperrt (die Weboberfläche hat ihre eigenen, festen Funktionen); Details unter [Offener Zugang](OFFENER_ZUGANG.md).
- **Grenzen:** Ruft die App Nachrichten aus der Warteschlange ab, sieht das Portal sie im Verlauf mit, die App aber nicht umgekehrt die Abrufe des Portals. Gleichzeitige Befehle aus App und Portal können sich in seltenen Fällen kreuzen. WLAN + Bluetooth + Webserver brauchen viel RAM; beide Geräte haben PSRAM.
- **Speicher:** Portal-Profile nutzen die Partition `huge_app.csv` (3 MB App, kein OTA, 896 KB SPIFFS); T-Deck Plus und T-Watch S3 je ca. 65 % von 3 MB. Nach dem Wechsel von 0.1.0/älter einmal komplett mit `factory.bin` flashen (Partitionstabelle ändert sich, Identität wird neu erzeugt).

## So wird die Oberfläche auf MeshCore abgebildet

| Reiter | MeshCore |
|---|---|
| **Chat** | Öffentlicher Kanal, weitere Kanäle (Slots 0–7) und Direktnachrichten an bekannte Chat-Kontakte |
| **Schnellmeldung** | Vorlagen (Trinkwasser, Strom, Medikamente …) mit Ort, gesendet an Kanal oder Kontakt |
| **Brett** | MeshCore-**Room-Server**: Portal tritt automatisch mit dem öffentlichen Gast-Passwort `hello` bei, Beiträge lesen und schreiben (ohne Passwort nur lesen) |
| **Karte** | Offline-OSM-Karten (Ahrtal, Heimersheim, Dernau, Altenahr, Sinzig) aus der Firmware. Angezeigt werden Knoten mit Standort in ihrer Bekanntgabe (Advert), Positionen aus Nachrichten und die eigene Position |
| **Standort** | GPS des Handys oder Ortsliste. „Position in Chat senden“ (Format `!DP1\|lat,lon\|zeit`) oder als Knoten-Standort setzen und per Advert bekanntgeben |
| **Gerät** | Status, Funkzähler, Kontakte neu lesen, Advert senden; ohne Anmeldung: Knotenname (`CMD_SET_ADVERT_NAME`), Funkprofil (`CMD_SET_RADIO_PARAMS`, nur feste Deutschland-Profile), Kanal mit Schlüssel (`CMD_SET_CHANNEL`), Verlauf löschen |

Nachrichten sind bei MeshCore auf **100 Byte** begrenzt.

## Bauen und Flashen

```bash
pip install platformio
python tools/embed_web.py                         # nach Änderungen an web/
pio run -e dar_LilyGo_TDeck_wifi -t upload        # bzw. dar_LilyGo_TWatchS3_wifi
pio run -e dar_LilyGo_TDeck_wifi -t uploadfs      # nur einmal! (Dateisystem, löscht Identität)
tools/build_device.sh LilyGo_TDeck dist           # dist/DisAHRsterRadio-MeshCore-0.9.2-LilyGo_TDeck.zip
tools/build_device.sh LilyGo_TWatchS3 dist        # dist/DisAHRsterRadio-MeshCore-0.9.2-LilyGo_TWatchS3.zip
```

Version und MeshCore-Basis stehen nur in der Datei `VERSION`; `python3 tools/version.py` überträgt sie in Oberfläche, Demo, Firmwarekennung und README (`--check` läuft im CI). Bibliotheken sind auf feste Versionen gesetzt (`tools/pin_libs.py`, CI prüft mit `--check`).

**CI und Releases** ([Actions](https://github.com/Pietwars/DisAHRsterRadio-MeshCore/actions)): „DisAHRsterRadio-Build“ (Web-Regressionstest, Versions-, Bibliotheks- und Kartenprüfung, Build von T-Deck Plus und T-Watch S3), „Alle Geräte (MeshCore-Konfigurator-Liste)“ (vorerst nur `LilyGo_TDeck` und `LilyGo_TWatchS3`), „Release“ (bei jedem Tag `v*`: beide Geräte bauen und prüfen, Release „DisAHRsterRadio-MeshCore v<Version>“; ist das Release mit hardware-getesteten Paketen schon angelegt, lädt der Lauf nichts hoch), „Live-Demo (GitHub Pages)“.

Die Weboberfläche liegt in `web/index.html`. `tools/embed_web.py` bettet die gzip-komprimierte Seite, Hintergrundbild (WebP, 38 KB) und Karten (gzip, ca. 190 KB) in `examples/companion_radio/PortalPage.h` und `PortalAssets.h` ein. Demo ohne Gerät: `web/index.html?demo=1` lokal öffnen.

## Funkprofil

**Funkprofil (alle Geräte, alle Rollen):** MeshCore-Standard-Preset **EU/UK (Narrow)** – **869,618 MHz, Bandbreite 62,5 kHz, SF8, CR 4/8**, MeshCore-Standard-Syncword (privat, 0x12). Gesetzt über die Build-Flags `LORA_FREQ=869.618`, `LORA_BW=62.5`, `LORA_SF=8`, `LORA_CR=8` in `platformio.ini` (gilt für alle Geräte und Profile). Teilband 869,4–869,65 MHz (SRD, ETSI EN 300 220 / BNetzA): max. 500 mW ERP (27 dBm), 10 % Sendezeit; die Firmware bleibt bei der bisherigen boardsicheren Sendeleistung (nicht erhöht). Alle Knoten eines Netzes müssen dasselbe Profil nutzen – Geräte mit älterem Stand (CR 4/5) einmal in der Weboberfläche auf „EU/UK Narrow“ stellen oder per CLI `set radio 869.618,62.5,8,8`.

## Weitere Geräte im Code (nicht im Release)

Der Code für die Geräte der [MeshCore-Konfigurator](https://meshcore.co.uk/configurator/)-Liste bleibt erhalten (`tools/devices.json`, `tools/devices.upstream.json`, Envs aus `tools/gen_devices.py` → `platformio.devices.ini`; Bau einzeln mit `tools/build_device.sh <Gerät> dist`). Sie werden weder im CI gebaut noch veröffentlicht, sind nicht auf Hardware geprüft und nutzen die normale MeshCore-Display-Oberfläche (nicht die Minimal-Anzeige).

## Grenzen / offen

- 0.9.2 und die T-Watch S3 sind noch nicht auf Hardware abgenommen; andere Boards nicht geprüft
- Am Gerät selbst keine Nachrichten (nur Advert-Knopf); alles Weitere über App oder Weboberfläche
- **Offenes WLAN:** Jede Person im WLAN kann mitlesen und senden sowie Kanäle, Knotenname und Funkprofil ändern und den Verlauf löschen
- Die Brett-Funktion braucht einen erreichbaren Room-Server (z. B. Profil `disaster_t3s3_sx1262_room` bzw. `disaster_ttgo_room` im Code, nicht im Release, nicht auf Hardware geprüft)
- Positionsfreigabe: Wer einen Standort als Knoten-Standort setzt, schaltet das Teilen der Position in Adverts ein (bei 0,0 wieder aus)
- Karten haben keine Navigation; Datenstand 16.10.2026

## Lizenz und Danksagung

- **MeshCore** © Scott Powell / rippleradios.com und Mitwirkende, **MIT-Lizenz** (`license.txt`). Original-README: [README.meshcore.md](../README.meshcore.md).
- WLAN-Portal, Zustell-Warteschlange, Room-/Repeater-Anpassungen: von Pietwars, MIT.
- Weboberfläche: aus [DisAHRsterRadio](https://github.com/Pietwars/DisAHRsterRadio), vom selben Autor, hier unter MIT.
- Kartendaten © OpenStreetMap-Mitwirkende, ODbL 1.0 (`web/map/ODbL-1.0.txt`).
