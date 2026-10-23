# Lokale Konfiguration / USB-Prüfstand (v0.9.6)

> **Korrektur zum aktuellen Entwicklungsstand:** In `dar_LilyGo_TDeck_wifi`
> und `LilyGo_TDeck_standalone` ist USB-Companion wegen LocalCompanion nicht
> aktiviert. USB-Konfiguration ist dort **nicht verfügbar**. BLE gewährt auch
> mit gültiger PIN nur öffentliche Kommandos. ESP32 migriert inzwischen die
> Standard-PIN 123456. Die folgenden älteren Untersuchungsnotizen sind keine
> Funktionszusage; maßgeblich ist `096-SECURITY-RADIO-ACCEPTANCE.md`.


**Status: Entwicklungsdokument, keine Hardware-Abnahme.** Gilt für die Companion-Firmware. Die Standalone-Variante muss getrennt geprüft werden.

## Bestehende Schnittstelle nutzen

Die Firmware besitzt bereits den MeshCore-Companion-Kommandoweg. `examples/companion_radio/OpenAccess.h` definiert eine Positivliste für öffentliche Kommandos und unterscheidet diese von vertrauenswürdigen Transporten. **USB ist der bevorzugte lokale Konfigurationskanal.** Wir implementieren keine parallele, ungesicherte HTTP-Admin-Schnittstelle.

- Offenes WLAN: Chat, Status und andere explizit öffentliche Kommandos. `/api/admin-login` ist gesperrt (503), privilegierte Web-Endpunkte sind gesperrt (403).
- USB: Vorhandene MeshCore-Companion- und CLI-Funktionen für Geräteeinstellungen nutzen, **sofern** das konkrete Board sie bereitstellt. Keine neue PIN-Eingabe im Webportal.
- Bluetooth: derzeit keine gleichwertige Sicherheitsgarantie. `OpenAccess.h` enthält noch einen **festen Test-Standard-PIN 123456** und eine weitgehende Freigabe bei gesetztem BLE-PIN. Vor Release müssen Pairing-Status, Standard-PIN, Reset und Berechtigungen auf Hardware geprüft und gehärtet werden.

## Implementierungs-/Abnahmefolge

1. Auf dem Zielgerät den tatsächlich verwendeten USB-Serial-Pfad, `MultiSerialInterface`-Routing und die CLI-/Companion-Berechtigungen prüfen. Vorhandene Schnittstelle wiederverwenden.
2. Über USB mit einer geeigneten MeshCore-App/CLI zunächst nur **lesende** Statusabfragen testen; keine Schlüssel, Zugangsdaten oder private Konfiguration in Logs kopieren.
3. Knotenname, Funkprofil und Kanal einzeln über USB ändern und nach Neustart verifizieren. Regionale Funkvorgaben und die Zulässigkeit des jeweiligen Profils separat prüfen.
4. Parallel über offenes WLAN nachweisen, dass dieselben Änderungen mit 403 verweigert werden, während Chat und Status weiter funktionieren.
5. USB-Abziehen, erneutes Verbinden, mehrere gleichzeitige Clients und Fehlerfälle testen. Keine automatischen Resets oder Datenlöschungen.
6. BLE-Härtung und Standalone-Berechtigungen separat abnehmen.

**Nicht erledigt:** Verifikation des tatsächlichen USB-Kommandoroutings auf dem T-Deck Plus, Build, Hardwaretests und Prüfung des Bluetooth-Pairings. Kein Merge, Tag oder Release ohne explizite Freigabe.

## Ergebnis Codeprüfung: USB-Companion ≠ Rescue-CLI

In `examples/companion_radio/main.cpp` wird bei `ENABLE_USB_INTERFACE` der `ArduinoSerialInterface` an `InterfaceType::USB` gebunden. In `MyMesh::loop()` wird im Normalbetrieb `checkSerialInterface()` aufgerufen. Die Klartext-Kommandos `get name`, `set name`, `set pin` werden dagegen durch `checkCLIRescueCmd()` nur im **Rescue-Modus** verarbeitet (Sonderfall: RP2040-WiFi ohne USB-Interface). Ein normales USB-Terminal darf daher **nicht** als funktionierende Klartext-CLI beschrieben werden. Für den Normalbetrieb ist ein kompatibler Companion-Client mit gerahmten Kommandos nötig.

**Offener Prüfpunkt:** Für die konkrete T-Deck-Plus-Build-Konfiguration muss `ENABLE_USB_INTERFACE` anhand der vollständig aufgelösten PlatformIO-Umgebung nachgewiesen werden. Erst dann darf USB-Konfiguration als verfügbar gelten. Rescue-CLI ist kein Ersatz für ein gesichertes Normalbetriebs-Protokoll.

**Sicherheitswarnung:** Über USB können im bestehenden Companion-Protokoll auch sensible Kommandos erreichbar sein. USB physisch kontrollieren; keine automatische Freigabe über WLAN oder ungekoppeltes BLE.

## T-Deck Plus: geprüfte Build-Vererbung

- `platformio.devices.ini`: `[env:dar_LilyGo_TDeck_wifi]` erbt von `env:LilyGo_TDeck_companion_radio_ble`.
- `platformio.standalone.ini`: `[env:LilyGo_TDeck_standalone]` erbt **ebenfalls** von `env:LilyGo_TDeck_companion_radio_ble`. Die Standalone-Variante entfernt WebFrontend und Webserver-Abhängigkeiten, nicht ausdrücklich USB.
- `main.cpp`: Nur bei gesetztem `ENABLE_USB_INTERFACE` wird die USB-Companion-Schnittstelle registriert.

**Noch nicht nachgewiesen:** Der geerbte `LilyGo_TDeck_companion_radio_ble`-Abschnitt wurde noch nicht aufgelöst. Deshalb folgt aus der gemeinsamen Vererbung allein **nicht**, dass USB in beiden Targets aktiv ist.

### Reproduzierbare lokale Prüfung

Mit installiertem PlatformIO im Repository:

```powershell
pio project config --environment dar_LilyGo_TDeck_wifi
pio project config --environment LilyGo_TDeck_standalone
pio run -e LilyGo_TDeck_standalone
```

Die effektiven `build_flags` und `build_unflags` einschließlich `ENABLE_USB_INTERFACE` kontrollieren. Die beiden `pio project config`-Aufrufe sind **kein** Ersatz für eine vollständige Präprozessor-/Buildprüfung; sie zeigen Konfiguration, aber keine Hardwarefunktion. Danach USB-Companion-Verbindung am realen Gerät testen. **Nicht flashen, bevor der Build erfolgreich und das richtige Boardprofil bestätigt ist.**
