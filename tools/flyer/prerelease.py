#!/usr/bin/env python3
"""Compact A5 prerelease flyer using the current firmware layout and release URL."""
from pathlib import Path
from io import BytesIO
import qrcode
from reportlab.pdfgen.canvas import Canvas
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.lib.colors import HexColor
from reportlab.lib.units import mm
from reportlab.lib.utils import ImageReader
from reportlab.platypus import Paragraph
from reportlab.lib.styles import ParagraphStyle

pdfmetrics.registerFont(TTFont('DarSans', '/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf'))
pdfmetrics.registerFont(TTFont('DarSans-Bold', '/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf'))
pdfmetrics.registerFontFamily('DarSans', normal='DarSans', bold='DarSans-Bold')

ROOT = Path(__file__).resolve().parents[2]
vals = dict(line.split('=', 1) for line in (ROOT / 'VERSION').read_text().splitlines() if '=' in line)
version = vals['VERSION']
release = f'https://github.com/Pietwars/DisAHRsterRadio-MeshCore/releases/tag/v{version}'
out = ROOT / 'docs/flyer'
style = ParagraphStyle('body', fontName='DarSans', fontSize=9, leading=12, textColor=HexColor('#e8f0e4'))
small = ParagraphStyle('small', parent=style, fontSize=8, leading=11)

def paragraph(c, text, x, top, width, sty=style):
    p = Paragraph(text, sty); w, h = p.wrap(width * mm, 100 * mm)
    p.drawOn(c, x * mm, top * mm - h)
    return top - h / mm

for digital in (False, True):
    dest = out / ('DisAHRsterRadio-MeshCore-Flyer' + ('-digital' if digital else '') + '.pdf')
    c = Canvas(str(dest), pagesize=(148 * mm, 210 * mm))
    c.setTitle(f'DisAHRsterRadio-MeshCore {version} - Pre-Release')
    c.setFillColor(HexColor('#07130f')); c.rect(0, 0, 148 * mm, 210 * mm, fill=1, stroke=0)
    c.setFillColor(HexColor('#b9f48d')); c.setFont('DarSans-Bold', 22)
    c.drawString(9 * mm, 196 * mm, 'DisAHRsterRadio')
    c.setFillColor(HexColor('#e8f0e4')); c.setFont('DarSans-Bold', 11)
    c.drawString(9 * mm, 187 * mm, f'MeshCore | {version} | Pre-Release')
    c.drawImage(str(out / 'disahrsterradio-hero.jpg'), 9 * mm, 150 * mm, width=130 * mm, height=30 * mm, preserveAspectRatio=False)
    paragraph(c, '<b>Funknetz fürs Ahrtal.</b> T-Deck Plus direkt bedienen: Kanal-Chat und <b>Privat</b> für gespeicherte Kontakte. Touch, Tastatur und Trackball. Ohne WLAN/Webportal, mit MeshCore-Bluetooth.', 9, 144, 130)
    paragraph(c, '<b>Drei getrennte Pakete</b><br/>T-Deck Plus Standalone · T-Deck Plus WLAN/BLE · T-Watch S3 WLAN/BLE', 9, 119, 130)
    c.drawImage(str(ROOT / 'docs/images/standalone-test6/vorschau-chat-direkt.png'), 9 * mm, 58 * mm, width=65 * mm, height=48.75 * mm)
    paragraph(c, '<b>Start in drei Schritten</b><br/>1. Passendes Paket wählen, Daten sichern und SHA-256 prüfen.<br/>2. Update: Firmware an 0x10000, ohne Löschen.<br/>3. Neuinstallation: Factory an 0x0, Flash löschen. Alle Daten gehen verloren.', 80, 106, 59, small)
    paragraph(c, 'Aktuelles Firmware-Layout mit Demodaten, kein Hardwarefoto.', 9, 55, 130, small)
    paragraph(c, '<b>Hardwareabnahme offen.</b> Privatchat/BLE-Parallelbetrieb, Migration und Funkmessung sind noch zu prüfen. Keine vollständige SOS-Funktion.', 9, 44, 130, small)
    qr = qrcode.make(release); buffer = BytesIO(); qr.save(buffer, format='PNG'); buffer.seek(0)
    c.drawImage(ImageReader(buffer), 9 * mm, 9 * mm, width=23 * mm, height=23 * mm)
    paragraph(c, '<b>Downloads und Anleitung</b><br/>github.com/Pietwars/<br/>DisAHRsterRadio-MeshCore<br/>Release v' + version, 37, 30, 102, small)
    if digital:
        c.linkURL(release, (9 * mm, 9 * mm, 139 * mm, 32 * mm), relative=0)
    c.setFillColor(HexColor('#ffb38a')); c.setFont('DarSans-Bold', 7)
    c.drawString(9 * mm, 4 * mm, 'Prototyp. Kein zertifiziertes Notrufsystem. Im Notfall 112.')
    c.showPage(); c.save(); print(dest)
