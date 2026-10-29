#include <cassert>
#include <cstdio>
#include <cstring>
#include "../examples/companion_radio/DeliveryOutbox.h"
#include "../examples/companion_radio/SosLocation.h"
uint32_t fake_millis=1000;
FakeFs SPIFFS;
static const uint8_t peer[32]={1},other[32]={2},ack[4]={1,2,3,4};
static uint32_t clockStamp=1792400000;
static uint32_t deviceClock(){return clockStamp;}
static void reset(){SPIFFS.files.clear();fsFaults()=FsFaults();fake_millis=1000;clockStamp=1792400000;}
static void acceptText(const DeliveryOutbox &box,int slot,char out[24]){char token[17];disaster::sosToken(box.get(slot),token);snprintf(out,24,"SOS OK %s",token);}
int main(){
 char location[40];disaster::sosLocation(false,true,50.5,7.1,location,sizeof(location));assert(!strstr(location,"50.5"));
 disaster::sosLocation(true,false,50.5,7.1,location,sizeof(location));assert(strstr(location,"Kein frischer"));
 disaster::sosLocation(true,true,NAN,7.1,location,sizeof(location));assert(strstr(location,"Kein frischer"));
 disaster::sosLocation(true,true,50.5,7.1,location,sizeof(location));assert(strstr(location,"50.5000,7.1000"));
 reset();{DeliveryOutbox box;box.setClockSource(deviceClock);box.begin();
  assert(box.enqueue(peer,"normal",clockStamp,clockStamp+3600)==0);
  assert(box.enqueueSos(peer,clockStamp,"GPS nicht freigegeben")==1);
  char text[24];acceptText(box,1,text);
  assert(!box.acceptHuman(peer,text)); // no accepted attempt yet
  assert(box.enqueueSos(peer,clockStamp,"ohne GPS")==-3); // debounce
  assert(box.next()==1&&box.start(1));box.tag(1,ack);box.acknowledge(ack);
  assert(box.get(1).state==disaster::SOS_RECEIVED); // transport ACK is not human acceptance
  uint8_t collision[32];memcpy(collision,peer,32);collision[31]=1;assert(!box.acceptHuman(collision,text));
  assert(!box.acceptHuman(other,text));assert(!box.acceptHuman(peer,"SOS OK bad"));
  fake_millis+=60000;clockStamp+=60;assert(box.next()==1&&box.start(1));
  assert(box.acceptHuman(peer,text)&&box.get(1).state==disaster::SOS_ACCEPTED);
  assert(box.next()==0); // normal job resumes
  DeliveryOutbox reboot;reboot.setClockSource(deviceClock);reboot.begin();
  assert(reboot.get(1).state==disaster::SOS_ACCEPTED&&!reboot.acceptHuman(peer,text));
 }
 reset();{DeliveryOutbox box;box.setClockSource(deviceClock);box.begin();assert(box.enqueueSos(peer,clockStamp,"ohne frischen GPS-Fix")==0);
  char text[24];acceptText(box,0,text);
  for(unsigned n=1;n<=4;++n){assert(box.next()==0&&box.start(0));assert(box.get(0).attempts==n);
   const uint32_t delay=disaster::sosRetryDelay(n);fake_millis+=delay;clockStamp+=delay/1000;
  }
  assert(box.next()==-1&&box.get(0).state==disaster::SOS_UNCONFIRMED&&!box.start(0));
  assert(box.acceptHuman(peer,text)); // human reply after bounded attempts
 }
 reset();{DeliveryOutbox box;box.setClockSource(deviceClock);box.begin();assert(box.enqueueSos(peer,clockStamp,"ohne GPS")==0);assert(box.start(0));
  char text[24];acceptText(box,0,text);assert(box.cancel(1));
  assert(!box.acceptHuman(peer,text)&&box.get(0).state==disaster::SOS_CANCELLED);
 }
 reset();{DeliveryOutbox box;box.setClockSource(deviceClock);box.begin();assert(box.enqueueSos(peer,clockStamp,"ohne GPS")==0);assert(box.start(0));
  char text[24];acceptText(box,0,text);fsFaults().writeLimit=0;
  assert(!box.acceptHuman(peer,text)&&!box.storageOK()&&box.get(0).state==disaster::SOS_QUEUED);
 }
 reset();{DeliveryOutbox box;box.setClockSource(deviceClock);box.begin();assert(box.enqueueSos(peer,clockStamp,"ohne GPS")==0);
  box.syncClock(clockStamp+86400);assert(box.now()==clockStamp); // untrusted caller cannot advance authoritative clock
  clockStamp+=1800;assert(box.next()==-1&&box.get(0).state==disaster::SOS_EXPIRED);
  assert(box.enqueueSos(peer,clockStamp,"ohne GPS")>=0);
 }
 reset();{DeliveryOutbox box;box.setClockSource(deviceClock);box.begin();
  for(unsigned i=0;i<7;++i)assert(box.enqueue(peer,"chat",clockStamp,clockStamp+3600)>=0);
  assert(box.enqueue(peer,"chat",clockStamp,clockStamp+3600)==-1);
  assert(box.enqueueSos(peer,clockStamp,"ohne GPS")==7&&box.next()==7);
 }
 puts("PASS: SOS priority, durable retries, transport/human separation, peer binding, cancel and expiry");
}
