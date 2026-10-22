# Sichere lokale Konfiguration über USB (v0.9.6)

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
