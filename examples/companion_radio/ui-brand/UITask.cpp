#include "UITask.h"
#include "../MyMesh.h"
#include "target.h"
#include "BrandLogo.h"
#include <helpers/esp32/SerialBLEInterface.h>
#ifdef PIN_USER_BTN
  #include <helpers/ui/MomentaryButton.h>
  // T-Deck-Trackball: kurz klicken = Display an/aus, 2,5 s halten = Flood-Advert. Kein Ausschalten.
  #ifndef BRAND_ADVERT_HOLD_MILLIS
    #define BRAND_ADVERT_HOLD_MILLIS 2500
  #endif
  static MomentaryButton brand_btn(PIN_USER_BTN, BRAND_ADVERT_HOLD_MILLIS, true, false, false);
#endif

#define BRAND_LOGO_Y 2   // Bild oben, volle Breite

#ifdef DAR_STANDALONE_UI
  #include <Wire.h>
  // Trackball directions of the T-Deck (Hall sensors, one pulse per step), counted in interrupts.
  static volatile uint16_t tb_count[4] = {0, 0, 0, 0};   // up, down, left, right
  static void IRAM_ATTR tb_up() { tb_count[0]++; }
  static void IRAM_ATTR tb_down() { tb_count[1]++; }
  static void IRAM_ATTR tb_left() { tb_count[2]++; }
  static void IRAM_ATTR tb_right() { tb_count[3]++; }
  #define TDECK_KEYBOARD_ADDR 0x55   // keyboard MCU on SDA 18 / SCL 8, character mode
  #include "TDeckTouch.h"
  static tdeck::Touch tdeck_touch;
#endif

// Farben (RGB565)
#define C_BG     0x08A3   // Hintergrund wie das Titelbild (#0b141c)
#define C_GREEN  0x4EF0   // Markengruen
#define C_WHITE  0xFFFF
#define C_GREY   0x8410
#define C_ORANGE 0xFD20
#define C_RED    0xF800

void UITask::begin(DisplayDriver* display, SensorManager* sensors, NodePrefs* node_prefs) {
  _display = display;
  _node_prefs = node_prefs;
#ifdef PIN_USER_BTN
  brand_btn.begin();
#endif
  readBattery();
  sampleBattery(millis());
#ifdef DAR_STANDALONE_UI
  _sui.begin(&dev::api());
  _sui.setLogo(BRAND_LOGO, BRAND_LOGO_W, BRAND_LOGO_H);   // boot logo, then Chat
  pushBattery();
  tdeck_touch.probe();
  const uint8_t pins[4] = {PIN_TB_UP, PIN_TB_DOWN, PIN_TB_LEFT, PIN_TB_RIGHT};
  void (*isr[4])() = {tb_up, tb_down, tb_left, tb_right};
  for (int i = 0; i < 4; i++) { pinMode(pins[i], INPUT_PULLUP); attachInterrupt(digitalPinToInterrupt(pins[i]), isr[i], FALLING); }
#endif
  if (_display) _display->turnOn();
  _auto_off = millis() + BRAND_AUTO_OFF_MILLIS;
  _dirty = true;
}

// ---- Akku ---------------------------------------------------------------------------------
// Momentanwerte lesen. T-Watch S3: AXP2101 (Spannung, Ladestand-%, USB, Laden).
// T-Deck Plus: nur die Spannung am ADC; Prozent daraus geschaetzt, USB/Laden nicht messbar.
void UITask::readBattery() {
  batt::Now b;
#if defined(LILYGO_TWATCH_S3)
  int mv = 0, pct = -1, usb = -1, chg = -1;
  if (board.readPower(mv, pct, usb, chg)) {
    b.mv = mv; b.pct = pct > 100 ? 100 : pct; b.pct_approx = false; b.usb = usb; b.charging = chg;
    if (b.mv > 0 && b.pct < 0) { b.pct = (int)(batt::percentFromMv(b.mv) + 0.5f); b.pct_approx = true; }
  }
#else
  int mv = board.getBattMilliVolts();
  if (mv >= 1000) { b.mv = mv; b.pct = (int)(batt::percentFromMv(mv) + 0.5f); b.pct_approx = true; }
#endif
  _batt = b;
}

// Ein Messpunkt pro Minute fuer den Trend, auch bei dunklem Display (eine ADC- bzw. I2C-Abfrage).
void UITask::sampleBattery(unsigned long now) {
  if (_next_sample && (long)(now - _next_sample) < 0) return;
  _next_sample = now + BATT_SAMPLE_SECS * 1000UL;
  readBattery();
  if (_batt.usb != _last_usb) { _trend.reset(); _last_usb = _batt.usb; }   // Stecker rein/raus
  if (_batt.mv <= 0 || _batt.charging == 1 || _batt.usb == 1) { _trend.reset(); return; }
  _trend.add(now / 1000, _batt.mv);
}

int UITask::stripHeight() const {
  int lines = _display->nativeWidth() <= 240 ? 2 : 1;
  return lines * _display->nativeTextHeight(1) + (lines - 1) + 2;
}

// Statuszeile zwischen Bild und Knopf: Akkusymbol, Ladestand + Spannung, Verbrauch (Schaetzung).
// T-Deck: eine Zeile (Verbrauch rechtsbuendig), T-Watch: zwei Zeilen (Verbrauch zentriert darunter).
void UITask::drawBattery(bool push) {
  DisplayDriver* d = _display;
  int W = d->nativeWidth(); bool small = W <= 240;
  const int m = 4;
  int y = BRAND_LOGO_Y + BRAND_LOGO_H + 2, h = stripHeight(), th = d->nativeTextHeight(1);
  batt::texts(_batt, _trend, _bl, sizeof(_bl), _be, sizeof(_be), 0);
  d->nativeFill(0, y, W, h, C_BG);
  // Akkusymbol, Fuellung nach Ladestand (gruen ab 40 %, orange ab 15 %, sonst rot)
  int iw = small ? 22 : 18, ih = small ? 12 : 8, iy = y + 1 + (th - ih) / 2;
  d->nativeRect(m, iy, iw, ih, C_WHITE);
  d->nativeFill(m + iw, iy + ih / 4, 2, ih - 2 * (ih / 4), C_WHITE);
  if (_batt.pct >= 0) {
    uint16_t c = _batt.pct >= 40 ? C_GREEN : (_batt.pct >= 15 ? C_ORANGE : C_RED);
    int fw = (iw - 4) * _batt.pct / 100; if (fw < 1 && _batt.pct > 0) fw = 1;
    if (fw > 0) d->nativeFill(m + 2, iy + 2, fw, ih - 4, c);
  }
  int tx = m + iw + 2 + (small ? 6 : 4);
  d->nativeText(tx, y + 1, 1, C_WHITE, _bl);
  // Verbrauch: laengste Textvariante, die passt
  char est[48], l[40]; est[0] = 0;
  int avail = small ? W - 2 * m : W - m - (tx + d->nativeTextWidth(_bl, 1) + 8);
  for (int v = 0; v < 3; v++) {
    batt::texts(_batt, _trend, l, sizeof(l), est, sizeof(est), v);
    if (d->nativeTextWidth(est, 1) <= avail) break;
    if (v == 2) est[0] = 0;
  }
  if (est[0]) {
    int ew = d->nativeTextWidth(est, 1);
    if (small) d->nativeText((W - ew) / 2, y + 1 + th + 1, 1, C_GREY, est);
    else d->nativeText(W - m - ew, y + 1, 1, C_GREY, est);
  }
  if (push) d->endFrame();
}

#ifdef DAR_STANDALONE_UI
// Battery level/consumption for the tab strip (every page) and the Gerät page (replaces the Start battery line).
void UITask::pushBattery() {
  char l[48], e[48]; batt::texts(_batt, _trend, l, sizeof(l), e, sizeof(e), 0);
  _sui.setBattery(_batt.pct, l, e);
}
#endif

void UITask::wake() {
  if (!_display) return;
  if (!_display->isOn()) {
    _display->turnOn(); readBattery(); _next_batt_draw = millis() + BRAND_BATT_REFRESH_MILLIS;
#ifdef DAR_STANDALONE_UI
    pushBattery();
#endif
  }
  _auto_off = millis() + BRAND_AUTO_OFF_MILLIS;
  _dirty = true;
}

void UITask::toggleDisplay() {
  if (!_display) return;
  if (_display->isOn() && !_pairing) _display->turnOff();
  else wake();
}

void UITask::requestAdvert() {
  wake();
  unsigned long now = millis();
  if (_advert_sent_once && now - _last_advert < BRAND_ADVERT_GAP_MILLIS) {
    _status = 2;
    _wait_secs = (int)((BRAND_ADVERT_GAP_MILLIS - (now - _last_advert) + 999) / 1000);
  } else {
    bool ok = the_mesh.floodAdvert();
    _status = ok ? 1 : 3;
    if (ok) { _last_advert = now; _advert_sent_once = true; }
  }
  _status_until = now + BRAND_STATUS_MILLIS;
  _dirty = true;
#ifdef DAR_STANDALONE_UI
  char t[40];
  if (_status == 2) snprintf(t, sizeof(t), "Advert: bitte %d s warten", _wait_secs);
  _sui.notify(_status == 1 ? "Advert gesendet" : _status == 2 ? t : "Advert-Fehler");
#endif
}

void UITask::buttonRect(int& x, int& y, int& w, int& h) const {
  // Bild moeglichst gross (volle Breite), darunter der Knopf bis kurz vor den unteren Rand
  int W = _display->nativeWidth(), H = _display->nativeHeight();
  x = W / 20; w = W - 2 * x; y = BRAND_LOGO_Y + BRAND_LOGO_H + 2 + stripHeight() + 2; h = H - y - 5;
}

static void centerText(DisplayDriver* d, int cx, int y, int size, uint16_t c, const char* s) {
  d->nativeText(cx - d->nativeTextWidth(s, size) / 2, y, size, c, s);
}

void UITask::render() {
  DisplayDriver* d = _display;
  int W = d->nativeWidth();
  bool small = W <= 240;
#ifdef DAR_STANDALONE_UI
  if (!_pairing) { _sui.render(d); return; }      // boot logo or page; the PIN screen below stays as before
#endif
  d->startFrame(C_BG);
  d->nativeImage((W - BRAND_LOGO_W) / 2, BRAND_LOGO_Y, BRAND_LOGO_W, BRAND_LOGO_H, BRAND_LOGO);
  drawBattery(false);

  int bx, by, bw, bh; buttonRect(bx, by, bw, bh);
  int big = small ? 2 : 3;
  int hs = d->nativeTextHeight(1), hb = d->nativeTextHeight(big);
  if (_pairing) {
    char pin[16]; snprintf(pin, sizeof(pin), "%06lu", (unsigned long)the_mesh.getBLEPin());
    int sp = 3, hp = d->nativeTextHeight(sp);
    d->nativeFill(bx, by, bw, bh, C_WHITE);
    centerText(d, W / 2, by + 2, 1, C_BG, "Bluetooth-Kopplung - PIN:");
    centerText(d, W / 2, by + (bh - hp) / 2 + 2, sp, C_BG, pin);
    centerText(d, W / 2, by + bh - hs - 1, 1, C_GREY, "am Handy eingeben");
  } else if (_status && millis() < _status_until) {
    char t[32];
    uint16_t c = _status == 1 ? C_GREEN : (_status == 2 ? C_ORANGE : C_RED);
    const char* s = _status == 1 ? "Advert gesendet" : (_status == 2 ? t : "Advert-Fehler");
    if (_status == 2) snprintf(t, sizeof(t), "Bitte %d s warten", _wait_secs);
    d->nativeFill(bx, by, bw, bh, c);
    centerText(d, W / 2, by + (bh - hb) / 2, big, C_BG, s);
  } else {
    d->nativeFill(bx, by, bw, bh, C_GREEN);
    d->nativeRect(bx, by, bw, bh, C_WHITE);
    int gap = (bh - hb - hs) / 3; if (gap < 1) gap = 1;
    centerText(d, W / 2, by + gap, big, C_BG, "Advert senden");
#if defined(UI_HAS_TOUCH)
    centerText(d, W / 2, by + 2 * gap + hb, 1, C_BG, "antippen");
#else
    centerText(d, W / 2, by + 2 * gap + hb, 1, C_BG, "Trackball 2,5 s halten");
#endif
  }
  d->endFrame();
}

void UITask::loop() {
  if (!_display) return;
  unsigned long now = millis();

#ifdef PIN_USER_BTN
  int ev = brand_btn.check();
#ifdef DAR_STANDALONE_UI
  // click = Enter on the pages (map: "Zurück"); dark display: the click only wakes it
  if (ev == BUTTON_EVENT_CLICK) {
    if (_display->isOn() && !_pairing && _sui.click()) input();
    else toggleDisplay();
  }
#else
  if (ev == BUTTON_EVENT_CLICK) toggleDisplay();
#endif
  else if (ev == BUTTON_EVENT_LONG_PRESS) requestAdvert();
#endif

#if defined(LILYGO_TWATCH_S3)
  int k = board.pollPowerKey();
  if (k == PKEY_SHORT) toggleDisplay();
  else if (k == PKEY_LONG) requestAdvert();
#endif

#if defined(UI_HAS_TOUCH)
  {
    int tx, ty;
    bool down = _display->getTouch(&tx, &ty);
    if (down) {
      if (!_touch_down) { _touch_x = tx; _touch_y = ty; }
      _touch_down = true;
    } else if (_touch_down) {
      _touch_down = false;
      if (!_display->isOn()) {
        wake();                                   // erster Tipp weckt nur
      } else {
        int bx, by, bw, bh; buttonRect(bx, by, bw, bh);
        if (!_pairing && _touch_x >= bx && _touch_x < bx + bw && _touch_y >= by && _touch_y < by + bh) requestAdvert();
        else _auto_off = now + BRAND_AUTO_OFF_MILLIS;
      }
    }
  }
#endif

  bool pairing = ble_pairing_active();
  if (pairing != _pairing) {
    _pairing = pairing;
    if (pairing) wake();                          // PIN nur waehrend der Kopplung zeigen
    _dirty = true;
  }
  if (_status && now >= _status_until) { _status = 0; _dirty = true; }
  sampleBattery(now);
#ifdef DAR_STANDALONE_UI
  pollKeyboard(now);
  pollTrackball(now);
  pollTouch(now);
  if (_sui.takeDisplayOff() && !_pairing) _display->turnOff();   // Gerät "Display aus" or key 'd'
  if (_sui.tick(now)) _dirty = true;
  if (_sui.takeIncoming()) wake();                // incoming message: display on (unread badge)
#endif

  if (_display->isOn()) {
    if (_pairing) _auto_off = now + BRAND_AUTO_OFF_MILLIS;
    if ((long)(now - _auto_off) > 0) {
      _display->turnOff();
    } else if (_dirty) {
      _dirty = false;
#ifdef DAR_STANDALONE_UI
      if (!_pairing && _sui.pending() == sui::UI::PART) _sui.renderPart(_display);
      else
#endif
      render();
      _next_batt_draw = now + BRAND_BATT_REFRESH_MILLIS;
    } else if ((long)(now - _next_batt_draw) >= 0) {
      // nur bei hellem Display: Akkuwerte neu lesen, Statuszeile nur bei geaendertem Text neu zeichnen
      _next_batt_draw = now + BRAND_BATT_REFRESH_MILLIS;
#ifdef DAR_STANDALONE_UI
      // standalone: battery in the tab strip and on Gerät; the UI asks for a redraw only if a shown value changed
      readBattery(); pushBattery();
      if (_sui.pending() != sui::UI::NONE) _dirty = true;
#else
      char ol[40], oe[48]; strcpy(ol, _bl); strcpy(oe, _be);
      readBattery();
      char nl[40], ne[48]; batt::texts(_batt, _trend, nl, sizeof(nl), ne, sizeof(ne), 0);
      if (strcmp(ol, nl) || strcmp(oe, ne)) drawBattery(true);
#endif
    }
  }
}

#ifdef DAR_STANDALONE_UI
void UITask::input() {
  _auto_off = millis() + BRAND_AUTO_OFF_MILLIS;
  _dirty = true;
}

void UITask::pollKeyboard(unsigned long now) {
  if ((long)(now - _next_keyboard) < 0) return;
  if (!_keyboard_present) {
    _next_keyboard = now + 5000;
    Wire.beginTransmission(TDECK_KEYBOARD_ADDR);
    _keyboard_present = Wire.endTransmission() == 0;
    if (_keyboard_present) {
      // character mode, in case an earlier firmware left the keyboard in raw matrix mode
      Wire.beginTransmission(TDECK_KEYBOARD_ADDR); Wire.write(uint8_t(0x04)); Wire.endTransmission();
      Serial.println("[TDECK] Tastatur erkannt (I2C 0x55, Zeichenmodus)");
    }
    return;
  }
  _next_keyboard = now + 30;
  if (Wire.requestFrom(uint8_t(TDECK_KEYBOARD_ADDR), uint8_t(1)) != 1) {
    _keyboard_present = false; _next_keyboard = now + 5000;
    Serial.println("[TDECK] Tastatur nicht erreichbar; erneuter Versuch in 5 s");
    return;
  }
  uint8_t key = Wire.read();
  if (!key || _pairing) return;
  if (!_display->isOn()) { wake(); return; }      // a key only wakes the display, never sends
  _sui.key(key);
  input();
}

// T-Deck Plus touch panel (GT911): a tap acts on release at the press position; dark display: wake only.
void UITask::pollTouch(unsigned long now) {
  if ((long)(now - _next_touch) < 0) return;
  _next_touch = now + 30;
  if (!tdeck_touch.present()) { if ((long)(now - _next_touch_probe) >= 0) { _next_touch_probe = now + 5000; tdeck_touch.probe(); } return; }
  int x, y;
  bool down = tdeck_touch.read(x, y);
  if (down) { if (!_touch_down) { _touch_x = x; _touch_y = y; } _touch_down = true; return; }
  if (!_touch_down) return;
  _touch_down = false;
  if (_pairing) return;
  if (!_display->isOn()) { wake(); return; }
  if (_sui.tap(_touch_x, _touch_y)) input();
  else _auto_off = now + BRAND_AUTO_OFF_MILLIS;
}

void UITask::pollTrackball(unsigned long now) {
  if ((long)(now - _next_ball) < 0) return;
  _next_ball = now + 60;
  uint16_t c[4];
  noInterrupts(); for (int i = 0; i < 4; i++) { c[i] = tb_count[i]; tb_count[i] = 0; } interrupts();
  int dy = int(c[1]) - int(c[0]), dx = int(c[3]) - int(c[2]);
  if (!dx && !dy) return;
  if (_pairing) return;
  if (!_display->isOn()) { wake(); return; }
  if (dx > 3) dx = 3;
  if (dx < -3) dx = -3;
  if (dy > 3) dy = 3;
  if (dy < -3) dy = -3;
  // the dominant axis only: a diagonal roll must not switch tabs while scrolling
  if (abs(dx) >= abs(dy)) dy = 0; else dx = 0;
  if (_sui.move(dx, dy)) input();
  else _auto_off = now + BRAND_AUTO_OFF_MILLIS;
}
#endif
