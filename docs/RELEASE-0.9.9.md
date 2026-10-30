# Release 0.9.9 · eindeutige WLAN-Namen

Der Betreiber hat beim Flashen zweier Boards festgestellt, dass beide denselben
WLAN-Namen erhalten. Ursache war die Auswahl des Herstellerpräfixes aus der
Byte-Darstellung von `ESP.getEfuseMac()` statt einer eindeutigen Gerätekennung.

Das Portal verwendet jetzt die vollständige Hardwareadresse als zwölfstellige
Kennung. Beispielsweise entstehen `DisAHRsterRadio-246F28123456` und
`DisAHRsterRadio-246F28123457`. Auch Boards unterschiedlicher Hersteller mit
denselben letzten Adressbytes bleiben unterscheidbar. Neustart und erneutes
Flashen desselben Boards ändern seinen Namen nicht.

Das OLED zeigt den gesamten WLAN-Namen auf zwei Zeilen. Die Weboberfläche nennt
ihn unter Gerät. Bestehende MeshCore-Identität, Kontakte, Einstellungen und
Bluetooth-PIN bleiben bei einem Firmware-Update ohne Löschen erhalten. Nach dem
Update muss am Handy das neue WLAN ausgewählt werden.

Die automatisierten Prüfungen decken gleiche Herstellerpräfixe, stabile Namen,
ungültige Hardwarekennungen und die OLED-Grenzen ab. Ein physischer Zwei-Board-Test
der neu gebauten Version 0.9.9 ist noch offen. Die bisherigen Hardware-Rückmeldungen
sind in [0.9.8](RELEASE-0.9.8.md) dokumentiert.
