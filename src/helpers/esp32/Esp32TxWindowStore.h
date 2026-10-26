#pragma once
#include <TxWindowLimiter.h>
#include <nvs.h>

namespace mesh {
// Separate from device settings and SD. Never erase/reinitialize NVS on errors.
class Esp32TxWindowStore : public TxWindowStore {
  nvs_handle_t handle = 0;
  bool opened = false;
  // V1 may predate intervening unguarded firmware: never trust its clean mark.
  static constexpr uint32_t CLEAN_V2 = 0x54574332u;
  static constexpr uint32_t DIRTY_V2 = 0x54574432u;
  bool open() {
    if (!opened) opened = nvs_open("dar-txwin", NVS_READWRITE, &handle) == ESP_OK;
    return opened;
  }
  bool save(uint32_t marker) {
    uint32_t readback = 0;
    return open() && nvs_set_u32(handle, "marker", marker) == ESP_OK &&
           nvs_commit(handle) == ESP_OK &&
           nvs_get_u32(handle, "marker", &readback) == ESP_OK && readback == marker;
  }
public:
  Esp32TxWindowStore() = default;
  Esp32TxWindowStore(const Esp32TxWindowStore&) = delete;
  Esp32TxWindowStore& operator=(const Esp32TxWindowStore&) = delete;
  ~Esp32TxWindowStore() { if (opened) nvs_close(handle); }
  bool loadClean() override {
    uint32_t marker = 0;
    return open() && nvs_get_u32(handle, "marker", &marker) == ESP_OK && marker == CLEAN_V2;
  }
  bool saveDirty() override { return save(DIRTY_V2); }
  bool saveClean() override { return save(CLEAN_V2); }
};
}
