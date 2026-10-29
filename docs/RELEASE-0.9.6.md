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

## Standalone-Geräteabnahme vom 28.10.2026

Der Betreiber bestätigt zum bereitgestellten T-Deck-Plus-Standalone-Testpaket:
„Test Ergebnis positiv alles bestätigt ✅“. Dies wird als positive Nutzerabnahme
der zuletzt angeleiteten Funktionsprüfungen erfasst: Senden nach der stillen
Stunde, SOS an einen privaten Kontakt mit Antwort `SOS OK <Kennung>`, Abbruch
und Neustart. Es werden keine zusätzlichen Messwerte oder Einzelprotokolle
unterstellt.

Zuordnung des bereitgestellten Pakets:

- Firmwarekennung: `v1.18.0-dar0.9.6-standalone-test6`.
- Quellcommit: `6ae5daba38d40fd8e00d9e1d1a2c713208c829e5`.
- CI-Lauf: https://github.com/Pietwars/DisAHRsterRadio-MeshCore/actions/runs/40340693448
- Artefakt: `dev-LilyGo_TDeck_standalone`, ID `12658459092`.
- SHA-256 des äußeren CI-Artefakt-ZIPs laut GitHub:
  `ad4994435a197d8d07b9f4750a527e34498ad00696301d0c2041413bcd2d54c4`.
  Dies ist keine Prüfsumme einer einzelnen Firmware-BIN.

Die zuvor hochgeladenen Bilder zeigen gestartete Standalone-Firmware,
Nachrichtenempfang, eine BLE-App-Verbindung, angezeigte 10 dBm im Narrow-Profil,
die Neustartsperre und die SOS-Oberfläche. Die anschließende ausdrückliche
Testbestätigung ergänzt diese Bilder um die oben genannte Funktionsabnahme.

Alle fünf CI-Workflows für den anschließenden Branchstand
`b3313ff58439d81ced6878ed0454245c3ba46dff` sind erfolgreich: reguläre Builds,
Open-access-Regression, Gerätekonfigurationen, zusätzliche Companion-/Browser-
Prüfungen und Release-Paketprüfung. Gegenüber dem getesteten Quellcommit wurde
nur die Browserauswahl der Release-CI auf vorinstalliertes Chrome umgestellt;
der Firmwarequellcode blieb gleich.

Die positive Nutzerabnahme gilt für dieses Standalone-Paket. Separate
WLAN-/T-Watch-Geräteabnahme sowie messtechnische HF-Nachweise werden daraus
nicht abgeleitet. Messwerte für ERP, Spektrum und reale Airtime sowie die genaue
Zuordnung weiterer Dauerlauf-/Recovery-Nachweise bleiben gesondert zu erfassen.
#47 wird durch die Dokumentation dieser Rückmeldung nicht automatisch geschlossen.

## Veröffentlichter Stand vom 24.10.2026

Die folgenden Angaben beschreiben ausschließlich die bisherigen Releasepakete.

### Änderungen

Beide Companion-Varianten verwenden die ursprüngliche MeshCore-Sendepolitik. Der Airtime-Faktor ist konfigurierbar; zusätzliche DAR-Fenster, Neustartsperre und 10-dBm-Kappe entfallen. Frische Einstellungen verwenden EU/UK Narrow; gespeicherte Werte bleiben erhalten. Die authentifizierte Bluetooth-App darf die treiberunterstützten Funkparameter setzen. Das offene HTTP-Portal erhält keine zusätzlichen Einstellrechte.

Standalone und Weboberfläche haben einen Privatbereich für hinzugefügte Chat-Kontakte. Keine Repeater, Roomserver oder nur entdeckten Knoten. Die Quelle ist der aktuelle gespeicherte Kontaktbestand. Der private Verlauf startet pro Variante einmal leer; gemeinsame Kanal- und Room-Verläufe bleiben erhalten.

SD: 800-kHz-Takt am T-Deck, getrennte Karten-/Archivzustände, erneuter Mountversuch beim expliziten Speichern nach fehlgeschlagenem Start und bytegenaues Rücklesen nach dem Schreiben. Keine automatische Formatierung.

## Verifizierungsgrenzen

Der Betreiber meldete am T-Deck Plus funktionierenden Privatversand und nach der SD-Korrektur funktionierende Erkennung sowie Laden. Ein getrennt protokollierter Schreib-/Langzeittest liegt nicht vor. Die neue WLAN/BLE-Integration und T-Watch benötigen eigene Gerätetests. Simulatorvorschauen, Hosttests und Firmware-Builds sind keine Hardwareabnahme. [SD-Gerätetest](SD-TEST-0.9.6.md).

Downloads: T-Deck Plus WLAN/BLE, T-Watch S3 WLAN/BLE sowie T-Deck Plus Standalone als getrennte Pakete. Die technische Standalone-Dateikennung `standalone-test6` bleibt aus Kompatibilitätsgründen bestehen. Update und Neuinstallation sind im Webflasher getrennt; Neuinstallation löscht Gerätedaten.
