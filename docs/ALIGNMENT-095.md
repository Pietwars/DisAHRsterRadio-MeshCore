# Entwicklungsabgleich 2026-10-22

Arbeitszweig in allen vier Repositories: `dev/align-disahr-095`.
Technische MeshCore-Vorlage: **DisAHRsterRadio-MeshCore 0.9.5**, nicht eine offizielle MeshCore-Version.
Kein Merge, Tag oder Release ohne ausdrückliche Freigabe. Hardwareabnahme der neuen HaldenFunk-Builds offen.

## Bestandsaufnahme

| Repository | Funkprotokoll | Entwicklungsstand | Ausgangscommit | PIO-Profile |
|---|---|---|---|---:|
| DisAHRsterRadio-MeshCore | MeshCore | 0.9.5 | `354a01df9242` | 838 |
| HaldenFunk-MeshCore | MeshCore | 0.9.5-dev | `ddd55a09cc0d` | 836 |
| DisAHRsterRadio | LoRaLayer2 | 0.9.4 | `115695a0d5ef` | 14 |
| HaldenFunk | LoRaLayer2 | 0.9.4-dev | `00299634c7e4` | 14 |

Die 0.9.5-MeshCore-Basis ist bereits in DisAHRsterRadio-MeshCore vorhanden. Bestehende Implementierungen
werden nicht erneut angelegt. HaldenFunk-MeshCore setzt auf den vier vorhandenen Commits bis `ddd55a09` auf.
DisAHRsterRadio (LoRaLayer2) steht auf 0.9.4 mit eigener T-Deck-UI und separaten Omi-Testprofilen.
HaldenFunk übernimmt diesen LoRaLayer2-Kern über `region/CORE` und das regionale Overlay, nicht den MeshCore-Stack.

## Protokolle und Hardware

- MeshCore: native Companion-Rahmen, vorhandene EU-Narrow-Einstellungen und T-Deck-Pins.
- LoRaLayer2: eigene Nachrichten-/Routingimplementierung unter `firmware/lib/LoRaLayer2`.
- Keine Übernahme von MeshCore-Transportcode in LoRaLayer2.
- Bestehende Boardprofile, Flashgrößen, Funkchips und Partitionen bleiben erhalten. Neue Standalone-Profile
  nutzen die jeweils vorhandene gerätespezifische Basis. Die SPI/I2C-Korrekturen der DAR-T-Deck-Vorlage
  werden im HaldenFunk-MeshCore-T-Deck übernommen (gemeinsamer SPI-Bus, SD-CS, kein zweiter Wire.begin).

## HaldenFunk-MeshCore-Standalone

Ziel: `HaldenFunk_TDeck_standalone`. Touch mit persistenter Kalibrierung, Tastatur/Trackball, Chat,
Schnellmeldungen, Brett, Hauptmenü, Akku/Display, GPS-Freigabe und SD-Einstellungen/Verlauf aus der Vorlage.
WLAN/Web-Schicht wird nicht in dieses Ziel gelinkt. Bluetooth bleibt HaldenFunk-spezifisch ohne PIN;
Verwaltungsbefehle bleiben eingeschränkt und private Schlüssel werden nicht an offene Funkverbindungen verteilt.
Kupfer/Anthrazit, bestehendes HaldenFunk-Bootmotiv, Karten und Orte aus dem Mansfelder Land.
Die DAR-Oberfläche bleibt unverändert. Kartengenerator unterstützt Bindestriche; SD-Kartenkennungen bis 31 Zeichen.

## HaldenFunk-LoRaLayer2

Eigenes Ziel `tdeck-standalone` aus der vorhandenen LoRaLayer2-Vorlage. Eigene Kupfer/Anthrazit-Farben,
Haldenmotiv und regionale Orte. Die Funktionen entsprechen dieser Vorlage: keine Gleichsetzung mit
MeshCore-Standalone, insbesondere keine Behauptung einer vollständig implementierten OSM-Vektorkarte.
Bestehender LittleFS-vor-HTTP-Startablauf bleibt erhalten.

## Prüfstand

Builds und Hosttests sind in Bearbeitung. Ergebnisse werden vor Abschluss dieses Arbeitslaufs ergänzt.
Für Hosttests in der hiesigen Sandbox bleibt AddressSanitizer/UBSan aktiv; LeakSanitizer muss wegen
fehlendem Zugriff auf `/proc/.../task` deaktiviert werden (`ASAN_OPTIONS=detect_leaks=0`).
CI verwendet die vollständigen Sanitizer-Einstellungen auf GitHub-Runnern.

## Hardwareabnahme offen

Touch-Ausrichtung/Kalibrierung, ALT+Backspace, Trackball, Bootlogo-Timing, Akkumessung,
Display aus/Wakeup, Senden/Empfangen zwischen realen Geräten, BLE-Kopplung/Zugriffsgrenzen,
GPS-Fix/Freigabe sowie SD-Karte und gemeinsamer SPI-Bus sind am jeweiligen Gerät zu prüfen.
Ein Hosttest oder erfolgreicher Build ersetzt diese Abnahme nicht.
