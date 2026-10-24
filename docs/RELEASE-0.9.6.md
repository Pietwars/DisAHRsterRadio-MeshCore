# Öffentlicher Release 0.9.6

Regulärer öffentlicher Release, vom Projektbetreiber ausdrücklich beauftragt.

## Änderungen

Beide Companion-Varianten verwenden die ursprüngliche MeshCore-Sendepolitik. Der Airtime-Faktor ist konfigurierbar; zusätzliche DAR-Fenster, Neustartsperre und 10-dBm-Kappe entfallen. Frische Einstellungen verwenden EU/UK Narrow; gespeicherte Werte bleiben erhalten. Die authentifizierte Bluetooth-App darf die treiberunterstützten Funkparameter setzen. Das offene HTTP-Portal erhält keine zusätzlichen Einstellrechte.

Standalone und Weboberfläche haben einen Privatbereich für hinzugefügte Chat-Kontakte. Keine Repeater, Roomserver oder nur entdeckten Knoten. Die Quelle ist der aktuelle gespeicherte Kontaktbestand. Der private Verlauf startet pro Variante einmal leer; gemeinsame Kanal- und Room-Verläufe bleiben erhalten.

SD: 800-kHz-Takt am T-Deck, getrennte Karten-/Archivzustände, erneuter Mountversuch beim expliziten Speichern nach fehlgeschlagenem Start und bytegenaues Rücklesen nach dem Schreiben. Keine automatische Formatierung.

## Verifizierungsgrenzen

Der Betreiber meldete am T-Deck Plus funktionierenden Privatversand und nach der SD-Korrektur funktionierende Erkennung sowie Laden. Ein getrennt protokollierter Schreib-/Langzeittest liegt nicht vor. Die neue WLAN/BLE-Integration und T-Watch benötigen eigene Gerätetests. Simulatorvorschauen, Hosttests und Firmware-Builds sind keine Hardwareabnahme. [SD-Gerätetest](SD-TEST-0.9.6.md).

Downloads: T-Deck Plus WLAN/BLE, T-Watch S3 WLAN/BLE sowie T-Deck Plus Standalone als getrennte Pakete. Die technische Standalone-Dateikennung `standalone-test6` bleibt aus Kompatibilitätsgründen bestehen. Update und Neuinstallation sind im Webflasher getrennt; Neuinstallation löscht Gerätedaten.
