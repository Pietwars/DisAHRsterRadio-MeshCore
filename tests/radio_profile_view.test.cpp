#include "FakeDisplay.h"
#include "../examples/companion_radio/ui-brand/LocalRadioProfileView.h"
#include <cassert>
#include <cstdio>
#include <string>
int main() {
  FakeDisplay display;
  disaster::LocalRadioProfileEditor editor;
  editor.key('P',1);
  for (uint8_t key : {'1','2','3'}) {
    editor.key(key,1); ui::localRadioProfileView(&display,editor,nullptr);
    assert(!display.outOfBounds);
    assert(display.allText.find("Nah / SF7")!=std::string::npos);
    assert(display.allText.find("Mittel / SF9")!=std::string::npos);
    assert(display.allText.find("Fern / SF11")!=std::string::npos);
  }
  editor.key('\r',1); ui::localRadioProfileView(&display,editor,"Bluetooth zuerst trennen");
  assert(display.allText.find("jetzt anwenden")!=std::string::npos);
  ui::localRadioProfileView(&display,editor,std::string(200,'x').c_str());
  assert(!display.outOfBounds);
  puts("radio_profile_view: three profiles, confirmation, feedback and display bounds passed");
}
