#!/usr/bin/env python3
"""Schreibt ANLEITUNG.md fuer ein Geraete-Paket."""
import json,sys
from pathlib import Path
dev,d,ver=sys.argv[1],Path(sys.argv[2]),sys.argv[3]
e={x['device']:x for x in json.load(open(Path(__file__).parent/'devices.json'))}[dev]
off={}
if (d/'.offsets').exists():
    for l in (d/'.offsets').read_text().split():
        f=l.split('|')
        if len(f)<6: f+=['-'+f[0],'teile-'+f[0]]
        off[f[0]]=(f[1],f[2],f[3],f[4],f[5])
files=sorted(str(p.relative_to(d)) for p in d.rglob('*') if p.is_file() and not p.name.startswith('.'))
NAMES={'LilyGo_TDeck':'LILYGO T-Deck Plus','LilyGo_TWatchS3':'LILYGO T-Watch S3','LilyGo_T3S3_sx1262':'LILYGO T3-S3 v1.3 SX1262 (868 MHz)'}
hardware_status = ('Akku-Anzeige der Testversion 0.9.4-test vom Nutzer erfolgreich erprobt; eine vollständige Hardwareabnahme der finalen Release-Dateien ist nicht dokumentiert.' if ver == '0.9.4' else f'Version {ver} ist **noch nicht auf echter Hardware getestet** (getestet wurde 0.9.2).')
if dev == 'LilyGo_T3S3_sx1262':
    hardware_status = f'Der Betreiber bestätigt am 30.10.2026 die Funktion des vorherigen OLED-Testpakets. Eine vollständige Hardwareabnahme der neu gebauten Release-Dateien {ver} ist nicht dokumentiert.'
L=[f'# DisAHRsterRadio-MeshCore {ver} für {NAMES.get(dev,dev)}','','> Offenes WLAN ohne Passwort, MeshCore-Bluetooth mit PIN-Kopplung (die PIN steht während der Kopplung auf dem Display). Komplett an **0x0** flashen. Alte Kopplungen vorher am Handy entfernen.','',f'Chip: **{e["chip"]}** · Web-Oberfläche: **{"ja" if e["webui"] else "nein – ohne Web-Oberfläche"}** · https://github.com/Pietwars/DisAHRsterRadio-MeshCore','',
*(['> 🧪 **TEST-Version '+ver+' – kein Release.** Nur zum Ausprobieren; Rückmeldung an Peter.',''] if '-test' in ver else []),'> ⚠️ Kein zertifiziertes Notrufsystem. In Lebensgefahr immer **112** wählen. '+(('Version 0.9.2 wurde auf diesem Gerät auf echter Hardware getestet (18.10.2026).' if ver=='0.9.2' else hardware_status) if dev in NAMES else 'Dieses Gerät ist **nicht auf echter Hardware getestet**.'),'',
'**Antenne anschrauben, bevor das Gerät Strom bekommt.** Funkprofil: MeshCore-Standard EU/UK 869,618 MHz, 62,5 kHz, SF8, CR 4/8 – auf allen Geräten gleich einstellen.','']
if e['chip'].startswith('esp32'):
    for r,(ch,b,fs,suf,teile) in off.items():
        L+=[f'## {"WLAN-Portal mit Web-Oberfläche" if r=="wifi" else r}','']
        if dev in NAMES: L+=[f'Am einfachsten: Web-Flasher https://pietwars.github.io/DisAHRsterRadio-MeshCore/flasher/ (Chrome/Edge am Computer) – grüner Knopf „Update auf {ver} – {NAMES[dev]} (Einstellungen bleiben)“ (empfohlen, nur `firmware.bin` an 0x10000) oder orange umrandeter Knopf „Neuinstallation – {NAMES[dev]} (alles löschen)“ (`factory.bin` an 0x0). Ohne Web-Flasher:','']
        L+=[
            '1. Chrome/Edge, https://espressif.github.io/esptool-js/ öffnen, **Connect**, Port wählen ('+('Trackball gedrückt halten und einschalten' if dev=='LilyGo_TDeck' else 'USB-C neu einstecken' if dev=='LilyGo_TWatchS3' else 'ggf. BOOT gedrückt halten und RST drücken')+').',
            f'2. Flash Address **`0x0`**, Datei `{d.name}{suf}-factory.bin`, **Program**, danach neu starten (löscht Einstellungen, Identität, Kontakte, Verlauf).',
            f'3. Update ohne Datenverlust: nur `{d.name}{suf}-firmware.bin` an **`0x10000`**.','',
            f'esptool: `python -m esptool --chip {ch} write_flash 0x0 {d.name}{suf}-factory.bin` bzw. `… write_flash 0x10000 {d.name}{suf}-firmware.bin`.','',
            f'Einzelteile in `{teile}/`: bootloader.bin {b}, partitions.bin 0x8000, boot_app0.bin 0xe000'+(f', spiffs.bin {fs}' if fs!='-' else '')+'; Firmware 0x10000.','']
        if r=='wifi': L+=['Danach mit dem offenen WLAN `DisAHRsterRadio-XXXXXX` verbinden (nichts eingeben). Im Browser http://192.168.4.1 öffnen.','','Parallel kann sich die offizielle MeshCore-App per **Bluetooth** verbinden (falls das Gerät BLE hat) – Kopplung per PIN (steht während der Kopplung auf dem Display, in der App änderbar). MeshCore-App über PIN-gekoppeltes Bluetooth: **alle** Einstellungen und Funktionen wie bei Standard-MeshCore (Name, Standort, Kanäle, Funkparameter, Kontakte, GPS, Neustart, Werksreset, CLI), auch **privaten Schlüssel exportieren/importieren** und eine **eigene 6-stellige PIN** setzen (wird gespeichert, gilt nach dem Neustart; danach die alte Kopplung am Handy entfernen und mit der neuen PIN neu koppeln). Weboberfläche (offenes WLAN, **ohne Anmeldung**): Knotenname, Funkprofil (nur feste Deutschland-Profile), Kanäle anlegen/ändern und Nachrichtenverlauf löschen im Reiter „Gerät“. **Achtung: Jede Person im offenen WLAN kann Kanäle, Knotenname und Funkprofil ändern und den Verlauf löschen.** Einen Web-Admin-Bereich (Admin-Passwort) gibt es nicht mehr.','','**Weboberfläche und MeshCore-App teilen die Nachrichten:** Empfangene Nachrichten bekommen beide, auch wenn die App erst später verbunden wird. In der App gesendete Nachrichten stehen auch im Web-Verlauf. Im Web gesendete Nachrichten erscheinen in der App als empfangene Nachricht mit Markierung (Kanal: Absender „<Knotenname> (Web)“, Direkt: Text beginnt mit „[von mir über Web]“) – das MeshCore-Protokoll kennt keine „von einem anderen Gerät gesendet“-Meldung.','','**Kontakte automatisch:** nur **Repeater und Room-Server** werden automatisch hinzugefügt. Chat-Kontakte (andere Handys/Companions), Sensoren und sonstige Typen werden **nicht** automatisch übernommen (manuell). Das ist die Werkseinstellung bei leerem Speicher (z. B. nach dem Flashen der factory-Datei); in der MeshCore-App über PIN-gekoppeltes Bluetooth änderbar. Beim reinen Firmware-Update (`firmware.bin` an 0x10000) bleibt eine bereits gespeicherte Einstellung erhalten. Chat-Kontakte manuell hinzufügen, bearbeiten oder löschen geht in der MeshCore-App über PIN-gekoppeltes Bluetooth (Direktnachrichten gehen nur mit Kontakten in der Kontaktliste).','']
elif e['chip'] in('nrf52','rp2040'):
    L+=['## Flashen per UF2','','1. Gerät per USB anschließen und **zweimal schnell Reset** drücken (RP2040: BOOTSEL gedrückt halten beim Einstecken). Es erscheint ein USB-Laufwerk.',
        '2. Die passende `.uf2`-Datei auf das Laufwerk kopieren. Das Gerät startet neu.','',
        'nRF52: Die `.zip` ist ein DFU-Paket für `adafruit-nrfutil` über USB (Bluetooth-DFU ist in dieser Firmware abgeschaltet).','',
        'Rollen: `companion` = MeshCore-Companion für die MeshCore-App (Bluetooth mit PIN bzw. USB), `repeater` = Repeater, `room_server` = Room-Server (Schwarzes Brett).','',
        'Konfiguration über die offizielle MeshCore-App bzw. https://config.meshcore.dev / https://meshcore.co.uk/configurator/ – **keine** DisAHRsterRadio-Web-Oberfläche (kein WLAN auf diesem Chip).','']
else:
    L+=['## STM32','','`.bin`/`.hex` mit STM32CubeProgrammer oder einem SWD-Programmer flashen. Ohne Web-Oberfläche.','']
BRAND={
 'LilyGo_TDeck':['## Anzeige und Tasten (LILYGO T-Deck Plus)','',
  'Das Display zeigt nur ein Bild des Ahrtals bei Nacht mit dem Schriftzug DisAHRsterRadio, darunter eine Akku-Zeile und den Knopf **„Advert senden“**. Keine weiteren Seiten, keine Nachrichten am Gerät (Nachrichten und Einstellungen über MeshCore-App und Weboberfläche).','',
  '| Bedienung | Wirkung |','|---|---|','| Trackball kurz klicken | Display an/aus |','| Trackball **2,5 s** halten | Flood-Advert senden |','| Tastatur (auch Enter) | keine Funktion |','| Ein/Aus | Schiebeschalter (kein Abschalten per Trackball) |',''],
 'LilyGo_TWatchS3':['## Anzeige und Tasten (LILYGO T-Watch S3)','',
  'Das Display zeigt nur ein Bild des Ahrtals bei Nacht mit dem Schriftzug DisAHRsterRadio, darunter eine Akku-Zeile und den Knopf **„Advert senden“**. Keine weiteren Seiten, keine Nachrichten am Gerät (Nachrichten und Einstellungen über MeshCore-App und Weboberfläche).','',
  '| Bedienung | Wirkung |','|---|---|','| Seitentaste kurz | Display an/aus (bei ausgeschalteter Uhr: einschalten) |','| Seitentaste ca. **2 s** halten | Flood-Advert senden |','| Seitentaste ca. **6 s** halten | Uhr ausschalten (Power-Chip) |','| Knopf „Advert senden“ antippen | Flood-Advert senden |','| Antippen bei dunklem Display | nur aufwecken |',''],
}
if dev == 'LilyGo_T3S3_sx1262':
    L += ['## T3-S3 v1.3: Web und Bluetooth', '',
          'Das OLED zeigt WLAN-Name und Browseradresse. Beim Koppeln in der MeshCore-App wacht es automatisch auf und zeigt die sechsstellige Geräte-PIN groß an. Nach Erfolg, Abbruch oder spätestens 60 Sekunden verschwindet sie. Meldungen und Seitenwechsel verdecken die PIN nicht.', '',
          'Weboberfläche und App funktionieren gleichzeitig. Unter Gerät stehen Bluetooth-Status und Kopplungshilfe. Die PIN selbst wird nicht im WLAN angezeigt. Nur für SX1262, nicht für SX1276 oder SX1280.', '',
          'Der Betreiber hat die Funktion des zuvor bereitgestellten OLED-Testpakets am 30.10.2026 bestätigt. Die neu gebauten Release-Dateien benötigen weiterhin eigene Gerätetests; eine vollständige Hardwareabnahme wird nicht behauptet.', '']
if dev in BRAND:
    AKKU={'LilyGo_TDeck':'**Akku-Zeile (T-Deck Plus):** Symbol, Ladestand „ca. NN %“ (aus der Spannung geschätzt) und Spannung. Das T-Deck misst nur die Akkuspannung – keinen Strom und nicht, ob es lädt. Rechts der **geschätzte Verbrauch**: Abfall des Ladestands in %/h und Restlaufzeit, berechnet aus dem Spannungsverlauf der letzten bis zu 60 min (erst nach 20 min, vorher „Verbrauch: Messung, noch N min“). Steigt die Spannung (Ladegerät), steht dort „Spannung steigt (Laden?)“.',
          'LilyGo_TWatchS3':'**Akku-Zeile (T-Watch S3):** Symbol, Ladestand in % (Ladestandsanzeige des Power-Chips AXP2101) und Spannung; darunter „USB: Laden“ bzw. „USB: kein Laden“ oder der **geschätzte Verbrauch**: Abfall des Ladestands in %/h und Restlaufzeit aus dem Spannungsverlauf der letzten bis zu 60 min (erst nach 20 min ohne USB). Der AXP2101 misst keinen Strom; ein gemessener Verbrauch in mA ist daher nicht verfügbar.'}
    L+=BRAND[dev]+[AKKU[dev],'','Jeder Advert ist ein echter **Flood-Advert** (wie „Advert senden → Flood“ in der App). Danach 2 s **„Advert gesendet“**; höchstens ein Advert je 10 s (sonst „Bitte N s warten“). Display geht nach 30 s ohne Bedienung aus; neue Nachrichten wecken es nicht.','',
      '**Bluetooth-PIN:** erscheint nur während einer laufenden Kopplung auf dem Display (ab der PIN-Abfrage des Handys bis die Kopplung fertig oder abgebrochen ist, höchstens 60 s; verbindet sich ein Handy ohne Kopplung, nach 2,5 s). Sie steht nicht in dieser Anleitung.','']
    if any(f.startswith('vorschau/') for f in files): L+=['`vorschau/` zeigt am PC nachgestellte Bildschirme (Simulation, PIN durch XXXXXX ersetzt).','']
L+=['## Dateien','']+[f'- `{f}`' for f in files]+['','Prüfsummen: `SHA256SUMS`.']
(d/'ANLEITUNG.md').write_text('\n'.join(L)+'\n',encoding='utf-8')
