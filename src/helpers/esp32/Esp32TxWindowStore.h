#pragma once
#include <TxWindowLimiter.h>
#include <nvs.h>

namespace mesh {
// Separate from device settings and SD. Never erase/reinitialize NVS on errors.
class Esp32TxWindowStore : public TxWindowStore {
  nvs_handle_t handle = 0;
  bool opened = false;
  static constexpr uint32_t CLEAN_V1 = 0x54574331u;
  static constexpr uint32_t DIRTY_V1 = 0x54574431u;
  bool open() {
    if (!opened) opened = nvs_open("dar-txwin", NVS_READWRITE, &handle) == ESP_OK;
    return opened;
  }
  bool save(uint32_t marker) {
    return open() && nvs_set_u32(handle, "marker", marker) == ESP_OK &&
           nvs_commit(handle) == ESP_OK;
  }
public:
  Esp32TxWindowStore() = default;
  Esp32TxWindowStore(const Esp32TxWindowStore&) = delete;
  Esp32TxWindowStore& operator=(const Esp32TxWindowStore&) = delete;
  ~Esp32TxWindowStore() { if (opened) nvs_close(handle); }
  bool loadClean() override {
    uint32_t marker = 0;
    return open() && nvs_get_u32(handle, "marker", &marker) == ESP_OK && marker == CLEAN_V1;
  }
  bool saveDirty() override { return save(DIRTY_V1); }
  bool saveClean() override { return save(CLEAN_V1); }
};
}
