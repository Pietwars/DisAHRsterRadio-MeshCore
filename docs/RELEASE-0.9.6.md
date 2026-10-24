# 0.9.6-rc1 – öffentlicher Pre-Release

Veröffentlichung vom Projektbetreiber am 24.10.2026 beauftragt. Hardwareabnahme offen.

## Enthalten

T-Deck Plus Standalone: Kanal-Chat und eigener Tab Privat für gespeicherte Kontakte, Touch, Tastatur und Trackball. Der lokale Privatverlauf beginnt beim ersten Öffnen dieser Version leer; Kontakte bleiben gespeichert. Ein neues Paket für T-Deck WLAN/BLE und T-Watch S3 wird separat gebaut und geprüft.

Offene HTTP-Portalzugriffe dürfen keine privilegierten Einstellungen ändern. Lokale Konfiguration und authentifizierte BLE-App bleiben die vorgesehenen Wege. Standalone verwendet die ursprüngliche MeshCore-Sendepolitik, WLAN/BLE die zusätzlichen DAR-Grenzen. Die früheren Entwicklungspläne sind keine Zusage, dass alle dort genannten Funktionen enthalten sind.

## Noch zu prüfen

- Empfang/Versand im Privatchat einschließlich Zustellanzeige und wechselnden Kontakten.
- Parallelbetrieb von Standalone und MeshCore-App, Pairing/PIN-Migration und Neustart.
- Beide T-Deck-Varianten sowie T-Watch S3 auf Hardware, Update mit Datenerhalt, Factory-Installation und Recovery.
- Reale Funkmessung und regionale Vorgaben. Keine Softwareprüfung ersetzt diese Messung.

Vollständige SOS-Zustandsmaschine, vollständiges Recovery und signierte Updates sind nicht Bestandteil dieses Pre-Releases.

## Installation und Nachweise

Nur die ausdrücklich passende Variante wählen. Update: firmware.bin an 0x10000 ohne Löschen; Factory-Abbild an 0x0 nach vollständigem Löschen (Datenverlust). Vorher Identität, Kontakte und Einstellungen sichern. Prüfsummen und Speicherberichte befinden sich in den Release-ZIPs, Standalone enthält zusätzlich candidate.json mit Commit und Firmware-SHA-256.

Die Vorschauen zeigen aktuelle Firmware-Layouts mit Demodaten, keine Hardwareaufnahmen. Vorherige positive Tests von 0.9.5 gelten nicht als Abnahme dieser Version. Hosttests und alle drei Firmware-Builds werden vor der Veröffentlichungsentscheidung separat geprüft. Der bestehende Release-Workflow selbst bleibt unverändert.
