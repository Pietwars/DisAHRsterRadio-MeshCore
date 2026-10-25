# Details zu DisAHRsterRadio-MeshCore 0.9.6

MeshCore 1.18.0. Drei getrennte Varianten: T-Deck Plus Standalone, T-Deck Plus WLAN/BLE und T-Watch S3 WLAN/BLE. Die ältere Detaildokumentation bleibt als [Archiv](archiv/DETAILS-alt.md) erhalten.

## Zugang auf einen Blick

| Zugang | Aktueller Funktionsumfang |
|---|---|
| Standalone | Lokale Bedienung per Touch, Tastatur und Trackball, MeshCore-App per Bluetooth, kein WLAN/Webportal |
| WLAN | Offenes Geräte-WLAN `DisAHRsterRadio-XXXXXX`, Weboberfläche unter `http://192.168.4.1` |
| Bluetooth | PIN-geschützte MeshCore-App. Vollständige Funk-/Geräteeinstellungen setzen eine authentifizierte Verbindung voraus. Die PIN erscheint nur während der Kopplung. |
| Web-Einstellungen | Privilegierte Änderungen sind im offenen HTTP-Portal gesperrt. Die alten Aussagen zu Einstellungen ohne Anmeldung gelten nicht mehr. |

## Nachrichten und Kontakte

Chat enthält Kanäle. Privat enthält nur hinzugefügte Chat-Kontakte; Repeater, Roomserver und bloß entdeckte Knoten sind getrennt. Der Kontaktbestand kommt aus dem Funkkern und wird auch bei mehr als 64 Kontakten aktuell gelesen. Der neue private Verlauf startet einmal leer; gemeinsame Kanal-/Room-Verläufe bleiben erhalten.

## Funk

Beide Companion-Varianten verwenden denselben ursprünglichen MeshCore-Sendeweg. Frische Einstellungen verwenden EU/UK Narrow. Gespeicherte Einstellungen bleiben erhalten; Airtime-Faktor, freie treiberunterstützte Funkparameter und geräteabhängige Leistung sind über die authentifizierte App einstellbar. Zusätzliche DAR-Neustartfenster und die frühere 10-dBm-Kappe entfallen. Fehler beim Anwenden der Hardwarekonfiguration sperren weiterhin den Sender.

## SD und Gerätetest

T-Deck-SD-Zugriff mit 800 kHz, getrennte Karten-/Archivzustände, Rücklesen nach Speichern. Der Betreiber bestätigt SD-Erkennung und Laden sowie Privatversand. Eigene WLAN-/T-Watch-Gerätetests und vollständige Hardwareabnahme stehen aus. [Testschritte](SD-TEST-0.9.6.md), [Verifizierungsstand](RELEASE-0.9.6.md), [Bedienung](STANDALONE_TOUCH.md).

## Pakete und Flashen

- `DisAHRsterRadio-MeshCore-0.9.6-LilyGo_TDeck.zip`
- `DisAHRsterRadio-MeshCore-0.9.6-LilyGo_TWatchS3.zip`
- `DisAHRsterRadio-MeshCore-0.9.6-LilyGo_TDeck_standalone-test6.zip`

Der [Webflasher](https://pietwars.github.io/DisAHRsterRadio-MeshCore/flasher/) trennt Update und Neuinstallation. Update schreibt nur Firmware an `0x10000`; Neuinstallation verwendet das Factory-Abbild an `0x0` und löscht Gerätedaten. [Flash-Anleitung](FLASHEN.md), [Speicherprüfung](SPEICHER.md).
