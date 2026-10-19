// Host test of the T-Deck Plus standalone pages (Issue #35): boot logo -> Chat, navigation (keys, trackball,
// touch), chat, unread, Schnellmeldung, Brett, map with "Zurück", Standort, Gerät (battery, advert, display off);
// every page stays inside 320 x 240.
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
  assert(theme::tabForKey('h') == -1 && theme::TAB_COUNT == 6);   // no Start tab any more
  // tab hit test matches the strip; the battery field is no tab
  assert(ui::navTabAt(2) == theme::TAB_CHAT && ui::navTabAt(theme::W - 10) == -1);
  int xs[theme::TAB_COUNT], ws[theme::TAB_COUNT]; ui::navLayout(xs, ws);
  assert(xs[theme::TAB_INFO] + ws[theme::TAB_INFO] == theme::W - theme::BATT_W);
  for (int i = 0; i < theme::TAB_COUNT; ++i) assert(ui::navTabAt(xs[i] + ws[i] / 2) == i);
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
  static const uint16_t logo[40 * 20] = {0};
  u.setLogo(logo, 40, 20);
  assert(u.begin(&fake));
  u.tick(ms);
  assert(u.history().size() == 8);              // 0x82 is no line; it marks the f4 delivered
  assert(u.unreadChat() == 0 && u.unreadBoard() == 0);   // everything before start counts as read
  assert(!u.takeIncoming());

  // Boot logo: brand image + version + 112 line, then Chat after BOOT_MS; no Start page
  assert(u.onBoot());
  u.render(&d); assert(d.outOfBounds == 0);
  assert(d.allText.find("standalone-test") != std::string::npos && d.allText.find("112") != std::string::npos);
  ms += sui::UI::BOOT_MS - 100; u.tick(ms); assert(u.onBoot());
  ms += 200; u.tick(ms); assert(u.current() == sui::CHAT_LIST);
  u.render(&d); assert(d.outOfBounds == 0);
  {   // the first key / trackball step / click / tap only leaves the logo (nothing typed, nothing opened)
    sui::UI b; b.begin(&fake); b.tick(ms);
    assert(b.onBoot() && b.key('x') && b.current() == sui::CHAT_LIST && !b.draft()[0]);
    sui::UI c; c.begin(&fake); c.tick(ms); assert(c.move(0, 1) && c.current() == sui::CHAT_LIST);
    sui::UI e; e.begin(&fake); e.tick(ms); assert(e.click() && e.current() == sui::CHAT_LIST);
    sui::UI f; f.begin(&fake); f.tick(ms); assert(f.tap(200, 120) && f.current() == sui::CHAT_LIST);
  }
  // battery in the tab strip (replaces the Start battery line)
  u.setBattery(87, "~87 % \xc2\xb7 4,05 V", "ca. 18 h Restlaufzeit");
  assert(u.pending() == sui::UI::FULL && ui::navBattery() == 87);
  u.render(&d); assert(d.allText.find("87%") != std::string::npos && d.outOfBounds == 0);
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

  // map: "Zurück" button top left (tap, trackball click, Del/Esc) back to the page before the map
  u.key('k'); assert(u.current() == sui::MAP && u.mapReturnPage() == sui::BOARD_LIST);
  u.render(&d); assert(d.outOfBounds == 0);
  assert(d.allText.find("\x1b Zur\x81" "ck") != std::string::npos);
  assert(d.fb[sui::UI::BACK_Y * 320 + sui::UI::BACK_X] == ui::FOCUS);      // drawn focused: the click target
  for (int i = 0; i < 6; ++i) assert(u.move(-1, 0) && u.current() == sui::MAP);   // trackball pans, never leaves
  u.move(0, -3); assert(u.current() == sui::MAP);
  assert(u.click() && u.current() == sui::BOARD_LIST);               // trackball click = Zurück
  u.key('k'); assert(!u.tap(200, 120) && u.current() == sui::MAP);    // a tap on the map itself does nothing
  assert(u.tap(sui::UI::BACK_X + 10, sui::UI::BACK_Y + 5) && u.current() == sui::BOARD_LIST);   // touch
  u.key('k'); u.key(sui::K_BACKSPACE); assert(u.current() == sui::BOARD_LIST);           // Del
  u.key('s'); u.key('k'); u.key(sui::K_ESC); assert(u.current() == sui::LOCATION);       // returns where it came from
  u.key('k'); assert(u.tap(ui::navTabAt(0) == theme::TAB_CHAT ? 5 : 0, 5) && u.current() == sui::CHAT_LIST);   // tab tap
  u.key('k');
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
  assert(d.allText.find("~87 %") != std::string::npos && d.allText.find("18 h") != std::string::npos);   // battery
  assert(d.allText.find("Advert senden") != std::string::npos && d.allText.find("Display aus") != std::string::npos);
  u.click(); assert(fake.refreshes == 1);
  int adv = fake.adverts;
  u.move(0, 1); u.click(); assert(fake.adverts == adv + 1);            // Advert senden
  u.move(0, 1); assert(!u.takeDisplayOff()); u.click(); assert(u.takeDisplayOff() && !u.takeDisplayOff());   // Display aus
  u.move(0, 1); u.click(); assert(fake.saves == 1);
  u.key('d'); assert(u.takeDisplayOff() && u.current() == sui::DEVICE);   // key 'd' outside text fields

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
  u.key(sui::K_ESC); assert(u.current() == sui::CHAT_LIST);   // former Start target: Chat

  // left/right walks the tabs from list pages and wraps (Gerät -> Chat)
  u.key('c'); u.move(1, 0); assert(u.current() == sui::QUICK);
  u.key(sui::K_ESC); u.key('g'); u.move(1, 0); assert(u.current() == sui::CHAT_LIST);
  u.move(-1, 0); assert(u.current() == sui::DEVICE);
  // Del outside text: back to Chat; on the chat list nothing to do
  u.key(sui::K_BACKSPACE); assert(u.current() == sui::CHAT_LIST); assert(!u.key(sui::K_BACKSPACE));
  // 'd' in a text field is text, not "display off"
  u.click(); u.key('d'); assert(!strcmp(u.draft(), "d") && !u.takeDisplayOff()); u.key(sui::K_BACKSPACE); u.key(sui::K_ESC);

  // not ready: lists say so
  fake.isReady = false; u.key('c'); u.render(&d); assert(d.allText.find("Funkkern startet") != std::string::npos);
  // strip with badges, focus frame and an unknown / low battery
  ui::navBattery() = -1; d.startFrame(0); ui::navStrip(&d, theme::TAB_INFO, 120, 3, theme::TAB_CHAT); assert(d.outOfBounds == 0);
  assert(d.allText.find("--%") != std::string::npos);
  ui::navBattery() = 7; d.startFrame(0); ui::navStrip(&d, theme::TAB_MAP, 0, 0); assert(d.allText.find("7%") != std::string::npos);
  puts("standalone_ui: ok");
  return 0;
}
