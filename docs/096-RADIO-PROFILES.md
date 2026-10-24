# 0.9.6 TEST – Funkprofile und Standalone-App-Bedienung

Standalone: kein Funkparameter-Editor auf dem Display. Änderungen erfolgen über
die originale MeshCore-App per Bluetooth. Frische Einstellungen und der Hardware-
Builddefault verwenden EU/UK Narrow / Switzerland: **869,618 MHz, 62,5 kHz, SF8,
CR 4/8**. Quelle: [MeshCore Switzerland](https://www.meshcore.ch/settings/).

Gespeicherte gültige Einstellungen werden beim Update beibehalten. Die drei bisherigen
Tupel Nah/Mittel/Fern (869,525 MHz, 250 kHz, CR 4/5, SF7/9/11) bleiben zusätzlich
akzeptiert, damit bestehende Geräte nicht durch das Update TX-gesperrt werden.
Auch freie App-Parameter und andere vom Funkchip unterstützte Presets sind erlaubt.
Die WLAN-Variante behält ihre drei Profile,
den Standard Mittel und den physischen Tastatur-Editor.

Standalone-App-Befehle prüfen Wertebereiche und anschließend den Funkchip-Erfolg.
TX erfordert eine exakt zu den Preferences passende erfolgreich angewandte Konfiguration.
Nicht endliche Werte, ungültige SF/CR-Kombinationen und unvollständig
angewandte Hardwarekonfigurationen autorisieren keinen TX. Ein Wechsel löscht
weder das rollierende Sendefenster noch den dauerhaften Neustartschutz.
Leistung bleibt auf -9 bis 10 dBm begrenzt; die interne 10-%-Politik bleibt fest.
Die bestehende lokale Rescue-CLI ist kein neuer Display-Editor; Remote-Mesh-CLI
bleibt von lokalen Funkänderungen ausgeschlossen.

Die Profilnamen ersetzen keine regionale Freigabe oder HF-Messung. Antennenleistung,
Spektrum, Funkabschluss/Abbruch, Stromausfall während Speicherung, App-Readback und
Wiederaufnahme müssen am realen T-Deck Plus geprüft werden. Keine Hardwarefreigabe
und kein Release aus Hosttests oder Builds.

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
