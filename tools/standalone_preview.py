#!/usr/bin/env python3
"""Vorschaubilder der T-Deck-Plus-Standalone-Anzeige (Issue #35) ohne Hardware.

  python3 tools/standalone_preview.py <Zielordner>

Baut tools/standalone_preview.cpp (echter Code von StandaloneUI/TDeckQuickPage, Demodaten aus
tests/ui_fakes/FakeApi.h) und schreibt vorschau-<seite>.png (320 x 240, auf Wunsch --scale 2).
Braucht die Schrift der Adafruit-GFX-Bibliothek aus einem PlatformIO-Build (.pio/libdeps/*/Adafruit GFX Library).
"""
import argparse, glob, re, subprocess, sys, tempfile
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('out'); ap.add_argument('--scale', type=int, default=1)
    a = ap.parse_args()
    fonts = sorted(glob.glob(str(ROOT / '.pio/libdeps/*/Adafruit GFX Library/glcdfont.c')))
    if not fonts:
        sys.exit('glcdfont.c fehlt: zuerst pio run -e LilyGo_TDeck_standalone')
    out = Path(a.out); out.mkdir(parents=True, exist_ok=True)
    firmware = re.search(r'FIRMWARE_VERSION=\x27"([^"\x27]+)',
                         (ROOT / 'platformio.standalone.ini').read_text(encoding='utf-8')).group(1)
    with tempfile.TemporaryDirectory() as d:
        exe = Path(d) / 'preview'
        subprocess.run(['g++', '-std=c++11', '-O1', '-Wall', '-Wextra', '-Wno-unused-parameter', '-Wno-misleading-indentation',
                        '-DLILYGO_TDECK', '-DDISASTER_STANDALONE', f'-DFONT_FILE="{fonts[0]}"',
                        f'-DPREVIEW_FIRMWARE="{firmware}"',
                        '-I', str(ROOT / 'src'), '-I', str(ROOT / 'tests/ui_fakes'),
                        str(ROOT / 'tools/standalone_preview.cpp'),
                        str(ROOT / 'examples/companion_radio/ui-brand/StandaloneUI.cpp'),
                        str(ROOT / 'examples/companion_radio/ui-brand/TDeckQuickPage.cpp'), '-o', str(exe)], check=True)
        # A page may be captured twice; only its final framebuffer exists on disk.
        ppms = dict.fromkeys(subprocess.run([str(exe), str(out)], check=True, capture_output=True,
                                           text=True).stdout.splitlines())
    from PIL import Image
    for p in ppms:
        img = Image.open(p)
        if a.scale > 1:
            img = img.resize((img.width * a.scale, img.height * a.scale), Image.NEAREST)
        png = Path(p).with_suffix('.png'); img.save(png); Path(p).unlink(); print(png)

if __name__ == '__main__':
    main()
