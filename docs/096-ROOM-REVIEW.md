# 0.9.6 TEST – Room-Rechte und Sendebudget

Erster Prüfblock: neue Passwortanmeldungen und per-client Sendepause.

- Das öffentliche Kennwort `hello` vergibt auch dann keine Adminrolle, wenn es
  versehentlich als Admin-Passwort eingestellt wurde. Ebenso kann ein mit dem
  Gastkennwort identisches Admin-Passwort keine Adminrolle vergeben.
- Ein eigenständiges, nicht öffentliches Admin-Passwort funktioniert weiterhin.
  Die Prüfung erzwingt Verschiedenheit, keine vollständige Passwortstärkeprüfung.
- Die 15-Sekunden-Pause wird vor dem Zurücksetzen des Zehn-Minuten-Fensters
  geprüft. Ein abgelehnter Versuch löscht weder Zeitstempel noch Zähler.
- Hosttests decken die Kennwortkollision, Grenze des Zeitfensters, sechs Beiträge,
  Zeitpunkt null und den 32-Bit-Zeitüberlauf ab.

**Weiter offen:** Bereits gespeicherte ACL-Adminrechte bleiben unverändert.
Blankes Passwort kann bei bereits bekannter, kryptographisch identifizierter
ACL-Identität weiterhin deren gespeicherte Rolle verwenden. Bei einer früheren
Admin/Gast-Kennwortkollision müssen die bestehenden Admin-Identitäten überprüft
und unerwünschte Rechte ausdrücklich entzogen werden; eine Passwortänderung allein
ist kein Nachweis dieser Bereinigung. Keine automatische Löschung legitimer ACLs.

Nächste Schritte: gespeicherte Rollen und Rechteentzug, Wiederverbindungen,
Teilnehmerplatz-Wiederverwendung, Replay-/ACK-Fälle sowie Portal-Client-ID-Wechsel,
volle Budgettabellen und parallele HTTP-Clients. Die Room-Server-Firmware benötigt
weiter einen eigenen Target-Build und Laufzeittest; grüne T-Deck-Companion-Builds
sind dafür kein vollständiger Nachweis. Keine Hardware- oder Releasefreigabe.

## Zweiter Prüfblock: Rechteentzug und Wiederholungsbestätigung

Die DISASTER_ROOM-Veröffentlichungsprüfung sperrt nun sowohl Gast (0) als auch
explizit nur Lesen (1). Zuvor wurde nur Rolle 0 abgewiesen, sodass ein auf Rolle 1
herabgestufter Teilnehmer weiter Beiträge einstellen konnte. Rollen 2 und 3 bleiben
schreibberechtigt, zusätzliche ACL-Flags ändern diese Rollenprüfung nicht.
Eine Herabstufung wirkt auch auf ACKs wiederholter früherer Beiträge.

Wiederholungs-ACKs erfordern außerdem einen passenden vollständigen Public Key des
Budget-/Bestätigungsbesitzers. Ein passender Zeitstempel allein genügt nach
Slot-Wiederverwendung oder ACL-Verschiebung nicht. Das kann konservativ eine ACK
nach Tabellenumbau unterdrücken; es darf keine fremde Annahme bestätigen.
Hosttests prüfen alle 256 Berechtigungsbytes sowie Besitzer-, Annahme- und
Zeitstempelabweichungen. Statische Tests binden diese Regeln an den Paketpfad.
Der CI-Workflow baut zusätzlich das vorhandene Target
`disaster_t3s3_sx1262_room`; der jeweilige Lauf muss separat erfolgreich sein.

Offene ACL-Prüfung: `ClientACL::putClient()` besitzt bei vollständig mit Admins
belegter Tabelle einen problematischen Fallback auf den letzten Platz. Die
Kapazitätsbehandlung und alle Aufrufer benötigen einen eigenen Folgeschritt.
Bestehende passwortbedingt erworbene ACL-Adminrechte werden weiterhin nicht
pauschal gelöscht. Portal-Budgets und reale Paket-/Neustarttests bleiben offen.
