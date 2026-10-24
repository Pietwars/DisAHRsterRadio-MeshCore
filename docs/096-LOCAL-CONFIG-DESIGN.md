# 0.9.6 – sicherer lokaler Konfigurationsweg (Entwurf)

Status: **Lokale Teilimplementierung; USB-/Gesamtarchitektur und Hardwarefreigabe weiterhin offen.** Bezug: #47 und docs/096-SECURITY-RADIO-ACCEPTANCE.md.

## Ausgangslage (Codeprüfung 24.10.2026)
- LocalCompanion.h besitzt exklusive Kontrolle über die Companion-Schnittstelle; ENABLE_USB_INTERFACE parallel zu LocalCompanion löst absichtlich einen Compile-Fehler aus. Diesen Schutz nicht entfernen.
- Offenes HTTP: privilegierte Routen 403, Admin-Login 503; keine PIN-/Token-Übertragung im Gast-WLAN einführen.
- BLE: öffentliche Positivliste; eine gepaarte Verbindung ist ohne verifizierten Authentifizierungsstatus kein vertrauenswürdiger Administrationskanal.
- Standalone: lokaler PIN-Editor vorhanden. Touch/Navigation/Display-Wakeup wurden vom Betreiber als funktionierend gemeldet; exakte Firmware-SHA noch offen.

## Vorgeschlagener Weg
1. **Zunächst physische lokale UI nutzen:** bestehende PIN-Änderung beibehalten. Weitere Konfigurationsoptionen nur nach Gerätebestätigung, mit klarer Rückmeldung und sicherer Speicherung; keine versteckten privilegierten BLE-/HTTP-Befehle.
2. **USB getrennt entwerfen:** keinen zweiten gleichzeitigen Companion-Eigentümer registrieren. Optionen: autorisierte Konfigurationsnachrichten über bestehende LocalCompanion-Queue oder expliziter Wartungsmodus, in dem LocalCompanion pausiert und der USB-Transport alleiniger Besitzer ist. Vor Implementierung Lifecycle, Eingabeprüfung und Rechte je Kommando dokumentieren.
3. **Sicherheitsgrenze:** physische Bestätigung auf dem Gerät für sensible Änderungen, kein automatisches Vertrauen durch USB-Anschluss allein; keine Klartext-Schlüssel oder PINs in Logs. Recovery mit Backup und bewusstem Resetweg.
4. **Funkprofil:** Region/Subband-Limits validieren, vor Änderung Warnung und Bestätigung; ungültige Profile zurückweisen.

## Akzeptanztests (noch offen)
- [ ] Build WLAN und Standalone ohne Verletzung der LocalCompanion-Exklusivität
- [ ] Öffentliche WLAN-Chat-/Statusfunktionen bleiben nutzbar, alle Admin-Routen 403 und Login 503
- [ ] BLE darf mit/ohne Pairing keine privaten Schlüssel exportieren und keine geschützten Einstellungen ändern
- [ ] PIN-Änderung lokal funktioniert und bleibt nach Neustart erhalten; keine bekannte Standard-PIN
- [ ] Unberechtigte/ungültige Eingaben, Abbruch, Stromverlust und parallele Zugriffe werden sicher behandelt
- [ ] Geräteprofil, Konfigurationspersistenz, Wiederherstellung und Firmware-SHA auf echter Hardware dokumentiert

**Nicht als erledigt markieren:** USB-Konfiguration, alle Admin-Einstellungen, Funkregulierung, Update-Signaturen und vollständige Hardwareabnahme.

## Lokale Bedienung im 0.9.6-Entwicklungsbranch
- Unter Gerät: **Gerätenamen ändern**. Maximal 24 Bytes, keine Steuerzeichen oder reinen Leerzeichen. Enter öffnet die Bestätigung; ein weiteres Enter/Trackball bestätigt. Bearbeiten setzt die Bestätigung zurück. Abbruch oder Tabwechsel verwirft den Entwurf.
- Der Name wird an den vorhandenen lokalen Companion übergeben, nicht direkt über Funk gesendet. Die Rückmeldung bedeutet Annahme in die lokale Queue, keinen Nachweis eines erfolgreichen Flash-Schreibens. Eine SD-Kopie bei Bedarf separat speichern.
- `/disaster/config.txt` wird beim Start nur als Snapshot vorgemerkt. Unter **SD-Konfiguration anwenden** werden Name, Karte, Standortfreigabe und Log angezeigt; erst Enter/Trackball übernimmt. Die Karte wird nach der Bestätigung nicht erneut eingelesen. Neue SD-Dateien werden beim nächsten Neustart vorgemerkt.
- Abbruch verändert keine Einstellungen. Eine erfolgreich gespeicherte neue SD-Konfiguration verwirft den alten vorgemerkten Import. Unbekannte Karten brechen den Import ab.
- **Verhaltensänderung:** Eine SD-Datei allein aktiviert keine Standortfreigabe und überschreibt keinen Namen mehr. Bereits lokal gespeicherte Freigabe/Karte bleiben beim Start erhalten. Ereignislogging ist ohne zuvor bestätigten Import ausgeschaltet; Nachrichtenarchiv und Ereignislog sind getrennt.
- Namensänderung/SD-Import werden bei bestehender BLE-Verbindung verweigert. Keine Freigabe über Gast-WLAN, kein Admin-Button und kein zweiter USB-Companion.
- Mehrere NVS-Einträge bilden keine atomare Transaktion: bei Speicherfehler meldet der Import ausdrücklich „teilweise angewendet“ und bleibt wiederholbar. Stromausfall-/Persistenztests bleiben offen.
- Drei lokal bestätigte Funkprofile sind in [096-RADIO-PROFILES.md](096-RADIO-PROFILES.md) beschrieben; Frequenz-/ERP-Messung und weitere Länder bleiben offen.
- Noch offen: Kanal-/Schlüsselverwaltung, gesicherter Recovery-/Wartungsmodus und entsprechender Weg für das WLAN-Target. Dies ist **keine vollständige Konfigurationsfreigabe**.
