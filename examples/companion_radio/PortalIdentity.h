#pragma once
#include <stdint.h>
#include <stdio.h>

namespace disaster {
static constexpr char PORTAL_SSID_PREFIX[] = "DisAHRsterRadio-";
static constexpr unsigned PORTAL_SSID_PREFIX_LENGTH = sizeof(PORTAL_SSID_PREFIX) - 1;

inline bool formatPortalSsid(char (&name)[33], const uint8_t (&mac)[6]) {
  name[0] = 0;
  bool nonzero = false, nonff = false;
  for (uint8_t byte : mac) { nonzero |= byte != 0; nonff |= byte != 0xff; }
  if (!nonzero || !nonff) return false;
  // Preserve all six bytes in network order. The first three bytes alone are
  // the vendor prefix, shared by many boards, not a device identifier.
  snprintf(name, sizeof(name), "%s%02X%02X%02X%02X%02X%02X", PORTAL_SSID_PREFIX,
           unsigned(mac[0]), unsigned(mac[1]), unsigned(mac[2]),
           unsigned(mac[3]), unsigned(mac[4]), unsigned(mac[5]));
  return true;
}
}
