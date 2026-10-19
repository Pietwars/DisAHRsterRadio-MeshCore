# T-Deck Plus Standalone (Issue #35)

Umsetzung von [Issue #35](https://github.com/Pietwars/DisAHRsterRadio-MeshCore/issues/35): eine zusätzliche
Firmware-Variante für das **LILYGO T-Deck Plus** (Tastatur, Trackball, SD-Karte, eingebautes GPS), die **ohne
Weboberfläche, ohne WLAN und ohne Webserver** auskommt. MeshCore läuft über Bluetooth mit PIN (MeshCore-App);
bedient wird direkt am Gerät über Tastatur und Trackball, mit Optik und Bedienlogik der Weboberfläche.

Die Web-Schicht wird im Standalone-Build **weggelassen, nicht per Flag unterdrückt**. Lokales Senden, Verlauf,
Sendepause, App-Echo und Brett-Anmeldung liegen in webfreien Bausteinen, die Web- und Standalone-Build gemeinsam
nutzen.

> **Status: Testbuild `0.9.5-standalone-test`** (Firmwarekennung `v1.18.0-dar0.9.5-standalone-test`).
> Alle Punkte aus Issue #35 sind umgesetzt und ohne Hardware geprüft (Hosttests, Builds, Vorschaubilder).
> Nicht im Release, nicht im Web-Flasher, nicht in `tools/devices.json` oder `tools/release_boards.json`.
> **Noch nicht auf echter Hardware geprüft** (siehe [Hardwaretest](#hardwaretest-offen)).

## Bedienung

Oben steht wie in der Weboberfläche die Reiterleiste **Start · Chat · Meldung · Brett · Karte · Standort · Gerät**
(„Meldung“ = Reiter „Schnellmeldung“ der Weboberfläche). Ungelesene Nachrichten erscheinen als weinrote Zahl an
„Chat“ und „Brett“ und auf den Start-Kacheln; eine neue eingehende Nachricht schaltet das dunkle Display ein.

| Eingabe | Wirkung |
|---|---|
| Buchstaben **h c m b k s g** (außerhalb eines Textfelds) | Start, Chat, Meldung, Brett, Karte, Standort, Gerät |
| Trackball **hoch/runter** | Auswahl in Listen und Feldern; im Chat ältere/neuere Nachrichten |
| Trackball **links/rechts** | vorheriger/nächster Reiter; im Chat/Brett links = zurück zur Liste; auf der Meldung-Seite Auswahl im Feld (Vorlage, Ort, Ziel) |
| Trackball **Klick** oder **Enter** | öffnen bzw. ausführen; im Chat **senden** |
| Trackball **2,5 s halten** | Flood-Advert (wie Schritt 1) |
| **Backspace** (Del) | löscht im Textfeld; bei leerem Feld bzw. außerhalb zurück (Liste → Start) |
| Start: Trackball bewegen, dann Klick | Kachel wählen und öffnen; Klick ohne gewählte Kachel = Display an/aus (wie Schritt 1) |
| Meldung: **1–8**, **ALT+C**, **Leertaste** | Vorlage, nächstes Feld, Auswahl wechseln (wie 0.9.5-test #34) |
| Karte: **+**/**z**, **-**/**x**, **i**, **a**, **n**/Klick, Trackball | Zoom, „Ich“, „Alles“, nächste Karte, verschieben |

Eine Taste oder ein Trackball-Schritt bei dunklem Display weckt nur und sendet nie. Während der Bluetooth-Kopplung
zeigt das Display wie bisher nur die PIN-Anzeige.

## Funktionen

### 1. Grundgerüst
- Env `LilyGo_TDeck_standalone` (`platformio.standalone.ini`). `WebFrontend.cpp` und `ESPNOWRadio.cpp` fehlen per
  `build_src_filter`, WLAN-/Webserver-/DNS-Bibliotheken per `lib_ignore`; Einbinden von `WiFi.h` bricht den Build ab.
- `DarBuild.h`: Funkregeln `DAR_MESH_POLICY` (10 dBm, 10 % Sendezeit, nur Repeater/Room-Server automatisch,
  ausdrücklich gesetzter Standort in Bekanntgaben), `DAR_LOCAL_COMPANION` (lokaler Client teilt die Warteschlange
  mit der App und spiegelt eigene Sendungen in die App), `DAR_STANDALONE_UI` (Seiten am Display).
  Standalone + Portal gleichzeitig ist ein Build-Fehler.
- Gleiche Partitionstabelle wie der normale T-Deck-Build (`huge_app.csv`): ein Update von 0.9.4 mit `firmware.bin`
  an 0x10000 behält Kontakte, Kanäle, Name, PIN und den Nachrichtenverlauf (`/dr-history.bin`).
- `tools/check_standalone.py` prüft in CI die gelinkte Firmware: kein WLAN-Treiber, kein IP-Stack, kein Webserver;
  einziger Rest ist der BT/WLAN-Koexistenz-Adapter aus `libesp_wifi.a`.

### 2. Chat
- **Kanalliste und Kontaktliste** (Kanäle, dann Chat-Kontakte wie in der Weboberfläche) mit letzter Nachricht und
  Ungelesen-Zahl; Auswahl per Trackball.
- **Chatfenster** mit Verlauf der Unterhaltung: eingehend links (Name · Zeit), eigene rechts („Du“, „gesendet“ bzw.
  „zugestellt“ bei Direktnachrichten mit Bestätigung). Zeiten in deutscher Ortszeit (MEZ/MESZ).
- **Tippen** über die Tastatur, **Enter** sendet (1–100 Byte, wie die Weboberfläche), 5 s Sendepause.
- Am Gerät gesendete Nachrichten erscheinen in der **MeshCore-App** (Herkunft „T-Deck“); in der App gesendete
  Nachrichten erscheinen im Gerät.
- Bausteine: `LocalSend.h` (Frames, Sendepause, Zeitstempel), `LocalCompanion.h` (Companion-Sitzung, Verlauf,
  Ausgangsspeicher, Brett-Anmeldung, App-Spiegel). `DisasterPortal.h` (Web) erbt davon und fügt nur HTTP/WLAN hinzu.

### 3. Schnellmeldung
- Dieselben Vorlagen und Ahrtal-Orte wie die Weboberfläche (`QuickData.h`, erzeugt von
  `tools/gen_quick_messages.py`, in CI gegen `web/index.html` geprüft), übernommen aus 0.9.5-test (#33/#34):
  `QuickModel`, `TDeckQuickPage`, Tastaturabfrage (I2C 0x55, Zeichenmodus), GPS-Frische-Prüfung (`FixFreshness.h`).
- Ziele: Kanäle und Chat-Kontakte. „Mein Standort (Geräte-GPS)“ nur mit frischem Fix **und** eingeschaltetem
  „Standort teilen“; sonst ist „Ohne Standort“ vorgewählt.

### 4. Brett (Room-Server)
- Liste der Room-Server mit Anmeldestatus und neuen Beiträgen; Beitragsliste mit Autor (aus den Kontakten).
- **Enter bei leerem Feld = Beitreten** (öffentliches Gast-Passwort wie in der Weboberfläche), mit Text =
  **Aushängen** (erst nach Anmeldung mit Schreibrecht).

### 5. Karte
- Die fünf Webkarten (Ahrtal-Übersicht, Heimersheim, Dernau · Ahrweiler, Altenahr · Mayschoß · Rech, Sinzig)
  vereinfacht für 320 × 240 im Flash (`ui-brand/MapData.h`, ca. 164 KB, erzeugt von `tools/gen_device_map.py` aus
  `web/map/*.json.gz`, in CI geprüft): Wasser, Bäche/Ahr, Straßen, Bahn, Orte, Apotheken/Wasserstellen.
- Gleiche Zoomstufen wie die Webkarte; Kontakte mit Standort, Positionen aus Nachrichten und der eigene GPS-Fix.
- Kartendaten © OpenStreetMap-Mitwirkende, ODbL 1.0 (Hinweis unten rechts auf jeder Karte).

### 6. GPS ins Mesh
- Reiter **Standort**: GPS-Empfänger an/aus, **Standort teilen** an/aus (ab Werk **aus**, Wahl bleibt nach Neustart),
  „Position in Chat senden“ (`!DP1|lat,lon|Zeit` wie die Weboberfläche), „Bekanntgabe senden“.
- Nur bei eingeschaltetem Teilen **und** frischem Fix (15 s) tragen Bekanntgaben und Meldungen die GPS-Position.
  Sonst gilt der in der App gesetzte Standort (oder keiner). Ein GPS-Fix wird nie als Gerätestandort gespeichert
  (`GpsShare.h`, Speicher-Haken in `MyMesh::savePrefs`).
- T-Deck-GPS: Baudraten-Erkennung über gültige NMEA-Sätze (9600/38400/115200) aus #34, nur im Standalone.

### 7. SD-Karte
- SD am **gemeinsamen SPI-Bus** von LoRa und Display (`ArchiveBusConfig.h`, T-Deck-Zweig, `tdeck_shared_spi`),
  CS **GPIO 39** laut LilyGO-Belegung, **auf Hardware zu prüfen**. Ohne Karte läuft alles weiter.
- `/disaster/config.txt`: nur `name`, `gps_teilen`, `karte`, `log` (`DeviceConfig.h`). Zeilen, deren Schlüssel
  nach PIN, Schlüssel, Passwort o. Ä. aussieht, werden abgelehnt; es wird nie eine PIN oder ein Schlüssel
  geschrieben. „Einstellungen auf SD speichern“ im Reiter Gerät.
- `/disaster/log.txt`: Ereignisse (Start, Senden mit Ziel­art und Länge, GPS-Fix, Teilen an/aus, wenig Speicher),
  **keine Nachrichtentexte**; ab 64 KB nach `log.old.txt` rotiert.
- Wie bei den SD-Builds der Web-Familie wird der Nachrichtenverlauf zusätzlich nach `/disaster/current.bin`
  archiviert.

## Aufbau

| Datei | Aufgabe |
|---|---|
| `platformio.standalone.ini` | Env, Pins (SD-CS, Trackball), Kennung |
| `examples/companion_radio/StandaloneFrontend.cpp` | Frontend: lokaler Companion-Client, `dev::Api`, SD, GPS-Teilen |
| `examples/companion_radio/LocalCompanion.h`, `LocalSend.h` | gemeinsamer webfreier Kern (auch für `DisasterPortal.h`) |
| `examples/companion_radio/GpsShare.h`, `DeviceConfig.h` | Standort teilen, SD-Einstellungen (reine Logik) |
| `examples/companion_radio/ui-brand/StandaloneUI.*` | Seiten Start/Chat/Brett/Karte/Standort/Gerät |
| `examples/companion_radio/ui-brand/TDeckQuickPage.*`, `QuickModel.h`, `QuickData.h` | Schnellmeldung |
| `examples/companion_radio/ui-brand/Theme.h`, `Widgets.h` | Farben, Reiter, Texte des Design-Systems (CP437-Schrift) |
| `examples/companion_radio/ui-brand/ChatLog.h`, `MapView.h`, `MapData.h`, `DeviceApi.h` | Verlauf, Karte, Schnittstelle |
| `examples/companion_radio/ui-brand/UITask.*` | Tastatur, Trackball, Startseite (unter `DAR_STANDALONE_UI`) |
| `tools/standalone_preview.py` | Vorschaubilder aller Seiten ohne Hardware |

Hosttests (C++11, ASAN/UBSAN, in `open-access.yml`): `local_companion`, `quick_model`, `quick_page`, `chat_log`,
`standalone_ui` (alle Seiten, Navigation, Bildschirmgrenzen), `gps_share`, `device_config`.

## Messwerte (lokal, Linkerwerte, PlatformIO espressif32 6.11.0)

| Messwert | normal `dar_LilyGo_TDeck_wifi` (main) | Standalone | Unterschied |
|---|---:|---:|---:|
| Flash laut Linker | 2.063.913 B | 1.621.705 B | **−442.208 B (−21,4 %)** |
| `firmware.bin` | 2.064.336 B | 1.622.128 B | −442.208 B |
| Statisches RAM (.data/.bss) | 160.988 B | 153.612 B | **−7.376 B (−4,6 %)** |
| Frei in App-Partition (3 MiB) | 1.081.392 B | 1.523.600 B | |

Gegenüber Schritt 1 (1.365.005 B Flash, 135.564 B RAM) kommen die Seiten, die Karten (ca. 164 KB), SD und der
Companion-Kern dazu. Der normale T-Deck-Build wird auf diesem Stand um 748 B Flash und 112 B RAM größer
(gemeinsamer Kern, GPS-Frische-Abfrage), sein Verhalten bleibt gleich. Heap zur Laufzeit: auf Hardware messen
(Reiter Gerät und serielle `[HEAP]`-Zeilen alle 30 s).

## Bauen und flashen

```bash
pio run -e LilyGo_TDeck_standalone
python3 tools/check_standalone.py --build-dir .pio/build/LilyGo_TDeck_standalone \
  --build-log <Buildlog> --output .pio/build/LilyGo_TDeck_standalone/standalone-report
python3 tools/standalone_preview.py <Ordner>      # Vorschaubilder
```

- Update eines T-Decks mit 0.9.4: nur `firmware.bin` an **0x10000**, ohne Löschen.
- Neuinstallation: `factory.bin` an **0x0** (Bootloader, Partitionen, boot_app0, Firmware; kein SPIFFS-Abbild).
  Das überschreibt den NVS-Bereich (u. a. Bluetooth-Kopplungen und „Standort teilen“).
- Zurück zum normalen Build: die normale 0.9.4-`firmware.bin` an 0x10000; für die Weboberfläche muss das
  SPIFFS-Abbild der 0.9.4 vorhanden sein, sonst die Neuinstallation aus dem 0.9.4-Paket nehmen.

## Hardwaretest (offen)

1. Start und Anzeige (Startseite, alle Reiter, Lesbarkeit), Display an/aus, Aufwachen bei neuer Nachricht.
2. PIN-Kopplung mit der MeshCore-App; Senden/Empfangen parallel in App und Gerät.
3. Update von 0.9.4 mit `firmware.bin`: Kontakte, Kanäle, Name, PIN und Verlauf bleiben.
4. Tastatur (Zeichenmodus, Enter, Backspace, ALT+C) und Trackball-Richtungen (Pins 3/15/1/2, Schrittweite).
5. Chat und App-Spiegelung (Kanal und Direkt, „zugestellt“), Sendepause, Brett: Beitreten und Aushängen.
6. GPS-Fix (Baudraten-Erkennung), „Standort teilen“: Bekanntgabe mit/ohne Position, `!DP1`-Meldung.
7. SD-Karte: CS-Pin 39 am gemeinsamen Bus, Einhängen neben LoRa/Display, `config.txt`, `log.txt`, Archiv.
8. Heap (frei/minimal) und Akkulaufzeit im Vergleich zu 0.9.4.
