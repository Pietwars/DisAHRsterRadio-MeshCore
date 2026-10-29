# Companion-Funkprofile mit DE-Sendeschutz

Für beide ESP32-Companions (Standalone und WLAN) gilt die separate
[DE-Sendepolicy](DE-TX-GUARD.md): nominal vollständig innerhalb
869,4–869,65 MHz, höchstens 10 dBm eingestellte Chip-Sendeleistung, festes
10-%-Softwarebudget und persistentes gleitendes Stundenfenster.

Neue Einstellungen verwenden Narrow: 869,618 MHz, 62,5 kHz, SF8, CR 4/8,
10 dBm. Nah/Mittel/Fern bleiben als kompatible Tupel erhalten
(869,525 MHz, 250 kHz, CR 4/5, SF7/9/11). Die breite Variante liegt nominal
exakt an beiden Bandgrenzen und benötigt besonders eine reale Spektrumsprüfung.
Freie App-Parameter sind nur innerhalb der DE-Policy und nach erfolgreicher
Treiberprüfung möglich; Frequenz auf ganze kHz, Bandbreite auf ganze Hz.
Das ist keine Freigabe für andere Länder oder Subbänder.

Gespeicherte Funkparameter und Leistung werden nicht automatisch auf ein
anderes Profil umgeschrieben. Ein Altgerät mit 22 dBm oder außerhalb des Bands
kann empfangen, darf aber nicht senden. Die vorhandene Funk-Sperranzeige bleibt
aktiv. In der PIN-authentifizierten MeshCore-App ein passendes Profil und
höchstens 10 dBm einstellen. Die App erhält 10 dBm als zulässige Maximalleistung.
CLI-Schreibbefehle für Funk, Leistung und Duty Cycle werden abgewiesen;
Statusabfragen bleiben möglich. Neue Tuning-Schreibbefehle akzeptieren nur
Faktor 9; Leseantworten zeigen die wirksame Policy statt alter Preferences.

Das MeshCore-Paketformat, Routing, Verschlüsselung und native App-Kommandos
bleiben unverändert. Die notwendige Ausnahme am Dispatcher aktiviert vorhandene
konservative Abrechnung, Reservierung und Persistenz für beide Frontends.
Der ursprüngliche TX-Code bleibt als nachvollziehbare Baseline erhalten.

Chip-Leistung ist keine gemessene ERP. Antennengewinn, Spektrum, reale Airtime,
Abbruch und Interoperabilität müssen am Gerät geprüft werden. Die Software wird
nicht als hardwareabgenommen oder allgemein regional zugelassen ausgegeben.

## Bluetooth-Autorisierung (Standalone)
Alle vorhandenen App-Einstellungsbefehle werden für den authentifizierten Besitzer
freigegeben. Dazu muss der konkrete empfangene Frame von der aktuellen erfolgreich
MITM-/PIN-authentifizierten ESP32-BLE-Verbindung stammen. Ein gespeicherter PIN
oder eine bloße Verbindung genügt nicht. Verbindungswechsel und Disconnect
widerrufen die zugehörigen Frame-Tokens, auch bei wiederverwendeter Connection-ID.
WLAN/Ethernet und nicht authentifizierte BLE-Frames erhalten dadurch keine Rechte.
Die WLAN-Firmware behält ihre Transportpolitik. Befehlsbezogene Parameterprüfungen
bleiben aktiv.

Hosttests prüfen falschen Peer, Authentifizierungsfehler, Just Works ohne MITM,
fehlenden PIN, alte Frames nach Reconnect und alle 256 Befehlswerte. Die ESP32-
Callbacks und die originale MeshCore-App bleiben zusätzlich real zu testen.
