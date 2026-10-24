#pragma once
#include <stdint.h>
#include <string.h>

// Caller serializes access between the BLE callback and application tasks.
// A token belongs to one authenticated connection and one received frame.
class BleFrameAuth {
  uint32_t generation = 0;
  uint16_t connection = 0;
  uint8_t peer[6] = {};
  bool connected = false, authenticated = false;
  void advance() { if (++generation == 0) ++generation; }
public:
  void connect(uint16_t id, const uint8_t address[6]) {
    advance(); connection=id; memcpy(peer,address,6);
    connected=true; authenticated=false;
  }
  void disconnect() { advance(); connected=authenticated=false; }
  void authenticate(const uint8_t address[6], bool success, bool mitm, uint32_t pin) {
    if (!connected || memcmp(peer,address,6)) return;
    authenticated=success && mitm && pin>=100000 && pin<=999999;
  }
  uint32_t capture(uint16_t id) const {
    return connected && authenticated && connection==id ? generation : 0;
  }
  bool allows(uint32_t token) const {
    return token && token==generation && connected && authenticated;
  }
};
