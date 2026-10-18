# Flashen (DisAHRsterRadio-MeshCore 0.9.2)

> ⚠️ Kein zertifiziertes Notrufsystem. Im Notfall immer **112**.

**Antenne anschrauben, bevor das Gerät Strom bekommt.** Die Erstinstallation (`factory.bin` an **0x0**) **überschreibt Einstellungen, Knotenidentität, Kontakte und Chatverlauf**. Ein Update ohne Datenverlust schreibt nur die Anwendung (`firmware.bin` an **0x10000**).

Geräte: **LILYGO T-Deck Plus** (`DisAHRsterRadio-MeshCore-0.9.2-LilyGo_TDeck.zip`) und **LILYGO T-Watch S3** (`DisAHRsterRadio-MeshCore-0.9.2-LilyGo_TWatchS3.zip`), beide ESP32-S3. Paket aus den [Releases](https://github.com/Pietwars/DisAHRsterRadio-MeshCore/releases) laden und entpacken, Prüfsummen kontrollieren (`sha256sum -c SHA256SUMS`, Windows: `certutil -hashfile Datei SHA256`). esptool installieren: `python -m pip install esptool` (Version 4.x oder 5.x).

> Der [Web-Flasher](https://pietwars.github.io/DisAHRsterRadio-MeshCore/flasher/) installiert noch die getestete T-Deck-Datei der 0.9.1 und wird erst mit dem Release 0.9.2 umgestellt. Für 0.9.2 bitte wie unten flashen.

## Port finden

| System | So findest du den Port |
|---|---|
| Windows | Geräte-Manager → „Anschlüsse (COM & LPT)“, z. B. `COM5` („USB Serial Device“/„USB JTAG/serial debug unit“) |
| macOS | `ls /dev/cu.*` – meist `/dev/cu.usbmodem…` |
| Linux | `ls /dev/ttyACM*` – meist `/dev/ttyACM0`; Benutzer in Gruppe `dialout` (`sudo usermod -aG dialout $USER`, neu anmelden) |

Kein Port bzw. „Failed to connect“: anderes (Daten-)Kabel. Download-Modus erzwingen – **T-Deck Plus:** Trackball gedrückt halten und einschalten; **T-Watch S3:** USB-C neu einstecken, ggf. Seitentaste kurz drücken und erneut versuchen.

## Erstinstallation (löscht Einstellungen)

`PORT` ersetzen (z. B. `COM5`, `/dev/ttyACM0`):

```
python -m esptool --chip esp32s3 --port PORT --baud 921600 write_flash 0x0 DisAHRsterRadio-MeshCore-0.9.2-LilyGo_TDeck-factory.bin
python -m esptool --chip esp32s3 --port PORT --baud 921600 write_flash 0x0 DisAHRsterRadio-MeshCore-0.9.2-LilyGo_TWatchS3-factory.bin
```

## Update ohne Datenverlust

```
python -m esptool --chip esp32s3 --port PORT --baud 921600 write_flash 0x10000 DisAHRsterRadio-MeshCore-0.9.2-LilyGo_TDeck-firmware.bin
python -m esptool --chip esp32s3 --port PORT --baud 921600 write_flash 0x10000 DisAHRsterRadio-MeshCore-0.9.2-LilyGo_TWatchS3-firmware.bin
```

Nur bei gleicher Partitionierung (ab 0.9.0) verwenden; bei Problemen die Erstinstallation machen. Die Einzelteile (Bootloader 0x0, Partitionen 0x8000, boot_app0 0xe000, Firmware 0x10000, Web-Dateisystem `spiffs.bin` 0x310000) liegen im Paket unter `teile/`.

## Ohne Installation: esptool-js im Browser

https://espressif.github.io/esptool-js/ in Chrome/Edge öffnen → **Connect** → Port wählen → Flash Address `0x0`, Datei `…-factory.bin` → **Program** → danach neu starten. (Update: `…-firmware.bin` an `0x10000`.)

## Danach

Das Display zeigt das DisAHRsterRadio-Logo und den Knopf **„Advert senden“** (Tasten siehe [README](../README.md#anzeige-am-gerät)). Offenes WLAN `DisAHRsterRadio-XXXXXX` wählen (ohne Passwort) und **http://192.168.4.1** öffnen. MeshCore-App per Bluetooth koppeln: die PIN steht während der Kopplung auf dem Display. Details: [Offener Zugang](OFFENER_ZUGANG.md) und die `ANLEITUNG.md` im Paket.
