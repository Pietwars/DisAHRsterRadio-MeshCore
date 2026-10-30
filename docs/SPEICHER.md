# Flash- und RAM-Prüfung

**Ab 0.9.8:** Zusätzlich wird LILYGO T3-S3 v1.3 mit SX1262 als Release-Board
geprüft: ESP32-S3, **4 MiB Flash**, 327.680 Byte Linker-RAM-Budget, App ab
0x10000 und SPIFFS ab 0x310000. Der Vertrag steht unabhängig von den
PlatformIO-Vorgaben in `tools/release_boards.json`. Die folgenden Messwerte
beziehen sich weiterhin auf den ausdrücklich genannten älteren Release;
aktuelle Werte stehen im jeweiligen ZIP. [Hardwarestatus 0.9.8](RELEASE-0.9.8.md).

**Release 0.9.4** enthält die Akku-Anzeige aus PR #31 und die Flash-/Speicherprüfungen
aus PR #32, gemeinsam nach `main` übernommen. Die Akku-Testversion 0.9.4-test wurde
vom Nutzer erfolgreich erprobt. Die noch offenen Hardwaretests der finalen
Release-Dateien sind unten aufgeführt; die Berichte im jeweiligen Release-ZIP
enthalten die Messwerte der tatsächlich veröffentlichten Dateien.

Release-Boards sind derzeit **LILYGO T-Deck Plus** (`dar_LilyGo_TDeck_wifi`)
und **LILYGO T-Watch S3** (`dar_LilyGo_TWatchS3_wifi`). Die übrigen Profile im
Repository gehören nicht zum unterstützten Release. Der unabhängige Boardvertrag
in [`tools/release_boards.json`](../tools/release_boards.json) verlangt für beide
ESP32-S3 (Image-Chip-ID 9), **16 MiB Flash** und **327.680 Byte Linker-RAM-Budget**.
Die tatsächlich eingelötete Flashgröße lässt sich erst am Gerät bestätigen.

## Verbrauch je Board

Gemessen an den tatsächlich veröffentlichten **Release-0.9.4-ZIPs** aus
[Release-Workflow 39206175790](https://github.com/Pietwars/DisAHRsterRadio-MeshCore/actions/runs/39206175790),
Tag `v0.9.4`, Commit `77f9538591ec27009e90b28066764b5783211d8b`.
PlatformIO 6.2.0, espressif32 6.11.0 / Arduino-ESP32 2.0.17; vollständige
WLAN/BLE-Firmware mit Akku-Anzeige. Die veröffentlichten ZIPs wurden nochmals
heruntergeladen und einschließlich äußerer und innerer Prüfsummen geprüft.

| Messwert (Bytes) | T-Deck Plus | T-Watch S3 |
|---|---:|---:|
| Physischer Flash laut Boardvertrag | 16.777.216 | 16.777.216 |
| App-Partition | 3.145.728 | 3.145.728 |
| Flash laut Linker | 2.064.017 | 2.096.149 |
| Tatsächliche firmware.bin | 2.064.432 | 2.096.560 |
| Frei in App-Partition nach Binärdatei | 1.081.296 | 1.049.168 |
| SPIFFS-Abbild | 917.504 | 917.504 |
| Factory-Abbild | 4.128.768 | 4.128.768 |
| Statisches RAM laut Linker | 160.988 | 161.496 |
| Linker-RAM-Budget | 327.680 | 327.680 |
| RAM nach statischer Belegung | 166.692 | 166.184 |
| Nicht partitionierter Flash | 12.582.912 | 12.582.912 |

**Linker-Flashverbrauch und Dateigröße von `firmware.bin` unterscheiden sich**:
Header, Segmentausrichtung, Prüfsumme und Digest kommen zur Binärdatei hinzu.
Die Größenprüfung verwendet deshalb die echte Dateilänge. Jeder neue CI-Build
liefert `memory-report.json` und `memory-report.md` mit Firmware-Dateigröße,
App-Restplatz, SPIFFS-Abbildgröße, Flash-Endadresse, nicht partitioniertem Flash,
statischem RAM, Partitionsliste und SHA256 der geprüften Builddateien.
Die Berichte stehen im Job Summary und in den Build-Artefakten; die Release-ZIPs
enthalten sie ebenfalls. So bleiben Messwerte an den konkreten Build gebunden.

Das bestehende `huge_app.csv` nutzt nur die ersten 4 MiB: NVS ab `0x9000`,
OTA-Daten ab `0xe000`, App ab `0x10000` (3 MiB), SPIFFS ab `0x310000`
(917.504 Byte) und Coredump ab `0x3f0000` (65.536 Byte). 12 MiB bleiben nicht partitioniert. Mehr physischer Flash erhöht
die App-Partition nicht automatisch. Das Layout wird durch diese Prüfung nicht
vergrößert; damit bleiben die vorhandenen Update-Offsets und gespeicherten Daten
kompatibel. Ein einzelner großer App-Slot ermöglicht kein A/B-OTA-Update.

Die RAM-Zahlen messen statische Belegung. Sie sind **keine Messung des freien
Laufzeit-Heaps**, des minimal freien Heaps, des größten zusammenhängenden Blocks,
des Task-Stacks oder von PSRAM. Insbesondere WLAN + BLE + Display benötigen
zusätzlich dynamischen Speicher.

## Automatische Freigabeprüfung

`tools/validate_memory.py` benötigt nur Python 3 und prüft:

- ESP32-Image-Magic, Headerlänge, Segmentzahl, Flashmodus/-frequenzcodes,
  Flashgröße gegen Boardvertrag und ESP32-S3-Chip-ID; sowohl App als auch Bootloader.
- Vollständige Segmente, XOR-Prüfsumme und optionalen SHA256-Image-Digest.
  Verschlüsselte/signierte Images und zusätzliche Image-Trailer werden nicht
  stillschweigend akzeptiert; sie benötigen eine eigene Prüferweiterung.
- Binäre Partitionstabelle inklusive MD5, sofern vorhanden, Abschlussmarkierung,
  eindeutiger Labels, Ausrichtung, Größen, Überschneidungen und Flashgrenzen.
- App an `0x10000`, vorhandene SPIFFS-Partition und tatsächliche Dateigrößen.
- RAM-/Flash-Linkerwerte aus dem Buildlog gegen RAM- und App-Budget.
- Bei Release-Paketen zusätzlich jedes Teilabbild und alle Füllbereiche des
  Factory-Abbilds, einschließlich `boot_app0.bin` innerhalb der OTA-Datenpartition.

Ein fehlgeschlagener Build oder Prüfschritt erzeugt kein Release-ZIP und entfernt
ein gegebenenfalls vorhandenes altes ZIP dieses Geräts. `verify_devices.py` prüft
die Binärdateien im fertigen Paket erneut und vergleicht sie mit dem Speicherbericht.
Damit reichen passende ZIP-Prüfsummen allein nicht zur Freigabe.

Beispiel nach einem Build (Bash mit `pipefail`, damit Buildfehler erhalten bleiben):

```bash
set -o pipefail
pio run -e dar_LilyGo_TDeck_wifi | tee tdeck-build.log
pio run -e dar_LilyGo_TDeck_wifi -t buildfs
python3 tools/validate_memory.py --env dar_LilyGo_TDeck_wifi \
  --build-dir .pio/build/dar_LilyGo_TDeck_wifi --build-log tdeck-build.log \
  --output .pio/build/dar_LilyGo_TDeck_wifi/memory-report
```

Für T-Watch S3 das Environment entsprechend ersetzen. Die reguläre Paketierung
`tools/build_device.sh LilyGo_TDeck dist/devices` bzw. `LilyGo_TWatchS3`
führt dieselben Prüfungen einschließlich Factory-Abbild automatisch aus.

## Automatisierte Validierung

- 15 neue Python-Hosttests: gültige Abbilder für beide Boards, beschädigte Header,
  falsche Flashgröße/Chip-ID, abgeschnittene Segmente, falsche Digests, defekte
  Partitionstabellen, Überlappung, fehlende Partitionen, Firmware-/SPIFFS-Überlauf,
  exakte Partitionsgrenze, falsche Speicherberichte, Factory-Abbildfehler und
  fehlerhafte Release-ZIPs trotz korrekter SHA256SUMS. Ein Buildfehler darf auch
  kein altes Release-ZIP zurücklassen.
- Bestehende Web-, Flasher-, Versions-, Bibliotheks-, Karten- und Portal-Routentests.
- Bestehende C++-Tests: OpenAccess, RoomAccess, SlotFile, AutoAdd, SharedQueue und BattTrend,
  mit AddressSanitizer und UndefinedBehaviorSanitizer. LeakSanitizer ist lokal
  wegen der Ausführung unter ptrace in der Sandbox deaktiviert (`ASAN_OPTIONS=detect_leaks=0`).
- Native PlatformIO-Suites (`native`, `native_kiss_modem`): **54/54 Testfälle bestanden**.
  `test_companion_node_prefs` wird von der vorhandenen Suite übersprungen.
- Beide ESP32-S3-Release-Profile vollständig kompiliert, SPIFFS gebaut, Factory-
  und Update-Abbilder paketiert und die fertigen ZIPs erneut validiert.

### Nicht ausführbare Hardwaretests

Hier sind keine Boards per USB angeschlossen. Folgendes ist deshalb **nicht getestet**
und vor einer Hardwarefreigabe auf **beiden** Release-Boards auszuführen:

1. Mit `esptool --chip esp32s3 --port PORT flash-id` den realen 16-MiB-Chip prüfen;
   Bootlog auf erkannte Flashgröße, Partitionslayout und PSRAM kontrollieren.
2. Neuinstallation an `0x0` auf einem Testgerät: Boot, Display, Advert-Taste bzw.
   Touch und SPIFFS-Verlauf nach Neustart prüfen. Neuinstallation löscht Gerätedaten.
3. Firmware-Update an `0x10000` auf einem eingerichteten Testgerät: Identität,
   Kontakte, Kanäle, PIN und Verlauf müssen erhalten bleiben.
4. Offenes WLAN und PIN-gekoppelte MeshCore-App **gleichzeitig** nutzen: Kanal-
   und Direktnachrichten in beide Richtungen, gemeinsame Warteschlange, Reconnects,
   Kartenabruf und Flood-Advert mit zweitem MeshCore-Knoten prüfen.
5. Unter dieser Last freien/minimal freien Heap, größten freien Block, PSRAM und
   Stackreserven messen; keine Neustarts, Brownouts, Speicherfehler oder BLE-Abbrüche.
6. Akku-Zeile auf beiden Geräten prüfen: Spannung und Ladestand, auf T-Watch
   zusätzlich USB-/Ladestatus; Trend erst nach mindestens 20 Minuten ohne USB
   beurteilen. Prozent und Verbrauch am T-Deck bleiben Schätzungen aus der Spannung.

Hosttests und Compiler-Builds ersetzen diese Hardwaretests nicht.
