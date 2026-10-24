# 0.9.6 Sicherheits-, Funk- und Hardwareabnahme

Status: **NICHT FREIGEGEBEN**. Bezug: Issue #47. Hosttests sind keine Funkmessung.
Die 0.9.5-UI-Basis und PR #44 sind getrennt von der 0.9.6-Härtung zu bewerten.
Vorhandenes Touch-/Empfangsfeedback ersetzt keine vollständige Hardwareabnahme.

## 1. Sicherheitsprüfung

| Zugang | Aktuelles Verhalten | Offener Nachweis |
|---|---|---|
| Offenes WLAN/HTTP | Privilegierte Schreib-Routen 403; Login 503; keine PIN-/Token-Authentifizierung; kein Admin-Button | Reale Negativtests einschließlich gefälschter Header, paralleler Clients und Neustart |
| BLE | Öffentliche Befehlsliste auch mit gesetzter PIN; Schlüsselimport/-export und Konfiguration gesperrt | Pairing, Bonding, Recovery und Wiederverbindung am Gerät |
| Lokale Standalone-UI | Namenseditor und SD-Import mit Gerätebestätigung; PIN-Eingabe mit Bestätigung; Änderung bei aktiver BLE-Verbindung verweigert; Neustart erforderlich | Persistenz, Sperre bei Verbindung, Zugriff am realen Display |
| USB | Nur bei tatsächlich aktivierter Schnittstelle vertrauenswürdiger Companion-Kanal | In beiden aktuellen LocalCompanion-T-Deck-Targets deaktiviert; nicht als nutzbare Konfiguration bewerben |

ESP32 ersetzt ungültige PINs und die bekannte Test-PIN 123456 durch eine zufällige
sechsstellige PIN; andere gültige PINs bleiben erhalten. Der lokale Editor lehnt
123456 ab. Nach Migration eventuell bestehende Telefon-Kopplung entfernen und neu
koppeln; ob alte Bonds weiterberechtigen, ist ausdrücklich noch zu prüfen.
Nicht-ESP32-Fallbacks sind damit nicht freigegeben.

**Softwareblocker für eine vollständige Freigabe:**
- Geschützter und tatsächlich nutzbarer Konfigurations-/Recoveryweg für alle Geräteeinstellungen.
- Region-/Subband-/Profilabhängige Limits; der vorhandene Faktor 9 ist keine allgemeine Zulässigkeitsprüfung.
- Client-ID-Wechsel und Bucket-Verdrängung im Portal: IDs sind keine Identität. Globalen Missbrauchsschutz und Room-Moderationsrechte gesondert härten.
- Update-Vertrauenskette, Signaturprüfung/Schlüsselverwaltung und sichere Wiederherstellung. SHA-256 allein ist kein Herkunftsnachweis.
- Web-Flasher nicht auf ein ungebautes/unabgenommenes 0.9.6-Paket umstellen.

## 2. Hosttests und Builds

```sh
python tools/run_096_host_tests.py
pio run -e dar_LilyGo_TDeck_wifi -e LilyGo_TDeck_standalone
```

Der Hostlauf umfasst die bestehenden Berechtigungs-, Room-, Speicher-, Chat-,
UI-, GPS-, Tasten-, Kalibrierungs- und Boardtests sowie den echten Dispatcher mit
Fake-Radio/-Uhr. Neue Fälle: Timeout-Abbuchung, erfolgreicher/fehlgeschlagener
Sendestart, Budget-Sperre, Paketlänge inklusive Transportcodes, unerreichbare
Reserve, Queue-Priorität, Zeitüberlauf und abgeschnittene Funk-Header.
Der PIN-Editor wird zusätzlich auf abweichende Bestätigung, Speichern, Touch und
Abbruch per Tabwechsel geprüft.

Die 150%-Reserve wird aus der konkreten Wire-Länge berechnet; das Paket bleibt
bis zur Freigabe mit seiner Priorität und Fälligkeit in der Queue. Eine Reserve
oberhalb des Gesamtbudgets blockiert, statt durch Abschneiden passend gemacht zu
werden. Dieses Profil kann dadurch die Queue blockieren: Profil ändern/prüfen,
kein automatischer Budget-Bypass. TX-Timeouts buchen die gesamte verstrichene
Zeit ab und erhöhen keine Erfolgszähler. Das ist konservative Softwarezeit,
keine direkt gemessene RF-Airtime.

Die 0.9.6-CI wird auch durch tools/tests/web/Build-Konfigurationsänderungen
angestoßen und legt Commitkennung und SHA256SUMS zu den Test-Binaries ab.
**Tor:** Hosttests und beide Builds für denselben finalen Commit erfolgreich;
Firmware-SHA256 und Artefaktquelle festhalten. Nicht auf ältere grüne Läufe stützen.

## 3. Funkmessung (zwei reale Geräte)

Pro Messreihe: Boardrevision, Commit, Firmware-SHA256, Frequenz, Bandbreite, SF,
CR, Leistung, Antenne, Region/Subband und begründetes Limit protokollieren.
Kanal, Direktnachricht, Advert, Weiterleitung, Wiederholung, Fehler/Timeout und
Prioritätskonkurrenz messen; kurze/maximale Pakete und Transportcodes abdecken.
Extern beobachtete TX-Zeit, Softwarezähler, Budget vor/nach TX, Queue-Länge und
Wartezeit vergleichen. Profilwechsel und Reboot gesondert prüfen: ein RAM-Budget
beweist keinen über Neustarts erhaltenen regulatorischen Zeitverlauf.
**Tor:** Keine unerfasste TX-Zeit/Limitumgehung; jede Abweichung erklärt und behoben.

## 4. Dauerlauf

Erst 30 Minuten unter Last, danach mindestens 24 Stunden als erste Teststufe.
WLAN/BLE: vier WLAN-Clients plus BLE, Verbindungswechsel und ungültige Anfragen.
Standalone separat: Display/Wakeup, GPS, SD und Funk. Last darf keine Funklimits
umgehen. Jede Minute Heap frei/Minimum, Uptime/Resetgrund, Queue-Länge,
TX-Warteereignisse/-zeit und Fehler notieren. Reconnects und SD-Fehler markieren.
**Tor:** Keine unerklärten Resets, dauerhaft wachsende Queue oder fortschreitender
Speicherverlust; Dauer, Last, Messdaten und Unterbrechungen dokumentieren.

## 5. Hardwareabnahme und Update/Recovery

Vorlage `STANDALONE-ABNAHME.md` verwenden, neue Messreihe mit aktuellem Hash.
Touch/Ausrichtung/Kalibrierung, Tastatur/ALT+Backspace, Trackball, Bootlogo, Akku,
Display-aus/Wakeup, Chat/Funk, GPS, SD und Bluetooth einzeln testen.
Update mit Datenerhalt, absichtlich falscher Prüfsumme, falschem Target,
unterbrochenem Update und Wiederherstellung prüfen; Backup und Rückweg vorab
sichern. Keine Ausfälle durch erfundene PASS-Einträge verdecken.

| Prüfblock | Ergebnis | Evidenz/Datum/Tester |
|---|---|---|
| Security-Review vollständig | OFFEN | |
| Hosttests finaler Commit | OFFEN – CI/Log eintragen | |
| WLAN/BLE-Build | OFFEN | |
| Standalone-Build | OFFEN | |
| Funkmessung aller vorgesehenen Profile | NICHT DURCHGEFÜHRT | |
| 24-h-Dauerlauf beider Targets | NICHT DURCHGEFÜHRT | |
| Hardware und Update/Recovery | NICHT DURCHGEFÜHRT | |

Kein Merge, Tag, Release oder Hardwarefreigabe durch dieses Protokoll.
SOS, zusätzliche Offline-Pakete und Sprachpakete folgen nach diesen Toren.
