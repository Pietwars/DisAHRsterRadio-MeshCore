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
