// SPDX-License-Identifier: MIT
#pragma once
#include "OutboxRecord.h"
#include <SPIFFS.h>
#include "SlotFile.h"
#include <limits.h>
class DeliveryOutbox {
 disaster::OutboxRecord jobs[disaster::OUTBOX_SLOTS]{};
 uint32_t due[disaster::OUTBOX_SLOTS]{},lastId=0,wall=0,wallMs=0;
 bool clockReady=false,healthy=true;
 uint32_t (*clockSource)()=nullptr;
 // Publish a transition only after the complete record has been read back.
 // A failed write may still have reached flash: stop until reboot/recovery.
 bool persist(size_t i, disaster::OutboxRecord r){
  if(!healthy || i>=disaster::OUTBOX_SLOTS || r.revision==UINT32_MAX)return false;
  ++r.revision;r.crc=disaster::outboxChecksum(r);
  if(!disaster::outboxValid(r))return false;
  File f=disaster::openSlotFile(SPIFFS,"/dr-outbox.bin");
  bool ok=f&&disaster::seekSlot(f,disaster::outboxOffset(i,r.revision))&&f.write((const uint8_t*)&r,sizeof(r))==sizeof(r);
  if(f)f.close();
  disaster::OutboxRecord check{};f=disaster::openSlotFileRead(SPIFFS,"/dr-outbox.bin");
  ok=ok&&f&&f.seek(disaster::outboxOffset(i,r.revision))&&f.read((uint8_t*)&check,sizeof(check))==sizeof(check)&&!memcmp(&check,&r,sizeof(r));
  f.close();
  if(!ok){healthy=false;return false;}
  jobs[i]=r;return true;
 }
public:
 void begin(){
  File f=disaster::openSlotFileRead(SPIFFS,"/dr-outbox.bin");
  for(size_t i=0;i<disaster::OUTBOX_SLOTS;++i){
   for(unsigned copy=0;copy<2;++copy){disaster::OutboxRecord r{};
    if(f&&f.seek((i*2+copy)*sizeof(r))&&f.read((uint8_t*)&r,sizeof(r))==sizeof(r)&&disaster::outboxValid(r)&&r.revision>jobs[i].revision)jobs[i]=r;
   }
   if(jobs[i].id>lastId)lastId=jobs[i].id;
   due[i]=millis()+(jobs[i].attempts?300000:0);
  }f.close();
 }
 void setClockSource(uint32_t (*source)()){clockSource=source;}
 void syncClock(uint32_t stamp){uint32_t current=now();wall=clockReady&&current>stamp?current:stamp;wallMs=millis();clockReady=true;}
 uint32_t now()const{if(clockSource)return clockSource();return clockReady?wall+(uint32_t)(millis()-wallMs)/1000:0;}
 bool ready()const{return healthy&&(clockSource?now()>=1700000000u:clockReady);}
 bool storageOK()const{return healthy;}
 uint32_t latestStamp()const{uint32_t stamp=0;for(const auto &j:jobs)if(j.stamp>stamp)stamp=j.stamp;return stamp;}
 const disaster::OutboxRecord &get(size_t i)const{return jobs[i];}
 int enqueue(const uint8_t recipient[32],const char *text,uint32_t stamp,uint32_t expires,bool sos=false){
  if(!healthy)return -2;
  if(!recipient||!text||!text[0]||strlen(text)>100||stamp<1700000000u||expires<=stamp||expires-stamp>86400||lastId==UINT32_MAX)return -2;
  if(!sos){unsigned ordinary=0;for(const auto &job:jobs)if(job.state==disaster::QUEUED)++ordinary;if(ordinary>=disaster::OUTBOX_SLOTS-1)return -1;}
  int slot=-1;for(size_t i=0;i<disaster::OUTBOX_SLOTS;++i)if(!jobs[i].id){slot=i;break;}
  if(slot<0)for(size_t i=0;i<disaster::OUTBOX_SLOTS;++i)if(!disaster::pendingState(jobs[i].state)&&(!disaster::awaitingHuman(jobs[i].state)||now()>=jobs[i].expires)&&(slot<0||jobs[i].id<jobs[slot].id))slot=i;
  if(slot<0)return -1;
  auto revision=jobs[slot].revision;disaster::OutboxRecord r{};r.revision=revision;r.id=lastId+1;r.stamp=stamp;r.expires=expires;r.state=sos?disaster::SOS_QUEUED:disaster::QUEUED;memcpy(r.recipient,recipient,32);strncpy(r.text,text,100);
  if(!persist(slot,r))return -2;
  lastId=r.id;due[slot]=millis();
  syncClock(stamp);return slot;
 }
 int enqueueSos(const uint8_t recipient[32],uint32_t stamp,const char *location){
  if(!location||!ready()||stamp<1700000000u||stamp>4102358400u)return -2;
  for(const auto &r:jobs)if(disaster::awaitingHuman(r.state)&&now()<r.expires)return -3;
  char token[17],text[101];disaster::OutboxRecord r{};r.stamp=stamp;r.id=lastId+1;disaster::sosToken(r,token);
  int n=snprintf(text,sizeof(text),"SOS %s | Hilfe | %s | Antwort: SOS OK %s",token,location,token);
  if(n<0||n>=int(sizeof(text)))return -2;
  return enqueue(recipient,text,stamp,stamp+1800,true);
 }
 bool sosDue()const{
  if(!ready())return false;
  for(size_t i=0;i<disaster::OUTBOX_SLOTS;++i)if(disaster::sosState(jobs[i].state)&&disaster::pendingState(jobs[i].state)&&now()<jobs[i].expires&&int32_t(millis()-due[i])>=0)return true;
  return false;
 }
 const disaster::OutboxRecord *latestSos()const{
  const disaster::OutboxRecord *latest=nullptr;
  for(const auto &r:jobs)if(disaster::sosState(r.state)&&(!latest||r.id>latest->id))latest=&r;
  return latest;
 }
 bool acceptHuman(const uint8_t from[32],const char *text){
  if(!ready())return false;
  for(size_t i=0;i<disaster::OUTBOX_SLOTS;++i)if(now()<jobs[i].expires&&disaster::humanAccepts(jobs[i],from,text)){
   auto r=jobs[i];r.state=disaster::SOS_ACCEPTED;return persist(i,r);
  }return false;
 }
 int next(){
  if(!ready())return -1;
  for(unsigned priority=0;priority<2;++priority)for(size_t i=0;i<disaster::OUTBOX_SLOTS;++i){auto &r=jobs[i];
   if(disaster::awaitingHuman(r.state)&&now()>=r.expires){auto changed=r;changed.state=disaster::SOS_EXPIRED;if(!persist(i,changed))return -1;continue;}
   if(!disaster::pendingState(r.state)||disaster::sosState(r.state)!=(priority==0))continue;
   if(now()>=r.expires){auto changed=r;changed.state=disaster::sosState(r.state)?disaster::SOS_EXPIRED:disaster::EXPIRED;if(!persist(i,changed))return -1;continue;}
   if((int32_t)(millis()-due[i])<0)continue;
   if(r.attempts>=4){auto changed=r;changed.state=r.state==disaster::SOS_RECEIVED?disaster::SOS_UNCONFIRMED_RECEIVED:disaster::sosState(r.state)?disaster::SOS_UNCONFIRMED:disaster::UNCONFIRMED;if(!persist(i,changed))return -1;continue;}
   return i;
  }return -1;
 }
 // Attempt count must reach durable storage BEFORE handing a frame to the core.
 bool start(size_t i){
  if(!ready()||i>=disaster::OUTBOX_SLOTS)return false;
  auto r=jobs[i];if(!disaster::pendingState(r.state)||r.attempts>=4||now()>=r.expires)return false;
  ++r.attempts;if(!persist(i,r))return false;
  due[i]=millis()+(disaster::sosState(r.state)?disaster::sosRetryDelay(r.attempts):disaster::retryDelay(r.attempts));return true;
 }
 void tag(size_t i,const uint8_t *tag){
  if(!healthy||i>=disaster::OUTBOX_SLOTS||!tag)return;
  auto r=jobs[i];if(disaster::pendingState(r.state)&&r.attempts&&r.attempts<=4){memcpy(r.tags[r.attempts-1],tag,4);persist(i,r);}
 }
 void acknowledge(const uint8_t *tag){
  if(!healthy||!tag)return;
  int match=-1;
  for(size_t i=0;i<disaster::OUTBOX_SLOTS;++i)if(disaster::hasTag(jobs[i],tag)){if(match>=0)return;match=i;}
  if(match>=0&&(jobs[match].state==disaster::QUEUED||jobs[match].state==disaster::UNCONFIRMED||jobs[match].state==disaster::SOS_QUEUED||jobs[match].state==disaster::SOS_UNCONFIRMED)){auto r=jobs[match];r.state=r.state==disaster::SOS_QUEUED?disaster::SOS_RECEIVED:r.state==disaster::SOS_UNCONFIRMED?disaster::SOS_UNCONFIRMED_RECEIVED:disaster::DELIVERED;persist(match,r);}
 }
 bool cancel(uint32_t id){if(!healthy)return false;for(size_t i=0;i<disaster::OUTBOX_SLOTS;++i)if(jobs[i].id==id&&(disaster::pendingState(jobs[i].state)||disaster::awaitingHuman(jobs[i].state))){auto r=jobs[i];r.state=disaster::sosState(r.state)?disaster::SOS_CANCELLED:disaster::CANCELLED;return persist(i,r);}return false;}
 void defer(size_t i){if(healthy&&i<disaster::OUTBOX_SLOTS)due[i]=millis()+300000;}
};
