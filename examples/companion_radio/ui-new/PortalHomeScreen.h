#pragma once
#include <helpers/ui/DisplayDriver.h>
#include "../PortalIdentity.h"

inline void renderPortalHomeScreen(DisplayDriver& display, const char* ssid,
                                   const char* ip, uint32_t messages, bool bluetooth, bool connected=false) {
  display.setColor(UIColor::primary_txt);
  display.setTextSize(1);
  if (strlen(ssid) == disaster::PORTAL_SSID_PREFIX_LENGTH + 12 &&
      strncmp(ssid, disaster::PORTAL_SSID_PREFIX, disaster::PORTAL_SSID_PREFIX_LENGTH) == 0) {
    // Two lines preserve the complete SSID, including every unique suffix byte.
    display.drawTextCentered(display.width() / 2, 22, disaster::PORTAL_SSID_PREFIX);
    display.drawTextCentered(display.width() / 2, 34, ssid + disaster::PORTAL_SSID_PREFIX_LENGTH);
  } else {
    display.drawTextCentered(display.width() / 2, 22, "WLAN nicht bereit");
  }
  char line[40];
  snprintf(line, sizeof(line), "Web: %s", ip);
  display.drawTextCentered(display.width() / 2, 45, line);
  snprintf(line, sizeof(line), "MSG: %lu%s BT:%s", (unsigned long)(messages > 99999 ? 99999 : messages),
           messages > 99999 ? "+" : "", !bluetooth ? "aus" : connected ? "verb." : "bereit");
  display.drawTextCentered(display.width() / 2, 56, line);
}
