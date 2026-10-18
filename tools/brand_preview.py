#!/usr/bin/env python3
"""Vorschau-PNGs der Marken-Anzeige (examples/companion_radio/ui-brand), am PC nachgestellt.

Bildet UITask::render() mit denselben Maßen nach (glcd-5x7-Schrift aus Adafruit GFX, Logo wie
tools/gen_brand_logo.py). Die PIN wird nie gezeigt (XXXXXX).
Aufruf:  brand_preview.py <Gerät> <Ausgabeordner> [<PlatformIO-Env>]   (braucht Pillow)
         brand_preview.py --has <Gerät>   -> Exit 0, wenn das Gerät die Marken-Anzeige hat
"""
import glob, re, sys
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
DEVICES = {'LilyGo_TDeck': (320, 240, 320, 150, False), 'LilyGo_TWatchS3': (240, 240, 240, 112, True)}
if sys.argv[1] == '--has':
    sys.exit(0 if sys.argv[2] in DEVICES else 1)
from PIL import Image, ImageDraw
dev, out = sys.argv[1], Path(sys.argv[2])
env = sys.argv[3] if len(sys.argv) > 3 else '*'
fonts = sorted(glob.glob(str(ROOT / '.pio/libdeps' / env / 'Adafruit GFX Library/glcdfont.c'))) or \
        sorted(glob.glob(str(ROOT / '.pio/libdeps/*/Adafruit GFX Library/glcdfont.c')))
if not fonts: sys.exit('glcdfont.c nicht gefunden (erst bauen)')
src = open(fonts[0]).read()
F = [int(x, 16) for x in re.findall(r'0x[0-9A-Fa-f]{2}', src[src.index('{'):src.index('};')])]
LOGO = ROOT / 'logo/disahrsterradio-display.png'
def c565(v): r, g, b = (v >> 11) & 31, (v >> 5) & 63, v & 31; return (r * 255 // 31, g * 255 // 63, b * 255 // 31)
def q(p): r, g, b = p; return c565(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3))
BG, GREEN, WHITE, GREY, ORANGE, RED = [c565(x) for x in (0x08A3, 0x4EF0, 0xFFFF, 0x8410, 0xFD20, 0xF800)]

class D:
    def __init__(s, W, H, lw, lh, touch):
        s.W, s.H, s.lw, s.lh, s.touch = W, H, lw, lh, touch
        s.im = Image.new('RGB', (W, H), BG); s.px = s.im.load(); s.dr = ImageDraw.Draw(s.im)
    def fill(s, x, y, w, h, c): s.dr.rectangle([x, y, x + w - 1, y + h - 1], fill=c)
    def rect(s, x, y, w, h, c): s.dr.rectangle([x, y, x + w - 1, y + h - 1], outline=c)
    def tw(s, t, sz): return 6 * sz * len(t)
    def text(s, x, y, sz, c, t):
        for ch in t:
            for i in range(5):
                l = F[ord(ch) * 5 + i]
                for j in range(8):
                    if l & (1 << j):
                        s.dr.rectangle([x + i * sz, y + j * sz, x + i * sz + sz - 1, y + j * sz + sz - 1], fill=c)
            x += 6 * sz
    def ctext(s, cx, y, sz, c, t): s.text(cx - s.tw(t, sz) // 2, y, sz, c, t)
    def logo(s):
        im = Image.open(LOGO).convert('RGB').resize((s.lw, s.lh), Image.LANCZOS)
        b = im.tobytes(); px = bytes(v for i in range(0, len(b), 3) for v in q((b[i], b[i + 1], b[i + 2])))
        im = Image.frombytes('RGB', im.size, px)
        s.im.paste(im, ((s.W - s.lw) // 2, 6))

def render(d, mode):
    W, H = d.W, d.H; small = W <= 240
    d.logo()
    top = 8 + d.lh + (14 if (H >= 240 and W <= 240) else 12)
    bx = W // 12; bw = W - 2 * bx; by = top; bh = H - top - (18 if W <= 240 else 22)
    big = 2 if small else 3
    if mode == 'kopplung':
        d.fill(bx, by, bw, bh, WHITE)
        d.ctext(W // 2, by + 6, 1 + (0 if small else 1), BG, "Bluetooth-Kopplung - PIN:")
        d.ctext(W // 2, by + bh // 2 - 4 * (3 if small else 4) + 4, 3 if small else 4, BG, "XXXXXX")
        d.ctext(W // 2, by + bh - 12, 1, GREY, "am Handy eingeben")
    elif mode in ('gesendet', 'warten'):
        c, t = (GREEN, "Advert gesendet") if mode == 'gesendet' else (ORANGE, "Bitte 7 s warten")
        d.fill(bx, by, bw, bh, c); d.ctext(W // 2, by + bh // 2 - 4 * big, big, BG, t)
    else:
        d.fill(bx, by, bw, bh, GREEN); d.rect(bx, by, bw, bh, WHITE)
        d.ctext(W // 2, by + bh // 2 - 4 * big - 4, big, BG, "Advert senden")
        d.ctext(W // 2, by + bh - 14, 1, BG, "antippen" if d.touch else "Trackball 2,5 s halten")

out.mkdir(parents=True, exist_ok=True)
for i, m in enumerate(['start', 'gesendet', 'warten', 'kopplung'], 1):
    d = D(*DEVICES[dev]); render(d, m)
    d.im.resize((d.W * 2, d.H * 2), Image.NEAREST).save(out / f'{i}-{m}.png')
print('Vorschau:', out)
