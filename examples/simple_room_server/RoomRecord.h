// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
namespace disaster {
constexpr size_t ROOM_SLOTS=32, ROOM_TEXT=152;
struct RoomRecord {uint32_t seq,stamp;uint8_t author[32];char text[ROOM_TEXT];uint32_t crc;};
static_assert(sizeof(RoomRecord)==196,"Room journal layout changed");
inline uint32_t roomChecksum(const RoomRecord &r){
 uint32_t h=2166136261u;const auto *p=reinterpret_cast<const uint8_t*>(&r);
 for(size_t i=0;i<offsetof(RoomRecord,crc);++i)h=(h^p[i])*16777619u;
 return h;
}
inline bool roomValid(const RoomRecord &r){return r.seq&&r.stamp&&memchr(r.text,0,ROOM_TEXT)&&r.text[0]&&r.crc==roomChecksum(r);}
inline size_t roomOffset(uint32_t seq){return ((seq-1)%ROOM_SLOTS)*sizeof(RoomRecord);}
}
