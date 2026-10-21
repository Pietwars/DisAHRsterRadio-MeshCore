# T-Deck Plus Standalone test5: Hardware-Abnahmeprotokoll

> Historische Vorlage zum alten test5-Prerelease. Für die neue Touch-Oberfläche
> und Release 0.9.5 gilt das [aktuelle Abnahmeblatt](RELEASE-0.9.5.md).
> Nutzerfeedback zum späteren Fixstand aus PR #42 bestätigt Lauf und Empfang;
> es füllt diese alten test5-Prüfungen nicht nachträglich mit „OK“.

**Status: AUSSTEHEND – keine Hardwareabnahme dokumentiert.** Dieses Protokoll ist eine Vorlage, kein bestandenes Testergebnis. Verbindlicher Aufgabenstand: [Issue #40](https://github.com/Pietwars/DisAHRsterRadio-MeshCore/issues/40).

| Feld | Bei echtem Test ausfüllen |
|---|---|
| Datum, Testperson | _offen_ |
| Hardware / Board-Revision | LILYGO T-Deck Plus / _Revision offen_ |
| Eingesetztes Firmware-ZIP | [standalone-test5 (Prerelease)](https://github.com/Pietwars/DisAHRsterRadio-MeshCore/releases/tag/standalone-test5) |
| Firmwarekennung | `v1.18.0-dar0.9.5-standalone-test5` (am Gerät bestätigen) |
| SHA-256 der geflashten `firmware.bin` | _offen_ |
| Flashmodus und Port | _offen_ |
| Zweiter MeshCore-Knoten / Funkprofil | _offen_ |
| MeshCore-App-Version / Telefon | _offen_ |
| SD-Karte / GPS-Testumgebung | _offen_ |
| Laufzeit- und Heap-Messung | _offen_ |
| Gesamturteil | **NICHT ABGENOMMEN** |

## Vorbedingungen und Datenverlust vermeiden

1. **Antenne vor dem Einschalten anbringen.** Zweiten MeshCore-Knoten mit passendem Funkprofil, Handy mit MeshCore-App, USB-Datenkabel und bei SD-Tests eine Testkarte bereitlegen.
2. Bei bereits eingerichteter **MeshCore-0.9.4**-Installation zunächst Daten/Identität sichern. Für einen nicht löschenden Funktionstest **nur** die Standalone-`firmware.bin` an Adresse `0x10000` schreiben. Vorher Paketprüfsummen prüfen.
3. **Nicht** einfach ein Update über eine vorhandene **LoRaLayer2**-Installation schreiben: Partitionslayout und Datenformat sind unterschiedlich. Eine `factory.bin` an `0x0` ist eine Neuinstallation und überschreibt unter anderem Kopplungen und Gerätestatus. Einen solchen Test nur auf geeignetem, gesichertem Gerät durchführen.
4. `standalone-test5` ist eine **experimentelle Vorabversion**; weder der stabile Web-Flasher noch der normale WLAN/BLE-Release `v0.9.4` werden dadurch ersetzt.
5. Vor jeder Prüfung beobachten: startet das Gerät, gibt es Bootschleifen, Brownouts oder ungewöhnliche Erwärmung? Bei Fehlern abbrechen und den seriellen Log **ohne** PIN, private Schlüssel, persönlichen Chatinhalt oder GPS-Privatdaten dokumentieren.

## Testfolge

**Ergebnis je Zeile:** `OK` (auf echter Hardware bestanden), `FEHLER` (mit Repro), `BLOCKIERT` (Grund) oder `OFFEN`. Nur reproduzierbare Gerätebeobachtung darf `OK` erhalten. CI/Hosttests sind kein Ersatz.

| ID | Testhandlung auf dem Gerät | Erfolgskriterium | Ergebnis |
|---|---|---|---|
| A01 | Kaltstart; Logo und Farben ansehen; vor 1,8 s Eingabe auslösen | Vollständiges Motiv, erwartete Farben, Überspringen per Eingabe, kein Crash | OFFEN |
| A02 | Ohne Kalibrierung starten; drei Ziele + Mittelprüfung; absichtlich fehlschlagen und wiederholen | Touch korrekt ausgerichtet, Fehlschlag nicht gespeichert, erfolgreiches Profil nach Neustart erhalten | OFFEN |
| A03 | Alle Reiter CHAT, HILFE, BRETT, KARTE, SETUP berühren; Statuszeile und Zurück verwenden | Touchziele erreichbar, keine falschen Seiten, hierarchisches Zurück | OFFEN |
| A04 | In Chat und Brett wischen, danach bewusst SEND auslösen | Wischen scrollt, sendet **nie**; SEND sendet genau einmal | OFFEN |
| A05 | In Chat/anderen Seiten ALT+Backspace und einzelnes Backspace prüfen | ALT+Backspace geht zurück, Backspace bearbeitet Text; ggf. fehlender Tastatur-Rohmodus (I²C `0x03`) nachvollziehbar dokumentiert | OFFEN |
| A06 | Trackball vier Richtungen, Klick, 2,5 s halten, Tastatur Enter/ALT+C/Zeichen testen | Richtige Navigation und Eingabe, Flood-Advert nur bewusst ausgelöst | OFFEN |
| A07 | 30 s warten; wecken per Touch, Taste und eingehender Nachricht; `d` und Setup-Funktion testen | Dunkel/hell stabil, Aufweck-Eingabe löst keine zweite ungewollte Aktion aus | OFFEN |
| B01 | PIN-Kopplung mit MeshCore-App, trennen, erneut verbinden | PIN nur beim Pairing sichtbar, erneute Verbindung erfolgreich | OFFEN |
| B02 | Kanal- und Direktnachricht T-Deck → zweiter Knoten und zurück | Empfänger, Inhalt, Absender, Verlauf und Sendepause korrekt | OFFEN |
| B03 | Nachrichten über App und über Gerät senden/empfangen | Beide Oberflächen sehen die vorgesehenen Nachrichten; keine unerwarteten Duplikate | OFFEN |
| B04 | Room-Server öffnen, beitreten, lesen, schreiben, unberechtigt schreiben versuchen | Anmeldung, Berechtigung und Beiträge verhalten sich wie erwartet | OFFEN |
| B05 | Advert 2,5 s per Trackball, zweiter Knoten beobachtet | Advert wird tatsächlich über Funk empfangen | OFFEN |
| C01 | GPS unter freiem Himmel aktivieren; Fix, Alter und Baudratenerkennung prüfen | Gültiger Fix und sinnvolle Koordinaten; ohne Fix keine vorgetäuschten Koordinaten | OFFEN |
| C02 | Standort teilen OFF/ON, Bekanntgabe mit/ohne frischen Fix, Neustart | Opt-in standardmäßig OFF; nur freigegebener frischer Fix (≤15 s) wird angehängt; Einstellung bleibt erhalten | OFFEN |
| C03 | Position mit `!DP1` senden; HILFE: Vorlage/Ort/Ziel wählen und Text bearbeiten | Text und Position auf zweitem Knoten korrekt, kein GPS-Versand bei fehlender Freigabe/ungültigem Fix | OFFEN |
| C04 | Fünf Karten, Zoom, Zentrieren, GPS-Folgen und manuelles Verschieben prüfen | Alle Karten sichtbar; Verschieben stoppt Folgen, Zentrieren aktiviert es erneut | OFFEN |
| D01 | SD einstecken, Gerät neu starten; ohne SD wiederholen | Karte über GPIO39/Shared SPI zugreifbar, LoRa/Display bleiben stabil, ohne SD kein Absturz | OFFEN |
| D02 | Geräte-Einstellungen sichern, Log und Archiv lesen | `/disaster/config.txt`, `log.txt`, `current.bin` wie dokumentiert; **keine PIN/Schlüssel oder Nachrichtentexte im Ereignislog** | OFFEN |
| D03 | Nach Backup Update von MeshCore-0.9.4 über `0x10000`, mit bestehenden Kontakten/PIN/Verlauf | Vorherige Daten erhalten; bei Fehlschlag dokumentierten Wiederherstellungsweg verwenden | OFFEN |
| D04 | Heap/Minimum/Fragmentierung und Akku bei Display, GPS, BLE und LoRa protokollieren | Keine Neustarts/Brownouts; Messwerte und Testdauer konkret erfasst (keine erfundenen Grenzwerte) | OFFEN |
| D05 | WLAN-Netze und Gerätedienste prüfen | Kein Standalone-Webportal/WLAN-AP; MeshCore via PIN-Bluetooth bleibt erreichbar | OFFEN |

### Ergebnisvorlage für jeden Fehler

```text
Test-ID:
Ergebnis: FEHLER / BLOCKIERT
Board-Revision:
Firmwarekennung + SHA256:
Startzustand:
Schritte zur Reproduktion:
Erwartet:
Beobachtet:
Serieller Log / Foto (ohne Geheimnisse):
Häufigkeit (z. B. 3/3):
Vermutete Komponente (optional):
```

### Freigaberegel

- Alle sicherheits-/datenrelevanten Kernfunktionen (Funk, Bedienung, Speicherung, Standortfreigabe und Updatepfad) müssen mit Hardwarebelegen abgenommen sein. Defekte, die diese Funktionen betreffen, blockieren eine stabile Freigabe.
- Für jeden verbleibenden Mangel ein getrenntes GitHub-Issue mit Reproduktionsschritten erfassen und im [Abnahme-Issue #40](https://github.com/Pietwars/DisAHRsterRadio-MeshCore/issues/40) verlinken.
- Bei vollständiger Abnahme das Datum, echte Hardwaremesswerte und das Ergebnis eintragen; erst danach über einen **neuen stabilen** Standalone-Tag/Release entscheiden. Dieses Protokoll selbst meldet nichts als bestanden.

**Abgrenzung:** [Issue #33](https://github.com/Pietwars/DisAHRsterRadio-MeshCore/issues/33) behandelt die Schnellmeldungsseite im *normalen* WLAN/BLE-T-Deck-Build; die HILFE-Seite des Standalone ist bereits im Testbuild enthalten. Die Hardwareabnahme von `v0.9.4` auf T-Deck und T-Watch bleibt ein eigenes Thema.
