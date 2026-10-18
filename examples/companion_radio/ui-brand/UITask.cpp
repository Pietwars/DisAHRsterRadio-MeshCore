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
  if (_display) _display->turnOn();
  _auto_off = millis() + BRAND_AUTO_OFF_MILLIS;
  _dirty = true;
}

void UITask::wake() {
  if (!_display) return;
  if (!_display->isOn()) _display->turnOn();
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
}

void UITask::buttonRect(int& x, int& y, int& w, int& h) const {
  int W = _display->nativeWidth(), H = _display->nativeHeight();
  int top = 8 + BRAND_LOGO_H + (H >= 240 && W <= 240 ? 14 : 12);
  x = W / 12; w = W - 2 * x; y = top; h = H - top - (W <= 240 ? 18 : 22);
}

static void centerText(DisplayDriver* d, int cx, int y, int size, uint16_t c, const char* s) {
  d->nativeText(cx - d->nativeTextWidth(s, size) / 2, y, size, c, s);
}

void UITask::render() {
  DisplayDriver* d = _display;
  int W = d->nativeWidth(), H = d->nativeHeight();
  bool small = W <= 240;
  d->startFrame(C_BG);
  d->nativeImage((W - BRAND_LOGO_W) / 2, 6, BRAND_LOGO_W, BRAND_LOGO_H, BRAND_LOGO);

  int bx, by, bw, bh; buttonRect(bx, by, bw, bh);
  int big = small ? 2 : 3;
  if (_pairing) {
    char pin[16]; snprintf(pin, sizeof(pin), "%06lu", (unsigned long)the_mesh.getBLEPin());
    d->nativeFill(bx, by, bw, bh, C_WHITE);
    centerText(d, W / 2, by + 6, 1 + (small ? 0 : 1), C_BG, "Bluetooth-Kopplung - PIN:");
    centerText(d, W / 2, by + bh / 2 - 4 * (small ? 3 : 4) + 4, small ? 3 : 4, C_BG, pin);
    centerText(d, W / 2, by + bh - 12, 1, C_GREY, "am Handy eingeben");
  } else if (_status && millis() < _status_until) {
    char t[32];
    uint16_t c = _status == 1 ? C_GREEN : (_status == 2 ? C_ORANGE : C_RED);
    const char* s = _status == 1 ? "Advert gesendet" : (_status == 2 ? t : "Advert-Fehler");
    if (_status == 2) snprintf(t, sizeof(t), "Bitte %d s warten", _wait_secs);
    d->nativeFill(bx, by, bw, bh, c);
    centerText(d, W / 2, by + bh / 2 - 4 * big, big, C_BG, s);
  } else {
    d->nativeFill(bx, by, bw, bh, C_GREEN);
    d->nativeRect(bx, by, bw, bh, C_WHITE);
    centerText(d, W / 2, by + bh / 2 - 4 * big - 4, big, C_BG, "Advert senden");
#if defined(UI_HAS_TOUCH)
    centerText(d, W / 2, by + bh - 14, 1, C_BG, "antippen");
#else
    centerText(d, W / 2, by + bh - 14, 1, C_BG, "Trackball 2,5 s halten");
#endif
  }
  d->endFrame();
}

void UITask::loop() {
  if (!_display) return;
  unsigned long now = millis();

#ifdef PIN_USER_BTN
  int ev = brand_btn.check();
  if (ev == BUTTON_EVENT_CLICK) toggleDisplay();
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

  if (_display->isOn()) {
    if (_pairing) _auto_off = now + BRAND_AUTO_OFF_MILLIS;
    if ((long)(now - _auto_off) > 0) {
      _display->turnOff();
    } else if (_dirty) {
      _dirty = false;
      render();
    }
  }
}
