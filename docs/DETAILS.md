# DisAHRsterRadio-MeshCore – Details

> Dieser Branch enthält **0.9.5-test** mit der zweiten T-Deck-Seite, Tastatur und Geräte-GPS. Bedienung und offene Hardwaretests: [T-Deck-Testanleitung](TDECK-0.9.5-TEST.md). Öffentliches Release und Web-Flasher bleiben auf 0.9.4.

> Ausführliche Doku zu Version 0.9.4 (Stand 19.10.2026). Die kurze Übersicht steht in der [README](../README.md).

> **Aktueller Stand (0.9.4):** offenes WLAN `DisAHRsterRadio-XXXXXX` → `http://192.168.4.1`, MeshCore-App per Bluetooth mit PIN-Kopplung (die PIN zeigt das Display nur während der Kopplung an; eigene PIN in der App setzbar), Brett (Room-Server) mit dem öffentlichen Gast-Passwort `hello` (sendet die Weboberfläche automatisch). Volle Einstellungen in der MeshCore-App über PIN-Bluetooth; in der Weboberfläche Knotenname, Funkprofil, Kanäle und Verlauf löschen ohne Anmeldung (jede Person im offenen WLAN kann das). Das Display am Gerät zeigt das Ahrtal-Bild mit Schriftzug, die Akku-Zeile und den Knopf „Advert senden“. [Zugang auf einen Blick](#zugang-auf-einen-blick) · [Zugang, Update und Abnahme](OFFENER_ZUGANG.md)

**🔴 Live-Demo (simulierte Daten, kein Gerät nötig): https://pietwars.github.io/DisAHRsterRadio-MeshCore/**

> **Version 0.9.4** – Firmware für **LILYGO T-Deck Plus** und **LILYGO T-Watch S3**, [Release v0.9.4](https://github.com/Pietwars/DisAHRsterRadio-MeshCore/releases/tag/v0.9.4) vom 19.10.2026. Die Akku-Anzeige der Testversion wurde vom Nutzer erfolgreich erprobt; eine vollständige Hardwareabnahme der finalen Release-Dateien ist nicht dokumentiert.
> - **Akku-Zeile:** Ladestand, Spannung, bei T-Watch USB-/Ladestatus; Verbrauch und Restlaufzeit als Schätzung.
> - **Flash-/RAM-Prüfung:** Header, Partitionen und Dateigrößen werden automatisch geprüft; Berichte liegen in jedem Release-ZIP. [Messwerte und Hardwaretests](SPEICHER.md).
> - **Nachrichtenabgleich Web/App** aus 0.9.3 bleibt erhalten.
>
> Aus 0.9.2:
> - **Minimal-Anzeige:** Das Display zeigt nur ein Bild des Ahrtals bei Nacht mit dem Schriftzug DisAHRsterRadio und einen Knopf **„Advert senden“** (Flood-Advert, höchstens einer je 10 s). Nachrichten und Einstellungen nur über MeshCore-App und Weboberfläche. Details: [Anzeige und Tasten](#anzeige-und-tasten).
> - **Bluetooth-PIN nur während der Kopplung** auf dem Display.
> - **LILYGO T-Watch S3** als zweites Gerät (Seitentaste über den Power-Chip AXP2101, Touch-Knopf, glatte Schrift FreeSans).
> - **Web-Flasher** mit „Update (Einstellungen bleiben)“ und „Neuinstallation (alles löschen)“.
> - **Schnellmeldung:** „Senden“-Knopf auf schmalen Handys wieder im Bild (Weboberfläche).

**Download:** [Releases](https://github.com/Pietwars/DisAHRsterRadio-MeshCore/releases) (je Gerät ein Paket-ZIP) · **Flashen:** [Web-Flasher](https://pietwars.github.io/DisAHRsterRadio-MeshCore/flasher/) oder [Anleitung](FLASHEN.md)

## Zugang auf einen Blick

| Zugang | Aktueller Stand |
|---|---|
| WLAN `DisAHRsterRadio-XXXXXX` | **offen, ohne Passwort** – verbinden, `http://192.168.4.1` öffnen, nichts eingeben (höchstens 4 Handys je Board) |
| Bluetooth (MeshCore-App) | **PIN-Kopplung**: PIN bei der Kopplung eingeben – das Display zeigt sie nur während der Kopplung an (bitte in der App eine eigene PIN setzen), eigene 6-stellige PIN in der App setzbar (gilt nach Neustart, dann neu koppeln); alte Kopplung vorher am Handy entfernen. Mit PIN-Kopplung hat die App alle Einstellungen wie Standard-MeshCore, auch Export/Import des privaten Schlüssels |
| Schwarzes Brett (Room-Server) | öffentliches MeshCore-Standard-Gast-Passwort **`hello`**: damit lesen und schreiben. Die Weboberfläche sendet `hello` beim „Beitreten“ automatisch, niemand muss etwas eintippen. In der MeshCore-App `hello` eingeben; ohne Passwort nur lesen |
| Kontakte | automatisch nur **Repeater und Room-Server**; Chat-Kontakte und Sensoren manuell (Werkseinstellung, in der MeshCore-App über PIN-Bluetooth änderbar; dort Kontakte auch manuell hinzufügen, bearbeiten, löschen) |
| Weboberfläche | **keine Anmeldung mehr:** Knotenname, Funkprofil (feste Deutschland-Profile), Kanäle anlegen/ändern, Verlauf löschen – für jede Person im offenen WLAN. Einen Web-Admin-Bereich gibt es nicht. |

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
| Release | ✅ [v0.9.3](https://github.com/Pietwars/DisAHRsterRadio-MeshCore/releases/tag/v0.9.3) vom 18.10.2026 (Pakete, Prüfsummen, Flyer von 0.9.2 unverändert); Web-Flasher installiert 0.9.3. Vorversion [v0.9.2](https://github.com/Pietwars/DisAHRsterRadio-MeshCore/releases/tag/v0.9.2) bleibt verfügbar |
| Basis | MeshCore `main` nach **v1.18.0** (meshcore-dev/MeshCore), Git-Historie erhalten |
| Firmware baut | ✅ LILYGO T-Deck Plus und LILYGO T-Watch S3 per GitHub Actions. Weitere Geräte der MeshCore-Konfigurator-Liste bleiben im Code, werden aber weder im CI gebaut noch veröffentlicht |
| Auf echter Hardware getestet | Akku-Anzeige der Testversion 0.9.4-test vom Nutzer erfolgreich erprobt; finale Release-Dateien nicht vollständig abgenommen. 0.9.2 auf beiden Geräten von Peter getestet (18.10.2026) |
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
| `dar_LilyGo_TDeck_wifi` | **LILYGO T-Deck Plus** (ESP32-S3, SX1262) | `DisAHRsterRadio-MeshCore-0.9.5-test-LilyGo_TDeck.zip` | `factory.bin` an **0x0**, Update `firmware.bin` an **0x10000** |
| `dar_LilyGo_TWatchS3_wifi` | **LILYGO T-Watch S3** (ESP32-S3, SX1262, AXP2101) | `DisAHRsterRadio-MeshCore-0.9.5-test-LilyGo_TWatchS3.zip` | `factory.bin` an **0x0**, Update `firmware.bin` an **0x10000** |

Beide nutzen `huge_app.csv` (3 MB App, kein OTA, SPIFFS ab 0x310000). Die `factory.bin` enthält Bootloader, Partitionen, Firmware und Dateisystem und **löscht beim Flashen alle Einstellungen** (Knotenidentität, Kontakte, Verlauf, Bluetooth-PIN). Firmwarekennung `v1.18.0-dar<Version>-wifi` (z. B. `v1.18.0-dar0.9.3-wifi`).

**T-Deck Plus:** Display und LoRa teilen sich einen SPI-Bus (Fix gegen das Hängenbleiben bei „Loading...“). **T-Watch S3:** Display 240×240 mit Touch, Seitentaste über die Tasten-Interrupts des AXP2101.

Offenes WLAN **`DisAHRsterRadio-XXXXXX`** (höchstens 4 Handys, nichts eingeben). Bei einem fehlgeschlagenen AP-Start erfolgt alle 5 s ein neuer Versuch.

## Anzeige und Tasten

Oberfläche: `examples/companion_radio/ui-brand/` (für beide Geräte).

**Bild** (`logo/disahrsterradio-display.png`): das Ahrtal bei Nacht – Lichter der Orte im Tal, ein Berg vor rötlichem Nachthimmel, links eine Person auf einem Holzgerüst mit leuchtendem Funkgerät, eingezeichnete Funkwellen und Mesh-Verbindungen zwischen Punkten im Tal. Oben rechts der Schriftzug **DisAHRsterRadio** mit „MeshCore-Funknetz fürs Ahrtal“ und „Regional · Lokal · OpenSource · Frei für alle“, unten rechts „© Pietwars“. Das Bild ist ein Ausschnitt unten rechts aus dem Titelbild `docs/flyer/disahrsterradio-header.jpg`; der Schriftzug wird aus dem Himmel herausgelöst und oben rechts wieder eingesetzt. Erzeugt mit `python3 tools/gen_brand_logo.py` zusammen mit `ui-brand/BrandLogo.h` (RGB565, **320×180** auf dem T-Deck, **240×140** auf der T-Watch, volle Breite oben) und dem Flyer-Kopfbild `docs/flyer/disahrsterradio-hero.jpg`.

**Knopf** darunter (grün, weiß umrandet): „Advert senden“, darunter „Trackball 2,5 s halten“ (T-Deck) bzw. „antippen“ (T-Watch). Schrift: T-Watch S3 glatte Schriften (FreeSans 9 pt, FreeSansBold 12 pt) in voller Auflösung 240×240; T-Deck Plus die eingebaute Pixelschrift.

- Angezeigt werden nur das Bild, die Akku-Zeile (ab 0.9.4, siehe unten) und der Knopf **„Advert senden“**. Keine weiteren Seiten, keine Nachrichtenvorschau, keine Nachrichten per Tastatur oder Touch.
- Jeder Advert ist ein **Flood-Advert** (wie „Advert senden → Flood“ in der App). Danach 2 s **„Advert gesendet“** (grün); höchstens ein Advert je **10 s**, sonst „Bitte N s warten“ (orange); bei einem Fehler „Advert-Fehler“ (rot).
- Display geht nach **30 s** ohne Bedienung aus; neue Nachrichten wecken es nicht.
- **Bluetooth-PIN:** nur während einer laufenden Kopplung (Passkey-Abfrage bis Kopplung fertig/abgebrochen, höchstens 60 s; bei einer Verbindung ohne Kopplung nach 2,5 s, ebenfalls höchstens 60 s). Das Display geht dafür an und bleibt so lange an; der Knopf zeigt dann die PIN statt „Advert senden“.
- Nachrichten, Kontakte und Einstellungen nur über MeshCore-App und Weboberfläche.

| T-Deck Plus | Wirkung |
|---|---|
| Trackball kurz klicken | Display an/aus |
| Trackball **2,5 s** halten | Flood-Advert (löst bei 2,5 s aus, Display geht an) |
| Tastatur (auch Enter) | keine Funktion |
| Ein/Aus | Schiebeschalter; kein Ausschalten per Trackball |

| T-Watch S3 | Wirkung |
|---|---|
| Seitentaste kurz | Display an/aus (bei ausgeschalteter Uhr: einschalten) |
| Seitentaste ca. **2 s** halten | Flood-Advert (Tasten-Interrupt des AXP2101 bei 2 s) |
| Seitentaste ca. **6 s** halten | Uhr ausschalten (harte Abschaltung durch den AXP2101) |
| Knopf „Advert senden“ antippen | Flood-Advert |
| Antippen bei dunklem Display | nur aufwecken |

Tastenereignisse der Uhr werden alle 20 ms abgefragt und mit 300 ms entprellt.

### Akku-Zeile (ab 0.9.4)

Zwischen Bild und Knopf steht eine Statuszeile: Akkusymbol (Füllung grün ab 40 %, orange ab 15 %, sonst rot), Ladestand, Spannung und der **geschätzte Verbrauch**. Code: `ui-brand/BattTrend.h` (Rechnung, Hosttest `tests/batt_trend.test.cpp`) und `UITask::drawBattery()`.

| | LILYGO T-Deck Plus | LILYGO T-Watch S3 |
|---|---|---|
| Messbar | nur **Akkuspannung** (Spannungsteiler 1:2 an GPIO 4, ADC mit eFuse-Kalibrierung) | Power-Chip **AXP2101**: Akkuspannung, Ladestand-% (Fuel Gauge), Akku erkannt, USB-Spannung da, lädt ja/nein |
| Nicht messbar | Strom, Ladezustand, USB | **Lade-/Entladestrom** (der AXP2101 hat dafür keinen Messkanal; XPowersLib bietet Strom nur für AXP192/AXP202) |
| Ladestand | „ca. NN %“ aus der Spannung (LiPo-Kennlinie, unter Last etwas zu niedrig) | „NN %“ vom AXP2101 |
| Laden | nicht erkennbar; steigt die Spannung: „Spannung steigt (Laden?)“ | „USB: Laden“ bzw. „USB: kein Laden“ |
| Darstellung | eine Zeile, Pixelschrift, Verbrauch rechts | zwei Zeilen FreeSans 9 pt, Verbrauch zentriert darunter |

**Verbrauch = Schätzung, kein Messwert:** Ein Messpunkt pro Minute (auch bei dunklem Display; eine ADC- bzw. I2C-Abfrage), Fenster bis 60 min, lineare Regression des aus der Spannung berechneten Ladestands → „Verbr. ca. X %/h, Rest ca. Y h“ (Restlaufzeit = Ladestand jetzt ÷ Abfall pro Stunde). Erst nach **20 min** Daten, vorher „Verbrauch: Messung, noch N min“. Neu gemessen wird nach einem Spannungssprung über 60 mV zwischen zwei Messpunkten, bei USB-Wechsel (T-Watch) und beim Neustart. Unter 0,1 %/h: „Verbrauch unter 0,1 %/h“; über 99 h: „mehr als 99 h“. Ein mA-Wert wird nirgends angezeigt, weil keines der Geräte den Strom misst.

**Stromverbrauch der Anzeige selbst:** Die Akku-Zeile wird nur bei hellem Display gezeichnet – beim Einschalten und danach höchstens alle 15 s, und nur wenn sich der Text geändert hat (T-Watch: nur dann wird der Bildpuffer übertragen).

T-Deck Plus: die Akkuspannung für die MeshCore-App (`getBattMilliVolts`) nutzt jetzt ebenfalls die kalibrierte ADC-Messung (`analogReadMilliVolts`) statt Rohwert × 3,3 V / 4096.

## WLAN-Portal und MeshCore-App gleichzeitig (Bluetooth)

Auf ESP32-Geräten mit Bluetooth sind Web-Portal und BLE-Companion beide am selben Companion angemeldet (MeshCore `MultiSerialInterface`): Die offizielle MeshCore-App verbindet sich per Bluetooth, während Handys im WLAN die Weboberfläche nutzen.

- **Bluetooth-Kopplung:** immer mit PIN (statischer Passkey, MITM-Schutz, Bonding; Standard-PIN ab Werk, auf dem Display nur während der Kopplung angezeigt, eigene 6-stellige PIN in der App oder per USB `set pin <6 Ziffern>`, gilt nach Neustart, dann neu koppeln). Alte Kopplung bei Bedarf am Handy entfernen. Mit PIN-Kopplung stehen in der MeshCore-App alle Einstellungen und Befehle zur Verfügung (wie Standard-MeshCore, inkl. Schlüssel-Export/-Import und eigener PIN). Über die Companion-Schnittstelle im offenen WLAN/Ethernet bleiben diese Befehle gesperrt (die Weboberfläche hat ihre eigenen, festen Funktionen); Details unter [Offener Zugang](OFFENER_ZUGANG.md).
- **Nachrichten teilen (seit 0.9.3):** Bis 0.9.2 holte die Weboberfläche jede empfangene Nachricht jede Sekunde aus der einzigen Warteschlange des Companions (`CMD_SYNC_NEXT_MESSAGE`) und entfernte sie damit auch für die App; im Web gesendete Nachrichten sah die App nie, in der App gesendete Nachrichten fehlten im Web-Verlauf. Jetzt:
  - **Empfangene Nachrichten** (Kanal und direkt) bleiben in der Warteschlange, bis Weboberfläche **und** App sie abgeholt haben (`examples/companion_radio/SharedQueue.h`). Eine später verbundene App bekommt sie nachgeliefert. Platz für 256 Nachrichten; ist die Warteschlange voll, fallen zuerst die ältesten Nachrichten heraus, die das Web schon hat.
  - **Antworten auf Abrufe und Sendebefehle** gehen nur noch an den Anfragenden (Web oder App), damit sich die Abrufe nicht gegenseitig stören. Andere Antworten (Kontakte, Kanäle, Geräteinfo) gehen weiter an beide.
  - **In der App gesendet:** erscheint im Web-Verlauf als eigene Nachricht („Du“), ohne Hinweis auf die App.
  - **Im Web gesendet:** Das MeshCore-Protokoll hat keine Meldung „von einem anderen Gerät gesendet“. Die App bekommt die Nachricht deshalb als **empfangene** Nachricht mit Markierung: im Kanal mit Absender „<Knotenname> (Web)“, bei Direktnachrichten im Chat des Empfängers mit dem Textanfang „[von mir über Web]“. Zustellbestätigungen erscheinen dazu in der App nicht.
  - Nur der erste Sendeversuch wird geteilt (keine Doppel bei Wiederholungen). Gleichzeitige andere Befehle aus App und Portal können sich in seltenen Fällen kreuzen.
- **Grenzen sonst:** WLAN + Bluetooth + Webserver brauchen viel RAM; beide Geräte haben PSRAM.
- **Speicher:** Portal-Profile nutzen die Partition `huge_app.csv` (3 MB App, kein OTA, 896 KB SPIFFS); T-Deck Plus und T-Watch S3 je ca. 65 % von 3 MB. Kommt das Gerät von einer anderen Firmware (andere Partitionstabelle), einmal die Neuinstallation (`factory.bin` an 0x0) verwenden; dabei wird die Identität neu erzeugt.

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
tools/build_device.sh LilyGo_TDeck dist           # dist/DisAHRsterRadio-MeshCore-0.9.5-test-LilyGo_TDeck.zip
tools/build_device.sh LilyGo_TWatchS3 dist        # dist/DisAHRsterRadio-MeshCore-0.9.5-test-LilyGo_TWatchS3.zip
```

Version und MeshCore-Basis stehen nur in der Datei `VERSION`; `python3 tools/version.py` überträgt sie in Oberfläche, Demo, Firmwarekennung und README (`--check` läuft im CI). Bibliotheken sind auf feste Versionen gesetzt (`tools/pin_libs.py`, CI prüft mit `--check`).

**CI und Releases** ([Actions](https://github.com/Pietwars/DisAHRsterRadio-MeshCore/actions)): „DisAHRsterRadio-Build“ (Web-Regressionstest, Versions-, Bibliotheks- und Kartenprüfung, Build von T-Deck Plus und T-Watch S3), „Alle Geräte (MeshCore-Konfigurator-Liste)“ (vorerst nur `LilyGo_TDeck` und `LilyGo_TWatchS3`), „Release“ (bei jedem Tag `v*`: beide Geräte bauen und prüfen, Release „DisAHRsterRadio-MeshCore v<Version>“; ist das Release mit hardware-getesteten Paketen schon angelegt, lädt der Lauf nichts hoch), „Live-Demo (GitHub Pages)“.

Die Weboberfläche liegt in `web/index.html`. `tools/embed_web.py` bettet die gzip-komprimierte Seite, Hintergrundbild (WebP, 38 KB) und Karten (gzip, ca. 190 KB) in `examples/companion_radio/PortalPage.h` und `PortalAssets.h` ein. Demo ohne Gerät: `web/index.html?demo=1` lokal öffnen.

## Funkprofil

**Funkprofil (alle Geräte, alle Rollen):** MeshCore-Standard-Preset **EU/UK (Narrow)** – **869,618 MHz, Bandbreite 62,5 kHz, SF8, CR 4/8**, MeshCore-Standard-Syncword (privat, 0x12). Gesetzt über die Build-Flags `LORA_FREQ=869.618`, `LORA_BW=62.5`, `LORA_SF=8`, `LORA_CR=8` in `platformio.ini` (gilt für alle Geräte und Profile). Teilband 869,4–869,65 MHz (SRD, ETSI EN 300 220 / BNetzA): max. 500 mW ERP (27 dBm), 10 % Sendezeit; die Firmware bleibt bei der bisherigen boardsicheren Sendeleistung (nicht erhöht). Alle Knoten eines Netzes müssen dasselbe Profil nutzen – andere MeshCore-Geräte bei Bedarf in der Weboberfläche bzw. App auf „EU/UK Narrow“ stellen oder per CLI `set radio 869.618,62.5,8,8`.

## Weitere Geräte im Code (nicht im Release)

Der Code für die Geräte der [MeshCore-Konfigurator](https://meshcore.co.uk/configurator/)-Liste bleibt erhalten (`tools/devices.json`, `tools/devices.upstream.json`, Envs aus `tools/gen_devices.py` → `platformio.devices.ini`; Bau einzeln mit `tools/build_device.sh <Gerät> dist`). Sie werden weder im CI gebaut noch veröffentlicht, sind nicht auf Hardware geprüft und nutzen die normale MeshCore-Display-Oberfläche (nicht die Minimal-Anzeige).

## Grenzen / offen

- Nur LILYGO T-Deck Plus und LILYGO T-Watch S3 sind auf Hardware getestet; die übrigen Boards im Code nicht
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
