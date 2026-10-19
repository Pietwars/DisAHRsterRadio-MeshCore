// T-Deck Plus standalone display (Issue #35). See StandaloneUI.h.
#if defined(DISASTER_STANDALONE)
#include "StandaloneUI.h"
#include "Widgets.h"
#include <stdio.h>
#include <math.h>

namespace sui {
using namespace theme;
namespace {
const Page TAB_PAGE[TAB_COUNT] = {START, CHAT_LIST, QUICK, BOARD_LIST, MAP, LOCATION, DEVICE};
int tabOf(Page p) {
  switch (p) {
    case CHAT_LIST: case CHAT_VIEW: return TAB_CHAT;
    case QUICK: return TAB_QUICK;
    case BOARD_LIST: case BOARD_VIEW: return TAB_BOARD;
    case MAP: return TAB_MAP;
    case LOCATION: return TAB_LOC;
    case DEVICE: return TAB_INFO;
    default: return TAB_HOME;
  }
}
constexpr int LIST_TOP = NAV_H + 2, ROW_H = 21, HINT_Y = H - 9;
constexpr int COMPOSE_Y = 203, COMPOSE_H = 26;
constexpr int LIST_ROWS = (HINT_Y - LIST_TOP) / ROW_H;   // 10
}

int mapIndexForKey(const char *key) {
  for (int i = 0; key && i < devmap::MAP_COUNT; ++i) if (!strcmp(devmap::MAPS[i].key, key)) return i;
  return -1;
}
const char *mapKey(int index) { return index >= 0 && index < devmap::MAP_COUNT ? devmap::MAPS[index].key : nullptr; }

bool UI::begin(dev::Api *a) {
  api = a;
  bool ok = log.begin();
  map.select(a ? a->mapIndex() : 0);
#ifdef SUI_HAS_QUICK
  quick.begin(a);
#endif
  dirty = FULL;
  return ok;
}

bool UI::quickText() const {
#ifdef SUI_HAS_QUICK
  return quick.textFocus();
#else
  return false;
#endif
}

void UI::setStatus(const char *s, uint32_t ms) {
  snprintf(status, sizeof(status), "%s", s);
  statusUntil = nowMs + ms;
}

unsigned UI::buildList(Page p, dev::Target *out, unsigned cap) const {
  unsigned n = 0; dev::Target t;
  if (!api) return 0;
  for (unsigned i = 0, c = api->targetCount(); i < c && n < cap; ++i) {
    if (!api->target(i, t)) continue;
    bool want = p == BOARD_LIST ? t.kind == dev::Kind::Room : (t.kind == dev::Kind::Channel || t.kind == dev::Kind::Chat);
    if (want) out[n++] = t;
  }
  return n;
}

void UI::open(Page p) {
  if (p == CHAT_VIEW || p == BOARD_VIEW) log.markRead(conv);
#ifdef SUI_HAS_QUICK
  if (p == QUICK && page != QUICK) { quick.setTarget(conv[0] && !convRoom ? conv : "c0"); quick.enter(); }
#endif
  page = p; scroll = 0; startSel = -1; dirty = FULL;
}

void UI::openConversation(const dev::Target &t, bool room) {
  if (strcmp(conv, t.id)) compose[0] = 0;   // a draft belongs to its conversation
  snprintf(conv, sizeof(conv), "%s", t.id);
  snprintf(convName, sizeof(convName), "%s %s", t.kind == dev::Kind::Channel ? "#" : room ? "Brett:" : "@", t.name);
  convRoom = room;
  open(room ? BOARD_VIEW : CHAT_VIEW);
}

void UI::tabStep(int dir) {
  int t = tabOf(page) + dir;
  if (t < 0) t = TAB_COUNT - 1;
  if (t >= TAB_COUNT) t = 0;
  open(TAB_PAGE[t]);
}

bool UI::listMove(Page p, int dy, unsigned n) {
  if (!n) return false;
  int s = listSel[p] + (dy > 0 ? 1 : -1);
  if (s < 0) s = 0;
  if (s >= int(n)) s = int(n) - 1;
  if (s == listSel[p]) return false;
  listSel[p] = s;
  if (s < listTop[p]) listTop[p] = s;
  if (s >= listTop[p] + LIST_ROWS) listTop[p] = s - LIST_ROWS + 1;
  return true;
}

void UI::sendCompose() {
  char st[96] = "";
  if (!api) return;
  if (!compose[0]) {
    if (convRoom) { api->roomLogin(conv, st, sizeof(st)); setStatus(st, 15000); }
    else setStatus("Zuerst Text eingeben");
    dirty = FULL; return;
  }
  bool ok = api->send(conv, compose, st, sizeof(st));
  setStatus(st, ok ? 15000 : 8000);
  if (ok) { compose[0] = 0; scroll = 0; }
  dirty = FULL;
}

void UI::sendPosition() {
  if (!api) return;
  dev::Gps g = api->gps();
  if (!g.share) { setStatus("Erst \"Standort teilen\" einschalten"); return; }
  if (!g.receiverOn || !g.fresh) { setStatus("Kein frischer GPS-Fix \xe2\x80\x93 bitte warten"); return; }
  uint32_t t = api->now();
  if (t < 1700000000u) { setStatus("Uhr fehlt: GPS-Fix abwarten / App verbinden"); return; }
  char text[64], st[96] = "";
  snprintf(text, sizeof(text), "!DP1|%.4f,%.4f|%lu", g.lat, g.lon, (unsigned long)t);   // web: Position in Chat senden
  const char *to = conv[0] && !convRoom ? conv : "c0";
  api->send(to, text, st, sizeof(st));
  setStatus(st, 15000);
}

// ---------------------------------------------------------------------------------------------- input
bool UI::key(uint8_t k) {
  if (!api || !k) return false;
  if (k == '\n') k = K_ENTER;
  if (k == 127) k = K_BACKSPACE;
  Redraw before = dirty;
  if (page == CHAT_VIEW || page == BOARD_VIEW) {
    size_t n = strlen(compose);
    if (k == K_ENTER) sendCompose();
    else if (k == K_BACKSPACE) {
      if (!n) open(convRoom ? BOARD_LIST : CHAT_LIST);
      else { compose[n - 1] = 0; if (dirty < PART) dirty = PART; }
    } else if (k == K_ESC) open(convRoom ? BOARD_LIST : CHAT_LIST);
    else if (k >= 32 && k <= 126) {
      if (n < COMPOSE_BYTES) { compose[n] = char(k); compose[n + 1] = 0; if (dirty < PART) dirty = PART; }
      else { setStatus("Text voll: max. 100 Byte"); if (dirty < PART) dirty = PART; }
    }
    return dirty != before || dirty != NONE;
  }
#ifdef SUI_HAS_QUICK
  if (page == QUICK) {
    if (!quick.textFocus() && k != ' ' && !(k >= '1' && k <= '8')) {
      int t = tabForKey(k); if (t >= 0) { open(TAB_PAGE[t]); return true; }
    }
    if (k == K_ESC || quick.key(k, api->now())) open(START);
    dirty = FULL; return true;
  }
#endif
  int t = tabForKey(k);
  if (t >= 0) { open(TAB_PAGE[t]); return true; }
  if (k == K_BACKSPACE || k == K_ESC) { if (page != START) { open(START); return true; } return false; }
  if (page == START) {
    if (k == K_ENTER) { open(TAB_PAGE[startSel >= 0 ? startSel + 1 : TAB_CHAT]); return true; }
    return false;
  }
  if (page == MAP) {
    if (k == '+' || k == 'z' || k == 'Z') map.zoom(2);
    else if (k == '-' || k == 'x' || k == 'X') map.zoom(0.5);
    else if (k == 'a' || k == 'A') map.fit();
    else if (k == 'i' || k == 'I') {
      dev::Gps g = api->gps();
      if (g.receiverOn && g.fresh) map.centerOn(g.lat, g.lon); else { setStatus("Kein frischer GPS-Fix"); }
    }
    else if (k == 'n' || k == 'N' || k == K_ENTER || k == ' ') { map.select(map.index() + 1); api->setMapIndex(map.index()); }
    else return false;
    dirty = FULL; return true;
  }
  if (k == K_ENTER || k == ' ') return click();
  return false;
}

bool UI::move(int dx, int dy) {
  if (!api || (!dx && !dy)) return false;
  switch (page) {
    case START: {
      int dir = dx ? dx : dy;
      if (startSel < 0) startSel = dir > 0 ? 0 : TAB_COUNT - 2;
      else startSel = (startSel + (dir > 0 ? 1 : TAB_COUNT - 2)) % (TAB_COUNT - 1);
      startSelUntil = nowMs + START_SELECT_MS; dirty = FULL; return true;
    }
    case CHAT_LIST: case BOARD_LIST: {
      if (dx) { tabStep(dx > 0 ? 1 : -1); return true; }
      dev::Target list[MAX_LIST]; unsigned n = buildList(page, list, MAX_LIST);
      if (listMove(page, dy, n)) { dirty = FULL; return true; }
      return false;
    }
    case CHAT_VIEW: case BOARD_VIEW:
      if (dx < 0) { open(convRoom ? BOARD_LIST : CHAT_LIST); return true; }
      if (dy) { scroll += dy < 0 ? 1 : -1; if (scroll < 0) scroll = 0; if (scroll > 95) scroll = 95; dirty = FULL; return true; }
      return false;
#ifdef SUI_HAS_QUICK
    case QUICK:
      if (quick.move(dx, dy)) { dirty = FULL; return true; }
      if (dx) { tabStep(dx > 0 ? 1 : -1); return true; }
      return false;
#endif
    case MAP:
      map.pan(dx * 32, dy * 32); dirty = FULL; return true;
    case LOCATION: case DEVICE: {
      if (dx) { tabStep(dx > 0 ? 1 : -1); return true; }
      int &sel = page == LOCATION ? locSel : devSel; int n = page == LOCATION ? 4 : 3;
      int s = sel + (dy > 0 ? 1 : -1); if (s < 0) s = 0; if (s >= n) s = n - 1;
      if (s == sel) return false;
      sel = s; dirty = FULL; return true;
    }
    default: return false;
  }
}

bool UI::click() {
  if (!api) return false;
  char st[96] = "";
  switch (page) {
    case START:
      if (startSel < 0) return false;
      open(TAB_PAGE[startSel + 1]); return true;
    case CHAT_LIST: case BOARD_LIST: {
      dev::Target list[MAX_LIST]; unsigned n = buildList(page, list, MAX_LIST);
      if (!n) { setStatus(page == BOARD_LIST ? "Kein Brett (Room-Server) bekannt" : "Noch keine Kan\xc3\xa4le geladen"); dirty = FULL; return true; }
      if (listSel[page] >= int(n)) listSel[page] = int(n) - 1;
      openConversation(list[listSel[page]], page == BOARD_LIST); return true;
    }
    case CHAT_VIEW: case BOARD_VIEW: sendCompose(); return true;
#ifdef SUI_HAS_QUICK
    case QUICK: if (quick.activate(api->now())) open(START); dirty = FULL; return true;
#endif
    case MAP: return key('n');
    case LOCATION: {
      dev::Gps g = api->gps();
      if (locSel == 0) api->setGpsReceiver(!g.receiverOn);
      else if (locSel == 1) { api->setGpsShare(!g.share); setStatus(!g.share ? "Standort wird geteilt (Bekanntgabe, Meldungen)" : "Standort wird nicht mehr geteilt"); }
      else if (locSel == 2) sendPosition();
      else { api->advert(st, sizeof(st)); setStatus(st); }
      dirty = FULL; return true;
    }
    case DEVICE:
      if (devSel == 0) { setStatus(api->refresh() ? "Kontakte werden neu gelesen" : "Funkkern besch\xc3\xa4" "ftigt \xe2\x80\x93 sp\xc3\xa4ter erneut"); }
      else if (devSel == 1) { api->advert(st, sizeof(st)); setStatus(st); }
      else { api->saveConfig(st, sizeof(st)); setStatus(st); }
      dirty = FULL; return true;
    default: return false;
  }
}

// ---------------------------------------------------------------------------------------------- state
bool UI::tick(uint32_t ms) {
  nowMs = ms;
  if (!api) return false;
  if (startSel >= 0 && int32_t(ms - startSelUntil) >= 0) { startSel = -1; if (page == START) dirty = FULL; }
  if (status[0] && int32_t(ms - statusUntil) >= 0) { status[0] = 0; if (dirty < PART) dirty = PART; }
#ifdef SUI_HAS_QUICK
  if (page == QUICK && quick.tick(ms)) dirty = FULL;
  quick.unreadChat = unreadChat(); quick.unreadBoard = unreadBoard();
#endif
  if (int32_t(ms - nextPoll) < 0) return dirty != NONE;
  nextPoll = ms + 250;
  uint32_t seq = api->historySeq();
  if (api->ready() && log.needsRebuild(seq)) {
    bool first = log.size() == 0 && wakeSeq == 0;
    api->historyScan(true);
    log.rebuild(seq, [this](size_t slot, disaster::HistoryRecord &r) { return api->historyRecord(slot, r); });
    api->historyScan(false);
    if (first) { log.setBaseline(log.newest()); lastIncoming = log.newest(); wakeSeq = 1; }
    if (page == CHAT_VIEW || page == BOARD_VIEW) log.markRead(conv);
    if (page != MAP || log.incomingAfter(lastIncoming)) dirty = FULL;
  }
  uint32_t sig = api->targetCount() * 131u + (api->ready() ? 1 : 0);
  dev::Room r = api->room(); sig = sig * 31u + r.rights * 7u + (r.pending ? 3 : 0);
  if (sig != lastTargetSig) { lastTargetSig = sig; if (page != MAP) dirty = FULL; }
  return dirty != NONE;
}

bool UI::takeIncoming() {
  if (!log.size() || !log.incomingAfter(lastIncoming)) return false;
  lastIncoming = log.newest();
  return true;
}

unsigned UI::unreadChat() const {
  dev::Target list[MAX_LIST]; unsigned n = buildList(CHAT_LIST, list, MAX_LIST), u = 0;
  for (unsigned i = 0; i < n; ++i) u += log.unread(list[i].id, false);
  return u;
}
unsigned UI::unreadBoard() const {
  dev::Target list[MAX_LIST]; unsigned n = buildList(BOARD_LIST, list, MAX_LIST), u = 0;
  for (unsigned i = 0; i < n; ++i) u += log.unread(list[i].id, true);
  return u;
}

const char *UI::authorName(const chat::Entry &e, char *buf, size_t cap, const char **body) const {
  *body = e.text;
  if (e.own) { snprintf(buf, cap, "Du"); return buf; }
  if (e.conv[0] == 'c') { if (!chat::splitSender(e.text, buf, cap, body)) snprintf(buf, cap, "?"); return buf; }
  if (e.room) {
    dev::Target t;
    for (unsigned i = 0, n = api ? api->targetCount() : 0; i < n; ++i)
      if (api->target(i, t) && t.id[0] == 'd' && !strncmp(t.id + 1, e.author, 8)) { snprintf(buf, cap, "%s", t.name); return buf; }
    snprintf(buf, cap, "%s", e.author); return buf;
  }
  snprintf(buf, cap, "%s", convName[0] == '@' ? convName + 2 : convName);
  return buf;
}

// ---------------------------------------------------------------------------------------------- drawing
void UI::renderStartTiles(DisplayDriver *d, int y, int h) {
  int w = W / (TAB_COUNT - 1);
  unsigned uc = unreadChat(), ub = unreadBoard();
  for (int i = 0; i < TAB_COUNT - 1; ++i) {
    int x = i * w, ww = i == TAB_COUNT - 2 ? W - x : w;
    bool sel = i == startSel;
    d->nativeFill(x + 1, y, ww - 2, h, sel ? MESH : BUTTON2);
    d->nativeRect(x + 1, y, ww - 2, h, sel ? ui::FOCUS : LINE);
    const char *l = TAB_LABEL[i + 1];
    int tw = int(strlen(l)) * CW;
    d->nativeText(x + (ww - tw) / 2, y + (h - CH) / 2, 1, sel ? INK : TEXT, l);
    unsigned n = i + 1 == TAB_CHAT ? uc : i + 1 == TAB_BOARD ? ub : 0;
    if (n) ui::badge(d, x + ww - 2, y + 1, n);
  }
}

void UI::drawHint(DisplayDriver *d) {
  if (status[0]) { ui::hint(d, status, PEND); return; }
  const char *h = "";
  switch (page) {
    case CHAT_LIST: case BOARD_LIST: h = "\x18\x19 w\xc3\xa4hlen \xc2\xb7 Klick: \xc3\xb6" "ffnen \xc2\xb7 \x1b\x1a Reiter \xc2\xb7 Del: Start"; break;
    case CHAT_VIEW: h = "Enter: senden \xc2\xb7 \x18\x19 bl\xc3\xa4ttern \xc2\xb7 \x1b/Del (leer): Liste"; break;
    case BOARD_VIEW: h = "Enter: Aush\xc3\xa4ngen (leer: Beitreten) \xc2\xb7 \x1b/Del: Liste"; break;
    case LOCATION: case DEVICE: h = "\x18\x19 w\xc3\xa4hlen \xc2\xb7 Klick/Enter: ausf\xc3\xbchren \xc2\xb7 \x1b\x1a Reiter"; break;
    default: break;
  }
  ui::hint(d, h);
}

void UI::drawList(DisplayDriver *d, Page p) {
  dev::Target list[MAX_LIST]; unsigned n = buildList(p, list, MAX_LIST);
  if (!api->ready()) { ui::text(d, 6, LIST_TOP + 10, MUTED, "Funkkern startet \xe2\x80\x93 Kontakte werden gelesen \xe2\x80\xa6"); return; }
  if (!n) {
    ui::text(d, 6, LIST_TOP + 10, MUTED, p == BOARD_LIST ? "Kein Brett (Room-Server) in den Kontakten." : "Keine Kan\xc3\xa4le oder Kontakte.");
    ui::text(d, 6, LIST_TOP + 22, MUTED, p == BOARD_LIST ? "Room-Server erscheinen nach ihrer Bekanntgabe." : "Ger\xc3\xa4t \xe2\x86\x92 Kontakte neu lesen.");
    return;
  }
  int &sel = listSel[p], &top = listTop[p];
  if (sel >= int(n)) sel = int(n) - 1;
  if (top > sel) top = sel;
  if (sel >= top + LIST_ROWS) top = sel - LIST_ROWS + 1;
  dev::Room room = api->room();
  for (int r = 0; r < LIST_ROWS && top + r < int(n); ++r) {
    const dev::Target &t = list[top + r];
    int y = LIST_TOP + r * ROW_H;
    bool on = top + r == sel;
    d->nativeFill(0, y, W, ROW_H - 1, on ? CARD : BG);
    if (on) d->nativeFill(0, y, 3, ROW_H - 1, MESH);
    char line[64];
    snprintf(line, sizeof(line), "%s %s", t.kind == dev::Kind::Channel ? "#" : p == BOARD_LIST ? "\xe2\x96\xa0" : "@", t.name);
    ui::text(d, 7, y + 2, p == BOARD_LIST ? BOARD : t.kind == dev::Kind::Channel ? MESH : TEXT, line, 40);
    bool roomList = p == BOARD_LIST;
    unsigned u = log.unread(t.id, roomList);
    if (u) ui::badge(d, W - 4, y + 1, u);
    const chat::Entry *e = log.last(t.id, roomList);
    char who[34], prev[80]; const char *body = "";
    if (roomList) {
      bool here = !strcmp(room.id, t.id);
      snprintf(prev, sizeof(prev), "%s", here && room.pending ? "Anmeldung l\xc3\xa4uft \xe2\x80\xa6" : here && room.rights >= 2 ? "beigetreten" : "nicht beigetreten");
      if (e) { authorName(*e, who, sizeof(who), &body); size_t l = strlen(prev); snprintf(prev + l, sizeof(prev) - l, " \xc2\xb7 %s: %s", who, body); }
    } else if (e) {
      authorName(*e, who, sizeof(who), &body);
      snprintf(prev, sizeof(prev), "%s: %s", who, body);
    } else snprintf(prev, sizeof(prev), "%s", "noch keine Nachrichten");
    ui::text(d, 15, y + 11, MUTED, prev, 50);
  }
  if (int(n) > LIST_ROWS) {   // scroll bar
    int bh = (HINT_Y - LIST_TOP) * LIST_ROWS / int(n), by = LIST_TOP + (HINT_Y - LIST_TOP - bh) * top / (int(n) - LIST_ROWS);
    d->nativeFill(W - 2, by, 2, bh, LINE);
  }
}

void UI::drawCompose(DisplayDriver *d) {
  bool typingNow = compose[0] != 0;
  d->nativeFill(0, COMPOSE_Y - 1, W, COMPOSE_H + 2, BG);
  d->nativeFill(2, COMPOSE_Y, W - 4, COMPOSE_H, CARD);
  d->nativeRect(2, COMPOSE_Y, W - 4, COMPOSE_H, typingNow ? ui::FOCUS : LINE);
  if (!typingNow) ui::text(d, 6, COMPOSE_Y + 4, MUTED, convRoom ? "Aushang (max. 100 Byte)" : "Nachricht (max. 100 Byte) \xe2\x80\xa6");
  else {
    int lines = 0;
    // last two lines of the draft, cursor at the end
    int total = countLines(compose, 51);
    wrap(compose, 51, [&](const char *s, size_t n) {
      if (lines++ < total - 2) return;
      ui::raw(d, 6, COMPOSE_Y + 3 + (lines - 1 - (total > 2 ? total - 2 : 0)) * 9, TEXT, s, n);
    });
  }
  char cnt[12]; snprintf(cnt, sizeof(cnt), "%u/100", unsigned(strlen(compose)));
  d->nativeText(W - 4 - int(strlen(cnt)) * CW - 2, COMPOSE_Y + COMPOSE_H - 8, 1, strlen(compose) >= COMPOSE_BYTES ? BAD : MUTED, cnt);
  drawHint(d);
}

void UI::drawConversation(DisplayDriver *d) {
  // header like the web .tgt row
  d->nativeFill(0, NAV_H, W, 11, CARD);
  ui::text(d, 4, NAV_H + 2, convRoom ? BOARD : MESH, convName, 34);
  if (convRoom) {
    dev::Room r = api->room();
    bool here = !strcmp(r.id, conv);
    const char *st = here && r.pending ? "Anmeldung l\xc3\xa4uft \xe2\x80\xa6" : here && r.rights >= 2 ? "beigetreten" : "Enter: Beitreten";
    ui::rightText(d, W - 4, NAV_H + 2, here && r.rights >= 2 ? MESH : PEND, st);
  } else ui::rightText(d, W - 4, NAV_H + 2, MUTED, "\x1b Liste");
  const int top = NAV_H + 13, bottom = COMPOSE_Y - 2;
  const chat::Entry *items[chat::Log::CAP]; size_t n = 0;
  log.forEach(conv, convRoom, [&](const chat::Entry &e) { if (n < chat::Log::CAP) items[n++] = &e; });
  if (!n) ui::text(d, 6, top + 6, MUTED, convRoom ? "Noch keine Beitr\xc3\xa4ge. Nach dem Beitreten werden sie abgerufen." : "Noch keine Nachrichten in dieser Unterhaltung.", 52);
  if (scroll > int(n) - 1) scroll = n ? int(n) - 1 : 0;
  int y = bottom;
  char cp[200], who[34], meta[80], when[16];
  for (int i = int(n) - 1 - scroll; i >= 0 && y > top; --i) {
    const chat::Entry &e = *items[i];
    const char *body; authorName(e, who, sizeof(who), &body);
    toCp437(body, cp, sizeof(cp));
    const int cols = 45, x = e.own ? 40 : 4, w = W - 44;
    int lines = countLines(cp, cols); if (!lines) lines = 1;
    int h = 11 + lines * 9 + 2;
    int y0 = y - h;
    stampText(e.stamp, when, sizeof(when));
    snprintf(meta, sizeof(meta), "%s \xc2\xb7 %s%s", who, when,
             e.own ? (e.conv[0] == 'd' ? (e.delivered ? " \xc2\xb7 zugestellt" : " \xc2\xb7 gesendet") : " \xc2\xb7 gesendet") : "");
    // clip at the header: draw only what fits (the oldest visible bubble may be cut at the top)
    int cy = y0 < top ? top : y0;
    d->nativeFill(x, cy, w, y - 1 - cy, e.own ? SELF : CARD);
    if (y0 >= top) ui::text(d, x + 3, y0 + 2, e.own ? SAGE : MUTED, meta, 44);
    int ly = y0 + 11;
    wrap(cp, cols, [&](const char *s, size_t len) {
      if (ly >= top && ly + 8 <= bottom) ui::raw(d, x + 3, ly, TEXT, s, len);
      ly += 9;
    });
    y = y0 - 2;
  }
  if (scroll) ui::rightText(d, W - 4, bottom - 9, PEND, "\x19 neuere");
  drawCompose(d);
}

void UI::drawMap(DisplayDriver *d) {
  static devmap::Mark marks[48];
  unsigned nm = 0; dev::Target t;
  for (unsigned i = 0, n = api->targetCount(); i < n && nm < 32; ++i) {
    if (!api->target(i, t) || !t.hasPos) continue;
    devmap::Mark &m = marks[nm++];
    m.lat = t.lat; m.lon = t.lon;
    m.color = t.kind == dev::Kind::Room ? BOARD : t.kind == dev::Kind::Repeater ? INFO : MESH;
    snprintf(m.label, sizeof(m.label), "%s", t.name);
  }
  for (size_t i = log.size(); i-- > 0 && nm < 48;) {   // positions from messages, newest first (web notePos)
    const chat::Entry &e = log.at(i); double la, lo;
    if (!chat::findPos(e.text, la, lo)) continue;
    devmap::Mark &m = marks[nm++]; m.lat = la; m.lon = lo; m.color = WINE;
    const char *body; char who[34]; authorName(e, who, sizeof(who), &body);
    if (e.conv[0] == 'd' && !e.own) { dev::Target c; if (dev::findTarget(*api, e.conv, c)) snprintf(who, sizeof(who), "%s", c.name); }
    snprintf(m.label, sizeof(m.label), "%s", who);
  }
  devmap::Mark me; bool haveMe = false;
  dev::Gps g = api->gps();
  if (g.receiverOn && g.fresh) { me.lat = g.lat; me.lon = g.lon; me.color = TEXT; snprintf(me.label, sizeof(me.label), "Ich"); haveMe = true; }
  map.render(d, marks, nm, haveMe ? &me : nullptr);
  d->nativeFill(0, NAV_H, W, 10, BG);
  d->nativeText(3, NAV_H + 1, 1, MESH, map.map().name);   // map names are CP437 already (MapData.h)
  char z[48]; snprintf(z, sizeof(z), "x%.0f  +/-  i  a  n", map.zoomFactor() < 1 ? 1.0 : map.zoomFactor());
  if (map.zoomFactor() < 1) snprintf(z, sizeof(z), "x1/2  +/-  i  a  n");
  d->nativeText(W - 3 - int(strlen(z)) * CW, NAV_H + 1, 1, MUTED, z);
  if (status[0]) ui::text(d, 3, H - 20, PEND, status);
}

void UI::drawLocation(DisplayDriver *d) {
  dev::Gps g = api->gps();
  int y = NAV_H + 4;
  char line[96];
  ui::text(d, 4, y, TEXT, "Standort", 20, 2); y += 20;
  const char *state = !g.detected ? "GPS nicht erkannt" : !g.receiverOn ? "GPS ausgeschaltet" : g.fresh ? "frischer Fix" : "sucht Satelliten \xe2\x80\xa6";
  snprintf(line, sizeof(line), "GPS: %s \xc2\xb7 %ld Satelliten", state, g.sats);
  ui::text(d, 4, y, g.fresh && g.receiverOn ? MESH : PEND, line); y += 11;
  if (g.receiverOn && g.fresh) snprintf(line, sizeof(line), "Position: %.5f, %.5f", g.lat, g.lon);
  else snprintf(line, sizeof(line), "Position: \xe2\x80\x93");
  ui::text(d, 4, y, TEXT, line); y += 14;
  const char *labels[4] = {"GPS-Empf\xc3\xa4nger", "Standort teilen", "Position in Chat senden", "Bekanntgabe senden"};
  for (int i = 0; i < 4; ++i) {
    bool on = i == locSel;
    int h = 20;
    if (i < 2) {
      bool v = i == 0 ? g.receiverOn : g.share;
      d->nativeFill(4, y, W - 8, h, on ? CARD : BG);
      d->nativeRect(4, y, W - 8, h, on ? ui::FOCUS : LINE);
      ui::text(d, 10, y + 6, TEXT, labels[i]);
      d->nativeFill(W - 54, y + 4, 44, 12, v ? MESH : BUTTON2);
      d->nativeText(W - 54 + (v ? 16 : 13), y + 6, 1, v ? INK : MUTED, v ? "an" : "aus");
    } else {
      bool enabled = i == 3 || (g.share && g.fresh && g.receiverOn);
      ui::button(d, 4, y, W - 8, h, labels[i], on, i == 2 && enabled);
      if (i == 2 && !enabled) ui::rightText(d, W - 10, y + 6, MUTED, g.share ? "kein Fix" : "Teilen aus");
    }
    y += h + 3;
  }
  dev::Target to; const char *id = conv[0] && !convRoom ? conv : "c0";
  snprintf(line, sizeof(line), "Ziel der Position: %s", dev::findTarget(*api, id, to) ? to.name : id);
  ui::text(d, 4, y, MUTED, line); y += 11;
  char info[200], cp[200];
  snprintf(info, sizeof(info), "%s Ab Werk aus; die Wahl bleibt nach einem Neustart erhalten.",
           g.share ? "Bekanntgaben und Meldungen enthalten den GPS-Standort, solange der Fix frisch ist (15 s)."
                   : "Teilen aus: das GPS bleibt im Ger\xc3\xa4t, Bekanntgaben nur mit dem in der App gesetzten Standort.");
  toCp437(info, cp, sizeof(cp));
  wrap(cp, 52, [&](const char *l, size_t n) { if (y + 8 <= HINT_Y) ui::raw(d, 4, y, MUTED, l, n); y += 9; }, 4);
  drawHint(d);
}

void UI::drawDevice(DisplayDriver *d) {
  dev::Info in; api->info(in);
  int y = NAV_H + 3;
  char v[96], when[16];
  auto row = [&](const char *k, const char *val, uint16_t c) {
    ui::text(d, 4, y, MUTED, k, 14);
    ui::text(d, 92, y, c, val, 37);
    y += 10;
  };
  row("Name", in.name[0] ? in.name : "\xe2\x80\x93", TEXT);
  row("Firmware", in.firmware, TEXT);
  snprintf(v, sizeof(v), "%s \xc2\xb7 %s", in.bleConnected ? "App verbunden" : "nicht verbunden", in.defaultPin ? "Standard-PIN" : "eigene PIN");
  row("Bluetooth", v, in.bleConnected ? MESH : TEXT);
  snprintf(v, sizeof(v), "%u Kan\xc3\xa4le \xc2\xb7 %u Kontakte", in.channels, in.contacts);
  row("Netz", v, TEXT);
  if (in.historyOK) snprintf(v, sizeof(v), "letzte Nr. %u (96 im Ger\xc3\xa4t)", in.history); else snprintf(v, sizeof(v), "FEHLER \xe2\x80\x93 wird nicht gespeichert");
  row("Verlauf", v, in.historyOK ? TEXT : BAD);
  stampText(api->now(), when, sizeof(when));
  row("Uhr", in.clockOK ? when : "nicht gestellt (App/GPS)", in.clockOK ? TEXT : PEND);
  snprintf(v, sizeof(v), "TX %lu/%lu \xc2\xb7 RX %lu/%lu (Flood/Direkt)", (unsigned long)in.txFlood, (unsigned long)in.txDirect,
           (unsigned long)in.rxFlood, (unsigned long)in.rxDirect);
  row("Funk", v, TEXT);
  snprintf(v, sizeof(v), "%lu KB frei \xc2\xb7 min. %lu KB", (unsigned long)(in.heapFree / 1024), (unsigned long)(in.heapMin / 1024));
  row("Speicher", v, in.heapMin && in.heapMin < 40 * 1024 ? BAD : TEXT);
  if (in.sdCard) snprintf(v, sizeof(v), "Config %s \xc2\xb7 Log %s%s%s", in.sdConfig ? "ja" : "nein", in.sdLog ? "an" : "aus",
                          in.sdNote[0] ? " \xc2\xb7 " : "", in.sdNote);
  else snprintf(v, sizeof(v), "%s", in.sdNote[0] ? in.sdNote : "keine Karte");
  row("SD-Karte", v, in.sdCard ? TEXT : MUTED);
  y += 3;
  const char *buttons[3] = {"Kontakte neu lesen", "Bekanntgabe senden", "Einstellungen auf SD speichern"};
  for (int i = 0; i < 3; ++i) { ui::button(d, 4, y, W - 8, 17, buttons[i], i == devSel); y += 19; }
  d->nativeFill(0, y, W, 11, WARN_BG);
  ui::text(d, 4, y + 2, BAD, "Kein zertifiziertes Notrufsystem \xc2\xb7 Lebensgefahr: 112");
  drawHint(d);
}

void UI::render(DisplayDriver *d) {
  dirty = NONE;
#ifdef SUI_HAS_QUICK
  if (page == QUICK) { quick.render(d); return; }
#endif
  d->startFrame(BG);
  ui::navStrip(d, tabOf(page), unreadChat(), unreadBoard());
  switch (page) {
    case CHAT_LIST: case BOARD_LIST: drawList(d, page); drawHint(d); break;
    case CHAT_VIEW: case BOARD_VIEW: drawConversation(d); break;
    case MAP: drawMap(d); break;
    case LOCATION: drawLocation(d); break;
    case DEVICE: drawDevice(d); break;
    default: break;
  }
  d->endFrame();
}

void UI::renderPart(DisplayDriver *d) {
  if (dirty == FULL || page == QUICK || page == MAP || page == START) { if (page != START) render(d); dirty = NONE; return; }
  dirty = NONE;
  if (page == CHAT_VIEW || page == BOARD_VIEW) drawCompose(d); else drawHint(d);
  d->endFrame();
}
}
#endif
