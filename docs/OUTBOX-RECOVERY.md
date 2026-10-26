# Gespeicherter Nachrichtenausgang: Fehler und Wiederanlauf

Teilaufgabe aus #47. Die Änderung betrifft ausschließlich die Anwendungsschicht
`examples/companion_radio/DeliveryOutbox.h`; Original-MeshCore-Kerndateien,
Funkpakete, Prioritäten und Sendelimits werden nicht geändert.

Statusübergänge werden als Kopie geschrieben, geschlossen und vollständig
zurückgelesen. Erst bei bytegleicher Bestätigung wird der sichtbare RAM-Datensatz
ersetzt. Das gilt für Einreihen, Versuchszähler, ACK-Tags, Empfängerquittung,
Abbruch, Ablauf und ausgeschöpfte Versuche. Ungültige Aufträge werden vor dem
Speichern abgewiesen. Das Dateiformat bleibt unverändert.

Bei einem Schreib-/Lesefehler stoppt die Outbox weitere Veränderungen und
Sendeversuche. Der sichtbare Status bleibt der letzte bestätigte Stand; ein Fehler
kann deshalb nicht fälschlich „zugestellt“ oder „abgebrochen“ anzeigen. Eine
Empfängerquittung bedeutet weiterhin keine menschliche Bestätigung.

Ein fehlgeschlagener Rückleseversuch kann einen tatsächlich geschriebenen
Datensatz betreffen. Deshalb nicht im laufenden Betrieb erneut schreiben oder
senden. Nach Neustart werden wie bisher die gültigen Doppelkopien ausgewertet.
Ein wiederhergestellter Auftrag mit mindestens einem Versuch wartet fünf Minuten,
bevor er erneut fällig wird. Es gibt keine atomare Transaktion zwischen Funk und
Dateisystem: Eine Wiederholung nach Stromausfall bleibt möglich. Die Änderung
verspricht keine Exactly-once-Zustellung und keine automatische Flash-Reparatur.

## Nachweise

`tests/delivery_outbox.test.cpp` prüft partielle Schreibvorgänge, fehlgeschlagene
Versuchsspeicherung, geschriebenen Versuch mit fehlgeschlagener Rückleseprüfung,
Neustart, abweichende aber checksumgültige Rücklesedaten, fehlgeschlagene
ACK-/Abbruch-/Ablaufspeicherung, Eingabevalidierung und Retry-Grenzen einschließlich
millis-Überlauf. Der erste Fehlerfall reproduziert den RAM-Statusfehler am alten
Code. Der Test ist in `tools/run_096_host_tests.py` eingebunden.

Lokal am 26.10.2026: neuer Outbox-Test und beide bestehenden Companion-Tests mit
UndefinedBehaviorSanitizer bestanden; statische Gates, Portal-Routentest und
Webregression bestanden. Die vollständige Address-/LeakSanitizer-Suite ist in der
lokalen Umgebung nicht lauffähig (siehe Baseline-PR #53). PlatformIO ist lokal
nicht installiert; Target-Builds werden separat durch die PR-CI geprüft.

Am echten Gerät ausstehend: voller/beschädigter Flash, Stromausfall während jeder
Zustandsänderung, Wiederanlauf sowie Direktversand und Empfängerquittung mit
zweitem MeshCore-Knoten. #47 bleibt bis zur vollständigen Abnahme offen.
