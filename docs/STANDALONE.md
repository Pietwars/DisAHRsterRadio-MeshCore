# T-Deck Plus Standalone (Issue #35)

## Privatnachrichten und Kontakte
Der neue Reiter **PRIVAT** enthält ausschließlich im Funkkern gespeicherte persönliche
Chat-Kontakte. Kanalnachrichten stehen unter CHAT, Room-Server unter BRETT. Repeater,
Sensoren und nur entdeckte, nicht hinzugefügte Geräte erscheinen nicht unter PRIVAT.
Die bestehende automatische Aufnahme von Repeatern/Rooms bleibt unverändert.

Der Kontaktbestand wird direkt aus dem Funkkern gelesen und ungefähr einmal pro
Sekunde auf der Oberfläche aktualisiert, einschließlich neuer Kontakte, Namen und
Löschungen aus der App. Die Liste ist nicht mehr auf 64 Kontakte begrenzt. Auch lokale
Privatnachrichten und die Outbox verwenden diesen aktuellen Bestand.

Der Privatverlauf beginnt beim ersten Start dieser UI-Version leer. Vorhandene
persönliche Kontakte bleiben sichtbar. Alte private Verlaufszeilen werden nur in
dieser Ansicht ausgeblendet, nicht aus der gemeinsamen Datei gelöscht. Der Startpunkt
wird gespeichert, sodass ein normaler Neustart neue Nachrichten nicht wieder ausblendet.
Kanal- und Brettverläufe bleiben erhalten. App-Parallelbetrieb und Darstellung müssen
weiterhin auf dem realen T-Deck Plus geprüft werden.


**Aktuelle Folgeversion: standalone-test6 im Release 0.9.5.** Die neue
Touch-Bedienung, Vorschauen und Abnahme stehen unter [Touch-Bedienung](STANDALONE_TOUCH.md),
[Galerie](STANDALONE_VORSCHAU.md) und [Release und Hardwarestatus](RELEASE-0.9.5.md).
Die unten dokumentierte test5-Historie ersetzt keine Abnahme der neuen Oberfläche.

Umsetzung von [Issue #35](https://github.com/Pietwars/DisAHRsterRadio-MeshCore/issues/35): eine zusätzliche
Firmware-Variante für das **LILYGO T-Deck Plus** (Tastatur, Trackball, SD-Karte, eingebautes GPS), die **ohne
Weboberfläche, ohne WLAN und ohne Webserver** auskommt. MeshCore läuft über Bluetooth mit PIN (MeshCore-App);
bedient wird direkt am Gerät über Tastatur und Trackball, mit Optik und Bedienlogik der Weboberfläche.

Die Web-Schicht wird im Standalone-Build **weggelassen, nicht per Flag unterdrückt**. Lokales Senden, Verlauf,
Sendepause, App-Echo und Brett-Anmeldung liegen in webfreien Bausteinen, die Web- und Standalone-Build gemeinsam
nutzen.

> **Status: experimentelle [Vorabversion `standalone-test5`](https://github.com/Pietwars/DisAHRsterRadio-MeshCore/releases/tag/standalone-test5)** mit Firmwarekennung `v1.18.0-dar0.9.5-standalone-test5`; **noch nicht auf MeshCore-Hardware abgenommen**. PR #39 wurde am 20.10.2026 gemergt, die Vorabversion danach veröffentlicht. **Kein stabiles Release und kein Eintrag im Web-Flasher.**
> Oberfläche aus `Pietwars/DisAHRsterRadio`, Branch `feature/tdeck-local-display-mvp`,
> Quellstand `50d21caba97a4cbbe97f75428214fd676e51dbe6`.
> Hardwareabnahme: [Protokoll](STANDALONE-ABNAHME.md) und [offenes Issue #40](https://github.com/Pietwars/DisAHRsterRadio-MeshCore/issues/40).

## Bedienung (test5)

Beim Einschalten erscheint dasselbe vollständige Tal-und-Funk-Bootmotiv wie im Standalone-Original
(320 × 213 RGB565, aus dem Original-JPEG mit vollständigen Farbkanälen neu kodiert), für 1,8 Sekunden oder bis zur ersten Eingabe. Danach öffnet sich
**CHAT** mit der MeshCore-Kanal-/Kontaktliste. Ohne gespeicherte Touch-Kalibrierung startet zuerst
die Drei-Punkt-Kalibrierung mit anschließender Mittelkontrolle. Gespeichert wird erst nach Erfolg;
Trackball-Klick oder Esc bricht ab. Unter **SETUP → Touch neu kalibrieren** erneut aufrufbar.

Die fünf grünen Touch-Reiter **CHAT · HILFE · BRETT · KARTE · SETUP** und die dunklen Flächen entsprechen
dem Standalone-Original. Darunter bleiben MeshCore-Seitentitel, Zurück, Ungelesen-Zahl und Akku sichtbar.
**SETUP → Standort / GPS / Teilen** öffnet die GPS-Einstellungen. ALT+Backspace führt weiterhin
Unterhaltung → Liste → ergänzendes Hauptmenü; dort sind alle sechs MeshCore-Seiten erreichbar.

| Eingabe | Wirkung |
|---|---|
| Tipp auf einen Reiter | Chatliste, Schnellmeldung, Room-Server-Liste, Karte oder Gerät/Setup öffnen |
| Tipp auf CHAT/BRETT in einer Unterhaltung | zurück zur jeweiligen Liste; Entwurf bleibt dieser Unterhaltung zugeordnet |
| Trackball hoch/runter | Auswahl bzw. Verlauf blättern; auf Karte verschieben |
| Trackball links/rechts | Auswahl innerhalb der Seite; bei HILFE Vorlage/Ort/Ziel ändern; auf Karte verschieben |
| Kurzer Klick / Enter | öffnen oder senden; **Ausnahme Karte: Klick zurück, Enter nächste Karte** |
| Chat/Brett: Tipp auf **SEND** | senden bzw. bei leerem Brett-Text beitreten; Tipp ins Textfeld sendet nicht |
| Chat/Brett: vertikal wischen | Verlauf bzw. Liste blättern; kein Senden durch Wischen |
| ALT+Backspace / Esc | eine Ebene zurück, auch bei gefülltem Entwurf |
| Backspace | Zeichen löschen; ohne Textfeld bzw. bei leerem Chatfeld zurück |
| Tipp auf Statuszeile unter den Reitern, links vom Akku | eine Ebene zurück |
| HILFE: 1–8, ALT+C, Leertaste | Vorlage, nächstes Feld, Auswahl ändern; Versand über die beschrifteten Sendeknöpfe |
| Karte: `+`/`z`, `-`/`x`, `i`, `a`, `n`/Enter/Leertaste | Zoom, Zentrieren/Folgen, Übersicht, nächste Karte |
| Karte: Touch unten | **- Zoom · Zentrieren · Zoom +** |
| Karte: Trackball-Klick / Zurück-Knopf / ALT+Backspace | Karte verlassen |
| `d` außerhalb Chat-/Brett-Text und HILFE / SETUP: Display aus | Display ausschalten |
| Trackball 2,5 s halten | Flood-Advert; dies gilt auch bei dunklem Display (Hardwareprüfung offen) |

Die Karte zentriert beim Öffnen auf einen frischen GPS-Fix und folgt neuen Fixes. Manuelles Verschieben
oder `a` beendet das Folgen; `i`/Zentrieren aktiviert es erneut. Dies ist lokale Anzeige und schaltet
**Standort teilen** nicht ein. Die fünf vorhandenen OSM-Karten bleiben erhalten; die einfache
Ortsübersicht des LoRaLayer2-Originals ersetzt sie nicht.

Der erste kurze Tastendruck, Trackball-Schritt, Klick oder Tipp bei dunklem Display weckt nur.
Neue eingehende Nachrichten wecken das Display ebenfalls. Auto-Aus nach 30 Sekunden ohne Eingabe.
Während Bluetooth-Kopplung erscheint die PIN-Anzeige.

Touch verwendet `TDeckTouchRaw.h` am gemeinsamen I2C-Bus und gespeicherte affine Kalibrierung,
Abfrage alle 20 ms. Einzelfinger-Gesten 25–900 ms; Mehrfinger werden verworfen. Reiterwechsel
nur bei Beginn und Ende im selben Reiter. Normale Tipps dürfen sich höchstens 20 Pixel je Achse
bewegen; vertikale Wischgesten in Chat/Brett werden separat verarbeitet. Trackball: CHANGE-Flanken,
vier Flanken je Schritt, 350 ms Pause. Hardwareprüfung von Orientierung, Treffern und Schrittweite bleibt nötig.

ALT+Backspace benötigt den Rohmodus der Tastatur (I2C 0x55; 0x03 Matrix lesen, danach 0x04 Zeichenmodus).
Alte Tastaturfirmware kann nur Backspace liefern. Rückfallweg: Status-Zurück oder Backspace im leeren Feld.

MeshCore-spezifische Anpassungen: Kanal-/Kontaktwahl, Room-Server, Ziel/Ort/Teilen in HILFE, Akku,
SD und PIN-Bluetooth bleiben erhalten. Deshalb sind Seiteninhalte und HILFE-Kachelhöhen an MeshCore
angepasst; die Oberfläche ist kein Austausch des Funkprotokolls und keine pixelgleiche Kopie aller Seiten.

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
- Seite **Standort**: GPS-Empfänger an/aus, **Standort teilen** an/aus (ab Werk **aus**, Wahl bleibt nach Neustart),
  „Position in Chat senden“ (`!DP1|lat,lon|Zeit` wie die Weboberfläche), „Bekanntgabe senden“.
- Seite **Gerät**: Name, Firmware, Bluetooth (ohne PIN-Ziffern), Netz, Verlauf, Uhr, Funk, Speicher, SD-Karte,
  **Akku** und **Verbrauch**; Knöpfe „Kontakte neu lesen“, „Advert senden (Bekanntgabe)“, „Display aus“,
  „Einstellungen auf SD speichern“; 112-Hinweis.
- Nur bei eingeschaltetem Teilen **und** frischem Fix (15 s) tragen Bekanntgaben und Meldungen die GPS-Position.
  Sonst gilt der in der App gesetzte Standort (oder keiner). Ein GPS-Fix wird nie als Gerätestandort gespeichert
  (`GpsShare.h`, Speicher-Haken in `MyMesh::savePrefs`).
- T-Deck-GPS: Baudraten-Erkennung über gültige NMEA-Sätze (9600/38400/115200) aus #34, nur im Standalone.

### Funkparameter in 0.9.6 TEST
Die Funkparameter werden am Standalone ausschließlich in der originalen MeshCore-App
per Bluetooth geändert. Der Display-Editor samt Touch-/Tastatur-/Trackball-Eingaben entfällt.
Standard für frische Einstellungen ist **EU/UK Narrow / Switzerland**:
869,618 MHz, 62,5 kHz, SF8, CR 4/8. Bereits gespeicherte gültige Einstellungen bleiben
erhalten; nach einem Update kann das gewünschte Profil in der App gewählt werden.
Weitere App-Presets und freie, vom Funkchip unterstützte Parameter sind möglich.
Funkleistung und Duty Cycle folgen der originalen MeshCore-Basis dieses Projekts
und lassen sich über die App einstellen. Der T-Deck-Build verwendet wieder den
originalen Leistungsstandard von 22 dBm. Gespeicherte Werte bleiben erhalten.

### TX-Budget: originales MeshCore-Verhalten
Standalone nutzt die ursprüngliche MeshCore-Tokenbudget-Steuerung. Die DAR-
Sonderregeln (feste 10 %, 10-dBm-Deckel, persistentes Stundenfenster und zusätzliche
60-Minuten-Neustartsperre) sind dort deaktiviert. Die WLAN-Variante behält sie.
Die DAR-Oberfläche und die PIN-authentifizierte App-Bedienung bleiben erhalten.
Keine Aussage über tatsächliche HF-Leistung oder Hardwareabnahme.

### Lokale Einstellungen in 0.9.6 TEST
Unter **Gerät** stehen zusätzlich **Gerätenamen ändern** (mit zweiter Bestätigung) und
**SD-Konfiguration anwenden** bereit. SD-Einstellungen werden beim Start nur vorgemerkt;
der Import zeigt Name, Karte, Standortfreigabe und Log vor der Bestätigung am Gerät.
Eine eingelegte SD-Datei allein ändert keine Einstellungen mehr. Abbruch/Tabwechsel
verwirft den Namensentwurf. Namensänderung und Import sind bei aktiver BLE-Verbindung
gesperrt. Ein angenommener Name wird über den vorhandenen lokalen Companion verarbeitet;
eine SD-Kopie wird bei Bedarf separat gespeichert. Ereignislogging ist standardmäßig aus,
bis ein Import es ausdrücklich aktiviert. Dies schaltet das Nachrichtenarchiv nicht ab.
Details und verbleibende Grenzen: [lokaler Konfigurationsweg](096-LOCAL-CONFIG-DESIGN.md).

### 7. SD-Karte
- SD am **gemeinsamen SPI-Bus** von LoRa und Display (`ArchiveBusConfig.h`, T-Deck-Zweig, `tdeck_shared_spi`),
  CS **GPIO 39** laut LilyGO-Belegung, **auf Hardware zu prüfen**. Ohne Karte läuft alles weiter.
- `/disaster/config.txt`: nur `name`, `gps_teilen`, `karte`, `log` (`DeviceConfig.h`). Zeilen, deren Schlüssel
  nach PIN, Schlüssel, Passwort o. Ä. aussieht, werden abgelehnt; es wird nie eine PIN oder ein Schlüssel
  geschrieben. „Einstellungen auf SD speichern“ in Gerät.
- `/disaster/log.txt`: Ereignisse (Start, Senden mit Ziel­art und Länge, GPS-Fix, Teilen an/aus, wenig Speicher),
  **keine Nachrichtentexte**; ab 64 KB nach `log.old.txt` rotiert.
- Wie bei den SD-Builds der Web-Familie wird der Nachrichtenverlauf zusätzlich nach `/disaster/current.bin`
  archiviert.

## Aufbau

Fingerfreundliche Folgeversion: [Touch-Bedienung und Testhinweise](STANDALONE_TOUCH.md).

### Korrekturen nach Hardware-Rueckmeldung zu test5

- I2C: `TDeckBoard::begin()` setzt SDA 18 / SCL 8 vor `ESP32Board::begin()`.
  Bisher startete die Basisklasse den generischen ESP32-S3-Bus; das spaetere
  `Wire.begin(18, 8)` konnte einen bereits gestarteten Bus nicht umkonfigurieren.
  Das betrifft Tastatur (0x55) und Touch (0x5d/0x14). Der Zeichenmodus 0x04
  entspricht dem offiziellen LilyGO-Controllerprotokoll und bleibt erhalten.
- Ohne gespeicherte gueltige Touch-Kalibrierung startet nach dem Logo der
  Vier-Punkt-Dialog. Drei Kreuze bestimmen die Transformation, das vierte
  prueft sie. Sorgfaeltige Beruehrungen bis 5 Sekunden sind jetzt erlaubt;
  Fortschritt und Anleitung werden angezeigt. Speicherung im NVS
  `dar-mc-touch/cal-v1`, unabhaengig von SD. Erneut unter Geraet / Touch neu
  kalibrieren. Abbrechen behaelt die bisherigen Werte.
- Beim Oeffnen der Karte ohne frischen GPS-Fix wird nur die Ansicht auf
  Heimersheim im Ahrtal zentriert, bei St. Mauritius im Ortskern: 50.543333 N, 7.175806 E
  (50 Grad 32 Minuten 36 Sekunden N, 7 Grad 10 Minuten 32,9 Sekunden E).
  Quelle: https://de.wikipedia.org/wiki/St._Mauritius_(Heimersheim).
  Keine Aenderung an GPS-Daten, NodePrefs, Standortfreigabe oder Sendedaten;
  kein Ich-Marker ohne frischen Fix. Manuelles Verschieben bleibt moeglich.

Protokollquelle: https://github.com/Xinyuan-LilyGO/T-Deck/blob/master/examples/Keyboard_ESP32C3/Keyboard_ESP32C3.ino
Busverhalten: https://github.com/espressif/arduino-esp32/blob/2.0.17/libraries/Wire/src/Wire.cpp
Hardware-Abnahme der Korrekturen steht aus.

| Datei | Aufgabe |
|---|---|
| `platformio.standalone.ini` | Env, Pins (SD-CS, Trackball), Kennung |
| `examples/companion_radio/StandaloneFrontend.cpp` | Frontend: lokaler Companion-Client, `dev::Api`, SD, GPS-Teilen |
| `examples/companion_radio/LocalCompanion.h`, `LocalSend.h` | gemeinsamer webfreier Kern (auch für `DisasterPortal.h`) |
| `examples/companion_radio/GpsShare.h`, `DeviceConfig.h` | Standort teilen, SD-Einstellungen (reine Logik) |
| `examples/companion_radio/ui-brand/StandaloneUI.*` | Boot-Logo, Hauptmenü, Seiten Chat/Meldung/Brett/Karte/Standort/Gerät, Touch-Ziele |
| `examples/companion_radio/ui-brand/TDeckQuickPage.*`, `QuickModel.h`, `QuickData.h` | Schnellmeldung |
| `examples/companion_radio/ui-brand/Theme.h`, `Widgets.h` | Farben, Statuszeile, Knöpfe, Texte des Design-Systems (CP437-Schrift) |
| `examples/companion_radio/ui-brand/ChatLog.h`, `MapView.h`, `MapData.h`, `DeviceApi.h` | Verlauf, Karte, Schnittstelle |
| `examples/companion_radio/ui-brand/UITask.*` | Tastatur (mit ALT+Backspace-Erkennung), Trackball, Touch, Akku, PIN-Anzeige (unter `DAR_STANDALONE_UI`) |
| `examples/companion_radio/ui-brand/TDeckTouch.h` | Touch GT911 (nur Standalone) |
| `tools/standalone_preview.py` | Vorschaubilder aller Seiten ohne Hardware |

Hosttests (C++11, ASAN/UBSAN, in `open-access.yml`): `local_companion`, `quick_model`, `quick_page`, `chat_log`,
`standalone_ui` (Reiter, SEND, Wischen, Navigation, Bildschirmgrenzen), `gps_share`, `device_config`,
`tdeck_input`, `tdeck_calibration`.

## Messwerte (lokal, Linkerwerte, PlatformIO espressif32 6.11.0, Stand test3)

| Messwert | normal `dar_LilyGo_TDeck_wifi` (main) | Standalone | Unterschied |
|---|---:|---:|---:|
| Flash laut Linker | 2.063.913 B | 1.626.121 B | **−437.792 B (−21,2 %)** |
| `firmware.bin` | 2.064.336 B | 1.626.544 B | −437.792 B |
| Statisches RAM (.data/.bss) | 160.988 B | 153.916 B | **−7.072 B (−4,4 %)** |
| Frei in App-Partition (3 MiB) | 1.081.392 B | 1.519.184 B | |

Gegenüber Schritt 1 (1.365.005 B Flash, 135.564 B RAM) kommen die Seiten, die Karten (ca. 164 KB), SD und der
Companion-Kern dazu. Der normale T-Deck-Build wird auf diesem Stand um 748 B Flash und 112 B RAM größer
(gemeinsamer Kern, GPS-Frische-Abfrage), sein Verhalten bleibt gleich. Heap zur Laufzeit: auf Hardware messen
(Seite Gerät und serielle `[HEAP]`-Zeilen alle 30 s). test3 gegenüber test: +3.640 B Flash, +304 B RAM
(Boot-Logo, Hauptmenü, Touch, ALT+Backspace, Statuszeile); der normale T-Deck-Build bleibt dabei gleich groß
(2.064.661 B / 161.100 B).

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

Die folgenden Kurzpunkte sind weiterhin **nicht auf echter Hardware abgeschlossen**. Die ausführliche Prüffolge mit Ergebnisfeldern und Datensicherungs-Hinweisen steht in [STANDALONE-ABNAHME.md](STANDALONE-ABNAHME.md); Ergebnisse und reproduzierbare Fehler in [Issue #40](https://github.com/Pietwars/DisAHRsterRadio-MeshCore/issues/40) verlinken.


1. Boot-Logo (1,8 s, erste Eingabe überspringt), danach CHAT bzw. Erstkalibrierung; alle Seiten, Lesbarkeit, Akku in der
   Statuszeile und in Gerät, „Display aus“ (Gerät, Taste d) und Wecken, Aufwachen bei neuer Nachricht.
1a. Touch: Ausrichtung (Menüeinträge, Listenzeilen, Knöpfe treffen), Statuszeile = zurück, „Zurück“ auf der Karte
   per Tipp, Trackball-Klick und ALT+Backspace.
1b. **ALT+Backspace:** kann die eingebaute Tastatur den Rohmodus (0x03)? Wird ALT+Backspace auf jeder Seite als
   „zurück“ erkannt, auch in Textfeldern, und löscht Backspace allein weiterhin nur ein Zeichen?
2. PIN-Kopplung mit der MeshCore-App; Senden/Empfangen parallel in App und Gerät.
3. Update von 0.9.4 mit `firmware.bin`: Kontakte, Kanäle, Name, PIN und Verlauf bleiben.
4. Tastatur (Zeichenmodus, Enter, Backspace, ALT+C) und Trackball-Richtungen (Pins 3/15/1/2, Schrittweite).
5. Chat und App-Spiegelung (Kanal und Direkt, „zugestellt“), Sendepause, Brett: Beitreten und Aushängen.
6. GPS-Fix (Baudraten-Erkennung), „Standort teilen“: Bekanntgabe mit/ohne Position, `!DP1`-Meldung.
7. SD-Karte: CS-Pin 39 am gemeinsamen Bus, Einhängen neben LoRa/Display, `config.txt`, `log.txt`, Archiv.
8. Heap (frei/minimal) und Akkulaufzeit im Vergleich zu 0.9.4.

## Herkunft der Eingaben (test4): Bedienung aus dem erprobten T-Deck-Profil

Vollstaendiges Tal-und-Funk-Startmotiv, impulsbasierter Trackball (vier Flanken je Schritt, 350 ms Ruhezeit) und gespeicherte Touch-Kalibrierung. Beim ersten Start drei Kreuze antippen, danach die Mitte bestaetigen. Erst nach erfolgreicher Kontrolle wird gespeichert. Unter Geraet: Touch neu kalibrieren. Klick oder Esc bricht ab. Die Kalibrierung blockiert den Funkbetrieb nicht.

MeshCore-Kanaele, Kontakte und Room-Server bleiben die Funkanbindung. Vor einem Wechsel von LoRaLayer2 komplett sichern und die MeshCore-Erstinstallationsanleitung verwenden: die Partitionen sind verschieden. Ein App-Update allein ist nur bei bereits passender MeshCore-Partitionierung geeignet.

Der eigene Test4-Geraetetest fuer Eingaben, Touch, Bluetooth, Funk, GPS/Karte und Dauerbetrieb steht noch aus.

## Prüfung der Oberflächenübernahme (test5)

- Hosttests aus `open-access.yml`: insbesondere fünf Reiter, Entwurf beim Reiterwechsel,
  nur SEND sendet, Wischbewegung sendet nicht, alle Seiten innerhalb 320 × 240.
- `tools/standalone_preview.py`: nutzt nun dieselbe vollständige Bootbild-Datei und denselben
  Bootrenderer wie die Firmware; Vorschauen sind simulierte Daten, kein Hardwarebeleg.
- Auf Gerät zusätzlich prüfen: Touch aller fünf Reiter, SEND/Beitreten, Wischen, GPS-Folgen,
  Zentrieren/Zoom, Setup → Standort und Kalibrierung. Die offene Abnahme aus test4 bleibt bestehen.
- Die test3-Speichertabelle oben gilt nicht als Messung für test5. Der test5-Firmwarebuild und
  `check_standalone` waren in CI erfolgreich; die Werte für den veröffentlichten Build sind anhand
  des zugehörigen Build-/Paketberichts festzuhalten. Die eigentliche Hardwareabnahme bleibt offen.
