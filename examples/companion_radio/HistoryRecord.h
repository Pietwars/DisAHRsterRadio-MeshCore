// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <stddef.h>
namespace disaster {
static constexpr size_t FRAME_BYTES = 176;
static constexpr size_t HISTORY_SLOTS = 96;
struct __attribute__((packed)) HistoryRecord {
  uint32_t seq;
  uint16_t len;
  uint8_t data[FRAME_BYTES];
  uint32_t crc;
};
static_assert(sizeof(HistoryRecord)==186,"History file layout must remain stable");
inline uint32_t checksum(const HistoryRecord &r) {
  uint32_t h=2166136261u;
  const uint8_t *p=reinterpret_cast<const uint8_t*>(&r);
  for(size_t i=0;i<offsetof(HistoryRecord,crc);++i)h=(h^p[i])*16777619u;
  return h;
}
inline bool valid(const HistoryRecord &r) {
  return r.seq && r.len && r.len<=FRAME_BYTES && checksum(r)==r.crc;
}
inline size_t offset(uint32_t seq) {return ((seq-1)%HISTORY_SLOTS)*sizeof(HistoryRecord);}
}
