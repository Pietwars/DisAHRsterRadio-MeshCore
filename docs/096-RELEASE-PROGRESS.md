# DisAHRsterRadio 0.9.6 – Release-Arbeitsplan

Stand: 24.10.2026. Entwicklungsbranch `dev/096-security-duty-sos`, Bezug: Issue #47.
**Kein Release, kein Hardware-PASS.** Dieser Plan ergänzt das Abnahmeprotokoll;
eine implementierte Teilfunktion schließt ihren Gesamtblock nicht automatisch.

## Nachgewiesene Entwicklungsschritte

| Änderung | Nachweis | Grenze |
|---|---|---|
| Scheduler-Frist nicht durch Polling verschieben | `0b61eece`, GitHub Actions erfolgreich | Keine HF-Messung |
| Treiberzeit fehlgeschlagener Sendestarts abrechnen | `2869d165`, gezielter Hosttest | Softwarezeit statt gemessener HF-Zeit |
| Namenseditor und SD-Import nach lokaler Vorschau/Bestätigung | `0c884f1d`, beide CI-Targets einschließlich Hosttests erfolgreich, Run 40053293342 | Keine vollständige Konfiguration, kein Recovery, Gerätetests offen |
| TX-Mehrverbrauch als Schuld erhalten, Null-Schätzung sperren, lange Fristen begrenzen | `26bafefa`, gezielter Hosttest mit UndefinedBehaviorSanitizer; beide CI-Targets mit Hosttests erfolgreich, Run 40053580325 | Keine HF-/Hardwarefreigabe |

## Reihenfolge und offene Ergebnisse

1. **Lokale Konfiguration und Recovery**: Kanal-/Schlüsselverwaltung, Funkprofil und
   Wiederherstellung am Gerät definieren und implementieren. Standalone bietet bisher
   PIN, Name und SD-Auswahl (Karte, Standortfreigabe, Ereignislog). WLAN-Target besitzt
   noch keinen vollständigen geschützten Administrationsweg. USB darf die exklusive
   LocalCompanion-Schnittstelle nicht parallel übernehmen. Persistenz-/Stromausfalltest
   bleibt nötig; mehrere NVS-Schreibvorgänge sind nicht atomar.
2. **Funkprofil und Zeitfenster**: zulässige Region/Subbänder, gesamte belegte Bandbreite,
   ERP/Antennengewinn und PHY-Grenzen validieren. Ein pauschaler Faktor 9 reicht nicht.
   Alle Sendepfade müssen zusätzlich den fortlaufenden Beobachtungszeitraum und
   Neustart/Profilwechsel berücksichtigen. Reservierung darf nie durch SOS, ACK,
   Advert, Weiterleitung oder Wiederholung umgangen werden.
3. **Rechte und Missbrauch**: vorhandene Portal-Gesamtgrenze und Room-Server-Schutz
   um negative Laufzeitfälle erweitern: ID-Wechsel, parallele Clients, Replay,
   Platzwiederverwendung, Moderation/Löschen, Rechteentzug, Neustart. BLE-Pairing/Bonds
   und alte Kopplungen nach PIN-Migration am Gerät prüfen.
4. **SOS und Zustellstatus**: eigene priorisierte, begrenzte SOS-Zustandsmaschine mit
   explizitem Start/Abbruch, frischem GPS-Fix bzw. ehrlicher Kennzeichnung ohne Fix,
   begrenzten Wiederholungen und eindeutiger Annahmequittung. Kein Umgehen des
   gemeinsamen Sendelimits. Queued, Funkabschluss, Empfängerquittung und menschliche
   Bestätigung trennen. Bestehende Schnellmeldungen nicht als vollständiges SOS ausgeben.
5. **Update und Wiederherstellung**: SHA-256-, falsches Target-, beschädigte Datei- und
   Abbruchfälle testen. Herkunftsprüfung, Signaturschlüssel/-rotation, vertrauenswürdige
   Schlüsselablage und Rückweg aus fehlgeschlagenen Updates vervollständigen.
   Eine Prüfsumme allein authentifiziert keine Firmware.
6. **Finaler Release-Kandidat**: Versionsdateien, Buildkennung mit Commit-SHA, Changelog,
   Geräteanleitungen und Release-Workflow auf denselben getesteten Stand ausrichten.
   Beide T-Deck-Targets vollständig bauen und Artefakte/Prüfsummen festhalten.
   T-Watch nicht ohne eigenen Nachweis als abgenommenes 0.9.6-Target übernehmen.
   Web-Flasher erst nach verfügbarer, freigegebener Firmware umstellen.
7. **Abnahme außerhalb reiner Softwaretests**: reale Funkmessung, WLAN/BLE-Dauerlauf,
   Update/Recovery mit Datenerhalt und T-Deck-Plus-Hardwareabnahme dokumentieren.
   Erst danach Merge-/Tag-/Releaseentscheidung. Keine automatische Veröffentlichung.

Sprachen, weitere Offline-Kartenpakete, Repeater-Energiesparen und Reichweitenkarten
bleiben Erweiterungen aus Issue #47. Ohne gesonderte Umsetzung nicht als enthalten bewerben.

## Funkrechtlicher Abgleich für Deutschland (noch keine Implementierungsfreigabe)

Primärquelle: [Bundesnetzagentur, Vfg. 91/2025](https://www.bundesnetzagentur.de/DE/Fachthemen/Telekommunikation/Frequenzen/Allgemeinzuteilungen/_DL/vfg91_2025.pdf?__blob=publicationFile&v=3),
Begriffsbestimmung Arbeitszyklus und Tabelle 2, Band 54; geprüft am 24.10.2026.
Grundsätzlich gilt ein fortlaufender einstündiger Beobachtungszeitraum. Für
869,4–869,65 MHz nennt die Tabelle alternativ zu den dort genannten Zugangstechniken
höchstens 10 % Arbeitszyklus und 500 mW ERP. Diese einzelne Tabellenzeile ist keine
allgemeine Freigabe aller Frequenzen, Modulationsprofile, Antennen oder Länder.

Codebefund: Der aktuelle Dispatcher füllt einen anfangs vollen Stunden-Tokenbucket
während derselben Stunde wieder auf. Damit ist die Summe in jedem gleitenden
Stundenfenster nicht allein durch die Bucket-Kapazität begrenzt. Die neuen
Schulden-/Reserveprüfungen beheben Fehlerfälle, ersetzen aber keine gleitende
Fensterrechnung. Ein Neustart löscht außerdem den RAM-Verlauf. Vor Release müssen
Fensterrechnung, belastbare Neustartstrategie und Bandwechsel gemeinsam gelöst und
mit adversarialen Zeitfolgen getestet werden. Bis dahin keine Funkregelkonformität behaupten.
