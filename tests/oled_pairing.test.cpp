#include "../examples/companion_radio/ui-new/PairingScreen.h"
#include <cassert>
#include <string>
#include <vector>

ColorVal UIColor::window_bkg=0, UIColor::title_bkg=0, UIColor::title_txt=1,
  UIColor::primary_txt=1, UIColor::secondary_txt=1, UIColor::warning_txt=1,
  UIColor::popup_bkg=0, UIColor::popup_txt=1, UIColor::corp_blue=1;

struct OledDisplay : DisplayDriver {
  struct Text { std::string value; int size; };
  int x=0, y=0, size=1;
  std::vector<Text> text;
  OledDisplay() : DisplayDriver(128, 64) {}
  bool isOn() override { return true; }
  void turnOn() override {} void turnOff() override {} void clear() override {}
  void startFrame(ColorVal) override { text.clear(); }
  void endFrame() override {}
  void setTextSize(int value) override { size=value; }
  void setColor(ColorVal) override {}
  void setCursor(int a, int b) override { x=a; y=b; }
  uint16_t getTextWidth(const char* value) override { return strlen(value)*6*size; }
  void print(const char* value) override {
    assert(x >= 0 && y >= 0);
    assert(x + getTextWidth(value) <= width() && y + 8*size <= height());
    text.push_back({value, size});
  }
  void fillRect(int, int, int, int) override {}
  void drawRect(int, int, int, int) override {}
  void drawXbm(int, int, const uint8_t*, int, int) override {}
};

int main() {
  // First/last device PIN and leading zeros must remain legible on the actual 128x64 geometry.
  for (uint32_t pin : {1u, 100000u, 654321u, 999999u}) {
    OledDisplay display;
    renderPairingScreen(display, pin);
    assert(display.text.size() == 4);
    char expected[7]; snprintf(expected, sizeof(expected), "%06lu", (unsigned long)pin);
    assert(display.text[2].value == expected && display.text[2].size == 2);
    assert(display.text.front().value == "MeshCore-App koppeln");
    assert(display.text.back().value == "Am Handy eingeben");
  }
}
