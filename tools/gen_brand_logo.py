#!/usr/bin/env python3
"""Erzeugt Logo-Bilder aus dem Titelbild docs/flyer/disahrsterradio-header.jpg (Pillow nötig):

- logo/disahrsterradio-display.png  Display-Bild: Ausschnitt mit dem Ahrtal (Tal, Berg, Lichter, Funknetz); der Schriftzug
                                    „DisAHRsterRadio“ wird aus dem Himmel des Originals herausgelöst und oben rechts
                                    wieder eingesetzt, damit Schrift UND Tal sichtbar sind.
- docs/flyer/disahrsterradio-hero.jpg  breiter Streifen (Tal ohne Schriftzug) als Kopfbild des Flyers.
- examples/companion_radio/ui-brand/BrandLogo.h  RGB565: 320x180 (T-Deck Plus), 240x140 (T-Watch S3, BRAND_SMALL).
Aufruf: python3 tools/gen_brand_logo.py
"""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFilter
ROOT = Path(__file__).resolve().parents[1]
HEADER = ROOT / 'docs/flyer/disahrsterradio-header.jpg'
LOGO = ROOT / 'logo/disahrsterradio-display.png'
HERO = ROOT / 'docs/flyer/disahrsterradio-hero.jpg'
OUT = ROOT / 'examples/companion_radio/ui-brand/BrandLogo.h'
TXT = (400, 85, 1230, 295)        # Schriftzug + Unterzeilen im Original (1280x851)
SIZES = {'': (320, 180), 'BRAND_SMALL': (240, 140)}

def clean(src):
    """Schriftzug aus dem Himmel entfernen: Textpixel durch den Zeilen-Median des Himmels ersetzen, weich verblenden."""
    img = src.copy(); px = img.load(); g = src.convert('L').load(); W = src.width
    x0, y0, x1, y1 = TXT
    m = Image.new('L', src.size, 0); mp = m.load()
    for y in range(y0, y1):
        for x in range(x0, x1):
            if g[x, y] > 55: mp[x, y] = 255
    m = m.filter(ImageFilter.MaxFilter(9)); mp = m.load()
    for y in range(y0 - 6, y1 + 6):
        bg = [px[x, y] for x in range(x0 - 40, min(W, x1 + 40)) if mp[x, y] == 0]
        if not bg: continue
        c = tuple(sorted(v[i] for v in bg)[len(bg) // 2] for i in range(3))
        for x in range(x0 - 6, min(W, x1 + 6)):
            if mp[x, y]: px[x, y] = c
    return Image.composite(img.filter(ImageFilter.GaussianBlur(8)), img, m.filter(ImageFilter.GaussianBlur(6)))

def display_image(src, cl):
    """Ausschnitt unten rechts (1100 x 619, 16:9) aus dem bereinigten Bild + Schriftzug oben rechts."""
    W, H = src.size; bw, bh = 1100, 619
    box = (W - bw, H - bh, W, H)
    img = cl.crop(box)
    t = src.crop(TXT)
    m = t.convert('L').point(lambda v: 0 if v < 70 else min(255, (v - 70) * 3)).filter(ImageFilter.GaussianBlur(1.2))
    x, y = bw - t.width - 30, 8
    sh = Image.new('L', img.size, 0); ImageDraw.Draw(sh).rectangle([x - 10, y - 6, x + t.width + 10, y + t.height + 6], fill=110)
    img = Image.composite(Image.new('RGB', img.size, (5, 10, 14)), img, sh.filter(ImageFilter.GaussianBlur(18)))
    img.paste(t, (x, y), m)
    return img

def fit(img, w, h):
    """auf Seitenverhältnis w:h zuschneiden (links bzw. oben weg, Schriftzug rechts oben bleibt), dann skalieren"""
    r = w / h
    if img.width / img.height > r:
        cw = round(img.height * r); img = img.crop((img.width - cw, 0, img.width, img.height))
    else:
        ch = round(img.width / r); img = img.crop((0, 0, img.width, ch))
    return img.resize((w, h), Image.LANCZOS)

def arr(img, w, h):
    im = fit(img, w, h).convert('RGB'); b = im.tobytes()
    vals = ['0x%04X' % (((b[i] >> 3) << 11) | ((b[i + 1] >> 2) << 5) | (b[i + 2] >> 3)) for i in range(0, len(b), 3)]
    lines = [', '.join(vals[i:i + 16]) for i in range(0, len(vals), 16)]
    return f'#define BRAND_LOGO_W {w}\n#define BRAND_LOGO_H {h}\nstatic const uint16_t BRAND_LOGO[{w * h}] PROGMEM = {{\n' + ',\n'.join(lines) + '\n};\n'

if __name__ == '__main__':
    src = Image.open(HEADER).convert('RGB'); cl = clean(src)
    logo = display_image(src, cl); logo.save(LOGO, optimize=True); print('geschrieben:', LOGO, logo.size)
    W, H = src.size; hh = round(W / 4.9)                     # Flyer-Kopf: 4,9:1 (Kopfbereich 128 x 26 mm)
    cl.crop((0, H - hh, W, H)).save(HERO, quality=90); print('geschrieben:', HERO)
    out = ['// Erzeugt von tools/gen_brand_logo.py - nicht von Hand aendern.',
           '// Display-Bild aus dem DisAHRsterRadio-Titelbild (docs/flyer/disahrsterradio-header.jpg): Ahrtal + Schriftzug.',
           '#pragma once', '#include <Arduino.h>',
           '#if defined(BRAND_SMALL)', arr(logo, *SIZES['BRAND_SMALL']), '#else', arr(logo, *SIZES['']), '#endif', '']
    OUT.write_text('\n'.join(out), encoding='utf-8'); print('geschrieben:', OUT)
