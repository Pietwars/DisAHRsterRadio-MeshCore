// Durable outbox transitions, including uncertain writes and reboot recovery.
#include <cassert>
#include <cstdio>
#include <cstring>
#include "../examples/companion_radio/DeliveryOutbox.h"
uint32_t fake_millis=1000;
FakeFs SPIFFS;
static constexpr uint32_t stamp=1792400000;
static const uint8_t recipient[32]={1};
static const uint8_t ack[4]={1,2,3,4};
static void reset(){SPIFFS.files.clear();fsFaults()=FsFaults();fake_millis=1000;}
static int enqueue(DeliveryOutbox &box){return box.enqueue(recipient,"Hilfe",stamp,stamp+3600);}
static void resume(DeliveryOutbox &box){box.begin();box.syncClock(stamp);}
static void substituteRecord(uint8_t *bytes,size_t size){
 if(size!=sizeof(disaster::OutboxRecord))return;
 disaster::OutboxRecord r;memcpy(&r,bytes,size);r.text[0]='X';
 r.crc=disaster::outboxChecksum(r);memcpy(bytes,&r,size);
}
int main(){
 // A partial enqueue does not publish a job or permit further disk mutations.
 reset();{DeliveryOutbox box;box.begin();fsFaults().writeLimit=10;
  assert(enqueue(box)==-2&&!box.storageOK()&&box.get(0).id==0);
  unsigned writes=fsFaults().writes;assert(enqueue(box)==-2&&!box.cancel(1));
  assert(fsFaults().writes==writes&&box.next()==-1);
  fsFaults()=FsFaults();DeliveryOutbox reboot;resume(reboot);assert(reboot.next()==-1);
 }
 // A failed attempt commit cannot consume the RAM attempt or hand off a frame.
 reset();{DeliveryOutbox box;box.begin();assert(enqueue(box)==0);
  fsFaults().writeLimit=0;assert(!box.start(0)&&box.get(0).attempts==0&&!box.ready());
  fsFaults()=FsFaults();DeliveryOutbox reboot;resume(reboot);
  assert(reboot.get(0).attempts==0&&reboot.next()==0&&reboot.start(0));
  reboot.tag(0,ack);reboot.acknowledge(ack);assert(reboot.get(0).state==disaster::DELIVERED);
 }
 // A write may succeed but its verification read fail. Stop, then recover
 // its durable attempt after reboot instead of sending again immediately.
 reset();{DeliveryOutbox box;box.begin();assert(enqueue(box)==0);
  fsFaults().failRead=true;assert(!box.start(0)&&box.get(0).attempts==0&&!box.storageOK());
  fsFaults()=FsFaults();DeliveryOutbox reboot;resume(reboot);
  assert(reboot.get(0).attempts==1&&reboot.next()==-1);
  fake_millis+=300000;assert(reboot.next()==0);
 }
 // A different checksum-valid record with the same ID/revision is not our write.
 reset();{DeliveryOutbox box;box.begin();fsFaults().afterRead=substituteRecord;
  assert(enqueue(box)==-2&&!box.storageOK()&&box.get(0).id==0);
 }
 // Cancellation and ACK states must not be shown before durable confirmation.
 for(bool cancel:{false,true}){reset();DeliveryOutbox box;box.begin();assert(enqueue(box)==0);
  assert(box.start(0));box.tag(0,ack);fsFaults().writeLimit=0;
  if(cancel)assert(!box.cancel(1));else box.acknowledge(ack);
  assert(box.get(0).state==disaster::QUEUED&&!box.storageOK());
  unsigned writes=fsFaults().writes;box.tag(0,ack);box.acknowledge(ack);
  assert(fsFaults().writes==writes);
 }
 // Never claim expiry/exhaustion in memory if the state could not be saved.
 reset();{DeliveryOutbox box;box.begin();assert(enqueue(box)==0);fake_millis+=3600000;
  fsFaults().writeLimit=0;assert(box.next()==-1&&box.get(0).state==disaster::QUEUED&&!box.ready());
 }
 // Invalid caller data must leave storage healthy and unchanged.
 reset();{DeliveryOutbox box;box.begin();char longText[102];memset(longText,'x',101);longText[101]=0;
  assert(box.enqueue(recipient,longText,stamp,stamp+10)==-2);
  assert(box.enqueue(recipient,"",stamp,stamp+10)==-2);
  assert(box.enqueue(nullptr,"x",stamp,stamp+10)==-2);
  assert(box.enqueue(recipient,"x",stamp,stamp)==-2);
  assert(box.enqueue(recipient,"x",stamp,stamp+86401)==-2);
  assert(box.storageOK()&&SPIFFS.files.empty());assert(enqueue(box)==0);
  assert(!box.start(disaster::OUTBOX_SLOTS));box.tag(disaster::OUTBOX_SLOTS,ack);
 }
 // Retry deadlines remain valid across millis wrap; at most four attempts.
 reset();{fake_millis=0xfffffff0u;DeliveryOutbox box;box.begin();assert(box.enqueue(recipient,"Hilfe",stamp,stamp+86400)==0);
  for(unsigned n=1;n<=4;++n){assert(box.next()==0&&box.start(0));
   assert(box.get(0).attempts==n);assert(box.next()==-1);
   fake_millis+=disaster::retryDelay(n);
  }
  // Exhausted records must never send a fifth attempt.
  assert(box.next()==-1&&!box.start(0));
 }
 puts("PASS: durable outbox failure, recovery, validation and retry bounds");
}
