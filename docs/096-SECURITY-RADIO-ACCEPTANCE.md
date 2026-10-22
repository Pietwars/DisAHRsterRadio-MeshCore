# 0.9.6 Sicherheits- und Funk-Abnahme (Entwicklungsstand)

Status: **NICHT FREIGEGEBEN**. Dieser Plan ist kein Nachweis bestandener Tests.

## Web-Sicherheit
- [ ] Einrichtungs-PIN nur über vertrauenswürdigen lokalen Kanal ausgeben, sicheren Wiederherstellungs-/Reset-Prozess festlegen.
- [ ] **Offenes WLAN:** Keine PIN oder Sitzungstoken als Klartext über HTTP senden. Vor produktiver Verwendung ein geeignetes Authentifizierungs-/Transportkonzept implementieren. Der aktuelle `/api/admin-login`-Prototyp erfüllt dies **nicht**.
- [ ] Eingebettete Weboberfläche (`web/index.html`, generiert durch `tools/embed_web.py`) an die Authentifizierung anbinden, **ohne Admin-Button**. Bis dahin sind geschützte Einstellungen aus der Oberfläche nicht nutzbar.
- [ ] `/api/name`, `/api/channel`, `/api/radio`, `/api/history-wipe` ohne Berechtigung verweigern; Chat und Meldungen weiterhin erlauben.
- [ ] Login-Fehlversuche, Sessionablauf, Wiederanlauf, Speicherfehler, Token-Rotation und Mehrbenutzerverhalten testen.
- [ ] PIN-Hashverfahren und Entropie prüfen: derzeit nur einzelner SHA-256-Durchlauf; für PIN-Schutz gegen Offline-Angriffe nicht ausreichend gehärtet.

## Airtime / Funk
- [ ] `DAR_MESH_POLICY` in jedem Ziel-Build nachweisen (10%-Budget nur dort); regionale Subband-Vorgaben separat validieren.
- [ ] Prüfen, ob 150%-Reserve im `Dispatcher::checkSend` bei jedem Profil erreichbar ist; aktueller Branch begrenzt die Reserve auf das maximale Budget, was keine strikte Pro-Paket-Begrenzung beweist.
- [ ] Packet-Airtime pro tatsächlicher Paketlänge vor TX gegen Budget prüfen, statt nur gegen geschätztes Maximum; reale TX-Dauer und Timeout berücksichtigen.
- [ ] Hosttests: Budget leer, gerade ausreichend, knapp darunter, große Pakete, Profilwechsel, `millis()`-Überlauf, TX-Timeout, Queue-Stau, CAD-Busy.
- [ ] T-Deck Plus: reale Funkmessung/Antennen-/Leistungs-/Airtimeprüfung.

## Builds und Freigabe
- [ ] Web-Assets neu generieren: `python3 tools/embed_web.py`.
- [ ] PlatformIO-Targets für WLAN/BLE und Standalone bauen, vorhandene Hosttests ausführen.
- [ ] Hardware-Abnahme mit SHA/Version/Board protokollieren.
- [ ] Kein Merge, Tag oder Release ohne ausdrückliche Freigabe.

Quellcode: `examples/companion_radio/DisasterPortal.h`, `web/index.html`, `tools/embed_web.py`, `src/Dispatcher.cpp`, `examples/companion_radio/MyMesh.cpp`.
