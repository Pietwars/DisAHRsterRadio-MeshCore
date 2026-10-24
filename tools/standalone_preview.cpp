// Host preview of the T-Deck Plus standalone pages (Issue #35): renders the real StandaloneUI / TDeckQuickPage
// code with demo data (tests/ui_fakes/FakeApi.h) into 320 x 240 images. Built and run by
// tools/standalone_preview.py; the GFX 5x7 font comes from the Adafruit GFX library of the PlatformIO build.
#include <stdio.h>
#include <string>
#include "FakeApi.h"
#include "FakeDisplay.h"
#include "../examples/companion_radio/ui-brand/StandaloneUI.h"
#include "../examples/companion_radio/ui-brand/BrandLogo.h"
#include "../examples/companion_radio/ui-brand/BattTrend.h"
#include FONT_FILE    // Adafruit GFX glcdfont.c: static const unsigned char font[]

static FakeApi fake;
dev::Api &dev::api() { return fake; }
static std::string out;
static uint32_t ms = 1000;
static void shot(FakeDisplay &d, const char *name) {
  std::string p = out + "/vorschau-" + name + ".ppm";
  if (!d.writePpm(p.c_str())) { fprintf(stderr, "cannot write %s\n", p.c_str()); exit(1); }
  if (d.outOfBounds) { fprintf(stderr, "%s: %u draws outside the screen\n", name, d.outOfBounds); exit(1); }
  printf("%s\n", p.c_str());
}
static void page(sui::UI &u, FakeDisplay &d, const char *name) { ms += 300; u.tick(ms); u.render(&d); shot(d, name); }
static void type(sui::UI &u, const char *s) { while (*s) u.key(uint8_t(*s++)); }
// ALT+Backspace up to the main menu, trackball to entry i, click
static void entry(sui::UI &u, int i) {
  while (u.key(sui::K_BACK)) {}
  for (int k = 0; k < 6; ++k) u.move(0, -1);
  for (int k = 0; k < i; ++k) u.move(0, 1);
  u.click();
}

int main(int argc, char **argv) {
  if (argc < 2) { fprintf(stderr, "usage: preview <dir>\n"); return 2; }
  out = argv[1];
  fake.demo();
#ifdef PREVIEW_FIRMWARE
  fake.firmwareLabel=PREVIEW_FIRMWARE;
#endif
  fake.g.share = true; fake.g.fresh = true; fake.g.lat = 50.54292; fake.g.lon = 7.11463; fake.g.sats = 9;
  sui::UI u; FakeDisplay d; d.font = font;
  static uint8_t splash[320 * 213 * 2];
  FILE *asset = fopen("examples/companion_radio/ui-brand/assets/tdeck-splash.rgb565", "rb");
  if (!asset || fread(splash, 1, sizeof(splash), asset) != sizeof(splash)) return 1;
  fclose(asset); u.setSplash(splash);
  u.begin(&fake); u.tick(ms);
  // battery as UITask passes it (T-Deck: voltage only, level estimated)
  batt::Now b; b.mv = 4050; b.pct = 87; b.pct_approx = true;
  batt::Trend t; char bl[48], be[48]; batt::texts(b, t, bl, sizeof(bl), be, sizeof(be), 0);
  u.setBattery(b.pct, bl, be[0] ? be : "ca. 18 h (Sch\xc3\xa4tzung)");
  u.render(&d); shot(d, "bootlogo");      // boot logo (2.5 s or until the first input)
  // two new messages after start: unread badges
  fake.channelIn(1, "Lea: Pumpe kommt um 15 Uhr zum Marktplatz", fake.clock - 120);
  fake.roomIn("dcafe0012ab34", "11223344", "Heute 18 Uhr: Treffen am Gemeindehaus", fake.clock - 60);
  ms += 300; u.tick(ms);
  ms += sui::UI::BOOT_MS; page(u, d, "chat-liste");
  u.key(sui::K_BACK); page(u, d, "hauptmenue");   // main menu after the logo
  u.click(); page(u, d, "chat-liste");
  u.click(); type(u, "Bin in 10 Min. mit Kanistern da"); page(u, d, "chat");
  u.tap(290, 12); page(u, d, "privat-liste");
  u.click(); type(u, "Bin gleich da"); page(u, d, "chat-direkt");
  entry(u, 1); u.key('3'); type(u, "Ibuprofen"); page(u, d, "meldung");
  u.tap(30, 80);
  page(u, d, "vorlagen");
  u.gesture(100, 210, 100, 100);
  page(u, d, "vorlagen-weiter");
  u.back();
  u.tap(60, 160);
  page(u, d, "ort-auswahl");
  u.back();
  u.tap(220, 160);
  page(u, d, "ziel-auswahl");
  u.back();
  entry(u, 2); page(u, d, "brett-liste");
  u.click(); fake.rm.rights = 2; snprintf(fake.rm.id, sizeof(fake.rm.id), "dcafe0012ab34"); type(u, "Biete 2 Bautrockner");
  page(u, d, "brett");
  entry(u, 3); page(u, d, "karte");
  u.key('n'); u.key('n'); u.key('i'); page(u, d, "karte-detail");
  entry(u, 4); page(u, d, "standort");
  entry(u, 5); page(u, d, "geraet");
  u.gesture(100, 210, 100, 100);
  page(u, d, "geraet-weiter");
  for (int i = 0; i < 7; ++i)
    u.move(0, 1);
  u.click();
  page(u, d, "geraeteinfo");
  return 0;
}
