# 0.9.6 TEST – drei EU868-Funkprofile

Vorgabe: genau **Nah, Mittel, Fern**. Keine zusätzlichen Narrow-/Legacy-Profile.
Alle Teilnehmer müssen Frequenz, Bandbreite, SF und CR gleich einstellen.
Die Bezeichnungen garantieren keine bestimmte Entfernung.

| Profil | ID | Frequenz | Bandbreite | SF | CR |
|---|---|---|---|---|---|
| Nah | `nah` | 869,525 MHz | 250 kHz | 7 | 4/5 |
| Mittel | `mittel` | 869,525 MHz | 250 kHz | 9 | 4/5 |
| Fern | `fern` | 869,525 MHz | 250 kHz | 11 | 4/5 |

Mittel ist für frische DAR-Einstellungen vorgesehen. SF7 benötigt bei gleicher
Nachrichtenlänge weniger Airtime als SF9/SF11. Alle drei nutzen denselben zentralen
Sendescheduler mit Stundenfenster und Neustartschutz; ein Profilwechsel darf die
verbrauchte Airtime nicht zurücksetzen. Der interne Leistungsdeckel bleibt 10 dBm.

EU868 ist die Bandbezeichnung; die gemeinsame Mittenfrequenz lautet ausdrücklich
869,525 MHz. Nominal reicht 250 kHz Bandbreite von 869,400 bis 869,650 MHz.
Das ist eine rechnerische Zuordnung, kein Nachweis des tatsächlich belegten Spektrums.
Antennen-/ERP-, Frequenzfehler-, Modulations- und Emissionsmessung bleiben offen.

Deutschland-Quelle: [Bundesnetzagentur Vfg. 91/2025](https://www.bundesnetzagentur.de/DE/Fachthemen/Telekommunikation/Frequenzen/Allgemeinzuteilungen/_DL/vfg91_2025.pdf?__blob=publicationFile&v=3),
Tabelle 2 Band 54. Dort gelten für nicht näher spezifizierte SRD in 869,4–869,65 MHz
unter den genannten Bedingungen alternativ höchstens 10 % Arbeitszyklus.
Andere EU-Länder sind damit nicht pauschal geprüft oder freigegeben. Der Name
EU868 und der Software-Leistungsdeckel ersetzen keine entsprechende Abnahme.

`DarRadioPolicy.h` ist die gemeinsame Profiltabelle. Alte IDs `de-narrow`,
`ahr-legacy`, `eu-legacy`, freie Parameter und nicht kanonische gespeicherte Werte
sind keine zusätzlichen unterstützten Profile. Die nachfolgende Geräteintegration
muss abweichende Altwerte ausdrücklich behandeln, TX bis zur gültigen Auswahl
sperren und den Wechsel lokal bestätigen lassen. Im offenen Gast-WLAN bleiben
Profiländerungen gesperrt; kein Admin-Button.
