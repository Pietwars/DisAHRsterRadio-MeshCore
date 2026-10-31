#include "../examples/companion_radio/ui-new/PairingScreen.h"
#include "../examples/companion_radio/ui-new/PortalHomeScreen.h"
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
    char expected[11]; snprintf(expected, sizeof(expected), "%06lu", (unsigned long)pin);
    assert(display.text[2].value == expected && display.text[2].size == 2);
    assert(display.text.front().value == "MeshCore-App koppeln");
    assert(display.text.back().value == "Am Handy eingeben");
  }
  OledDisplay invalid;
  renderPairingScreen(invalid, 1000000u);
  assert(invalid.text.empty());
  const uint8_t mac[6] = {0x24, 0x6f, 0x28, 0x12, 0x34, 0x56};
  char ssid[33];
  assert(disaster::formatPortalSsid(ssid, mac));
  for (uint32_t messages : {0u, 64u, UINT32_MAX}) {
    for (bool bluetooth : {false, true}) {
      OledDisplay home;
      renderPortalHomeScreen(home, ssid, "255.255.255.255", messages, bluetooth);
      assert(home.text.size()==4);
      assert(home.text[0].value + home.text[1].value == ssid);
      assert(home.text[1].value == "246F28123456");
    }
  }
  OledDisplay connected;
  renderPortalHomeScreen(connected,ssid,"192.168.4.1",64,true,true);
  assert(connected.text.back().value.find("BT:verb.")!=std::string::npos);
}
