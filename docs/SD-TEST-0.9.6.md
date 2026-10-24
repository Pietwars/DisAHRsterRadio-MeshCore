# T-Deck Plus: SD-Test nach 0.9.6-rc1

Entwicklungsstand ohne neue Hardwarefreigabe. Gemeldet wurde: Karte nicht erkannt, Lesen und Speichern nicht moeglich. Kapazitaet, Dateisystem und serielle Fehlermeldung sind noch unbekannt.

Der T-Deck verwendet jetzt 800 kHz und den MISO-Pull-up entsprechend LilyGOs UnitTest-/HelloWorld-Beispielen. Andere SD-Boards behalten 4 MHz. Kartenstatus und Archivstatus werden getrennt behandelt. Speichern versucht eine fehlgeschlagene Einbindung erneut und bestaetigt Erfolg erst nach bytegenauem Ruecklesen. Die Karte wird nicht automatisch formatiert; der gemeinsame SPI-Bus wird nicht neu gestartet.

Quellen: [LilyGO UnitTest](https://github.com/Xinyuan-LilyGO/T-Deck/blob/master/examples/UnitTest/UnitTest.ino), [LilyGO HelloWorld](https://github.com/Xinyuan-LilyGO/T-Deck/blob/master/examples/HelloWorld/HelloWorld.ino).

## Geraetetest

1. SD-Daten sichern. Bekannte funktionierende FAT32-Karte bei ausgeschaltetem T-Deck einsetzen; Kapazitaet und Format notieren.
2. Testfirmware als Update installieren. Unter Geraet/Info den SD-Status pruefen und bei Fehlern den genauen Wortlaut notieren.
3. Einstellungen auf SD speichern. Danach am Computer `/disaster/config.txt` pruefen.
4. Neu starten: Konfiguration muss zur bestaetigten Uebernahme angeboten werden und darf vor der Bestaetigung nichts umstellen.
5. Optional ohne Karte starten, Karte einsetzen und explizit speichern: Einbindung wird erneut versucht. Nach bereits erfolgreicher Einbindung erfordert ein Kartenwechsel einen Neustart.
6. Mit eingelegter Karte Anzeige, Bluetooth, Funkempfang und Privatnachrichten pruefen. Archiv-/Logdateien nach Aktivierung und Nachrichtenverkehr kontrollieren.

Falls Erkennung weiter fehlschlaegt: Kapazitaet, Dateisystem, Firmware-Commit und USB-Startausgabe bei 115200 angeben. Hosttests und erfolgreiche Builds ersetzen diese Geraetepruefung nicht.
