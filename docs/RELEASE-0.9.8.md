# Release 0.9.8 · T3-S3 v1.3 SX1262

## Enthalten

- T3-S3 v1.3, SX1262, 868 MHz: Weboberfläche und MeshCore-Bluetooth gleichzeitig.
- Eigene OLED-Kopplungsanzeige: automatisches Aufwecken, sechs große PIN-Ziffern,
  Vorrang vor Meldungen und Navigation, Ausblenden nach Ende oder spätestens 60 Sekunden.
- OLED-Startseite mit WLAN-Name, Browseradresse und Nachrichtenanzahl.
- Bluetooth-Status und Kopplungshilfe im Web-Reiter Gerät; keine PIN über HTTP.
- Release-Paket, 4-MB-Flashvertrag, Build- und Speicherprüfungen für das neue Board.
- T-Deck Plus WLAN/BLE, T-Watch S3 WLAN/BLE und T-Deck Plus Standalone bleiben getrennte Pakete.

## Rückmeldung am Gerät

Der Betreiber hat am **30.10.2026** die Funktion des zuvor bereitgestellten
T3-S3-OLED-Testpakets mit „läuft“ bestätigt. Einzelne Prüfschritte wurden dabei
nicht protokolliert. Die neu gebauten Dateien mit Versionskennung 0.9.8 werden
separat gebaut und geprüft; der bestätigte Funktionstest ist keine vollständige
Abnahme dieser neuen Binärdateien.

Weiter am Gerät prüfen: falsche PIN, Abbruch, 60-Sekunden-Zeitablauf, gespeicherte
Kopplung nach Neustart, Web/App-Parallelbetrieb und Nachrichten mit zwei Geräten.
Frühere T-Deck-/T-Watch-Rückmeldungen gelten nur im dort dokumentierten Umfang.

## Dateien und Flashen

Alle WLAN/BLE-ZIPs enthalten Factory-Abbild an 0x0, Firmware-Update an 0x10000,
Einzelteile, Anleitung, SHA256SUMS und Flash-/RAM-Berichte. Das T3-S3-Paket ist
ausschließlich für **SX1262**, nicht für SX1276 oder SX1280.

Updates ohne Löschen benötigen ein kompatibles Partitionslayout. Neuinstallation
löscht gespeicherte Gerätedaten. [T3-S3-Anleitung](T3-S3-V1.3.md).

Nachweis zum bestätigten Ausgangspaket (Firmware-SHA256):

`d63bcdd9ed66736cc109ec43cb343ee3a39ba58385e136370151978f4da79964`
