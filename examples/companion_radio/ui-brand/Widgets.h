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
// Battery level shown at the right end of every status line (set by the UI from UITask's battery reading;
// -1 = unknown). One value for all pages, including the Schnellmeldung page that draws its own status line.
inline int &navBattery() { static int pct = -1; return pct; }
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
// Slim status line on top of every page (replaces the tab bar): "← Title" left (tap = one level back; orange
// frame when the back action has the focus), unread total as a wine badge, battery right.
constexpr int BACK_ZONE_W = theme::W - theme::BATT_W;   // tapping the status line left of the battery = back
inline void statusLine(DisplayDriver *d, const char *title_utf8, bool back, unsigned unread, bool back_focus = false) {
  d->nativeFill(0, 0, theme::W, theme::NAV_H, theme::NAV_BG);
  d->nativeFill(0, theme::NAV_H - 1, theme::W, 1, theme::LINE);
  int x = 4;
  if (back) { d->nativeText(x, 4, 1, theme::MESH, "\x1b"); x += 2 * theme::CW; }
  text(d, x, 4, back ? theme::TEXT : theme::MESH, title_utf8, 30);
  if (back_focus) d->nativeRect(0, 0, x + textWidth(title_utf8) + 4, theme::NAV_H - 1, FOCUS);
  if (unread) {
    char t[12]; snprintf(t, sizeof(t), "%u neu", unread > 99 ? 99 : unread);
    int bw = int(strlen(t)) * theme::CW + 4, bx = theme::W - theme::BATT_W - 4 - bw;
    d->nativeFill(bx, 3, bw, 10, theme::WINE);
    d->nativeText(bx + 2, 4, 1, theme::TEXT, t);
  }
  navBatteryField(d);
}
inline void hint(DisplayDriver *d, const char *utf8, uint16_t c = theme::MUTED) {
  d->nativeFill(0, theme::H - 9, theme::W, 9, theme::BG);
  text(d, 2, theme::H - 8, c, utf8);
}
}
