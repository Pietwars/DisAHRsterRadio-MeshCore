#pragma once
// Drawing helpers of the T-Deck Plus standalone display (Issue #35), web look (Theme.h) on 320 x 240.
#include <helpers/ui/DisplayDriver.h>
#include "Theme.h"

namespace ui {
// UTF-8 text in the GFX font, cut after max_chars characters and always at the right screen edge.
inline void text(DisplayDriver *d, int x, int y, uint16_t c, const char *utf8, int max_chars = 53, int size = 1) {
  char buf[160];
  int room = (theme::W - x) / (theme::CW * size);
  if (max_chars > room) max_chars = room;
  if (max_chars < 0) max_chars = 0;
  theme::toCp437(utf8, buf, sizeof(buf), size_t(max_chars));
  d->nativeText(x, y, size, c, buf);
}
// Text that is already CP437 (or plain ASCII).
inline void raw(DisplayDriver *d, int x, int y, uint16_t c, const char *cp, size_t n, int size = 1) {
  char buf[160]; if (n >= sizeof(buf)) n = sizeof(buf) - 1;
  memcpy(buf, cp, n); buf[n] = 0; d->nativeText(x, y, size, c, buf);
}
inline int textWidth(const char *utf8, int size = 1) {
  char buf[160]; return int(theme::toCp437(utf8, buf, sizeof(buf))) * theme::CW * size;
}
inline void rightText(DisplayDriver *d, int xr, int y, uint16_t c, const char *utf8) { text(d, xr - textWidth(utf8), y, c, utf8); }
inline void centerText(DisplayDriver *d, int cx, int y, uint16_t c, const char *utf8, int size = 1) {
  text(d, cx - textWidth(utf8, size) / 2, y, c, utf8, 53, size);
}
// Web buttons: .b (mesh green, dark text) and .b2 (#24384a, light text); focus = orange outline like the
// T-Deck quick page of 0.9.5-test.
constexpr uint16_t FOCUS = theme::rgb(0xffa500);
inline void button(DisplayDriver *d, int x, int y, int w, int h, const char *label, bool focused, bool primary = false) {
  d->nativeFill(x, y, w, h, primary ? theme::MESH : theme::BUTTON2);
  d->nativeRect(x, y, w, h, focused ? FOCUS : (primary ? theme::MESH : theme::LINE));
  if (focused) d->nativeRect(x + 1, y + 1, w - 2, h - 2, FOCUS);
  centerText(d, x + w / 2, y + (h - theme::CH) / 2, primary ? theme::INK : theme::TEXT, label);
}
inline void badge(DisplayDriver *d, int xr, int y, unsigned n, uint16_t bg = theme::WINE) {
  char t[8]; snprintf(t, sizeof(t), "%u", n > 99 ? 99 : n);
  int w = int(strlen(t)) * theme::CW + 4;
  d->nativeFill(xr - w, y, w, 9, bg); d->nativeText(xr - w + 2, y + 1, 1, theme::TEXT, t);
}
// Battery level shown at the right end of every tab strip (set by the UI from UITask's battery reading;
// -1 = unknown). One value for all pages, including the Schnellmeldung page that draws its own strip.
inline int &navBattery() { static int pct = -1; return pct; }
// Tab layout of the strip: tabs share W - BATT_W; returns the tab under x (or -1 on the battery field).
inline void navLayout(int *xs, int *ws) {
  int chars = 0; for (int i = 0; i < theme::TAB_COUNT; ++i) chars += int(strlen(theme::TAB_LABEL[i]));
  const int tabs_w = theme::W - theme::BATT_W;
  int pad = (tabs_w - chars * theme::CW) / (2 * theme::TAB_COUNT), x = 0;
  for (int i = 0; i < theme::TAB_COUNT; ++i) {
    int w = int(strlen(theme::TAB_LABEL[i])) * theme::CW + 2 * pad;
    if (i == theme::TAB_COUNT - 1) w = tabs_w - x;
    xs[i] = x; ws[i] = w; x += w;
  }
}
inline int navTabAt(int x) {
  int xs[theme::TAB_COUNT], ws[theme::TAB_COUNT]; navLayout(xs, ws);
  for (int i = 0; i < theme::TAB_COUNT; ++i) if (x >= xs[i] && x < xs[i] + ws[i]) return i;
  return -1;
}
// Battery field: outline icon filled by level (green from 40 %, amber from 15 %, else red) and "87%".
inline void navBatteryField(DisplayDriver *d) {
  const int x0 = theme::W - theme::BATT_W, pct = navBattery();
  d->nativeFill(x0, 0, theme::BATT_W, theme::NAV_H - 1, theme::NAV_BG);
  d->nativeFill(x0, 2, 1, theme::NAV_H - 5, theme::LINE);
  const int ix = x0 + 4, iy = 4, iw = 13, ih = 7;
  d->nativeRect(ix, iy, iw, ih, theme::TEXT);
  d->nativeFill(ix + iw, iy + 2, 2, ih - 4, theme::TEXT);
  uint16_t c = pct >= 40 ? theme::MESH : pct >= 15 ? theme::PEND : theme::BAD;
  if (pct > 0) { int fw = (iw - 4) * (pct > 100 ? 100 : pct) / 100; if (fw < 1) fw = 1; d->nativeFill(ix + 2, iy + 2, fw, ih - 4, c); }
  char t[8]; if (pct >= 0) snprintf(t, sizeof(t), "%d%%", pct > 100 ? 100 : pct); else snprintf(t, sizeof(t), "--%%");
  d->nativeText(theme::W - 2 - int(strlen(t)) * theme::CW, 4, 1, pct >= 0 && pct < 15 ? theme::BAD : theme::TEXT, t);
}
// Tab strip like the web <nav>: active tab in mesh green with a 2 px underline; unread badges on Chat/Brett;
// focus_tab draws the orange focus frame on one tab (Schnellmeldung: "back to Chat" action); battery at the right.
inline void navStrip(DisplayDriver *d, int active, unsigned unread_chat, unsigned unread_board, int focus_tab = -1) {
  d->nativeFill(0, 0, theme::W, theme::NAV_H, theme::NAV_BG);
  d->nativeFill(0, theme::NAV_H - 1, theme::W, 1, theme::LINE);
  int xs[theme::TAB_COUNT], ws[theme::TAB_COUNT]; navLayout(xs, ws);
  for (int i = 0; i < theme::TAB_COUNT; ++i) {
    int x = xs[i], w = ws[i], lw = int(strlen(theme::TAB_LABEL[i])) * theme::CW;
    bool on = i == active;
    unsigned n = i == theme::TAB_CHAT ? unread_chat : i == theme::TAB_BOARD ? unread_board : 0;
    // with an unread badge the label moves left so that label + badge fit into the tab
    int lx = n ? x + 2 : x + (w - lw) / 2;
    d->nativeText(lx, 3, 1, on ? theme::MESH : theme::rgb(0xccffee), theme::TAB_LABEL[i]);
    if (on) d->nativeFill(x + 1, theme::NAV_H - 3, w - 2, 2, theme::MESH);
    if (i == focus_tab) d->nativeRect(x, 0, w, theme::NAV_H - 1, FOCUS);
    if (n) {
      char t[4]; snprintf(t, sizeof(t), n > 9 ? "9+" : "%u", n);
      int bw = int(strlen(t)) * theme::CW + 2;
      d->nativeFill(lx + lw + 1, 2, bw, 10, theme::WINE);
      d->nativeText(lx + lw + 2, 3, 1, theme::TEXT, t);
    }
  }
  navBatteryField(d);
}
inline void hint(DisplayDriver *d, const char *utf8, uint16_t c = theme::MUTED) {
  d->nativeFill(0, theme::H - 9, theme::W, 9, theme::BG);
  text(d, 2, theme::H - 8, c, utf8);
}
}
