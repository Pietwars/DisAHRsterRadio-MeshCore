# T-Deck Plus: Touch-Bedienung, Release 0.9.6

Öffentliche Standalone-Variante für T-Deck Plus. Die technische Dateikennung
`standalone-test6` bleibt erhalten. Der Betreiber hat Privatversand sowie
SD-Erkennung und Laden am eigenen T-Deck Plus bestätigt. Vollständige
Hardwareabnahme und Tests der neuen WLAN-Integration werden nicht behauptet.
[Verifizierungsstand](RELEASE-0.9.6.md).

**Dokumentationskorrektur:** Der Kopfabsatz „Kein Release, kein Merge“ in der
ursprünglich mit dem 0.9.6-ZIP gelieferten `BEDIENUNG.md` ist überholt.
0.9.6 ist ein regulärer öffentlicher Release. Diese aktualisierte Anleitung
ergänzt das unveränderte veröffentlichte Firmware-Paket.

## Bedienung

| Bereich | Touch-Bedienung |
|---|---|
| Navigation | Oben zwischen Chat, Privat, Meldung, Brett, Karte und Geraet wechseln. Darunter links ist immer dieselbe Zurueck-Flaeche. Zurueck aus einer Unterhaltung fuehrt zur Liste, danach zum Kachelmenue. |
| Hauptmenue | Sechs grosse Kacheln in zwei Spalten; auch Standort ist direkt erreichbar. |
| Chat / Privat / Brett | Vier getrennte Zeilen pro Bildschirm. Antippen oeffnet die Unterhaltung. Hoch/runter wischen blaettert mehrere Zeilen; der letzte Ausschnitt bleibt erhalten. |
| Unterhaltung | Nachrichtenverlauf vertikal wischen. Text mit der physischen Tastatur schreiben. Nur der separate grosse Senden-Button oder Enter/Trackball-Klick sendet. Im Brett ohne Text: Beitreten. |
| Meldung | Vorlage, Ort oder Empfaenger antippen: eigene Auswahlliste. Wischen blaettert; eine Zeile waehlt aus. Zurueck schliesst zuerst die Auswahl. Texteingabe per Tastatur, Leeren ist separat. Meldung und reine Position haben getrennte Sendeaktionen. |
| Karte | Mit einem Finger ziehen. Unten Zoom minus, Mein GPS, Zoom plus. Karte > waehlt die naechste Offline-Karte. Kein Pinch-Zoom noetig. |
| Standort | Grosse GPS- und Freigabeschalter. Position senden und Bekanntgabe sind getrennte Aktionen. |
| Geraet | Aktionen vertikal wischen: Kontakte, Bekanntgabe, Display, SD, Touch-Kalibrierung, Standort, Geraeteinfo/Akku. Technische Infos stehen auf einer eigenen Seite. |

Vom linken Bildschirmrand nach rechts wischen geht eine Ebene zurueck. Auf der
Karte dient ein Zug im Kartenbereich zum Verschieben; der Rand bleibt fuer
Zurueck reserviert. Tastatur, ALT+Backspace und Trackball funktionieren weiterhin.
Die Touch-Bereiche sind auf das echte 320 x 240 Display abgestimmt: sechs gleichmaessig verteilte Tabs,
Listen und Einstellungsaktionen 312 x 34, Menuekacheln 154 x 48, Senden 86 x 40
Pixel. Zwischen benachbarten Aktionen liegen freie Abstaende.

## Schutz vor Fehlbedienung

- Tippen wird erst beim Loslassen auf derselben dargestellten Aktion erkannt.
- Mehr als 12 Pixel Bewegung, Wegziehen und Zurueckkehren oder ein Halten ueber
  700 ms loest keinen Tipp aus. Eindeutige Wischbewegungen werden getrennt erkannt.
- Wischen auf Senden loest keine Nachricht aus. Die Textflaeche selbst sendet nie.
- Aendert sich der dargestellte Inhalt unter einem aufgelegten Finger, wird der
  Kontakt verworfen. Ein erneutes Antippen ist erforderlich.
- Kontakt- und Empfaengerauswahl verwenden die angezeigte ID. Aendert sich die
  Reihenfolge oder verschwindet ein Kontakt, wird kein anderer Empfaenger gewaehlt.
- Mehrfachkontakt wird wie bisher verworfen. Ein dunkles Display wird nur geweckt.
- Kalibrierung bleibt im NVS gespeichert und kann unter Geraet erneut gestartet
  werden. Die laengere Beruehrungsdauer der Kalibrierung bleibt separat erhalten.
- GPS-Freigabe, frischer Fix und Heimersheim als reine Kartenansicht bleiben
  unveraendert. Eine Karte oder ein Wischen aktiviert niemals Standortfreigabe.

## Pruefung und Vorschau

`tests/touch_ui.test.cpp` prueft Loslassen, Abstaende, Scrollen, Abbruch von
Sende-Gesten, stabile Empfaenger, alle Menuekacheln, Einstellungen und GPS-Schutz.
Die bestehenden UI-, Tastatur-/Trackball-, Nachrichten- und GPS-Tests laufen
weiter. CI fuehrt die C++-Tests mit ASAN/UBSAN und den Standalone-Firmwarebuild
einschliesslich Abbild-/Partitions- und WLAN-/Web-Ausschlusspruefung aus.

`tools/standalone_preview.py` rendert denselben C++-Code mit Demodaten und der
echten GFX-Schrift. Die PNGs werden dem CI-Testartefakt beigelegt. Sie zeigen das
reale Layout, sind aber keine Fotos des Geraets. Die Firmwarekennung lautet
`v1.18.0-dar0.9.5-standalone-test6`.

Am Geraet bitte pruefen: Fingergenauigkeit nach Kalibrierung, Wischen langer
Listen, Kartenbewegung, Abbrechen von Sende-Gesten, Empfang waehrend Touchkontakt,
Erhalt der Kalibrierung nach Neustart sowie Tastatur-/Trackballbedienung.

## Privatchat in 0.9.6

Oben **Privat** öffnen, einen gespeicherten persönlichen Kontakt wählen und die Nachricht mit der beschrifteten Sendeaktion verschicken. **Chat** enthält nur Kanäle. Kontakte werden aus dem maßgeblichen MeshCore-Speicher geladen. Beim ersten Öffnen des neuen Privatverlaufs werden ältere Direktnachrichten ausgeblendet; Kontakte und gemeinsamer Verlauf bleiben erhalten. Die Anzeige einer Übergabe ist keine menschliche Empfangsbestätigung. Privatversand ist vom Betreiber bestätigt; ein gesondert dokumentierter Parallelbetriebs-/Langzeittest steht noch aus.
