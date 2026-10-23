# 0.9.6 – sicherer lokaler Konfigurationsweg (Entwurf)

Status: **Architekturentscheidung offen; noch keine Implementierung oder Hardwarefreigabe.** Bezug: #47 und docs/096-SECURITY-RADIO-ACCEPTANCE.md.

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
