# Web-Flasher: Prüfung vor dem Schreiben

Teilaufgabe aus #47. Keine Änderungen an MeshCore-Funkkern oder Firmware.

Beim Bereitstellen verifiziert `tools/flasher_fetch.py` weiterhin die Releasebytes.
Zusätzlich lädt der Browser nach ausdrücklicher Auswahl von Gerät und Updateart
Manifest und Firmware ohne Cache. Manifest, Chipfamilie, Datei, Schreiboffset,
Version und Löschoption müssen zur in der Seite hinterlegten Releasekonfiguration
passen. Danach werden SHA-256 und der ESP32-S3-Imageheader geprüft.

Erst dann wird der Installationsknopf freigegeben. ESP Web Tools erhält ein
Blob-Manifest mit absoluten Blob-Dateipfaden für exakt die geprüften Bytes.
Ein erneuter Netzwerkdownload kann diese Bytes nicht mehr ersetzen. Fehler beim
Laden, Hashen oder Vorbereiten lassen die Installation gesperrt. Ein erneuter
Prüfversuch ist möglich. Die Prüfstufe öffnet keinen USB-Port und schreibt nichts.
Der anschließende Installationsdialog und die Unterscheidung Update/Neuinstallation
bleiben bei ESP Web Tools 10.4.0. Eine Neuinstallation löscht weiterhin Gerätedaten.

Die Blob-Unterstützung und der Download vor dem Löschen wurden im gepinnten
Originalcode nachvollzogen:
https://github.com/esphome/esp-web-tools/blob/10.4.0/src/flash.ts
https://github.com/esphome/esp-web-tools/blob/10.4.0/src/util/manifest.ts

## Grenzen

SHA-256 prüft Integrität gegen die mitgelieferte Referenz. Ein Angreifer, der Seite,
Referenzhash und Code austauschen kann, wird damit nicht erkannt. Dies ist keine
Signaturprüfung, Secure-Boot-Einführung oder Hersteller-Authentifizierung.
Die Chipprüfung unterscheidet ESP32-S3 von anderen Chips, aber nicht zwei Boards
mit demselben Chip. T-Deck/T-Watch und WLAN/Standalone müssen korrekt ausgewählt
werden. Keine automatische Boarderkennung behaupten.

Unterbrochenes Schreiben und Rollback/Recovery müssen weiterhin am echten Gerät
getestet werden. Eine erfolgreiche Browserprüfung beweist weder Hardwareabnahme
noch Wiederherstellbarkeit nach jedem Stromausfall.

## Tests

`node tests/flasher_integrity.test.mjs`: alle sechs Update-/Neuinstallationsmodi,
beschädigte Bytes, falsche Chipfamilie/Datei/Offsets/Löschoption, zusätzliche
Manifestteile, Download-/Hash-/Blobfehler und bytegleicher geprüfter Snapshot.

`node tests/flasher_browser.test.cjs`: Chromium-Bedienung aller sechs Modi,
verborgene/gesperrte Installation vor Prüfung, Fehler und Wiederholung sowie
unveränderte Blobbytes bei nachträglich veränderter Netzwerkantwort. Der Test
verwendet simulierte Downloads und ein ESP-Web-Tools-UI-Stub; kein USB-/Flash-Test.
Chromium wird durch den bestehenden Vorschau-CI-Job installiert. Lokal fehlt die
Chromium-Binärdatei; deshalb kein lokales Browser-PASS ohne CI-Nachweis.

`tools/flasher_fetch.py --check` prüft die eingebettete Browserkonfiguration gegen
die Releasekonfiguration und kontrolliert die anfängliche Installationssperre.
