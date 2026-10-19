// Host test of the T-Deck Plus standalone pages (Issue #35): navigation, chat, unread, Schnellmeldung, Brett,
// map, Standort, Gerät; every page stays inside 320 x 240.
// g++ -std=c++11 -Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined -DLILYGO_TDECK -DDISASTER_STANDALONE
//     -I src -I tests/ui_fakes tests/standalone_ui.test.cpp examples/companion_radio/ui-brand/StandaloneUI.cpp
//     examples/companion_radio/ui-brand/TDeckQuickPage.cpp
#include <assert.h>
#include <stdio.h>
#include <string>
#include "FakeApi.h"
#include "FakeDisplay.h"
#include "../examples/companion_radio/ui-brand/StandaloneUI.h"
#include "../examples/companion_radio/ui-brand/Widgets.h"

static FakeApi fake;
dev::Api &dev::api() { return fake; }
static uint32_t ms = 1000;
static void step(sui::UI &u, FakeDisplay &d) { ms += 300; u.tick(ms); if (u.pending() == sui::UI::PART) u.renderPart(&d); else if (u.pending()) u.render(&d); }
static void type(sui::UI &u, const char *s) { while (*s) u.key(uint8_t(*s++)); }

static void testTheme() {
  char out[64];
  theme::toCp437("\xc3\xa4\xc3\xb6\xc3\xbc\xc3\x9f \xe2\x80\x93 \xe2\x80\xa6 \xe2\x86\x92 \xf0\x9f\x98\x80", out, sizeof(out));
  assert(!strcmp(out, "\x84\x94\x81\xe1 - ... \x1a ?"));
  theme::toCp437("abcdef", out, sizeof(out), 3); assert(!strcmp(out, "abc"));
  theme::toCp437("\xc3\xa4\xc3\xa4", out, 2); assert(!strcmp(out, "\x84"));      // never splits; cap respected
  char t[20];
  theme::stampText(1782900000u, t, sizeof(t)); assert(!strcmp(t, "01.07. 12:00"));    // CEST: UTC+2
  theme::stampText(1767268800u, t, sizeof(t)); assert(!strcmp(t, "01.01. 13:00"));    // CET: UTC+1
  theme::stampText(5, t, sizeof(t)); assert(!strcmp(t, "--.--. --:--"));
  int lines = theme::countLines("eins zwei drei vier fuenf sechs", 10); assert(lines == 4);
  assert(theme::tabForKey('C') == theme::TAB_CHAT && theme::tabForKey('q') == -1);
}

static void testMap() {
  devmap::View v; v.select(0);
  double x0 = -5, y0 = 100, x1 = 400, y1 = 100;
  assert(devmap::View::clip(x0, y0, x1, y1) && x0 == 0 && x1 == 319);
  x0 = -5; y0 = 5; x1 = -1; y1 = 300; assert(!devmap::View::clip(x0, y0, x1, y1));
  assert(v.inside(50.53, 7.05));                 // Ahrtal overview contains Dernau / Ahrweiler
  assert(!v.inside(52.52, 13.40));               // Berlin is not on the map
  double before = v.ppd(); v.zoom(2); assert(v.ppd() > 1.9 * before);
  v.zoom(1000); assert(v.zoomFactor() <= 32.0001);
  v.centerOn(50.5432, 7.1130); assert(v.ppd() >= 29999); assert(v.inside(50.5432, 7.1130));
  FakeDisplay d; d.startFrame(0);
  for (int i = 0; i < devmap::MAP_COUNT; ++i) { v.select(i); unsigned s = v.render(&d, nullptr, 0, nullptr); assert(s > 50); }
  assert(d.outOfBounds == 0);
  assert(d.allText.find("OpenStreetMap") != std::string::npos);
}

int main() {
  testTheme();
  testMap();
  fake.demo();
  sui::UI u; FakeDisplay d;
  assert(u.begin(&fake));
  u.tick(ms);
  assert(u.history().size() == 8);              // 0x82 is no line; it marks the f4 delivered
  assert(u.unreadChat() == 0 && u.unreadBoard() == 0);   // everything before start counts as read
  assert(!u.takeIncoming());

  // Start: click without a tile toggles the display (caller), trackball selects a tile, click opens it
  assert(u.onStart() && !u.click());
  assert(u.move(1, 0) && u.startSelection() == 0);
  ms += sui::UI::START_SELECT_MS + 1; u.tick(ms); assert(u.startSelection() == -1);
  u.move(1, 0); assert(u.click() && u.current() == sui::CHAT_LIST);
  u.render(&d); assert(d.outOfBounds == 0);
  assert(d.allText.find("Ahrtal-Hilfe") != std::string::npos);

  // chat list -> conversation; letters are text there, Enter sends, own line appears
  assert(u.click() && u.current() == sui::CHAT_VIEW && !strcmp(u.conversation(), "c0"));
  u.render(&d); assert(d.outOfBounds == 0);
  assert(d.allText.find("Apotheke am Markt") != std::string::npos);
  type(u, "bin unterwegs, komme in 10 min");
  assert(u.current() == sui::CHAT_VIEW && !strcmp(u.draft(), "bin unterwegs, komme in 10 min"));
  assert(u.pending() == sui::UI::PART); u.renderPart(&d);
  u.key(sui::K_BACKSPACE); assert(!strcmp(u.draft(), "bin unterwegs, komme in 10 mi"));
  u.key(sui::K_ENTER);
  assert(fake.sends == 1 && fake.lastId == "c0" && fake.lastText == "bin unterwegs, komme in 10 mi" && !u.draft()[0]);
  step(u, d);
  assert(u.history().size() == 9 && u.history().at(8).own);
  assert(d.allText.find("Gesendet") != std::string::npos);
  // 100-byte limit
  for (int i = 0; i < 120; ++i) u.key('x');
  assert(strlen(u.draft()) == 100);
  while (u.draft()[0]) u.key(sui::K_BACKSPACE);
  // empty Enter does not send
  u.key(sui::K_ENTER); assert(fake.sends == 1);
  // Backspace on an empty draft goes back to the list
  u.key(sui::K_BACKSPACE); assert(u.current() == sui::CHAT_LIST);

  // unread indicator and wake-up
  fake.channelIn(1, "Lea: Pumpe kommt um 15 Uhr", fake.clock);
  step(u, d);
  assert(u.takeIncoming() && !u.takeIncoming());
  assert(u.unreadChat() == 1);
  u.render(&d); assert(d.allText.find("Pumpe") != std::string::npos);
  u.move(0, 1); u.click(); assert(!strcmp(u.conversation(), "c1"));
  assert(u.unreadChat() == 0);
  u.key(sui::K_ESC); assert(u.current() == sui::CHAT_LIST);
  // direct chat: delivered mark
  u.move(0, 1); u.click(); assert(!strcmp(u.conversation(), "d0a1b2c3d4e5f"));
  u.render(&d); assert(d.allText.find("zugestellt") != std::string::npos);
  u.move(-1, 0); assert(u.current() == sui::CHAT_LIST);

  // Brett: join with an empty Enter, post with text
  u.key('b'); assert(u.current() == sui::BOARD_LIST);
  u.render(&d); assert(d.outOfBounds == 0 && d.allText.find("Brett Ahrweiler") != std::string::npos);
  u.click(); assert(u.current() == sui::BOARD_VIEW);
  u.render(&d); assert(d.allText.find("Bautrockner") != std::string::npos);
  assert(d.allText.find("Anna (Dernau)") != std::string::npos);     // author resolved by key prefix
  u.key(sui::K_ENTER); assert(fake.logins == 1 && !strcmp(fake.rm.id, "dcafe0012ab34"));
  fake.rm.pending = false; fake.rm.rights = 2; step(u, d);
  assert(d.allText.find("beigetreten") != std::string::npos);
  type(u, "Biete Kaffee"); u.click();
  assert(fake.sends == 2 && fake.lastId == "dcafe0012ab34" && fake.lastText == "Biete Kaffee");
  fake.roomIn("dcafe0012ab34", "0a1b2c3d", "Danke!", fake.clock);
  u.key(sui::K_ESC); step(u, d);
  assert(u.unreadBoard() == 1);

  // map
  u.key('k'); assert(u.current() == sui::MAP);
  u.render(&d); assert(d.outOfBounds == 0);
  u.key('+'); assert(u.mapView().zoomFactor() > 1.9);
  u.move(1, 1); u.key('a'); assert(u.mapView().zoomFactor() < 1.01);
  u.key('n'); assert(fake.map == 1 && u.mapView().index() == 1);
  u.render(&d); assert(d.outOfBounds == 0);
  u.key('i'); assert(std::string(u.statusText()).find("GPS") != std::string::npos);   // no fix yet
  for (int i = 0; i < 4; ++i) u.key('n');
  assert(fake.map == 0);

  // Standort: share off by default; position only with share + fresh fix
  u.key('s'); assert(u.current() == sui::LOCATION);
  u.render(&d); assert(d.outOfBounds == 0 && d.allText.find("Standort teilen") != std::string::npos);
  u.move(0, 1); u.move(0, 1); u.click();
  assert(fake.sends == 2 && std::string(u.statusText()).find("Standort teilen") != std::string::npos);
  u.move(0, -1); u.click(); assert(fake.g.share);
  u.move(0, 1); u.click(); assert(fake.sends == 2);           // no fresh fix
  fake.g.fresh = true; fake.g.lat = 50.54321; fake.g.lon = 7.11299;
  u.click(); assert(fake.sends == 3 && fake.lastText.compare(0, 23, "!DP1|50.5432,7.1130|179") == 0);
  assert(fake.lastId == "c0");
  u.move(0, 1); u.click(); assert(fake.adverts == 1);
  u.move(0, -1); u.move(0, -1); u.move(0, -1); u.click(); assert(!fake.g.receiverOn);
  u.click(); assert(fake.g.receiverOn);
  u.render(&d); assert(d.outOfBounds == 0);

  // Gerät
  u.key('g'); assert(u.current() == sui::DEVICE);
  u.render(&d); assert(d.outOfBounds == 0);
  assert(d.allText.find("112") != std::string::npos && d.allText.find("Standard-PIN") != std::string::npos);
  assert(d.allText.find("123456") == std::string::npos);       // never PIN digits
  u.click(); assert(fake.refreshes == 1);
  u.move(0, 1); u.move(0, 1); u.click(); assert(fake.saves == 1);

  // Schnellmeldung: preset, letters into the text, Enter path to "Meldung senden"
  fake.g.fresh = true;                        // receiver was switched off/on above: wait for a new fix
  u.key('m'); assert(u.current() == sui::QUICK);
  u.render(&d); assert(d.outOfBounds == 0);
  u.key('1'); assert(u.draft()[0] == 0);     // the quick page has its own draft
  type(u, " bitte");                          // text focus: letters do not switch tabs
  assert(u.current() == sui::QUICK);
  u.move(0, 1); u.move(0, 1); u.move(0, 1);  // PlaceSelect, Target, Send
  u.click();
assert(fake.sends == 4 && fake.lastId == "c0");
  assert(fake.lastText.find("bitte") != std::string::npos && fake.lastText.find("50.5432") != std::string::npos);
  u.render(&d); assert(d.outOfBounds == 0);
  u.key(sui::K_ESC); assert(u.onStart());

  // left/right walks the tabs from list pages
  u.key('c'); u.move(1, 0); assert(u.current() == sui::QUICK);
  u.key(sui::K_ESC); u.key('g'); u.move(1, 0); assert(u.onStart());

  // not ready: lists say so
  fake.isReady = false; u.key('c'); u.render(&d); assert(d.allText.find("Funkkern startet") != std::string::npos);
  // Start tiles with badges
  d.startFrame(0); u.renderStartTiles(&d, 196, 30); assert(d.outOfBounds == 0);
  ui::navStrip(&d, theme::TAB_INFO, 120, 3, true); assert(d.outOfBounds == 0);
  puts("standalone_ui: ok");
  return 0;
}
