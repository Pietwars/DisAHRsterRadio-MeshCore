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

// Start page as drawn by UITask (logo, battery line, tiles, hint)
static void start(sui::UI &u, FakeDisplay &d) {
  ms += 300; u.tick(ms);
  d.startFrame(theme::BG);
  d.nativeImage(0, 2, BRAND_LOGO_W, BRAND_LOGO_H, BRAND_LOGO);
  batt::Now b; b.mv = 4050; b.pct = 87; b.pct_approx = true;
  batt::Trend t; char bl[40], be[48]; batt::texts(b, t, bl, sizeof(bl), be, sizeof(be), 0);
  int y = 2 + BRAND_LOGO_H + 2;
  d.nativeRect(4, y + 1, 18, 8, theme::TEXT); d.nativeFill(22, y + 3, 2, 4, theme::TEXT);
  d.nativeFill(6, y + 3, 12, 4, theme::MESH);
  d.nativeText(28, y + 1, 1, theme::TEXT, bl);
  if (be[0]) d.nativeText(320 - 4 - int(strlen(be)) * 6, y + 1, 1, theme::MUTED, be);
  int by = y + 10 + 2, bh = 240 - by - 5;
  u.renderStartTiles(&d, by, bh - 10);
  const char *hint = "Trackball: w\x84hlen, Klick: \x94" "ffnen, 2,5 s: Advert";
  d.nativeText(160 - int(strlen(hint)) * 3, by + bh - 8, 1, theme::MUTED, hint);
  shot(d, "start");
}

int main(int argc, char **argv) {
  if (argc < 2) { fprintf(stderr, "usage: preview <dir>\n"); return 2; }
  out = argv[1];
  fake.demo();
  fake.g.share = true; fake.g.fresh = true; fake.g.lat = 50.54292; fake.g.lon = 7.11463; fake.g.sats = 9;
  sui::UI u; FakeDisplay d; d.font = font;
  u.begin(&fake); u.tick(ms);
  // two new messages after start: unread badges
  fake.channelIn(1, "Lea: Pumpe kommt um 15 Uhr zum Marktplatz", fake.clock - 120);
  fake.roomIn("dcafe0012ab34", "11223344", "Heute 18 Uhr: Treffen am Gemeindehaus", fake.clock - 60);
  ms += 300; u.tick(ms);
  u.move(1, 0);                           // tile "Chat" selected
  start(u, d);
  u.click(); page(u, d, "chat-liste");
  u.click(); type(u, "Bin in 10 Min. mit Kanistern da"); page(u, d, "chat");
  u.key(sui::K_ESC); u.move(0, 1); u.move(0, 1); u.click(); page(u, d, "chat-direkt");
  u.key(sui::K_ESC); u.key('m');
  u.key('3'); type(u, "Ibuprofen");
  page(u, d, "meldung");
  u.key(sui::K_ESC); u.key('b'); page(u, d, "brett-liste");
  u.click(); fake.rm.rights = 2; snprintf(fake.rm.id, sizeof(fake.rm.id), "dcafe0012ab34"); type(u, "Biete 2 Bautrockner");
  page(u, d, "brett");
  u.key(sui::K_ESC); u.key('k'); page(u, d, "karte");
  u.key('n'); u.key('n'); u.key('i'); page(u, d, "karte-detail");
  u.key('s'); page(u, d, "standort");
  u.key('g'); page(u, d, "geraet");
  return 0;
}
