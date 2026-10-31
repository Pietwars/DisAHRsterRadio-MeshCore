# Release 0.9.10 · Geräteübersicht

Im Web-Reiter **Gerät** stehen jetzt der aktuelle Gerätename, der vollständige
WLAN-Name, der Bluetooth-Verbindungszustand und das Funkprofil gemeinsam oben.
Die Übersicht zeigt auch Frequenz, Bandbreite, SF, Coding Rate und konfigurierte
Sendeleistung. Eigene Einstellungen werden als Eigenes Profil gekennzeichnet.
Weitere Diagnosewerte sind unter Weitere Gerätedaten einklappbar.

Die Anzeige übernimmt die aktuellen Einstellungen aus MeshCore. Änderungen
über die gekoppelte App erscheinen nach der nächsten Statusabfrage. Auf dem
T3-S3-OLED steht Bluetooth bereit, verbunden oder aus; die vollständige
WLAN-Kennung bleibt sichtbar. Die PIN-Anzeige beim Koppeln hat weiterhin Vorrang.

## Hardwarestatus

Der Betreiber bestätigt am 31.10.2026 die Tests der **Version 0.9.9**:
zwei Boards, getrennte WLAN-Namen, Web und MeshCore-App parallel, Kanal- und
Privatnachrichten sowie Neustarts mit erhaltenen Gerätedaten; keine beobachteten
Abbrüche. Eine genaue Laufzeit wurde nicht protokolliert.

Diese Rückmeldung ist in [RELEASE-0.9.9.md](RELEASE-0.9.9.md) festgehalten.
Die neu gebauten Dateien 0.9.10 benötigen noch einen eigenen Gerätetest,
insbesondere Namensanzeige, Wechsel des Funkprofils in der App sowie Bluetooth
verbinden und trennen bei gleichzeitig geöffnetem Webportal.
