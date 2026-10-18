#!/usr/bin/env python3
"""Erzeugt examples/companion_radio/ui-brand/BrandLogo.h aus logo/disahrsterradio-display.png.

Quelle: Ausschnitt (Schriftzug + Funkwellen) aus dem Flyer-/Video-Titelbild docs/flyer/disahrsterradio-header.jpg.
Zwei Groessen als RGB565: 320x150 (T-Deck Plus) und 240x112 (T-Watch S3).
Aufruf: python3 tools/gen_brand_logo.py   (braucht Pillow)
"""
from pathlib import Path
from PIL import Image
ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / 'logo/disahrsterradio-display.png'
OUT = ROOT / 'examples/companion_radio/ui-brand/BrandLogo.h'

def arr(w, h):
    im = Image.open(SRC).convert('RGB').resize((w, h), Image.LANCZOS)
    vals = []
    for r, g, b in im.getdata():
        vals.append('0x%04X' % (((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)))
    lines = [', '.join(vals[i:i + 16]) for i in range(0, len(vals), 16)]
    return f'#define BRAND_LOGO_W {w}\n#define BRAND_LOGO_H {h}\nstatic const uint16_t BRAND_LOGO[{w * h}] PROGMEM = {{\n' + ',\n'.join(lines) + '\n};\n'

out = ['// Erzeugt von tools/gen_brand_logo.py - nicht von Hand aendern.',
       '// Schriftzug und Logo aus dem DisAHRsterRadio-Flyer-/Video-Titelbild (docs/flyer/disahrsterradio-header.jpg).',
       '#pragma once', '#include <Arduino.h>',
       '#if defined(BRAND_SMALL)', arr(240, 112), '#else', arr(320, 150), '#endif', '']
OUT.write_text('\n'.join(out), encoding='utf-8')
print('geschrieben:', OUT)
