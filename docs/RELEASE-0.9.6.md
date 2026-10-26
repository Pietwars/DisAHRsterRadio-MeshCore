# Öffentlicher Release 0.9.6

Regulärer öffentlicher Release, vom Projektbetreiber ausdrücklich beauftragt.

## Aktueller Entwicklungsstand (26.10.2026)

Der Integrationsbranch `update/096-current` kombiniert SOS aus #57 und
DE-Sendeschutz aus #58 auf Basis der bereits integrierten Fixes #54–56.
Die Versionskennung bleibt 0.9.6; Commit und SHA-256 unterscheiden diesen
Teststand vom unveränderten veröffentlichten Release. Downloads und Webflasher
verweisen weiterhin auf die bisherigen Releasebytes.

- [SOS-Bedienung](SOS.md): zweistufiger Start, verschlüsselte Direktnachricht,
  maximal vier Übergaben in 30 Minuten, Abbruch, getrennte ACK/Annahme.
- [DE-Sendeschutz](DE-TX-GUARD.md): Band-/Leistungsprüfung, 10-%-Stundenbudget,
  Fehler-/Timeout-Abrechnung und persistenter Neustartschutz für beide Frontends.
  Gespeicherte 22 dBm sperren TX; höchstens 10 dBm in der authentifizierten App
  wählen. Unbekannte Historie verlangt eine volle stille Stunde; Neustart beginnt
  sie erneut. SOS erhält keine Ausnahme und kann innerhalb seiner Frist ablaufen.
- Outbox-Speicherabsicherung, SHA-256-Prüfung der tatsächlich geflashten Bytes
  und Offline-Cache der HTTPS-Vorschau sind bereits integriert.
- Die BLE-Zugriffstests für Standalone und Portal bestehen lokal mit UBSan.
  nRF52/RP2040 melden keine bestätigte Frame-Authentifizierung und erhalten über
  den gemeinsamen Transportmanager keine privilegierten Befehle. T-Deck/T-Watch
  verwenden ESP32; alte Bonds, Wiederverbindung und Rechteentzug bleiben Gerätetests.

Gemeinsame CI-Ergebnisse sind dem konkreten Integrationscommit zuzuordnen.
Die bisherigen Einzel-PR-Ergebnisse ersetzen diese Prüfung nicht. Neue SOS-
und HF-Abnahme sowie Zuordnung der Dauerlauf-/Recovery-Nachweise bleiben offen;
#47 wird durch diese Integration nicht geschlossen. Frühere Nutzerabnahmen
bleiben anerkannt, werden aber nicht auf neue SOS-/Funkfunktionen übertragen.

## Veröffentlichter Stand vom 24.10.2026

Die folgenden Angaben beschreiben ausschließlich die bisherigen Releasepakete.

### Änderungen

Beide Companion-Varianten verwenden die ursprüngliche MeshCore-Sendepolitik. Der Airtime-Faktor ist konfigurierbar; zusätzliche DAR-Fenster, Neustartsperre und 10-dBm-Kappe entfallen. Frische Einstellungen verwenden EU/UK Narrow; gespeicherte Werte bleiben erhalten. Die authentifizierte Bluetooth-App darf die treiberunterstützten Funkparameter setzen. Das offene HTTP-Portal erhält keine zusätzlichen Einstellrechte.

Standalone und Weboberfläche haben einen Privatbereich für hinzugefügte Chat-Kontakte. Keine Repeater, Roomserver oder nur entdeckten Knoten. Die Quelle ist der aktuelle gespeicherte Kontaktbestand. Der private Verlauf startet pro Variante einmal leer; gemeinsame Kanal- und Room-Verläufe bleiben erhalten.

SD: 800-kHz-Takt am T-Deck, getrennte Karten-/Archivzustände, erneuter Mountversuch beim expliziten Speichern nach fehlgeschlagenem Start und bytegenaues Rücklesen nach dem Schreiben. Keine automatische Formatierung.

## Verifizierungsgrenzen

Der Betreiber meldete am T-Deck Plus funktionierenden Privatversand und nach der SD-Korrektur funktionierende Erkennung sowie Laden. Ein getrennt protokollierter Schreib-/Langzeittest liegt nicht vor. Die neue WLAN/BLE-Integration und T-Watch benötigen eigene Gerätetests. Simulatorvorschauen, Hosttests und Firmware-Builds sind keine Hardwareabnahme. [SD-Gerätetest](SD-TEST-0.9.6.md).

Downloads: T-Deck Plus WLAN/BLE, T-Watch S3 WLAN/BLE sowie T-Deck Plus Standalone als getrennte Pakete. Die technische Standalone-Dateikennung `standalone-test6` bleibt aus Kompatibilitätsgründen bestehen. Update und Neuinstallation sind im Webflasher getrennt; Neuinstallation löscht Gerätedaten.
