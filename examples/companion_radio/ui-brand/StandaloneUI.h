#pragma once
// Native operation of the LilyGO T-Deck Plus standalone build (Issue #35): the pages of the web UI
// (web/index.html: Chat, Schnellmeldung, Brett, Karte, Standort, Gerät) on 320 x 240 with keyboard and trackball.
// Hardware-free: everything goes through dev::Api (DeviceApi.h), so host tests and the preview harness
// (tools/standalone_preview.py) run the same code as the firmware.
#include "DeviceApi.h"
#include "ChatLog.h"
#include "MapView.h"
#include "Theme.h"
#include <helpers/ui/DisplayDriver.h>
#if defined(LILYGO_TDECK) && defined(DISASTER_STANDALONE)
#include "TDeckQuickPage.h"
#define SUI_HAS_QUICK 1
#endif

namespace sui {
// Key codes of the T-Deck keyboard (character mode, I2C 0x55) used here.
constexpr uint8_t K_ENTER = 0x0D, K_BACKSPACE = 0x08, K_ALT_C = 0x0C, K_ESC = 0x1B;
enum Page : uint8_t { START, CHAT_LIST, CHAT_VIEW, QUICK, BOARD_LIST, BOARD_VIEW, MAP, LOCATION, DEVICE, PAGE_COUNT };
constexpr size_t COMPOSE_BYTES = 100;   // disaster::LOCAL_TEXT_LIMIT
constexpr unsigned MAX_LIST = 72;       // 8 channels + 64 contacts

// Map keys of MapData.h ("ahrtal", ...) for the SD settings file; -1 / nullptr if unknown.
int mapIndexForKey(const char *key);
const char *mapKey(int index);

class UI {
public:
  enum Redraw : uint8_t { NONE = 0, PART = 1, FULL = 2 };
private:
  dev::Api *api = nullptr;
  chat::Log log;
  devmap::View map;
  Page page = START;
  int startSel = -1;                 // selected Start tile (-1: none, click toggles the display)
  uint32_t startSelUntil = 0, nowMs = 0;
  int listSel[PAGE_COUNT] = {0}, listTop[PAGE_COUNT] = {0};
  int locSel = 0, devSel = 0, scroll = 0;
  char conv[14] = "c0", convName[40] = "";
  bool convRoom = false;
  char compose[COMPOSE_BYTES + 1] = "";
  char status[96] = "";
  uint32_t statusUntil = 0, lastIncoming = 0, wakeSeq = 0, nextPoll = 0, lastTargetSig = 0;
  Redraw dirty = FULL;
#ifdef SUI_HAS_QUICK
  TDeckQuickPage quick;
#endif
  unsigned buildList(Page p, dev::Target *out, unsigned cap) const;
  void open(Page p);
  void openConversation(const dev::Target &t, bool room);
  void setStatus(const char *s, uint32_t ms = 6000);
  bool typing() const { return page == CHAT_VIEW || page == BOARD_VIEW || (page == QUICK && quickText()); }
  bool quickText() const;
  void sendCompose();
  void sendPosition();
  void tabStep(int dir);
  bool listMove(Page p, int dy, unsigned n);
  // drawing
  void drawList(DisplayDriver *d, Page p);
  void drawConversation(DisplayDriver *d);
  void drawCompose(DisplayDriver *d);
  void drawMap(DisplayDriver *d);
  void drawLocation(DisplayDriver *d);
  void drawDevice(DisplayDriver *d);
  void drawHint(DisplayDriver *d);
  const char *authorName(const chat::Entry &e, char *buf, size_t cap, const char **body) const;
public:
  static constexpr uint32_t START_SELECT_MS = 10000;
  bool begin(dev::Api *a);
  Page current() const { return page; }
  bool onStart() const { return page == START; }
  // Input. Each returns true if the page must be redrawn.
  bool key(uint8_t k);
  bool move(int dx, int dy);
  // Trackball click: Enter on every page; on Start only with a selected tile (else false = caller toggles the
  // display, step-1 behaviour).
  bool click();
  // Poll the history / status; returns true if something visible changed.
  bool tick(uint32_t ms);
  // true once per new incoming message (display wake-up).
  bool takeIncoming();
  unsigned unreadChat() const;
  unsigned unreadBoard() const;
  Redraw pending() const { return dirty; }
  void render(DisplayDriver *d);              // whole page (not Start; see renderStartTiles)
  void renderPart(DisplayDriver *d);          // only the compose box / status line
  void renderStartTiles(DisplayDriver *d, int y, int h);
  void forceRedraw() { dirty = FULL; }
  void notify(const char *s) { setStatus(s, 4000); if (dirty < PART) dirty = PART; }
  // for tests
  const char *conversation() const { return conv; }
  const char *draft() const { return compose; }
  const char *statusText() const { return status; }
  int startSelection() const { return startSel; }
  const devmap::View &mapView() const { return map; }
  const chat::Log &history() const { return log; }
#ifdef SUI_HAS_QUICK
  const TDeckQuickPage &quickPage() const { return quick; }
#endif
};
}
