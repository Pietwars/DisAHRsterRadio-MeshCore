# 0.9.6 TEST – gleitendes Sendefenster und Neustartschutz

Status: Softwareimplementierung für die DAR-Companion-Targets auf ESP32.
Keine Funkmessung, keine Hardwarefreigabe, keine vollständige Funkzulässigkeitsprüfung.
Der feste Faktor 9 ersetzt weiterhin keine Region-/Subband-/Profilvalidierung.

## Sichtbares Verhalten

- Nach einem Neustart mit unbekannter Sendehistorie wird **eine volle Stunde
  kontinuierliche Laufzeit ohne neue Aussendungen** beobachtet. Das betrifft auch
  den ersten Start nach dem Update von einer Firmware ohne diesen Marker, eine
  gelöschte/defekte NVS-Partition oder eine neue Installation.
- **Empfang und lokale Bedienung laufen weiter.** Bereits eingegebene Nachrichten
  können in der normalen Queue warten; Annahme durch den Funkkern bedeutet noch
  keinen tatsächlichen Funkabschluss und keine Zustellung.
- Keine Ausnahme für SOS, höchste Priorität, ACK, Chat, Weiterleitung oder Advert.
  Die Begrenzung gilt für alle Sendepfade durch denselben Dispatcher.
- Standalone: Gerät → Geräteinfo/Akku → Funk zeigt Neustartschutz, Speicherschutz
  oder wartendes Sendebudget. Das Webportal zeigt den Grund zusätzlich oberhalb
  der Seiten. Die Restzeit wird auf Minuten aufgerundet.
- Ein erfolgreicher Neustart nach nachweislich abgelaufener Aktivitätsspur erfordert
  keine zusätzliche Startpause. Ein weiterer Neustart während der Schutzzeit
  startet sie erneut; ausgeschaltete Zeit, App-Uhr und GPS werden nicht angerechnet.
- Ein fehlgeschlagener Marker-Schreibvorgang sperrt neue Aussendungen auch nach
  Ablauf der Startpause. Keine automatische Löschung oder Formatierung zur Umgehung.

Diese konservative Erststart-/Neustartpause ist ein bewusster Funktionseingriff,
kein bestätigter Hardwarefehler. Eine kürzere Wiederaufnahme erfordert später eine
verlässlich restaurierbare Historie oder eine unabhängig vertrauenswürdige Uhr.

## Gleitendes Fenster

`TxWindowLimiter` hält 61 Minutenbehälter ohne dynamische Speicherbelegung.
Jeder verbuchte Betrag bleibt mindestens 60 und höchstens 61 Minuten erhalten.
Ein abgelaufenes Minutenstück kann also bis zu knapp einer Minute länger sperren.
Die Summe wird gegen das konfigurierte Stundenbudget geprüft; laufendes Auffüllen
des zusätzlichen Tokenbudgets kann diese Grenze nicht umgehen.

Pro abgeschlossenem Versuch wird am Abschlusszeitpunkt mindestens die bereits
geforderte 150-%-Airtime-Reserve verbucht, bei längerer gemessener Treiberzeit diese
längere Zeit. Das gilt auch bei fehlgeschlagenem Sendestart und Timeout. Dadurch
sind Nutzung und Wiederfreigabe konservativer als reine gemessene Funkzeit.
`getTotalAirTime()` bleibt separat die beobachtete Softwarezeit; das angezeigte
Restbudget ist das Minimum aus Token- und Fensterbudget. Echte HF-Airtime und
korrektes Abschalten des Funkchips müssen weiterhin gemessen werden.

Das Fenster nutzt ausschließlich die lokale Millisekundenuhr mit unsigned
Differenzen. Es wird in jedem Dispatcher-Durchlauf fortgeschrieben, auch ohne
wartende Pakete; normale 32-Bit-Überläufe werden berücksichtigt. Ein Thread, der
für einen ganzen Millisekunden-Zählerumlauf nicht läuft, ist kein freigegebener
Betriebsfall. Profile dürfen das Fenster nicht durch Neuinitialisierung umgehen.
Band-/Regionwechsel sind weiterhin gesondert zu lösen; diese Änderung schafft
keinen neuen Funkprofil-Editor und keine pauschale Länderfreigabe.

## Dauerhafter Aktivitätsmarker

`Esp32TxWindowStore` verwendet die separate NVS-Namespace `dar-txwin`, Schlüssel
`marker`, versionierte Clean-/Dirty-Werte. Nur ein lesbarer bekannter Clean-Wert
belegt eine zuvor beobachtete stille Stunde. Fehlender Schlüssel, falscher Typ,
unbekannter Wert oder Lesefehler gelten als unbekannte Vorgeschichte.

Vor dem Entfernen eines Pakets aus der Queue und vor `startSendRaw()` muss ein
Dirty-Marker erfolgreich mit `nvs_set_u32()` **und `nvs_commit()`** gesichert sein.
Bei Fehler bleibt das Paket mit Priorität und Fälligkeit in der Queue. Wiederholte
Schreibfehler werden höchstens einmal pro Sekunde erneut versucht. Beim erfolgreichen
laufenden Senden wird der Marker wiederverwendet, statt für jedes Paket Flash zu
schreiben. Erst wenn kein Versuch mehr läuft und alle Gebühren verfallen sind,
wird Clean geschrieben. Ein unklarer Clear-Erfolg erzwingt vor der nächsten TX
immer einen neuen Dirty-Commit. Eine Unterbrechung zwischen Marker und Funkstart
führt deshalb konservativ zur Startpause.

Die einmal geprüfte Queue-Fälligkeit wird während eines möglichen langsamen
Flash-Commits beibehalten: Ein danach neu fälliges größeres Paket darf nicht den
bereits geprüften Kandidaten ersetzen. Fehlerhafte Pakete ohne Funkaufruf können
den Versuch abbrechen; ein aufgerufener Treiber benötigt stets Abschlussabrechnung.

Quelle für den Speichervertrag: [Espressif ESP-IDF NVS API](https://docs.espressif.com/projects/esp-idf/en/v4.4.7/esp32/api-reference/storage/nvs_flash.html),
Abschnitte `nvs_set_*`, `nvs_commit` und `nvs_close`. Die Firmware wertet die
Erfolgscodes aus; die dauerhafte Speicherung und Stromausfallsicherheit bleiben
Teil der realen Geräteabnahme.

## Nachweise und verbleibende Abnahme

- Reine Fensterlogik: Quotengrenze, keine Zwischenauffüllung, Minutenrand,
  Überlauf, Neustart mitten im Versuch, fehlender Speicher, Schreib-/Clear-Fehler,
  lange Versuche; 10.000 deterministische Abläufe gegen eine unabhängige Historie.
- Echter Dispatcher mit Fake-Radio/-Uhr: Queue-Erhalt bei Speicherfehler,
  höchste Priorität, Annahme nach Ablauf der Sperre, RX während Startschutz,
  Erfolg/Timeout/fehlgeschlagener Start, langsamer Flash-Commit und Paketauswahl.
- Produktions-Speicheradapter mit Fake-NVS: Öffnen/Lesen/Schreiben/Commit-Fehler,
  fehlende/unbekannte Marker und Neustart nach bestätigtem Dirty-Commit.
- UI-/Webtests: Sperrgründe, Rundung, keine Anzeige einer Sperre ohne Grund,
  unveränderte Bildschirmgrenzen und reproduzierbare eingebettete Web-Assets.

Auf echter Hardware offen: Stromunterbrechung an allen Marker-/TX-Grenzen,
Watchdog/Reset, erste Aktualisierung und NVS-Fehler, paralleles RX während NVS,
LoRa-TX-Abschluss/Abbruch, Dauerlauf und Funkmessungen. Nicht-ESP32-DAR-Profile,
Room-Server-Firmware außerhalb dieses Companion-Dispatchers und ältere Firmware
sind durch diese Integration nicht als entsprechend geschützt freigegeben.

Hostaufruf: `python tools/run_096_host_tests.py` (Python, Node.js und g++).
GitHub Actions baut anschließend WLAN und Standalone; kein Flash/Release.
