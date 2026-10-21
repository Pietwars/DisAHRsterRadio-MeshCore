# 0.9.5: Release und Hardwarestatus

Veröffentlichung am **21.10.2026** vom Projektbetreiber ausdrücklich beauftragt: „ok release bitte mit allem was es braucht in der readme“. Diese Freigabe ist keine Behauptung, dass alle Hardwaretests durchgeführt wurden.

## Inhalt

- T-Deck Plus Standalone: vollstaendige lokale Bedienung, grosse Touch-Ziele,
  Kachelmenue, scrollbare Listen und Auswahllisten, explizite Sendeaktionen.
- I2C-Eingaben und dauerhafte Touch-Kalibrierung aus PR #42 bleiben erhalten.
- Ohne frischen Fix ist Heimersheim Mitte ausschliesslich Kartenansicht.
- Tastatur, Trackball, MeshCore-Nachrichten, Bluetooth, SD und GPS-Freigabe bleiben
  erhalten. Keine WLAN-/Web-Schicht in der Standalone-Firmware.
- Der normale WLAN/BLE-Betrieb auf T-Deck Plus und T-Watch S3 bleibt eine eigene
  Variante. Die neuen Standalone-Funktionen werden der T-Watch nicht zugeschrieben.

## Dateien und Installation

- WLAN/BLE: `DisAHRsterRadio-MeshCore-0.9.5-LilyGo_TDeck.zip` und `DisAHRsterRadio-MeshCore-0.9.5-LilyGo_TWatchS3.zip`.
- T-Deck Plus Standalone: `DisAHRsterRadio-MeshCore-0.9.5-LilyGo_TDeck_standalone-test6.zip`, Kennung `v1.18.0-dar0.9.5-standalone-test6`. Experimentelle zusätzliche Variante, ohne WLAN/Webportal.
- Alle ZIP-Dateien haben interne SHA256SUMS, Anleitung und geprüfte Flash-Abbilder. Die externe `SHA256SUMS-DisAHRsterRadio-MeshCore-0.9.5.txt` prüft die ZIP-Dateien.
- Update bei passender bestehender MeshCore-Installation an **0x10000 ohne Löschen**. Neuinstallation mit Factory-Abbild an **0x0 nach vollständigem Löschen**, mit Verlust von Identität, Kontakten, Verlauf und Einstellungen. Vorher sichern.
- Standalone enthält `candidate.json` mit Firmware-SHA, Quellcommit, Hardwarestatus und Veröffentlichungsstatus. Die Firmwarekennung bleibt test6, damit die Testlinie erkennbar bleibt.
- Der Web-Flasher bleibt bis zu einer gesondert geprüften Umstellung auf den dort beschrifteten 0.9.4-Dateien. Neue 0.9.5-Dateien über die Release-ZIPs verwenden.

## Prüfungen und offene Hardwaretests

Der Nutzer hat den vorherigen Fixstand aus PR #42 am Gerät als lauffähig mit Nachrichtenempfang bestätigt. Eine vollständige Hardwareabnahme der neuen Touch-UI und der final veröffentlichten Binärdateien ist nicht dokumentiert.

Automatisch geprüft werden Hosttests einschließlich Touch-Abbruch, stabile Empfängerauswahl und GPS-Schutz, Firmware-Builds, Ausschluss der WLAN-/Web-Schicht im Standalone-Build, Header/Partitionen/Speichergrenzen, Factory-Offsets, Paketvollständigkeit, SHA-256 und 18 Layout-Vorschauen. Der Release-Workflow veröffentlicht nur nach erfolgreichem Bau und Paketprüfung aller drei Varianten.

Offen bleiben am realen Gerät: Update und Neuinstallation, Kalibrierung nach Neustart, alle Touch-Kacheln und langen Listen, Abbrechen von Sendegesten, Empfang/Versand Kanal und Direkt, Brett, Tastatur/Trackball, SD, GPS-Freigabe ohne/mit Fix, Laufzeit und Stabilität. Testergebnisse bitte mit `source_commit` und Firmware-SHA aus dem Paket dokumentieren.

[Bedienung](STANDALONE_TOUCH.md) · [Vorschauen mit Demodaten](STANDALONE_VORSCHAU.md) · [Release](https://github.com/Pietwars/DisAHRsterRadio-MeshCore/releases/tag/v0.9.5)
