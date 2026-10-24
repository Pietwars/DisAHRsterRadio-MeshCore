#pragma once
#include <stdint.h>
#include <string.h>

namespace mesh {
// Implementations must durably commit the marker before returning true.
// Missing, corrupt or unreadable storage must never be reported as clean.
class TxWindowStore {
public:
  virtual ~TxWindowStore() {}
  virtual bool loadClean() = 0;
  virtual bool saveDirty() = 0;
  virtual bool saveClean() = 0;
};

// Conservative sliding hour, independent of wall clock, GPS and companion time.
// Charges are placed at completion, so they cannot expire before the RF interval.
// 61 minute buckets retain each charge for at least one hour (up to 61 minutes).
class TxWindowLimiter {
public:
  static constexpr uint32_t WINDOW_MS = 3600000;
  static constexpr uint32_t SLICE_MS = 60000;
  static constexpr unsigned SLOTS = 61;
private:
  uint64_t buckets[SLOTS] = {};
  uint64_t used_ms = 0;
  uint32_t last_seen = 0, last_rotation = 0, quiet_ms = WINDOW_MS;
  uint32_t reserve_ms = 0, failed_write_at = 0;
  unsigned cursor = 0;
  bool dirty_durable = false, in_flight = false, write_failed = false;
  bool clean_checkpoint_pending = true;
  TxWindowStore* store = nullptr;
public:
  void begin(uint32_t now, TxWindowStore* storage) {
    memset(buckets, 0, sizeof(buckets)); used_ms = 0; cursor = 0;
    last_seen = last_rotation = now;
    store = storage;
    quiet_ms = store && store->loadClean() ? 0 : WINDOW_MS;
    clean_checkpoint_pending = quiet_ms != 0;
    reserve_ms = 0; dirty_durable = in_flight = write_failed = false;
  }
  void tick(uint32_t now) {
    uint32_t elapsed = now - last_seen;
    last_seen = now;
    quiet_ms = elapsed >= quiet_ms ? 0 : quiet_ms - elapsed;
    uint32_t steps = uint32_t(now - last_rotation) / SLICE_MS;
    last_rotation += steps * SLICE_MS;
    if (steps >= SLOTS) {
      memset(buckets, 0, sizeof(buckets)); used_ms = 0;
      cursor = (cursor + steps % SLOTS) % SLOTS;
    } else {
      while (steps--) {
        cursor = (cursor + 1) % SLOTS;
        used_ms -= buckets[cursor]; buckets[cursor] = 0;
      }
    }
    if ((dirty_durable || clean_checkpoint_pending) && !in_flight && !quiet_ms && !used_ms) {
      // Even a failed clear may have reached flash. Never rely on the previous
      // dirty marker for the next send: it must be written again first.
      if (store) store->saveClean();
      dirty_durable = clean_checkpoint_pending = false;
    }
  }
  uint32_t available(uint32_t limit) const {
    return quiet_ms || used_ms >= limit ? 0 : uint32_t(limit - used_ms);
  }
  uint32_t startupSeconds() const { return (quiet_ms + 999) / 1000; }
  bool storageBlocked() const { return write_failed; }
  uint64_t used() const { return used_ms; }
  bool start(uint32_t now, uint32_t reserve, uint32_t limit) {
    tick(now);
    if (in_flight || !reserve || reserve > available(limit)) return false;
    if (!dirty_durable) {
      if (write_failed && uint32_t(now - failed_write_at) < 1000) return false;
      if (!store || !store->saveDirty()) {
        write_failed = true; failed_write_at = now; return false;
      }
      dirty_durable = true; clean_checkpoint_pending = true; write_failed = false;
    }
    reserve_ms = reserve; in_flight = true;
    return true;
  }
  void complete(uint32_t now, uint32_t elapsed) {
    if (!in_flight) return;
    tick(now); // Cannot clear the durable marker while an attempt is in flight.
    uint64_t charge = elapsed > reserve_ms ? elapsed : reserve_ms;
    buckets[cursor] += charge; used_ms += charge;
    reserve_ms = 0; in_flight = false;
  }
  // Only for an attempt cancelled BEFORE the radio driver is called.
  void cancelBeforeRadio() { reserve_ms = 0; in_flight = false; }
};
}
