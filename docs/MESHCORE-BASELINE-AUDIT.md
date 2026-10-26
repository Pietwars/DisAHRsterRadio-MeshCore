# MeshCore-Baseline und Architekturprüfung

Stand: 26.10.2026. Bezug: #52, #47, #48. Untersucht: main
`cda6f6675f259c0b219a765e5c3288e62f4e73b5`.

## Nachfolgende begründete Ausnahme (#47)

Der folgende Audit beschreibt den historischen main-Stand oben. Die separate
[DE-Sendepolicy](DE-TX-GUARD.md) hebt anschließend die Original-TX-Ausnahme für
beide DAR-Companions gezielt auf: Ohne diese Änderung waren Timeout-Airtime,
Neustarts und gleitende regionale Grenzen ungesichert. Dispatcher.cpp/.h wählen
nun den bereits vorhandenen konservativen Pfad, mit einem eigenen Policy-Flag.
Kein neues Paketformat, keine Änderung an Mesh.cpp, Packet oder Kryptografie.

## Nachgewiesene Herkunft

Quelle: https://github.com/meshcore-dev/MeshCore
Baseline: **81dbae592ffad40b4212bc0b37e82b38da08f4bc**.
Der erste DisAHRsterRadio-Commit `84731cb64ba1d61d4ec848d15094592c6d852dd8`
hat genau diese Baseline als einzigen Parent. Der Commit ist auch im
Originalrepository abrufbar. Damit ist die Herkunft aus der Git-Historie belegt;
eine beliebige aktuelle Upstream-Version wird nicht als Vergleich verwendet.

Die lokale Windows-Arbeitskopie und die dort gebaute Standalone-test6-BIN sind
damit weiterhin keinem exakten Commit bzw. SHA-256 zugeordnet.

## Geschützter Kern und tatsächliche Abweichungen

Bytegleich zur Baseline sind:
`src/Mesh.cpp`, `src/Mesh.h`, `src/Packet.cpp`, `src/Packet.h`,
`src/Identity.cpp`, `src/Identity.h`, `src/MeshCore.h`,
`src/Utils.cpp`, `src/Utils.h`.
Damit wurden diese Implementierungen von Paketformat, Kryptographie und
Mesh-Verarbeitung im untersuchten Stand nicht verändert. Das beweist weder
vollständige Verhaltensgleichheit noch Funkregelkonformität.

Alle 28 gegenüber der Baseline hinzugefügten/geänderten src-Dateien:

| Dateien unter src/ | Einordnung und Reviewbedarf |
|---|---|
| CompanionRadioMode.h; OriginalMeshCoreTx.inc | Isolierte Auswahl/Kopie der ursprünglichen TX-Politik für DISASTER_STANDALONE und DISASTER_PORTAL. Kein vollständig unveränderter Kern: zusätzlicher Hardware-Konfigurationsschutz und gemeinsame Dispatcher-Änderungen bleiben aktiv. |
| Dispatcher.cpp; Dispatcher.h | Sicherheitskorrekturen (Parser-Längen-/Nullprüfung), Diagnose-/Adapter-APIs und eigenständige Scheduler-Erweiterungen. Zusätzlich geänderte Zeitvergleiche, Loglängen und Timeoutbehandlung. Pro Änderung separat prüfen; kein pauschales Zurücksetzen. |
| TxBudgetDiagnostics.h; TxWindowLimiter.h; helpers/esp32/Esp32TxWindowStore.h | Eigenständige DAR-Diagnose/Fenster-/Persistenzschicht. Der persistente Limiter ist in beiden Original-TX-Companionvarianten deaktiviert. Andere Buildrollen separat prüfen. |
| helpers/StaticPoolPacketManager.cpp; helpers/StaticPoolPacketManager.h | Queue-Erweiterung peekNextOutbound, ohne Entnahme. Eigene API-Abweichung; Scheduler-/Prioritätsregressionen weiter testen. |
| helpers/BaseSerialInterface.h; helpers/MultiSerialInterface.h; helpers/BleFrameAuth.h; helpers/esp32/SerialBLEInterface.cpp; helpers/esp32/SerialBLEInterface.h | Authentifizierter BLE-Frame-Kontext, Weiterleitung und ESP32-Pairing. Sicherheits-/Transporterweiterung; Bond-/Reconnect-/Rechteentzugtests am Gerät erforderlich. |
| helpers/nrf52/SerialBLEInterface.cpp; helpers/rp2040/SerialBLEInterface.cpp | Sicherheitsrelevante Abweichung: Just-Works/verschlüsselt ohne MITM statt ursprünglicher PIN/MITM-Anforderung. nRF52 exponiert DFU nicht mehr. Keine T-Deck-Abnahme auf diese Plattformen übertragen; gesonderter Review erforderlich. |
| helpers/radiolib/CustomSX1262Wrapper.h; helpers/radiolib/RadioLibWrappers.cpp; helpers/radiolib/RadioLibWrappers.h | Hardware-Sicherheitskorrektur: Rückgabewerte für Parameter/Leistung/Präambel prüfen, fehlerhafte Konfiguration sperren, vor Umkonfiguration idle. Treiber-/Fehlerfalltest erforderlich; keine neue Protokollimplementierung. |
| helpers/sensors/EnvironmentSensorManager.cpp; helpers/sensors/FixFreshness.h; helpers/sensors/LocationProvider.h; helpers/sensors/MicroNMEALocationProvider.h | GPS-/Sensorerweiterungen einschließlich Fix-Aktualität. Eigene Hardware-/Service-Schicht. |
| helpers/ui/DisplayDriver.h; helpers/ui/LGFXDisplay.cpp; helpers/ui/LGFXDisplay.h; helpers/ui/ST7789LCDDisplay.cpp; helpers/ui/ST7789LCDDisplay.h | Anzeige-/Eingabeerweiterungen; eigene UI-/Hardware-Schicht. |

Diese Klassifikation beschreibt die Funktion der Änderungen, keine pauschale
Freigabe ihrer Notwendigkeit. Versehentliche Abweichungen sind noch nicht
abschließend ausgeschlossen. Die Inventarliste betrifft src; Anwendungs-, Board-,
Build-, Bibliotheks- und Roomserveränderungen außerhalb davon benötigen ebenfalls Review.

## Erweiterungsgrenzen und vorhandene Schnittstellen

- UI/Anwendung: `examples/companion_radio/LocalCompanion.h`, Frontends,
  `ui-brand/`, Portal und SD-Module. Neue Bedienfunktionen dort halten.
- Lokaler Versand nutzt denselben Companion-Befehlspfad wie das Portal.
  `MyMesh.cpp` delegiert Direkt-/Kanalversand an `sendMessage` bzw.
  `sendGroupMessage`; keine eigenen verschlüsselten Funkpakete in der UI bauen.
- `MyMesh`/BaseChatMesh und vorhandene Mesh-Sende-APIs sind die Adaptergrenze.
  Standort-/SOS-Erweiterungen benötigen eigene Zustandslogik oberhalb davon.
- BLE-Berechtigungen am Frame-Ursprung prüfen; offene HTTP-Clients erhalten
  dadurch keine administrativen Einstellrechte.
- Board-/SPI-/SD-/Displayänderungen in Varianten und Hardwarehelfern halten.

`dev/096-security-duty-sos` bei `3c11aa06` ist bereits Vorfahr des untersuchten
main. Die Arbeiten sind enthalten; kein erneuter Merge oder Parallelbau nötig.
Es gab bei der Prüfung keine offenen PRs.

## Überprüfung und Grenzen

Am 26.10.2026 lokal:
- Git-Diff gegen exakt genannte Baseline und Bytegleichheitsprüfung der neun
  oben genannten zentralen Dateien erfolgreich.
- `tools/check_096_static.py`, `tests/portal_routes.test.py` und
  `node tests/web_regression.cjs` bestanden. Statische Treffer können auch
  inaktiven Code betreffen; sie beweisen keine aktive Funkpolicy.
- `tests/original_meshcore_tx.test.cpp` mit Dispatcher/Packet/StaticPoolPacketManager
  separat für DISASTER_STANDALONE und DISASTER_PORTAL kompiliert und bestanden.
- `tests/archive_sd.test.cpp` mit SD-Fakes kompiliert und bestanden.
- Vollständiger `tools/run_096_host_tests.py`-Lauf nach den ersten Gates durch
  LeakSanitizer beendet: Zugriff auf /proc/task in dieser Umgebung nicht möglich.
  Auch der Wiederholungsversuch mit ASAN_OPTIONS=detect_leaks=0 scheiterte daran.
  Die genannten gezielten C++-Tests liefen ohne Sanitizer; kein voller Suite-PASS.
- Kein neuer PlatformIO-Build und kein eigener Hardware-/HF-Test.

## Verbleibende Issue-Abnahme

**#52:** Herkunft, Inventar und Modulgrenzen sind jetzt nachvollziehbar.
Einzelreview der Kernabweichungen, notwendige Isolierungen und reale
Kompatibilitätsabnahme bleiben offen. Nicht automatisch schließen.

**#48:** Nach docs/RELEASE-0.9.6.md hat der Betreiber nach der Korrektur
SD-Erkennung und Laden bestätigt. Die alte Aussage „SD wird nicht erkannt“
ist damit historisch; ein separat protokollierter Schreib-/Langzeittest sowie
exakte Build-/BIN-Zuordnung fehlen weiterhin. Keine neue Fehlermeldung erfinden.

**#47:** docs/096-RELEASE-PROGRESS.md beschreibt einen historischen
Entwicklungsstand vom 24.10.2026. main und docs/RELEASE-0.9.6.md belegen inzwischen
die Veröffentlichung und Original-TX-Auswahl. Alte Angaben über aktive
DAR-Fenster/Profilkappen nicht als aktuellen Companionstand übernehmen.
Die in #47 geforderte fortlaufende Airtime-/Regionsprüfung ist dadurch nicht
abgenommen. Auch SOS-Zustandsmaschine, getrennte Bestätigungsstufen,
Update-Herkunft/Recovery und die fehlenden Mess-/Dauerlaufnachweise bleiben
prüfpflichtig. Veröffentlichung allein schließt diese Kriterien nicht.

#45, #46 und #51 sind eigenständige App-/Kamera-/Sensorvorhaben; #51 ist ausdrücklich
als Nachfolgeprojekt vorgemerkt. Diese Bestandsaufnahme implementiert sie nicht.
