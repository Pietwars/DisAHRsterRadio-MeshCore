#pragma once
#include <helpers/ui/DisplayDriver.h>
#include "../LocalRadioProfileEditor.h"
#include <stdio.h>

namespace ui {
// Physical keyboard maintenance page on the 320x240 T-Deck WLAN display.
inline void localRadioProfileView(DisplayDriver* d, const disaster::LocalRadioProfileEditor& editor, const char* feedback) {
  constexpr uint16_t bg=0x08a3, white=0xffff, orange=0xfd20, green=0x4ef0, grey=0x8410;
  char message[49];
  snprintf(message,sizeof(message),"%s",feedback && feedback[0] ? feedback : "1 Nah | 2 Mittel | 3 Fern");
  d->startFrame(bg);
  d->nativeText(16,12,2,white,"EU868 Funkprofil");
  d->nativeText(16,36,1,orange,message);
  for (unsigned i=0;i<disaster::DAR_RADIO_PROFILES;++i) {
    disaster::RadioPreset p; disaster::radioProfileAt(i,p);
    int y=60+int(i)*36;
    if (i==editor.selected()) d->nativeFill(12,y-4,296,28,green);
    char label[24]; snprintf(label,sizeof(label),"%s / SF%u",disaster::radioProfileName(i),unsigned(p.sf));
    d->nativeText(20,y,2,i==editor.selected()?bg:white,label);
  }
  d->nativeText(16,172,1,grey,"869,525 MHz | 250 kHz | CR 4/5");
  d->nativeText(16,202,1,white,editor.confirming()?"Enter: jetzt anwenden":"Enter: Auswahl pruefen");
  d->nativeText(16,224,1,grey,"Esc / Backspace: abbrechen");
  d->endFrame();
}
}
