#!/usr/bin/env python3
"""Release-Text aus CHANGELOG.md (Abschnitt der Version) + feste Hinweise. Aufruf: release_notes.py <Version> <Repo> <Präfix> <Board=Name> …"""
import re, sys
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
version, repo, prefix, *boards = sys.argv[1:]
log = (ROOT / 'CHANGELOG.md').read_text(encoding='utf-8')
m = re.search(r'^## [^\n]*?v?' + re.escape(version) + r'\b[^\n]*\n(.*?)(?=^## |\Z)', log, re.S | re.M)
body = m.group(1).strip() if m else '(Kein eigener CHANGELOG-Abschnitt für diese Version gefunden.)'
rows = '\n'.join(f'| {n} | `{prefix}-{version}-{b}.zip` |' for b, n in (x.split('=', 1) for x in boards))
print(f"""> ⚠️ **Kein zertifiziertes Notrufsystem.** In Lebensgefahr immer **112** wählen. Hardwarestatus und offene Prüfungen stehen in den Änderungen und in `docs/SPEICHER.md`.

## Änderungen
{body}

## Standalone für T-Deck Plus
Die zusätzliche Variante `DisAHRsterRadio-MeshCore-{version}-LilyGo_TDeck_standalone-test6.zip` bietet Chat, Privat, Meldung, Brett, Karten, Standort und Geräteaktionen direkt am Display, ohne WLAN/Webportal, mit MeshCore-Bluetooth. Touch, Tastatur und Trackball bleiben nutzbar. Die Kennung bleibt `standalone-test6`; Privatversand sowie SD-Erkennung und Laden sind vom Betreiber am T-Deck Plus bestätigt; weitere Prüfungen sind in den Änderungen benannt. Veröffentlichung vom Projektbetreiber beauftragt. Anleitung, SHA256SUMS, Commitmanifest und Vorschauen liegen im ZIP.

Standalone: `firmware.bin` als Update an **0x10000 ohne Löschen**; `factory.bin` an **0x0 nach vollständigem Löschen** (Datenverlust). GPS wird nur bei bewusster Freigabe und frischem Fix geteilt. [Bedienung](https://github.com/{repo}/blob/v{version}/docs/STANDALONE_TOUCH.md).

## WLAN/BLE-Pakete
| Board | Datei |
|---|---|
{rows}

Jedes Paket enthält `…-factory.bin` (komplett an **0x0** flashen, z. B. mit https://espressif.github.io/esptool-js/), die Einzelteile (`firmware.bin` für das Update an **0x10000**), `SHA256SUMS` und eine deutsche `ANLEITUNG.md`. **Antenne zuerst anschrauben!**

T-Deck Plus und T-Watch S3 zeigen das DisAHRsterRadio-Logo, die Akku-Zeile und den Knopf „Advert senden“ (Flood-Advert). Das T3-S3-OLED zeigt WLAN-Name, Browseradresse und Nachrichtenanzahl; bei einer Bluetooth-Kopplung wacht es automatisch auf und zeigt die Geräte-PIN. Nachrichten über die MeshCore-App und die Weboberfläche im offenen WLAN; Bluetooth-Status und Kopplungshilfe stehen im Reiter Gerät. Verbrauch und Restlaufzeit sind Schätzungen.

Automatisch gebaut vom Workflow „Release“ aus dem Tag `v{version}` ({repo}); vor der Veröffentlichung auf Vollständigkeit, Prüfsummen, Flash-Header, Partitionen und Speichergrenzen geprüft. Markdown-/JSON-Speicherberichte liegen in jedem ZIP.""")
