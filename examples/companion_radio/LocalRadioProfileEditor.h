#pragma once
#include <stdint.h>
#include "DarRadioPolicy.h"

namespace disaster {
// Keyboard-only physical maintenance flow for the T-Deck WLAN display.
// This model never touches the radio, persistent state or a public transport.
class LocalRadioProfileEditor {
  bool active_ = false, confirm_ = false;
  unsigned selected_ = DAR_DEFAULT_RADIO_PROFILE;
public:
  enum Action { None, Changed, Apply };
  bool active() const { return active_; }
  bool confirming() const { return confirm_; }
  unsigned selected() const { return selected_; }
  void resetConfirmation() { confirm_ = false; }
  void cancel() { active_ = confirm_ = false; }
  Action key(uint8_t k, int current) {
    if (!active_) {
      if (k != 'p' && k != 'P') return None;
      selected_ = current >= 0 && current < int(DAR_RADIO_PROFILES) ? unsigned(current) : DAR_DEFAULT_RADIO_PROFILE;
      active_ = true; confirm_ = false; return Changed;
    }
    if (k == 8 || k == 127 || k == 27) { cancel(); return Changed; }
    if (k >= '1' && k <= '3') {
      selected_ = k - '1'; confirm_ = false; return Changed;
    }
    if (k == '\r' || k == '\n') {
      if (!confirm_) { confirm_ = true; return Changed; }
      confirm_ = false; return Apply;
    }
    confirm_ = false; return Changed;
  }
};
}
