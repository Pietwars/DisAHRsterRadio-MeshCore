# Flashen (DisAHRsterRadio-MeshCore 0.9.3)

> ⚠️ Kein zertifiziertes Notrufsystem. Im Notfall immer **112**.

**Antenne anschrauben, bevor das Gerät Strom bekommt.** Die Erstinstallation (`factory.bin` an **0x0**) **überschreibt Einstellungen, Knotenidentität, Kontakte und Chatverlauf**. Ein Update ohne Datenverlust schreibt nur die Anwendung (`firmware.bin` an **0x10000**).

Geräte: **LILYGO T-Deck Plus** (`DisAHRsterRadio-MeshCore-0.9.3-LilyGo_TDeck.zip`) und **LILYGO T-Watch S3** (`DisAHRsterRadio-MeshCore-0.9.3-LilyGo_TWatchS3.zip`), beide ESP32-S3. Paket aus den [Releases](https://github.com/Pietwars/DisAHRsterRadio-MeshCore/releases) laden und entpacken, Prüfsummen kontrollieren (`sha256sum -c SHA256SUMS`, Windows: `certutil -hashfile Datei SHA256`). esptool installieren: `python -m pip install esptool` (Version 4.x oder 5.x).

## Am einfachsten: Web-Flasher

[Web-Flasher](https://pietwars.github.io/DisAHRsterRadio-MeshCore/flasher/) in **Chrome oder Edge am Computer** öffnen (nicht am Handy, nicht Firefox/Safari). Pro Gerät zwei Knöpfe:

| Knopf | Was passiert | Wann |
|---|---|---|
| grün, **empfohlen:** **„Update auf 0.9.3 – LILYGO T-Deck Plus (Einstellungen bleiben)“** bzw. „… – LILYGO T-Watch S3 …“ | schreibt nur `…-firmware.bin` an 0x10000; Identität, Kontakte, Kanäle, Verlauf, Name, Funkprofil, eigene PIN bleiben. Im Fenster „Erase device“ das Kästchen **nicht** anhaken. | Gerät läuft schon mit DisAHRsterRadio-MeshCore 0.9.x |
| orange umrandet: **„Neuinstallation – LILYGO T-Deck Plus (alles löschen)“** bzw. „… – LILYGO T-Watch S3 …“ | löscht den ganzen Speicher und schreibt `…-factory.bin` an 0x0 – **neue Identität, Kontakte und Einstellungen sind weg** | neues Gerät, andere Firmware vorher, oder das Update startet nicht |

Der Flasher lädt die Dateien aus dem Release v0.9.3 und prüft ihre SHA-256-Werte. Ohne Chrome/Edge: wie unten mit esptool flashen.

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
python -m esptool --chip esp32s3 --port PORT --baud 921600 write_flash 0x0 DisAHRsterRadio-MeshCore-0.9.3-LilyGo_TDeck-factory.bin
python -m esptool --chip esp32s3 --port PORT --baud 921600 write_flash 0x0 DisAHRsterRadio-MeshCore-0.9.3-LilyGo_TWatchS3-factory.bin
```

## Update ohne Datenverlust

```
python -m esptool --chip esp32s3 --port PORT --baud 921600 write_flash 0x10000 DisAHRsterRadio-MeshCore-0.9.3-LilyGo_TDeck-firmware.bin
python -m esptool --chip esp32s3 --port PORT --baud 921600 write_flash 0x10000 DisAHRsterRadio-MeshCore-0.9.3-LilyGo_TWatchS3-firmware.bin
```

Nur verwenden, wenn auf dem Gerät schon DisAHRsterRadio-MeshCore 0.9.x läuft (gleiche Partitionierung); bei Problemen die Erstinstallation machen. Die Einzelteile (Bootloader 0x0, Partitionen 0x8000, boot_app0 0xe000, Firmware 0x10000, leerer Datenbereich `spiffs.bin` 0x310000) liegen im Paket unter `teile/`. Im Datenbereich speichert das Gerät Knotenidentität, Kontakte, Kanäle und Einstellungen – `spiffs.bin` daher **nie** beim Update schreiben.

## Ohne Installation: esptool-js im Browser

https://espressif.github.io/esptool-js/ in Chrome/Edge öffnen → **Connect** → Port wählen → Flash Address `0x0`, Datei `…-factory.bin` → **Program** → danach neu starten. (Update: `…-firmware.bin` an `0x10000`.)

## Danach

Das Display zeigt das Ahrtal-Bild mit dem Schriftzug DisAHRsterRadio und den Knopf **„Advert senden“** (Tasten siehe [README](../README.md#anzeige-am-gerät)). Offenes WLAN `DisAHRsterRadio-XXXXXX` wählen (ohne Passwort) und **http://192.168.4.1** öffnen. MeshCore-App per Bluetooth koppeln: die PIN steht nur während der Kopplung auf dem Display. Details: [Offener Zugang](OFFENER_ZUGANG.md) und die `ANLEITUNG.md` im Paket.
