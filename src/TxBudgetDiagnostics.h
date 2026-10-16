// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <limits.h>
namespace mesh {
// Observation only: does not modify the scheduler's transmit decisions or budget.
class TxBudgetDiagnostics {
  bool waiting_ = false;
  uint32_t since_ = 0, events_ = 0;
  uint64_t elapsed_ = 0;
public:
  void set(bool waiting, uint32_t now) {
    if (waiting_) elapsed_ += uint32_t(now - since_);
    if (waiting && !waiting_ && events_ != UINT32_MAX) ++events_;
    waiting_ = waiting;
    since_ = now;
  }
  bool waiting() const { return waiting_; }
  uint32_t events() const { return events_; }
  uint32_t seconds(uint32_t now) const {
    const uint64_t ms = elapsed_ + (waiting_ ? uint32_t(now - since_) : 0);
    const uint64_t sec = ms / 1000;
    return sec > UINT32_MAX ? UINT32_MAX : uint32_t(sec);
  }
};
}
