# 0.9.5: Release-Vorbereitung und Abnahme

**Status: VORBEREITET, NICHT FREIGEGEBEN.** Es wurde kein Tag oder Release
angelegt und nichts nach main zusammengefuehrt. Veröffentlicht bleibt v0.9.4;
der stabile Web-Flasher verweist weiterhin auf dessen Dateien.

## Geplanter Inhalt

- T-Deck Plus Standalone: vollstaendige lokale Bedienung, grosse Touch-Ziele,
  Kachelmenue, scrollbare Listen und Auswahllisten, explizite Sendeaktionen.
- I2C-Eingaben und dauerhafte Touch-Kalibrierung aus PR #42 bleiben erhalten.
- Ohne frischen Fix ist Heimersheim Mitte ausschliesslich Kartenansicht.
- Tastatur, Trackball, MeshCore-Nachrichten, Bluetooth, SD und GPS-Freigabe bleiben
  erhalten. Keine WLAN-/Web-Schicht in der Standalone-Firmware.
- Der normale WLAN/BLE-Betrieb auf T-Deck Plus und T-Watch S3 bleibt eine eigene
  Variante. Die neuen Standalone-Funktionen werden der T-Watch nicht zugeschrieben.

## Was bereits beobachtet wurde

Der Nutzer hat das CI-Artefakt aus Lauf **39546599293**, basierend auf PR #42,
am eigenen Geraet getestet: „das passt alles schon, laeuft, Nachrichten kommen
auch an“. Das bestaetigt diesen vorigen Teststand, nicht die neue Touch-UI.
Es ist Nutzerfeedback, kein von uns durchgefuehrter Hardwaretest und keine
pauschale Freigabe aller 0.9.5-Dateien.

## Verbindliche Abnahme des neuen Testpakets

| Feld | Auszufuellen |
|---|---|
| Testpaket / Actions-Lauf | CI-Artefakt `LilyGo_TDeck_standalone-0.9.5-abnahme` aus PR #43; genauen Lauf eintragen |
| Firmwarekennung | `v1.18.0-dar0.9.5-standalone-test6` |
| CI-Commit | `source_commit` aus `candidate.json` |
| SHA-256 der getesteten firmware.bin | aus `candidate.json` / `SHA256SUMS` uebernehmen |
| Datum / Testperson / Board-Revision | OFFEN |
| Update an 0x10000 und Neuinstallation mit factory.bin an 0x0 (nach Sicherung) | OFFEN |
| Kalibrierung beim Erststart und nach Neustart | OFFEN |
| Kacheln, Tabs, Zurueck, lange Kontakt-/Empfaengerlisten | OFFEN |
| Wischen auf/weg von Senden, Finger auflegen/wegziehen | OFFEN |
| Nachrichtenempfang und Versand Kanal + Direkt | OFFEN |
| Brett: Beitreten und Aushang | OFFEN |
| Tastatur und Trackball | OFFEN |
| Karte ziehen, Zoom, Heimersheim ohne GPS | OFFEN |
| Freigabe AUS / kein Fix: keine Geraete-GPS-Position senden | OFFEN |
| SD-Speichern und Touch erneut kalibrieren | OFFEN |
| Stabilitaet, Laufzeit, Heap | OFFEN |
| Gesamturteil | NICHT ABGENOMMEN |
| Ausdrueckliches Okay zur Veroeffentlichung | NICHT ERTEILT |

Das CI-Testpaket enthaelt die Update-Firmware, BIN-Einzelteile, Pruefsummen,
Commit-/SHA-Manifest, Bedienung, Anleitung und Layout-PNGs mit Demodaten.
Es enthaelt `firmware.bin` fuer Updates an **0x10000** sowie `factory.bin` fuer
Neuinstallationen an **0x0**. Letztere erfordern vorheriges vollstaendiges
Loeschen und vernichten Identitaet, Kontakte, Einstellungen und Verlauf. Beide
Wege sind getrennt abzunehmen; Details stehen in seiner ANLEITUNG.md.

## Bereits vorbereitet

- README und Dokumentation unterscheiden veroeffentlichten Stand und Kandidaten.
- Alle neuen Geraete-Vorschauen werden aus demselben C++-Layout gerendert wie die
  Firmware; Hauptseite/Galerie kennzeichnen sie als Demodaten und Vorbereitung.
- Die Hauptseite der Live-Demo bekommt einen Link auf die separate Standalone-
  Vorschau. Der normale Web-Flasher bleibt bei den veroeffentlichten Dateien.
- `tools/prepare_standalone_candidate.py` baut nur ein verifiziertes Test-ZIP.
  Das Werkzeug kann weder Tags noch Releases anlegen und behauptet keine Abnahme.
- Der untenstehende Release-Text ist als Entwurf fertig.

## Erst nach Hardwareabnahme und ausdruecklichem Okay

1. Obige Tabelle und tatsaechlich getestete SHA-256/CI-Commit dokumentieren.
   Bei Codeaenderungen nach dem Test erneut bauen und die betroffenen Punkte
   erneut abnehmen; fremde oder alte Builddateien nicht zum neuen Tag umbenennen.
2. Zielumfang festlegen: Standalone als klar benannte zusaetzliche T-Deck-Variante.
   Die bisherigen zwei WLAN/BLE-Pakete bleiben getrennt. Standalone ist derzeit
   bewusst nicht in `tools/devices.json` / `tools/release_boards.json` und nicht
   im stabilen Flasher. Das vorbereitete Standalone-ZIP enthaelt bereits
   Update-/Factory-Abbild, validierte Offsets, Anleitung und Pruefsummen. Es ist
   als klar benannte dritte Variante neben den beiden bisherigen Paketen
   zu verwenden; keine WLAN-Firmware durch die Standalone-Datei ersetzen.
3. Fuer den finalen Release-Commit `VERSION=0.9.5` setzen und `python3 tools/version.py`
   ausfuehren. Web-/Paketkennungen und die dokumentierten Downloadnamen folgen
   daraus. Kandidatenhinweise erst nach tatsaechlicher Freigabe umstellen.
4. Normale T-Deck-/T-Watch-Pakete und Standalone-Finalpaket aus genau diesem
   Commit bauen, Header/Partitionen/RAM und SHA-256 pruefen. Versionsaenderungen
   erzeugen neue Bytes; die Abnahme muss zur tatsaechlich veroeffentlichten Datei
   passen. CI-Erfolg ersetzt diesen Nachweis nicht.
5. Erst auf ausdruecklichen Auftrag PRs zusammenfuehren, Tag `v0.9.5` und Release
   erzeugen. Der bestehende Release-Workflow veroeffentlicht bei einem v*-Tag:
   deshalb jetzt **keinen** solchen Tag setzen oder Workflow manuell starten.
6. Erst nachdem die Assets existieren den stabilen Flasher auf sie umstellen,
   SHA-256 gegenpruefen und die Pages-Seite veroeffentlichen.

## Fertiger Release-Text (Entwurf, noch nicht veroeffentlichen)

DisAHRsterRadio-MeshCore 0.9.5 erweitert den T-Deck Plus um die vollstaendige
Standalone-Bedienung ohne WLAN: Chat, Schnellmeldung, Brett, Offlinekarten,
Standort und Geraet direkt am Display. Grosse getrennte Touch-Flaechen,
scrollbare Listen und eine feste Zurueck-Navigation machen die Bedienung mit
einem Finger einfacher. Tastatur und Trackball bleiben nutzbar.

Die Touch-Kalibrierung wird dauerhaft gespeichert und ist wiederholbar.
Senden bleibt eine explizite Aktion. Eine Wischbewegung sendet keine Nachricht;
die Empfaengerauswahl bleibt auch beim Aktualisieren von Kontakten eindeutig.
Ohne frischen GPS-Fix zeigt die Karte Heimersheim Mitte, ohne diese Ansicht
als eigene Position zu behandeln oder zu senden.

T-Deck Standalone und die normalen WLAN/BLE-Varianten sind getrennte Pakete.
Die T-Watch S3 behaelt ihre bisherige Bedienung. Installation, Pruefsummen,
Hardwarestatus und die zugehoerigen Dateien muessen vor Freigabe dieses Textes
aus der finalen Abnahme ergaenzt werden.
