# T-Deck-Teststand 0.9.5-test

Umsetzung von [Issue #33](https://github.com/Pietwars/DisAHRsterRadio-MeshCore/issues/33):
die Schnellmeldungsansicht der Handy-Weboberfläche als zweite Seite auf dem
**LILYGO T-Deck Plus**, mit eingebauter Tastatur und Geräte-GPS. Die Anordnung
ist an 320 × 240 Pixel angepasst. Release und Web-Flasher bleiben auf **0.9.4**;
dieser Branch ist ein Teststand und wird nicht automatisch veröffentlicht.

## Bedienung am T-Deck

Bei ausgeschaltetem Display weckt der erste Tastendruck nur auf und sendet nichts.

| Taste / Aktion | Wirkung |
|---|---|
| `M` auf der Hauptseite | Schnellmeldungsseite öffnen |
| `Alt+C` | Nächstes Feld wählen (orange Markierung) |
| `Enter` oder Trackball kurz | Ausgewählte Aktion ausführen |
| `1`–`8` im Vorlagenfeld (Ziffern per `SYM`) | Vorlage übernehmen, Texteingabe öffnen |
| Textfeld | Zeichen eingeben; Backspace löscht ein vollständiges Zeichen |
| `Enter` im Textfeld | Zur Ortsauswahl wechseln; sendet noch nichts |
| Leertaste im Orts-/Zielfeld | Nächsten Ort bzw. nächstes Sendeziel wählen |
| `(` / `)` im Orts-/Zielfeld (per `SYM`) | Vorherige/nächste Auswahl |
| Feld „Leeren“ + Enter | Text vollständig leeren |
| Feld „Zurück“ + Enter oder Backspace außerhalb des Textfelds | Zur Hauptseite zurück |
| Trackball 2,5 s halten | Wie bisher Flood-Advert senden und Hauptseite zeigen |

Die Felder werden in dieser Reihenfolge durchlaufen: Vorlagen, Text, Ort, Ziel,
Meldung senden, Position senden, Zurück, Leeren. Standard-Tastaturfirmware hat
keine eigene Tab-/Escape-Taste; `Alt+C` liefert den vom Hersteller vorgesehenen
Steuercode. Tab/Escape werden zusätzlich unterstützt, falls eine alternative
Tastaturfirmware sie liefert. Texteingabe verwendet die ASCII-Zeichen der
Originaltastatur; Umlaute aus den Vorlagen bleiben als UTF-8 im Meldungstext erhalten.

## Gleiche Meldungen wie im Web

Die acht Vorlagen sind Trinkwasser, Strom/Laden, Medikamente, Verletzt,
Evakuierung, Mir geht es gut, Ich biete Hilfe und Strom vorhanden. Kategorien
und Texte sowie die zwölf Ahrtal-Orte werden mit `tools/gen_quick_messages.py`
direkt aus `web/index.html` erzeugt. CI prüft die Übereinstimmung.

Text, Ort und Ziel werden wie im Web vor dem Senden gewählt. Vorgabe ist Kanal 0;
andere eingerichtete Kanäle und Chat-Kontakte lassen sich auswählen. Die Ziel-ID
wird zusammen mit dem angezeigten Namen festgehalten: Eine zwischenzeitliche
Kontaktaktualisierung darf eine Meldung nicht an einen anderen Empfänger umleiten.
Fehlende oder mehrdeutige Kontakte werden beim Senden abgelehnt.

Maximal **100 UTF-8-Bytes einschließlich Standortzusatz**. Zu lange Meldungen
werden abgelehnt und nicht abgeschnitten. Leertaste und Feldwechsel senden keine
Meldung; nur Enter/Trackball auf der jeweiligen Sendefläche tut das.

## Position aus dem Geräte-GPS

„Mein Standort“ ist voreingestellt und verwendet den **GPS-Empfänger des T-Deck**.
Das Öffnen der Seite schaltet ein erkanntes GPS über dieselbe Einstellung wie
in der MeshCore-App ein; diese Einstellung wird gespeichert.

- Beim Start werden die konfigurierte Baudrate sowie 9600, 38400 und 115200
  geprüft (Dopplungen ausgelassen). Erkannt wird GPS erst anhand neuer,
  prüfsummengültiger NMEA-Sätze. Es werden keine Konfigurationen ins GPS geschrieben.
  Ohne erkannten Empfänger kann dies bis ungefähr neun Sekunden zusätzlich dauern.
- Die Anzeige unterscheidet fehlenden Empfänger, ausgeschaltetes GPS, Suche und
  frischen Fix; die Satellitenzahl wird angezeigt.
- Gesendet wird nur mit gültigen Koordinaten und einem neuen gültigen GGA-/RMC-Fix,
  dessen Eingang höchstens **15 Sekunden** zurückliegt. Direkt vom GPS-Provider
  lesen verhindert die Verwendung des älteren Advert-Standortcaches.
- Ohne Fix wird weder `0,0` noch eine gespeicherte GPS-Position als aktueller
  Standort gesendet. „Position senden“ verwendet das gleiche `!DP1`-Format wie im Web.
- Ein ausdrücklich aus der Liste gewählter Ort erscheint als **„manuell“**.
  Er kann auch ohne GPS verwendet werden. „Ohne Standort“ erlaubt eine reine
  Schnellmeldung; „Position senden“ braucht dann weiterhin einen frischen GPS-Fix.
- Eine gültige Gerätezeit ist erforderlich. GPS synchronisiert sie nach dem Fix;
  alternativ kann die MeshCore-App die Zeit setzen.

Die Herstellerbeispiele bestätigen Tastaturadresse 0x55, SDA18/SCL8 und die
unterschiedlichen GPS-Varianten:
[Tastatur](https://github.com/Xinyuan-LilyGO/T-Deck/blob/master/examples/Keyboard_ESP32C3/Keyboard_ESP32C3.ino),
[GPS](https://github.com/Xinyuan-LilyGO/T-Deck/blob/master/examples/GPSShield/GPSShield.ino).

## Senden und Kompatibilität

Das Gerät verwendet die vorhandene serialisierte Portal-/Companion-Sendestrecke.
Die globale Sendepause von fünf Sekunden, Funkwarteschlange und vorhandene
MeshCore-ACK-Verarbeitung bleiben wirksam. WLAN und PIN-Bluetooth laufen weiter.
Gerätesendungen werden im Web-Verlauf gespeichert und in der App als lokale
Nachrichten mit Markierung **„T-Deck“** gespiegelt. Normale Web-Sendungen behalten
ihre Markierung „Web“.

„Vorgemerkt“ ist noch keine Sendebestätigung. „Funkkern angenommen“ ist noch keine
Zustellbestätigung. Bei unklarer Antwort erfolgt kein automatischer Wiederholversuch;
wiederholtes Enter auf dem unveränderten Entwurf erzeugt keine zweite Sendung.
Die bisherige Hauptseite mit Akku und Advert bleibt erhalten. T-Watch erhält
keine Schnellmeldungsseite.

## Test und Flashen

Für ein Gerät mit 0.9.4 und gleicher Partitionierung nur die passende
`…-LilyGo_TDeck-firmware.bin` an **0x10000** schreiben, ohne Erase. Die Factory-Datei
an 0x0 ist für Neuinstallationen und überschreibt Gerätedaten. Nicht den weiterhin
auf 0.9.4 stehenden Web-Flasher für diesen Test verwenden.

Hosttests prüfen UTF-8-/Byte-Grenzen, gültige und veraltete Positionen, Zeitüberlauf,
Frameformate, konkrete Tastatureingaben, Empfänger-Snapshots, ausdrückliches Senden
und die Zeichenpositionen innerhalb des Displays. Beide Board-Profile werden gebaut
und mit Flash-/RAM-Bericht paketiert.

**Noch auf echter Hardware zu prüfen:**

1. Startlog: Tastatur erkannt und gültiges NMEA bei einer gefundenen Baudrate.
2. `M`, `Alt+C`, `SYM`-Ziffern, Texteingabe, Löschen, Zurück und Trackball testen.
3. Im Freien Fix abwarten und Koordinaten mit dem tatsächlichen Standort vergleichen.
4. GPS-Daten aussetzen lassen: nach 15 Sekunden darf die GPS-Position nicht mehr
   gesendet werden. Fehlenden Empfänger und manuelle Ortsauswahl getrennt testen.
5. Schnellmeldung und `!DP1`-Position auf einem zweiten MeshCore-Knoten empfangen;
   gleichzeitig WLAN und BLE verwenden, Web-Verlauf und App-Spiegelung kontrollieren.
6. Freien/minimalen Heap, LoRa/Display-SPI-Verhalten, Akkuanzeige und Reconnects unter
   Last beobachten. T-Watch auf unveränderte Bedienung prüfen.

## Geprüfter Build

Beide ZIPs einschließlich Factory-Abbild, ESP32-S3-Header (16 MiB),
Partitionstabelle und Firmwaregrenzen erfolgreich geprüft. Statische RAM-Werte
sind Linkerwerte; freier Heap zur Laufzeit muss auf Hardware gemessen werden.

| Board | Firmwaredatei | App-Platz frei | statisches RAM / Budget |
|---|---:|---:|---:|
| LilyGo_TDeck | 2,074,160 B | 1,071,568 B | 161,428 / 327,680 B |
| LilyGo_TWatchS3 | 2,096,432 B | 1,049,296 B | 161,504 / 327,680 B |

54 native MeshCore-Tests bestanden; zusätzlich Hosttests für Schnellmeldungsmodell
und echte Seitenlogik mit ASAN/UBSAN sowie Web-, Versions- und Speicherprüfungen.
Hardwaretests aus der obigen Liste sind nicht ausgeführt.
