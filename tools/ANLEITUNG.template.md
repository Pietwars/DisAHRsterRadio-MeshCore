# @PREFIX@ für @BOARD@

MeshCore-Companion mit WLAN-Weboberfläche · Version @VERSION@ · Commit @COMMIT@ · @REPO@

> ⚠️ Kein zertifiziertes Notrufsystem. In Lebensgefahr immer **112** wählen. Diese Firmware ist **noch nicht auf echter Hardware getestet**.

## 1. Zuerst die Antenne!

**Antenne anschrauben, bevor das Board Strom bekommt.**

## Neu in 0.9.2

- **Minimal-Anzeige** (Geräte mit Display aus dem Release): nur Schriftzug DisAHRsterRadio mit Logo und Knopf **„Advert senden“** (Flood-Advert, höchstens einer je 10 s). Keine weiteren Seiten und keine Nachrichten am Gerät – Nachrichten und Einstellungen über MeshCore-App und Weboberfläche.
- **Bluetooth-PIN** steht nur während einer laufenden Kopplung auf dem Display (höchstens 60 s).
- Parallel zum WLAN-Portal kann sich die offizielle MeshCore-App per **Bluetooth** verbinden (Kopplung mit PIN). Über PIN-gekoppeltes Bluetooth: **alle** Einstellungen und Funktionen wie bei Standard-MeshCore (Name, Standort, Kanäle, Funkparameter, Kontakte, GPS, Neustart, Werksreset, CLI), auch **privaten Schlüssel exportieren/importieren** und eine **eigene 6-stellige PIN** setzen (gilt nach dem Neustart; danach die alte Kopplung am Handy entfernen und neu koppeln). Weboberfläche (offenes WLAN, **ohne Anmeldung**): Knotenname, Funkprofil (nur feste Deutschland-Profile), Kanäle anlegen/ändern und Nachrichtenverlauf löschen im Reiter „Gerät“. **Achtung: Jede Person im offenen WLAN kann Kanäle, Knotenname und Funkprofil ändern und den Verlauf löschen.**
- **Kontakte automatisch:** nur **Repeater und Room-Server** (Werkseinstellung bei leerem Speicher, in der MeshCore-App über PIN-Bluetooth änderbar). Beim reinen Firmware-Update bleibt eine gespeicherte Einstellung erhalten.

## 2. Erstinstallation

1. Board per USB-Datenkabel anschließen, Chrome oder Edge am PC/Mac.
2. https://espressif.github.io/esptool-js/ öffnen, **Connect**, Port wählen (ggf. Download-Modus: BOOT bzw. Trackball gedrückt halten und einschalten).
3. Optional **Erase Flash**.
4. Flash Address **`0x0`**, Datei `@PREFIX@-@ZIP@-factory.bin`, **Program**.
5. Danach **RST** drücken.

Enthalten: Bootloader (@BOOT@), Partitionen (0x8000), boot_app0 (0xe000), Firmware mit Weboberfläche (0x10000), SPIFFS-Dateisystem (@FSOFF@).

## 3. Verbinden

Am Handy das offene WLAN `DisAHRsterRadio-XXXXXX` auswählen und verbinden (höchstens 4 Handys, nichts eingeben). Bei „Kein Internet“ verbunden bleiben, dann `http://192.168.4.1` öffnen. Achtung: offenes, unverschlüsseltes WLAN – jede Person in Reichweite kann mitlesen und senden. Bluetooth (MeshCore-App) koppelt mit PIN – sie steht während der Kopplung auf dem Display; bei alten Kopplungen oder nach einer PIN-Änderung am Handy entkoppeln und neu verbinden.

Es gibt keinen Admin-Bereich in der Weboberfläche mehr: Knotenname, Funkprofil, Kanäle und Verlauf löschen sind ohne Anmeldung zugänglich – jede Person im offenen WLAN kann sie ändern.

## 4. Update (nur Firmware)

**Nur `teile/firmware.bin` an `0x10000`** flashen. **Nicht** die factory-Datei: sie überschreibt das SPIFFS mit Schlüssel (Knotenidentität), Kontakten und Verlauf.

## 5. Prüfsummen

`SHA256SUMS` – Linux/Mac `sha256sum -c SHA256SUMS`, Windows `certutil -hashfile Datei SHA256`.

| Datei | Adresse |
|---|---|
| factory.bin | 0x0 |
| teile/bootloader.bin | @BOOT@ |
| teile/partitions.bin | 0x8000 |
| teile/boot_app0.bin | 0xe000 |
| teile/firmware.bin | 0x10000 |
| teile/spiffs.bin | @FSOFF@ |

MeshCore © Scott Powell / rippleradios.com u. a. (MIT). Karten © OpenStreetMap-Mitwirkende (ODbL).

