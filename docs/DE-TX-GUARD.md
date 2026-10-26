# DE-Sendeschutz für DAR-Companions (#47)

Stand: 26.10.2026. Gilt für dar_LilyGo_TDeck_wifi,
LilyGo_TDeck_standalone und dar_LilyGo_TWatchS3_wifi auf ESP32.
Unabhängiger Fix auf main, ohne Abhängigkeit vom noch offenen SOS-PR #57.
Keine automatische Veröffentlichung oder Hardwarefreigabe.

## Nachgewiesenes Problem und notwendige Kernausnahme

Die geprüfte Basis 81dbae592ffad40b4212bc0b37e82b38da08f4bc verwendete ein
Tokenbudget, nicht eine persistente gleitende Stundenbilanz. Im aktiven
Companionpfad waren der DAR-Fensterlimiter und dessen NVS-Backend deaktiviert.
Faktor 1 entspricht rechnerisch 50 %, Faktor 9 allein garantiert noch kein
gleitendes 10-%-Limit. Neustarts restaurierten freie Tokens; Timeouts wurden
nicht abgerechnet. Freie technisch gültige Funkparameter waren nicht an eine
Region gebunden. Die bisherigen Original-TX-Tests dokumentieren diese Fälle.

Die isolierte Ausnahme zur Architektur #52 aktiviert mit
DAR_REGIONAL_TX_GUARD den bereits vorhandenen konservativen Dispatcherpfad
für beide Frontends. DAR_ORIGINAL_MESHCORE_TX bleibt als Kompatibilitätsflag
für App-/BLE- und Parametereinstieg bestehen; es bedeutet dort keine Ausnahme
vom Sendeschutz mehr. OriginalMeshCoreTx.inc bleibt bytegleich erhalten.
Mesh.cpp, Paketformat, Routing, Verschlüsselung und native Retries werden nicht
verändert. Bereits vorhandene Prioritäten bleiben erhalten; SOS kann weder
Budgets noch persistente Sperren umgehen. Andere Buildrollen werden durch das
neue Flag nicht umgestellt und erhalten dadurch keine Freigabe.

## DE-Regel und Grenzen

Primärquelle: Bundesnetzagentur, Vfg. 91/2025, Band 54 (Seite 17):
https://www.bundesnetzagentur.de/DE/Fachthemen/Telekommunikation/Frequenzen/Allgemeinzuteilungen/_DL/vfg91_2025.pdf?__blob=publicationFile&v=3

Für 869,4–869,65 MHz nennt die Verfügung maximal 500 mW ERP sowie
Frequenzzugangs-/Störungsminderungsverfahren, alternativ Arbeitszyklus <=10 %.
Dieser Fix wählt den konservativen Duty-Cycle-Weg. CAD allein wird nicht als
Nachweis alternativer Verfahren ausgegeben. Keine pauschale EU-/UK-Freigabe.

Die App-Policy akzeptiert nur technische Werte mit nominaler vollständiger
Bandbreiten-Einbettung in dieses Subband, kanonischer Frequenz in ganzen kHz
und Bandbreite in ganzen Hz; SF/CR und diskrete Bandbreite prüft zusätzlich
der Treiber. Der Leistungsdeckel von 10 dBm ist eine konservative
Chip-Einstellung, keine automatische ERP-Zertifizierung. Externe Verstärker,
Antennengewinn, Nebenaussendungen und tatsächliche Bandgrenzen benötigen eine
reale HF-Prüfung. Andere Subbänder/Länder sind in dieser Policy gesperrt.

## Abrechnung und Persistenz

Alle Mesh-Ausgänge, einschließlich ACK, Advert, Weiterleitung, App und
lokaler Nachrichten, erreichen denselben Dispatcher. Vor Entnahme aus der
Queue werden exakte Paketlänge, mindestens 150 % geschätzte Airtime, freies
Tokenbudget, gleitendes Fenster, Profil und Hardwarezustand geprüft. Fehlende
Schätzung, ungültige Konfiguration oder Speicherfehler geben keinen TX frei.
Die bestehende Queue-Fälligkeit wird während langsamer Flash-Commits eingefroren.

Erfolg, fehlgeschlagener Start und Timeout werden einschließlich Treiber-/
Abbruchzeit konservativ abgerechnet: mindestens die Reserve, ansonsten die
längere verstrichene Zeit. Überschreitungen bleiben als Schuld erhalten und
blockieren weitere Freigaben. Softwarezeit ist keine gemessene HF-Airtime;
ein hängenbleibender Funkchip kann nicht durch eine Hostprüfung ausgeschlossen
werden. 61 Minutenbehälter halten jede Gebühr mindestens eine Stunde vor.

Der feste Faktor 9 ergibt 360000 ms pro Stunde; Profile teilen dieselbe
Historie. Eingehende App-Zeit, GPS-Zeit, Tuningbefehle, CLI und Profilwechsel
setzen sie nicht zurück. Das Fenster läuft mit der lokalen Millisekundenuhr.
NVS dar-txwin/marker muss vor TX Dirty V2 committen und identisch zurücklesen.
Kein Backend, Commit-/Lesefehler oder unbekannter Marker bedeutet keine Freigabe.

Erstinstallation/Update mit fehlendem, Dirty- oder altem V1-Marker verlangt eine
volle Stunde kontinuierliche stille Laufzeit. Auch ein historischer V1-Clean-
Marker ist unzuverlässig, weil zwischenzeitlich ungeschützte Firmware gesendet
haben kann. Empfang/UI laufen weiter. Neustart während der Pause beginnt sie
neu; ausgeschaltete Zeit zählt nicht. Clean V2 wird erst nach verfallener
Historie und ohne laufenden Versuch geschrieben. Kein automatisches NVS-Löschen.
Downgrade auf Firmware ohne Guard kann diese Sicherung nicht gewährleisten;
solche Downgrades sind kein freigegebener Betriebs-/Recoveryweg.

## Einstellungen und Bedienung

Neue Preferences verwenden Narrow (869,618 MHz/62,5 kHz/SF8/CR4:8), 10 dBm und
Faktor 9. Gespeicherte Frequenz, Bandbreite und Leistung werden nicht heimlich
überschrieben. Unzulässige Altwerte sperren TX und bleiben diagnostizierbar.
Die PIN-authentifizierte App kann ein passendes Profil und <=10 dBm anwenden;
der Treibererfolg bleibt Voraussetzung. CLI liest weiter Status, darf diese
Funk-/Leistungs-/Duty-Cycle-Policy aber nicht umgehen. Wirksames Tuning wird
als Faktor 9 gemeldet, unabhängig von alten gespeicherten Werten.

## Prüfung und verbleibende Abnahme

Hosttests prüfen in beiden Frontend-Konfigurationen den tatsächlich gewählten
Dispatcher, Speicherfehler, Timeout-/Startfehler-Abrechnung, Budgetschuld,
exakte Reserven, Queue-Priorität, langsamen Commit und millis-Überlauf.
Unabhängige Historienvergleiche im Fenstertest prüfen adversariellen Verkehr
über mehrere Zählerüberläufe. Policytests prüfen Bandkanten, Überlappungen,
NaN/Inf und 2255 kanonische Frequenz/Bandbreiten-Kombinationen. NVS-Tests
prüfen Commit, Readback, alte Marker und Neustarts. Statische Prüfungen sind
nur ergänzend; Firmware-CI muss die echten Frontends kompilieren.

Vor Integration auf echtem Gerätepaar prüfen: einstündige Migrationspause bei
weiterlaufendem Empfang/UI; Freigabe danach; App-Readback und Korrektur eines
22-dBm-Altstands; Ablehnung überlappender Profile/zu hoher Leistung; Profil-
wechsel ohne neues Budget; Neustart nach TX und Stromausfall am Marker;
langsame/fehlende TX-done/Abbruchfälle; HF-Abschaltung, Airtime, ERP/Spektrum
und realer Nachrichtenaustausch. Test-BIN mit Commit und SHA256 protokollieren.
Bestehende Nutzerabnahme wird nicht auf diese neue Funkpolicy übertragen.

Lokaler Nachweis: vollständige Host-Gates mit UndefinedBehaviorSanitizer bestanden.
Die Dispatcherregression scheitert erwartungsgemäß am unveränderten main
(f356bf54, aktives DISASTER_STANDALONE), besteht mit dem Fix für beide
Frontendflags. AddressSanitizer/LeakSanitizer wird wegen der lokalen /proc-
Beschränkung erst in GitHub-CI bewertet. Keine lokale PlatformIO-Installation,
keine unabhängige HF-/Hardwareprüfung.
