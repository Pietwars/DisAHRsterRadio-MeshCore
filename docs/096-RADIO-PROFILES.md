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

## Anwendung und Migration

`DarRadioPolicy.h` ist die gemeinsame Profiltabelle. Alte IDs `de-narrow`,
`ahr-legacy`, `eu-legacy` sind keine auswählbaren Profile mehr. Bereits gespeicherte
Parameter, die exakt einem der drei Profile entsprechen (z.B. ehemaliges EU-alt
als Fern), bleiben gültig. Abweichende Werte wie Narrow werden nicht stillschweigend
als neues TX-Profil gespeichert oder freigegeben. Sie sperren TX; für RX wird das
begrenzte Mittel-Ersatzprofil an den Treiber übergeben. Nach gültiger lokaler
Auswahl kann TX wieder zugelassen werden; Stundenfenster/Neustartschutz bleiben aktiv.

- **Standalone:** Gerät → Funkprofil wählen. Mit 1/2/3, Trackball oder Touch
  auswählen, dann zweimal prüfen/bestätigen. Auswahländerung, Abbruch oder
  Tabwechsel verwirft die Bestätigung. Fehlgeschlagene Übernahme braucht eine
  neue Bestätigung. Bei aktiver BLE-Verbindung ist die Änderung gesperrt.
- **T-Deck WLAN:** P auf der physischen Tastatur öffnet die Profilseite; 1/2/3
  wählen, zweimal Enter bestätigen. Esc/Backspace bricht ab. Auf dunklem Display
  weckt die erste Taste nur; alte Bestätigung wird verworfen. Pairing beendet den
  Editor, eine aktive BLE-Verbindung verhindert die Übernahme. Touch löst dort
  keinen versteckten Advert aus. Logo, Akku und Advert bleiben außerhalb des
  Editors erhalten.
- **Gast-WLAN und BLE:** keine neue Konfigurationsberechtigung. Web zeigt nur die
  drei Profile und den Sperrgrund, die Auswahl ist deaktiviert. Kein Admin-Button.
- **Bestehender lokaler Rescue-CLI, falls am Target erreichbar:** `set radio nah`,
  `set radio mittel`, `set radio fern`. Freie Zahlenparameter sind in DAR nicht
  mehr erlaubt. Eine Remote-Mesh-CLI bekommt diese lokale Änderungsberechtigung
  nicht, auch nicht durch einen manipulierten Zeitstempel.
- **Vertrauenswürdiger interner Companion:** binäre Profiländerungen müssen exakt
  einem der drei Tupel entsprechen. Öffentliche Transport-Positivlisten bleiben
  davor unverändert wirksam. Leistung bleibt -9 bis 10 dBm; CLI-Duty-Cycle-Änderungen
  können die feste 10-%-Politik nicht lockern.

Die beiden T-Deck-Builds starten bereits den Funkchip mit Mittel; andere Upstream-
Builddefaults werden dadurch nicht geändert. Frische DAR-Preferences verwenden
Mittel. Nicht endliche gespeicherte Frequenz/Bandbreite wird abgefangen und bleibt
TX-gesperrt, statt als NaN/Unendlich an den Treiber zu gelangen.

Profiländerungen erfolgen im vorhandenen Arduino-Hauptloop, nicht in einem neuen
Worker oder HTTP-Handler. Laufende TX bzw. erkannter RX verhindern die Übernahme.
Für den T-Deck-SX1262 werden die Rückgaben von Frequenz-, SF-, Bandbreiten-, CR-,
Präambel- und Leistungsänderung ausgewertet. Ein Fehler oder eine nur teilweise
angewandte Konfiguration sperrt TX. Die angenommene Konfiguration muss zusätzlich
mit den aktuell ausgewählten Preferences übereinstimmen.

Speichern nutzt die bestehende `savePrefs()`-Schnittstelle ohne Erfolgsrückgabe;
Flash-Persistenz, Stromunterbrechung, wirkliche Parameter/Leistung und
Wiederaufnahme nach Fehler sind weiter am Gerät zu verifizieren. Auch I2C-Tastatur,
Bluetooth-Parallelfälle, unveränderte Queues und Funkprofile im echten Netz bleiben
Hardware-Abnahmepunkte. Keine Hardware-/EU-Zulassungsfreigabe aus Softwaretests.
