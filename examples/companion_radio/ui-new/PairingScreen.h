#pragma once
#include <stdio.h>
#include <helpers/ui/DisplayDriver.h>

// Own the complete frame: alerts and normal pages must never cover the PIN.
inline void renderPairingScreen(DisplayDriver& display, uint32_t pin) {
  char digits[7];
  snprintf(digits, sizeof(digits), "%06lu", (unsigned long)pin);
  display.setColor(UIColor::primary_txt);
  display.setTextSize(1);
  display.drawTextCentered(display.width() / 2, 2, "MeshCore-App koppeln");
  display.drawTextCentered(display.width() / 2, 16, "Bluetooth-PIN");
  display.setTextSize(2);
  display.drawTextCentered(display.width() / 2, 29, digits);
  display.setTextSize(1);
  display.drawTextCentered(display.width() / 2, 53, "Am Handy eingeben");
}
