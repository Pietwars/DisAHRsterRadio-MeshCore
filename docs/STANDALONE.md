# T-Deck Plus Standalone (Issue #35)

Umsetzung von [Issue #35](https://github.com/Pietwars/DisAHRsterRadio-MeshCore/issues/35): eine zusätzliche
Firmware-Variante für das **LILYGO T-Deck Plus** (mit SD-Karte und eingebautem GPS), die **ohne Weboberfläche,
ohne WLAN und ohne Webserver** auskommt. MeshCore läuft über Bluetooth; bedient wird später direkt am Gerät
über Tastatur und Trackball.

Der Ansatz aus dem Issue: ein eigenes, schlankes Target mit Kernlogik, Bluetooth sowie Display- und
Tastatursteuerung. Die Web-Schicht wird im Standalone-Build **weggelassen, nicht per Flag unterdrückt**, damit
nichts doppelt gepflegt werden muss.

> **Status: Schritt 1 von 6 (Testbuild `0.9.4-standalone-test`).** Nicht im Release, nicht im Web-Flasher,
> nicht in `tools/devices.json` oder `tools/release_boards.json`. Noch nicht auf echter Hardware geprüft.

## Schritt 1: schlankes Target `LilyGo_TDeck_standalone` (dieser Stand)

Gleiche Funktion wie der normale T-Deck-Build `dar_LilyGo_TDeck_wifi` (0.9.4), nur ohne WLAN und Weboberfläche:

- MeshCore-Companion über **PIN-Bluetooth** wie in DAR-MC: gespeicherte PIN oder Standard-PIN, Kopplung nur
  mit PIN, PIN-Anzeige während der Kopplung.
- **Minimal-Anzeige** wie bisher: Bild, Akkuzeile, Knopf „Advert senden“. Trackball kurz = Display an/aus,
  2,5 s halten = Flood-Advert.
- **Gleiche Funkregeln** wie die DAR-MC-Builds mit Weboberfläche: höchstens 10 dBm, fester Sendezeitanteil
  von 10 %, automatisch werden nur Repeater und Room-Server als Kontakt übernommen, ein ausdrücklich gesetzter
  Standort wird in Bekanntgaben geteilt. Gleiche Kanalzahl (8) und Warteschlangengröße (64).
- **Gleiche Partitionstabelle** (`huge_app.csv`): Ein T-Deck mit 0.9.4 behält beim Wechsel (nur
  `firmware.bin` an 0x10000) Kontakte, Kanäle und Einstellungen. Die Webdateien im SPIFFS bleiben liegen,
  werden aber nicht mehr benutzt. Auf einem neuen oder gelöschten Gerät wird das SPIFFS beim ersten Start
  formatiert; ein Dateisystem-Abbild wird nicht gebraucht.
- Firmwarekennung `v1.18.0-dar0.9.4-standalone-test` (folgt `VERSION`, geprüft von `tools/version.py`).

### Aufbau: Weglassen statt Abschalten

| Baustein | Aufgabe |
|---|---|
| `platformio.standalone.ini` | Env `LilyGo_TDeck_standalone`, abgeleitet vom T-Deck-BLE-Profil |
| `examples/companion_radio/Frontend.h` | Naht zwischen Companion-Kern (`main.cpp`) und optionaler Zusatzoberfläche: `mountFilesystem()`, `begin()`, `sensorsReady()`, `loop()` |
| `examples/companion_radio/WebFrontend.cpp` | Die Web-Schicht (WLAN-Zugangspunkt, `DisasterPortal`), vorher direkt in `main.cpp` |
| `examples/companion_radio/PlainFrontend.cpp` | Builds ohne Web-Schicht: nichts außer dem Companion-Kern (auch Standalone) |
| `examples/companion_radio/DarBuild.h` | Build-Arten `DISASTER_PORTAL` / `DISASTER_STANDALONE`, gemeinsame Funkregeln `DAR_MESH_POLICY` |
| `tools/check_standalone.py` | Prüft die gelinkte Firmware (siehe unten), läuft in CI |

- `main.cpp` enthält keinen WLAN- oder Portal-Code mehr. Die Web-Builds binden `WebFrontend.cpp` ein,
  `DisasterPortal.h` wird nur noch von dort eingebunden.
- Der Standalone-Build entfernt `WebFrontend.cpp` und `helpers/esp32/ESPNOWRadio.cpp` (ESP-NOW nutzt das
  WLAN-Funkteil) per `build_src_filter`. Er schließt die WLAN- und Webserver-Bibliotheken per `lib_ignore`
  aus. Bindet etwas `WiFi.h` oder `WebServer.h` ein, bricht der Build ab. So fiel `ESPNOWRadio.cpp` auf.
- `DarBuild.h` bricht mit Fehler ab, wenn ein Build `DISASTER_STANDALONE` und `DISASTER_PORTAL` gleichzeitig setzt.
- Die Funkregeln standen bisher unter `DISASTER_PORTAL`; sie stehen jetzt unter `DAR_MESH_POLICY`. Das gilt
  für beide Build-Arten, das Verhalten der Web-Builds bleibt gleich. Nur webspezifische Teile von `MyMesh` bleiben
  unter `DISASTER_PORTAL` und fehlen im Standalone-Build: Web-Echo in der App, zweiter Leser der Warteschlange.

### Nachweis: keine WLAN-/Web-Schicht in der Firmware

`tools/check_standalone.py` prüft den fertigen Build, nicht die Quelltexte:

1. **Linker-Map**: Aus `libWiFi`, `libWebServer`, `libDNSServer`, `libESPmDNS`, AsyncTCP/AsyncWebServer,
   `libnet80211`, `libpp`, `libwpa_supplicant`, `liblwip`, `libesp_netif`, `libsmartconfig`, `libespnow`,
   `WebFrontend.cpp` und `ESPNOWRadio.cpp` landet **kein Byte** in Flash, IRAM oder DRAM.
2. **Symbole** (`nm`): keine `WiFiClass`/`WiFiGenericClass`/`WebServer`/`DNSServer`/`DisasterPortal`,
   kein `esp_wifi_init`/`esp_wifi_start`/`esp_netif_init`/`esp_now_init`, keine `lwip_*`.
3. **firmware.bin**: keine eingebettete HTML-Seite, keine Portal-Texte, Kennung enthält `-standalone`.
4. ESP32-S3-Header, Flashgröße, Partitionstabelle und App-Partition wie beim normalen T-Deck-Build.

Übrig bleiben 401 Byte aus `libesp_wifi.a`, nämlich der Koexistenz-Adapter (`esp_adapter.c`) und eine
Log-Voreinstellung (`wifi_init.c`). Der Bluetooth-Controller des ESP32-S3 braucht diesen Adapter, weil WLAN
und Bluetooth dasselbe 2,4-GHz-Funkteil nutzen. Daneben gibt es ROM-Adressen und Funktionen für Funkteil und
Stromversorgung mit `wifi` im Namen (`libphy`, `libbtdm_app`), die Bluetooth ebenfalls benutzt. Ein
WLAN-Treiber oder IP-Stack ist nicht enthalten. Gegenprobe: Auf den normalen T-Deck-Build angewandt, schlägt
die Prüfung fehl, unter anderem wegen `libWiFi`, `libWebServer`, `libDNSServer`, `libnet80211`, `liblwip` und
`libwpa_supplicant`.

### Messwerte (lokal, Linkerwerte, PlatformIO espressif32 6.11.0)

| Messwert | normal `dar_LilyGo_TDeck_wifi` | Standalone | Ersparnis |
|---|---:|---:|---:|
| Flash laut Linker | 2.063.913 B | 1.365.005 B | **698.908 B (−33,9 %)** |
| `firmware.bin` | 2.064.336 B | 1.365.424 B | 698.912 B |
| Statisches RAM (.data/.bss) | 160.988 B | 135.564 B | **25.424 B (−15,8 %)** |
| Frei in App-Partition (3 MiB) | 1.081.392 B | 1.780.304 B | |
| SPIFFS-Abbild nötig | ja (Weboberfläche, Karten) | nein | |

Der normale Build wurde von `main` (f75b35c5) gebaut. Durch die Naht wird er auf diesem Branch um
180 Byte Flash größer, sein statisches RAM bleibt gleich. Der Laufzeit-Heap ist nicht gemessen. Ohne
WLAN-Zugangspunkt, IP-Stack und Webserver sollte zur Laufzeit zusätzlich deutlich mehr Heap frei sein; das
muss auf Hardware gemessen werden.

### Bauen und flashen

```bash
pio run -e LilyGo_TDeck_standalone
python3 tools/check_standalone.py --build-dir .pio/build/LilyGo_TDeck_standalone \
  --build-log <Buildlog> --output .pio/build/LilyGo_TDeck_standalone/standalone-report
```

- Update eines T-Decks mit 0.9.4: nur `firmware.bin` an **0x10000**, ohne Löschen.
- Neuinstallation: `factory.bin` an **0x0** (Bootloader, Partitionen, boot_app0, Firmware; kein SPIFFS-Abbild).
  Das überschreibt den NVS-Bereich (u. a. Bluetooth-Kopplungen); ein vorhandenes SPIFFS bleibt unberührt,
  ein leeres oder gelöschtes wird beim ersten Start formatiert.
- Zurück zum normalen Build: die normale 0.9.4-`firmware.bin` an 0x10000 schreiben. Für die
  Weboberfläche muss das SPIFFS-Abbild der 0.9.4 vorhanden sein. Wurde das SPIFFS inzwischen formatiert,
  die Neuinstallation aus dem 0.9.4-Paket nehmen.

CI baut den Standalone in einem eigenen Job `standalone` in `build.yml` (nicht bei Versions-Tags) und
führt `tools/check_standalone.py` aus. Die Dateien landen nur als Workflow-Artefakt.

### Auf Hardware zu prüfen

1. Start, Display mit Bild, Akkuzeile und Advert-Knopf; Trackball kurz/lang.
2. Kopplung mit der MeshCore-App über PIN-Bluetooth, Senden und Empfangen, Advert.
3. Update von 0.9.4 mit `firmware.bin`: Kontakte, Kanäle, Name und PIN bleiben erhalten.
4. Neuinstallation mit `factory.bin` nach vollständigem Löschen: SPIFFS wird beim ersten Start formatiert.
5. Freier und minimaler Heap im Vergleich zu 0.9.4, Akkulaufzeit ohne WLAN.

## Nächste Schritte

Jeder Schritt baut auf diesem Target auf und nutzt Kernlogik, die dafür aus der Web-Schicht in gemeinsame,
webfreie Bausteine verschoben wird. Die Weboberfläche nutzt danach dieselben Bausteine, damit nichts doppelt
gepflegt wird.
Bisher steckt diese Logik in `DisasterPortal.h`: Sendewarteschlange mit Sendepause (`DeliveryOutbox.h`),
Nachrichtenverlauf (`HistoryRecord.h`/`SlotFile.h`), Room-Server-Anmeldung und SD-Archiv (`ArchiveSd.h`).
Die Hilfsdateien sind schon eigene Header. Die Steuerung selbst muss aus der Portal-Klasse herausgelöst werden.

### Schritt 2: gemeinsamer Kern für lokales Senden und Verlauf

Voraussetzung für Schritt 3 und 4.

- **Lokaler Sender** als webfreier Baustein (`LocalSender` o. ä.). Er nutzt denselben seriellen
  Companion-Weg wie die Weboberfläche, also die Sendepause von 5 s, die Funkwarteschlange und die ACK-Auswertung.
  `WebFrontend.cpp` und ein künftiges `StandaloneFrontend.cpp` nutzen ihn beide.
- **Echo in der App** für am Gerät gesendete Nachrichten. Heute erledigt das `MyMesh::queueWebEchoForApp` unter
  `DISASTER_PORTAL`. Daraus wird ein allgemeiner „lokal gesendet“-Weg mit Herkunftsmarke, ohne Web-Bezug.
- **Verlauf** (Kanäle, Direktnachrichten, Brett) im Gerät, dieselben Datensätze wie im Web
  (`HistoryRecord`/`SlotFile`). Ohne zweiten Leser der Warteschlange: Das Standalone liest mit, statt zu entnehmen,
  damit die App weiterhin alles bekommt (gleiche Regel wie `SharedQueue.h`).
- Hosttests wie bisher (`tests/shared_queue.test.cpp` als Vorlage).

### Schritt 3: Schnellauswahl / Schnellmeldung am Gerät

Übernahme aus Draft-PR #34 (`feat/tdeck-quick-messages`, Issue #33). Der PR selbst bleibt unverändert.

- **Direkt übernehmbar**, weil webfrei: `ui-brand/QuickModel.h` (Eingabe, UTF-8-Grenze 100 Byte, Orte, Fokus),
  `ui-brand/QuickData.h` mit `tools/gen_quick_messages.py` (Vorlagen und Ahrtal-Orte aus `web/index.html`,
  CI-Abgleich), `ui-brand/TDeckQuickPage.*` (Seite für 320 × 240), die Tastaturabfrage in `UITask`
  (I2C 0x55, Zeichenmodus) und die GPS-Frische-Prüfung (`FixFreshness.h`, Baudraten-Erkennung im
  `EnvironmentSensorManager`). Dazu die Hosttests `tests/quick_model.test.cpp` und `tests/quick_page.test.cpp`.
- **Anzupassen**:
  - Im PR gilt `DAR_TDECK_QUICK` nur für `LILYGO_TDECK && DISASTER_PORTAL` und muss um `DISASTER_STANDALONE` erweitert werden.
  - `QuickBridge.h` (`deviceQuickTarget`, `deviceQuickSend`, `deviceQuickStatus`) ist im PR in `main.cpp` über
    `disaster_portal` umgesetzt. Nach dieser Naht gehört die Web-Umsetzung nach `WebFrontend.cpp`. Für das
    Standalone kommt eine zweite Umsetzung in `StandaloneFrontend.cpp` auf Basis des lokalen Senders aus Schritt 2.
  - Die Herkunftsmarke „T-Deck“ (`setPortalOriginProvider`) läuft dann über den allgemeinen Echo-Weg.
- **Hinweis zum Zusammenführen**: PR #34 ändert `main.cpp` an Stellen, die dieser Branch nach
  `WebFrontend.cpp` verschiebt. Je nach Reihenfolge der Merges müssen diese Hunks dort neu eingesetzt werden.

### Schritt 4: Brett (Room-Server)

- **Kern**: Anmeldung am Room-Server mit dem öffentlichen Gast-Passwort (`RoomAccess.h`), Merker für
  Raum, Rechte und laufende Anmeldung, Beiträge abrufen und schreiben. Heute steckt das in
  `/api/room-login` und im Sendeweg von `DisasterPortal.h`. Es wird in einen webfreien Baustein
  `RoomSession` verschoben, den Web und Standalone gemeinsam nutzen.
- **Anzeige**: Seite „Brett“ mit Raumauswahl (bekannte Room-Server), Anmeldestatus (nicht angemeldet /
  läuft / nur lesen / Schreibrecht) und Beitragsliste (neueste zuerst, wie im Web markiert). Blättern mit dem
  Trackball, Schreiben über die Tastatur (gleiche Texteingabe wie Schnellmeldung).
- **Braucht**: Verlauf aus Schritt 2, Seitenwechsel im `UITask` (Hauptseite / Schnellmeldung / Brett / Karte)
  und die Richtungen des Trackballs (bisher wird nur der Klick ausgewertet).

### Schritt 5: vereinfachte Karte

- **Daten**: Die Weboberfläche nutzt fünf Vektorkarten als `web/map/*.json.gz` (zusammen etwa 190 KB,
  © OpenStreetMap-Mitwirkende, ODbL). Ein Werkzeug erzeugt daraus aus derselben Quelle
  (`tools/make_map.py`/`tools/shrink_maps.py`) ein kompaktes Binärformat mit Linien und Punkten in
  ganzzahligen Koordinaten, vereinfacht für 320 × 240. Ablage im Flash (es sind etwa 1,7 MB frei) oder auf SD.
  Die ODbL-Namensnennung wird auf der Kartenseite angezeigt.
- **Inhalt** wie im Web, reduziert: eigener Standort (GPS), Knoten mit Standort in der Bekanntgabe, Positionen
  aus Meldungen (`!DP1`), Brett-Standorte, Apotheke/Arzt und Wasser als Punkte. Keine Navigation.
- **Bedienung**: Kartenwahl und Zoom über Tasten, Verschieben mit dem Trackball, „Ich“ und „Alles“ wie im Web.
- **Braucht**: Zeichenroutinen für Linien/Punkte auf dem ST7789 (vorhanden über Adafruit GFX), Positions-
  speicher aus dem Verlauf (Schritt 2), Trackball-Richtungen (Schritt 4). Zeichenzeit und SPI-Teilung mit
  LoRa sind auf Hardware zu prüfen.

### Schritt 6: GPS ins Mesh und SD-Karte für Konfiguration und Logs

**GPS ins Mesh**

- Das GPS ist im T-Deck-Profil schon eingebunden (`ENV_INCLUDE_GPS`). PR #34 bringt Baudraten-Erkennung und
  Frische-Prüfung.
- Daraus wird: Standort aus frischem Fix in die Bekanntgabe (MeshCore-Einstellung „GPS“ und
  `advert_loc_policy`) und Positionsmeldung `!DP1` auf Tastendruck (Schnellmeldung).
- Optional ein Intervall für automatisches Teilen, innerhalb des 10-%-Sendezeitbudgets.
- Voreinstellung: Teilen nur nach ausdrücklicher Einschaltung am Gerät. Ohne frischen Fix wird nichts gesendet,
  niemals `0,0`.

**SD-Karte**

- Die SD-Karte des T-Deck liegt nach Herstellerangaben am selben SPI-Bus wie LoRa und Display und hat einen
  eigenen CS-Pin; das ist auf Hardware zu prüfen. `ArchiveBusConfig.h` kennt bisher nur Boards mit eigenem
  SPI-Controller für SD (TTGO, T3-S3) und braucht eine T-Deck-Variante für den geteilten Bus, mit sauberer
  Bus-Übergabe zwischen LoRa, Display und SD.
- **Logs**: Nachrichtenarchiv über `ArchiveSd.h` (`DISASTER_SD`, rotierende Dateien), dazu ein schlankes
  Ereignislog (Start, Kopplung, Sendefehler, Akkustand) mit Größenbegrenzung.
- **Konfiguration**: eine Textdatei auf der Karte, beim Start gelesen (z. B. Gerätename, Kanalvorgaben, Karte,
  GPS-Teilen). Inhalt wird als nicht vertrauenswürdige Eingabe streng geprüft. Keine Schlüssel und keine
  Bluetooth-PIN im Klartext auf der Karte; die PIN bleibt in der App bzw. am Gerät einstellbar.
- Ohne Karte läuft alles weiter wie bisher; der Speicher im Gerät bleibt die Hauptquelle.

### Abschluss

Wenn alle Schritte auf Hardware geprüft sind: eigenes Release-Paket für das Standalone. Erst dann wird
entschieden, ob es in `tools/release_boards.json`, `release.yml` und den Web-Flasher aufgenommen wird.
Bis dahin bleibt es ein Testbuild.
