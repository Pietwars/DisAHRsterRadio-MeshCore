// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
namespace disaster {
constexpr size_t OUTBOX_SLOTS=8;
enum DeliveryState:uint8_t {QUEUED=1,DELIVERED=2,EXPIRED=3,UNCONFIRMED=4,CANCELLED=5};
struct __attribute__((packed)) OutboxRecord {
 uint32_t revision,id,stamp,expires;
 uint8_t recipient[32];char text[101];uint8_t attempts,state,tags[4][4];uint32_t crc;
};
static_assert(sizeof(OutboxRecord)==171,"Outbox disk layout changed");
inline uint32_t outboxChecksum(const OutboxRecord &r){uint32_t h=2166136261u;const auto *p=(const uint8_t*)&r;for(size_t i=0;i<offsetof(OutboxRecord,crc);++i)h=(h^p[i])*16777619u;return h;}
inline bool outboxValid(const OutboxRecord &r){return r.revision&&r.id&&r.stamp>=1700000000u&&r.expires>r.stamp&&r.expires-r.stamp<=86400&&r.attempts<=4&&r.state>=QUEUED&&r.state<=CANCELLED&&r.text[0]&&memchr(r.text,0,sizeof(r.text))&&r.crc==outboxChecksum(r);}
inline size_t outboxOffset(size_t slot,uint32_t revision){return (slot*2+(revision&1))*sizeof(OutboxRecord);}
inline uint32_t retryDelay(uint8_t attempts){return attempts==1?300000u:attempts==2?900000u:attempts==3?2700000u:300000u;}
inline bool hasTag(const OutboxRecord &r,const uint8_t tag[4]){if(!(tag[0]|tag[1]|tag[2]|tag[3]))return false;for(unsigned i=0;i<r.attempts;++i)if(!memcmp(r.tags[i],tag,4))return true;return false;}
}
