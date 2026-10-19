#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "../examples/companion_radio/ui-brand/TDeckQuickPage.h"

static unsigned sends = 0;
static char sentTarget[14], sentText[101];
static const char* result = "Vorgemerkt";
static const char* recipient = "d010203040506";
bool deviceQuickTarget(unsigned i, char* id, size_t id_size, char* name, size_t name_size) {
  if (i > 1) return false;
  snprintf(id, id_size, "%s", i ? recipient : "c0");
  snprintf(name, name_size, "%s", i ? "Kontakt" : "Oeffentlich"); return true;
}
bool deviceQuickSend(const char* id, const char* text, uint32_t, char* status, size_t size) {
  ++sends; snprintf(sentTarget, sizeof(sentTarget), "%s", id);
  snprintf(sentText, sizeof(sentText), "%s", text);
  snprintf(status, size, "%s", result); return true;
}
void deviceQuickStatus(char* status, size_t size) { snprintf(status, size, "%s", result); }

struct Screen : DisplayDriver {
  Screen() : DisplayDriver(320, 240) { }
  bool isOn() override { return true; }
  void turnOn() override { } void turnOff() override { } void clear() override { }
  void startFrame(ColorVal) override { }
  void setTextSize(int) override { } void setColor(ColorVal) override { }
  void setCursor(int, int) override { } void print(const char*) override { }
  void fillRect(int x, int y, int w, int h) override { assert(x >= 0 && y >= 0 && x+w <= 320 && y+h <= 240); }
  void drawRect(int x, int y, int w, int h) override { fillRect(x,y,w,h); }
  void drawXbm(int, int, const uint8_t*, int, int) override { }
  uint16_t getTextWidth(const char* s) override { return strlen(s) * 6; }
  void nativeText(int x, int y, int size, uint16_t, const char* s) override {
    assert(x >= 0 && y >= 0 && y + size * 8 <= 240);
    assert(x + int(strlen(s)) * 6 * size <= 320);
  }
  void endFrame() override { }
};

int main() {
  SensorManager sensors; Screen screen; const uint32_t stamp = 1792400000;
  TDeckQuickPage p; p.begin(&sensors); p.tick(0); p.render(&screen);
  // ALT+C moves focus; typing/ENTER must not transmit until Send is explicitly selected.
  p.key('1', stamp); p.key('4', stamp); p.key('\r', stamp); // edit -> place
  assert(sends == 0);
  p.key(12, stamp); p.key(12, stamp); p.key('\r', stamp); // target -> send -> attempt
  assert(sends == 0); // no device GPS fix
  sensors.gps.fresh = true;
  p.key('\r', stamp); assert(sends == 1);
  assert(strcmp(sentTarget, "c0") == 0 && strstr(sentText, "50.5426,7.1761"));
  p.key('\r', stamp); assert(sends == 1); // repeated key cannot duplicate an accepted submission
  result = "Antwort unklar; nicht erneut senden"; assert(p.tick(1000));
  p.key('\r', stamp); assert(sends == 1);
  p.render(&screen);

  TDeckQuickPage contact; contact.begin(&sensors);
  contact.key(12, stamp); contact.key('A', stamp); // edit
  contact.key(12, stamp); contact.key(12, stamp); contact.key(' ', stamp); // choose contact
  recipient = "dffffffffffff"; // refreshed contact list must not retarget an existing draft
  contact.key(12, stamp); contact.key('\r', stamp);
  assert(sends == 2 && strcmp(sentTarget, "d010203040506") == 0);

  TDeckQuickPage position; position.begin(&sensors);
  for (int i = 0; i < 5; ++i) position.key(12, stamp);
  sensors.gps.fresh = false; position.key('\r', stamp); assert(sends == 2);
  sensors.gps.fresh = true; sensors.enabled = false;
  position.key('\r', stamp); assert(sends == 2);
  sensors.enabled = true; position.key('\r', stamp); assert(sends == 3);
  assert(strcmp(sentText, "!DP1|50.5426,7.1761|1792400000") == 0);
  position.render(&screen);
  TDeckQuickPage keys; keys.begin(&sensors);
  for (int i = 0; i < 4; ++i) assert(!keys.key(12, stamp));
  keys.key(' ', stamp); assert(sends == 3); // space cannot activate the send button
  for (int i = 0; i < 2; ++i) keys.key(12, stamp);
  assert(keys.key('\r', stamp)); // reachable Home action on the original keyboard
  TDeckQuickPage clear; clear.begin(&sensors);
  for (int i = 0; i < 7; ++i) clear.key(12, stamp);
  clear.key('\r', stamp); // Clear -> Text; Backspace remains an edit, not Home
  assert(!clear.key(8, stamp));
  clear.key('X', stamp); clear.render(&screen);
  clear.key('\r', stamp); assert(clear.key(8, stamp)); // Backspace outside text returns Home
  puts("quick_page.test.cpp: keyboard, recipients, GPS and explicit sending passed");
}
