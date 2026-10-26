# SOS über native MeshCore-Direktnachrichten

Teilaufgabe aus #47. Kein neues LoRa-Paketformat, keine Änderung der
Verschlüsselung und keine Umgehung des MeshCore-Schedulers.

## Bedienung

**T-Deck Standalone:** Unter Meldung einen hinzugefügten privaten Chat-Kontakt als
Ziel wählen. SOS öffnet zunächst nur die Übersicht. „Vorbereiten“ und anschließend
rechts „Jetzt SOS“ innerhalb von zehn Sekunden bestätigen. Mindestens 750 ms
Abstand verhindern den unmittelbaren Doppeltipp; Tastatur/Trackball müssen vom
Vorbereiten-Feld zum Bestätigen-Feld wechseln. Zurück bricht die Vorbereitung ab.
Ein gestartetes SOS kann rechts mit „SOS stoppen“ beendet werden. Direkt nach dem
Start verhindert eine Sekunde Sperre versehentliches Abbrechen durch denselben
Doppeltipp.

**WLAN-Weboberfläche:** Unter Privat den Chat-Kontakt wählen. Optional frisches
Geräte-GPS für genau dieses SOS ausdrücklich freigeben (standardmäßig aus).
„SOS vorbereiten“ zeigt den Empfänger, erst der getrennte Bestätigungsknopf startet.
Die Auswahl wird an den vollständigen gespeicherten öffentlichen Kontaktschlüssel
gebunden; ein veränderter Kontakt muss neu bestätigt werden. Die Demo sendet kein
SOS. Der offene WLAN-Zugang bleibt ein gemeinsamer Kommunikationszugang und
wird durch diese Bedienbestätigung nicht zu einer authentifizierten Administration.

**Annahme:** Der Empfänger liest eine normale verschlüsselte Direktnachricht mit
einer eindeutigen Kennung und antwortet über seine vorhandene MeshCore-App exakt:
`SOS OK <Kennung>` (die vollständige Antwort steht in der Nachricht).
Nur eine entschlüsselte native Direktnachricht desselben vollständigen
Empfängerschlüssels mit passender Kennung beendet den Vorgang als angenommen.
Kanal-/CLI-/HTTP-Nachrichten, falsche Schlüssel, alte Kennungen und Antworten nach
Abbruch/Ablauf übernehmen diesen Status nicht. Der Text ist eine ausdrückliche
Annahmeerklärung des Empfängers, kein Nachweis tatsächlicher Hilfeleistung oder
der Anwesenheit einer bestimmten Person.

## Zustände und Wiederholungen

| Anzeige | Bedeutung |
|---|---|
| SOS: Annahme offen | Dauerhaft gespeichert; Übergabe bzw. Empfängerantwort noch offen |
| Gerätequittung; Annahme offen | Native ACK empfangen; noch keine passende Annahmeantwort |
| Versuche beendet | Keine weiteren automatischen Versuche; Annahmeantwort bis Ablauf möglich |
| SOS vom Empfänger angenommen | Passende ausdrückliche Antwort desselben Schlüssels gespeichert |
| Abgebrochen | Weitere Versuche gestoppt; bereits übergebene Pakete können noch ankommen |
| Abgelaufen | 30-Minuten-Frist beendet; keine weitere automatische Sendung/Annahme |

Maximal vier Übergaben an den ursprünglichen Companion-Sendepfad. Abstände nach
Versuch eins bis drei: 1, 3 und 10 Minuten; nach dem vierten Versuch endet die
Wiederholung. Eine Gerätequittung allein stoppt die begrenzten SOS-Wiederholungen
nicht. Nach Neustart warten bereits begonnene Aufträge wie die übrige Outbox
mindestens fünf Minuten und benötigen eine gültige Gerätezeit. Die Frist wird
nicht verlängert. Ein Speicherfehler stoppt weitere Übergaben und veröffentlicht
keinen unbestätigten Zustandswechsel.

Die Outbox wählt fällige SOS-Aufträge vor gewöhnlichen gespeicherten Aufträgen.
Neue normale lokale/HTTP-Kommandos warten, wenn ein SOS fällig ist. Ein Platz der
acht Outboxplätze bleibt für SOS reserviert: maximal sieben gewöhnliche offene
Aufträge. Bereits gespeicherte Aufträge werden nicht gelöscht. Auf schon früher
vollständig belegten Altbeständen kann SOS bis zur Freigabe eines Platzes
abgewiesen werden. Bereits an den Funkkern übergebene Pakete werden nicht
unterbrochen oder umsortiert; ACKs, Routing und Sendelimits bleiben im Originalkern.
Das ist Anwendungsvorrang, kein Nachweis einer über alle Kernqueues hinweg
präemptiven Funkpriorisierung oder regionaler Funkregelkonformität.

Frische GPS-Daten werden am Standalone nur bei aktivierter Standortfreigabe
übernommen; im Web nur nach ausdrücklicher Freigabe für diese Nachricht.
Ohne Freigabe bzw. frischen Fix steht der Grund im Nachrichtentext. Der Standort
ist eine Momentaufnahme beim Start, keine laufende Teilnehmerverfolgung.

## Speicherung und Rückweg

Recordgröße und normale Zustände bleiben unverändert. Neue SOS-Zustände werden
von älterer Firmware jedoch nicht erkannt. Ein Downgrade setzt diese SOS-Aufträge
nicht fort; vor einem Rückweg aktive Vorgänge bewusst beenden und Daten sichern.
Versuchszähler wird vor der Übergabe gespeichert. Stromausfall zwischen Funk und
Speichern kann weiterhin Wiederholungen verursachen; keine Exactly-once-Zusage.

## Nachweise und reale Abnahme

Hosttests: Priorität/Reserve, maximal vier Versuche, Speicherfehler,
Neustart/Annahme-Persistenz, Abbruch/Ablauf, falscher Schlüssel mit gleichem
Kurzpräfix, native Gerätequittung versus Annahme, Standortfreigabe und ungültiger
GPS-Fix. Companion-Test verwendet die echten CMD_SEND_TXT_MSG-/ACK-Pfade.
UI-Tests prüfen getrennte Bestätigung, Verzögerung, Ablauf und Abbruch. Ein
Browsertest verwendet simulierte HTTP-Endpunkte und keinen echten Funkknoten.

Auf zwei echten Geräten neu prüfen: verschlüsselte SOS-Nachricht, gültige/falsche
Antwort, Gerätequittung ohne Annahme, Ausfall des Empfängers, Abbruch, Neustart,
GPS ohne/mit Freigabe und Speicher-/Funklimitfälle. Vorherige Nutzerabnahme gilt
für den vorherigen Stand und beweist diese neu eingeführten Funktionen nicht.
