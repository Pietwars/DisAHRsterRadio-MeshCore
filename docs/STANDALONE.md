# T-Deck Plus Standalone (Issue #35)

Umsetzung von [Issue #35](https://github.com/Pietwars/DisAHRsterRadio-MeshCore/issues/35): eine zusätzliche
Firmware-Variante für das **LILYGO T-Deck Plus** (Tastatur, Trackball, SD-Karte, eingebautes GPS), die **ohne
Weboberfläche, ohne WLAN und ohne Webserver** auskommt. MeshCore läuft über Bluetooth mit PIN (MeshCore-App);
bedient wird direkt am Gerät über Tastatur und Trackball, mit Optik und Bedienlogik der Weboberfläche.

Die Web-Schicht wird im Standalone-Build **weggelassen, nicht per Flag unterdrückt**. Lokales Senden, Verlauf,
Sendepause, App-Echo und Brett-Anmeldung liegen in webfreien Bausteinen, die Web- und Standalone-Build gemeinsam
nutzen.

> **Status: Testbuild `0.9.5-standalone-test4`** (Firmwarekennung `v1.18.0-dar0.9.5-standalone-test4`; test3 =
> Boot-Logo, Hauptmenü statt Reiterleiste, Trackball nur innerhalb einer Seite, ALT+Backspace = zurück, Touch
> überall, Akku und Seitentitel in einer schmalen Statuszeile oben).
> Alle Punkte aus Issue #35 sind umgesetzt und ohne Hardware geprüft (Hosttests, Builds, Vorschaubilder).
> Nicht im Release, nicht im Web-Flasher, nicht in `tools/devices.json` oder `tools/release_boards.json`.
> **Noch nicht auf echter Hardware geprüft** (siehe [Hardwaretest](#hardwaretest-offen)).

## Bedienung

**Ablauf:** Beim Einschalten zeigt das Display nur das **DisAHRsterRadio-Logo** mit Firmwarekennung und
112-Hinweis, für 2,5 s oder bis zur ersten Taste, Trackball-Bewegung, zum ersten Klick oder Tipp (diese erste
Eingabe überspringt nur das Logo). Danach erscheint das **Hauptmenü** mit den Einträgen **Chat, Meldung, Brett,
Karte, Standort, Gerät** („Meldung“ = Schnellmeldung der Weboberfläche). Jeder Eintrag zeigt eine Kurzinfo
(z. B. „neue Nachrichten“, GPS/Teilen, Akku) und ungelesene Nachrichten als weinrote Zahl.

Es gibt **keine Reiterleiste und keine Buchstaben-Kürzel** für die Menüwahl mehr. Oben steht auf jeder Seite eine
schmale **Statuszeile**: „← Seitentitel“ links, die Zahl ungelesener Nachrichten („2 neu“) und rechts der
**Akkustand** (Symbol und Prozent, grün ab 40 %, gelb ab 15 %, sonst rot). Eine neue eingehende Nachricht
schaltet das dunkle Display ein.

| Eingabe | Wirkung |
|---|---|
| Trackball **hoch/runter** (in Listen auch **links/rechts**) | Auswahl innerhalb der Seite: Menüeinträge, Kanäle/Kontakte, Knöpfe; im Chat ältere/neuere Nachrichten; auf der Meldung-Seite Felder (links/rechts: Vorlage, Ort, Ziel wechseln) |
| Trackball auf der **Karte** | Karte verschieben |
| Trackball **Klick** oder **Enter** | gewählten Eintrag öffnen bzw. ausführen; im Chat **senden**; auf der Karte **„Zurück“** |
| Trackball **2,5 s halten** | Flood-Advert (wie Schritt 1) |
| **ALT+Backspace** | **eine Ebene zurück, auf jeder Seite**, auch aus Textfeldern und von der Karte: Unterhaltung → Liste → Hauptmenü; Karte, Meldung, Standort, Gerät → Hauptmenü. Der Entwurf bleibt bei seiner Unterhaltung. Esc (externe Tastatur) wirkt gleich. |
| **Backspace** allein | löscht im Textfeld ein Zeichen. Bei leerem Feld und auf Seiten ohne Textfeld geht er ebenfalls eine Ebene zurück (Rückfallweg für Tastaturen ohne ALT-Erkennung, siehe unten) |
| **Tippen** (Touch) | Menüeintrag, Listenzeile oder Knopf auswählen und ausführen; Tipp auf das Eingabefeld = senden; Tipp auf die **Statuszeile** (links vom Akku) = zurück |
| **d** (außerhalb von Textfeldern, nicht auf der Meldung-Seite) | Display aus |
| Meldung: **1–8**, **ALT+C**, **Leertaste** | Vorlage, nächstes Feld, Auswahl wechseln (wie 0.9.5-test #34) |
| Karte: **+**/**z**, **-**/**x**, **i**, **a**, **n**/Enter/Leertaste | Zoom, „Ich“, „Alles“, nächste Karte |
| Karte: Knopf **„← Zurück“** oben links | Tippen, Trackball-Klick oder ALT+Backspace: zurück |

**Karte verlassen:** Auf der Karte verschiebt der Trackball nur die Karte. Damit man nie festsitzt, ist der Knopf
**„← Zurück“** oben links immer orange markiert: ein **Trackball-Klick** löst ihn aus (die nächste Karte wählt man
mit **n**, Enter oder Leertaste), ebenso ein **Tipp** darauf oder auf die Statuszeile, **ALT+Backspace** oder
Backspace. Die Fußzeile sagt „Trackball: verschieben · Klick/ALT+Del: Zurück“.

**ALT+Backspace erkennen:** Die T-Deck-Tastatur (LilyGO-Firmware, I2C 0x55, Zeichenmodus) sendet für Backspace mit
und ohne ALT denselben Code 0x08; ALT allein sendet nichts. Kommt 0x08, fragt die Firmware deshalb sofort die
Tastenmatrix ab (Rohmodus-Befehl 0x03, danach zurück in den Zeichenmodus 0x04): sind ALT (Spalte 0, Zeile 4)
und Backspace (Spalte 4, Zeile 3) gleichzeitig gedrückt, gilt das als „zurück“. Den Rohmodus gibt es in der
Tastatur-Firmware von LilyGO ab Juni 2025; ältere Tastaturen melden dann nur Backspace. Dafür bleibt der
Rückfallweg: Backspace bei leerem Feld bzw. auf Seiten ohne Textfeld geht zurück, ebenso ein Tipp auf die
Statuszeile. **Hardwaretest:** ob die eingebaute Tastatur den Rohmodus kann und ALT+Backspace erkannt wird.

Was früher auf der Startseite stand:

| Früher (Start) | Jetzt |
|---|---|
| Akkuzeile (Ladestand, Spannung, Verbrauch/Restlaufzeit) | Prozent in der Statuszeile auf jeder Seite; Ladestand, Spannung und Verbrauch als Zeilen „Akku“ und „Verbrauch“ in **Gerät**; Kurzinfo im Hauptmenü |
| „Advert senden“ (Trackball 2,5 s) | Trackball **2,5 s halten** wie bisher, zusätzlich Knopf **„Advert senden (Bekanntgabe)“** in Gerät (und „Bekanntgabe senden“ in Standort) |
| Klick = Display an/aus | Knopf **„Display aus“** in Gerät oder Taste **d**; jede Taste, jeder Trackball-Schritt, Klick oder Tipp weckt; nach 30 s ohne Bedienung geht es wie bisher von selbst aus |

Eine Taste, ein Trackball-Schritt oder ein Tipp bei dunklem Display weckt nur und sendet nie. Während der
Bluetooth-Kopplung zeigt das Display wie bisher nur die PIN-Anzeige (Logo, Akkuzeile, PIN).

**Touch** (T-Deck Plus, GT911 am I2C-Bus, `ui-brand/TDeckTouch.h`): Menüeinträge, Listenzeilen, Knöpfe in Standort
und Gerät, Eingabefeld, alle Felder und Knöpfe der Meldung-Seite, „Zurück“ der Karte und die Statuszeile. Der
Touch wird alle 30 ms abgefragt; fehlt er, wird alle 5 s erneut gesucht. Die Ausrichtung (x = Roh-y,
y = 240 − Roh-x, wie in den LilyGO-Beispielen) ist **auf Hardware zu prüfen**.

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
`standalone_ui` (alle Seiten, Navigation, Bildschirmgrenzen), `gps_share`, `device_config`.

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

1. Boot-Logo (2,5 s, erste Eingabe überspringt), danach Hauptmenü; alle Seiten, Lesbarkeit, Akku in der
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

## Test4: Bedienung aus dem erprobten T-Deck-Profil

Vollstaendiges Tal-und-Funk-Startmotiv, impulsbasierter Trackball (vier Flanken je Schritt, 350 ms Ruhezeit) und gespeicherte Touch-Kalibrierung. Beim ersten Start drei Kreuze antippen, danach die Mitte bestaetigen. Erst nach erfolgreicher Kontrolle wird gespeichert. Unter Geraet: Touch neu kalibrieren. Klick oder Esc bricht ab. Die Kalibrierung blockiert den Funkbetrieb nicht.

MeshCore-Kanaele, Kontakte und Room-Server bleiben die Funkanbindung. Vor einem Wechsel von LoRaLayer2 komplett sichern und die MeshCore-Erstinstallationsanleitung verwenden: die Partitionen sind verschieden. Ein App-Update allein ist nur bei bereits passender MeshCore-Partitionierung geeignet.

Der eigene Test4-Geraetetest fuer Eingaben, Touch, Bluetooth, Funk, GPS/Karte und Dauerbetrieb steht noch aus.
