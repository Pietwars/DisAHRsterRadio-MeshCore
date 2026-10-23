// Exercise the production dispatcher, packet serializer and real priority queue.
// Radio/clock are fake; no RF or regulatory compliance claim is made.
#include <Dispatcher.h>
#include <helpers/StaticPoolPacketManager.h>
#include <cassert>
#include <cstdio>
struct Clock : mesh::MillisecondClock {
  unsigned long now = 100;
  unsigned long getMillis() override { return now; }
};
struct Radio : mesh::Radio {
  bool complete=false, startOK=true, busy=false;
  int starts=0, finishes=0, length=0, estimatedLength=0;
  uint32_t estimate=100;
  int recvRaw(uint8_t*,int) override { return 0; }
  uint32_t getEstAirtimeFor(int n) override { estimatedLength=n;return estimate; }
  float packetScore(float,int) override { return 0; }
  bool startSendRaw(const uint8_t*,int n) override { ++starts;length=n;return startOK; }
  bool isSendComplete() override { return complete; }
  void onSendFinished() override { ++finishes; }
  bool isInRecvMode() const override { return true; }
  bool isReceiving() override { return busy; }
};
struct Dispatcher : mesh::Dispatcher {
  int success=0, failed=0;
  Dispatcher(Radio& r,Clock& c,StaticPoolPacketManager& m):mesh::Dispatcher(r,c,m){}
  mesh::DispatcherAction onRecvPacket(mesh::Packet*) override { return ACTION_RELEASE; }
  float getAirtimeBudgetFactor() const override { return 9; }
  unsigned long getDutyCycleWindowMs() const override { return 10000; }
  void logTx(mesh::Packet*,int) override { ++success; }
  void logTxFail(mesh::Packet*,int) override { ++failed; }
};
int main() {
  // Static lifetime matches the embedded pool (it has no destructor).
  static StaticPoolPacketManager pool(8);
  Clock c; Radio r; Dispatcher d(r,c,pool); d.begin();
  auto send=[&](uint8_t route=ROUTE_TYPE_FLOOD){
    auto p=d.obtainNewPacket(); assert(p);p->header=route;p->payload_len=10;
    d.sendPacket(p,1);++c.now;d.loop();
  };
  send();assert(r.starts==1&&r.length==12);
  c.now+=200;d.loop(); // TX timeout: 200ms must be charged, not counted as success.
  assert(d.getTotalAirTime()==200&&d.getRemainingTxBudget()==800);
  assert(d.failed==1&&d.success==0&&r.finishes==1&&pool.getFreeCount()==8);
  send(ROUTE_TYPE_TRANSPORT_DIRECT);assert(r.length==16&&r.estimatedLength==16);
  c.now+=100;r.complete=true;d.loop();
  assert(d.getTotalAirTime()==300&&d.success==1&&d.getNumSentDirect()==1);
  assert(d.getRemainingTxBudget()==710);
  // Failed start has no measured TX interval, no success, and releases its packet.
  r.complete=false;r.startOK=false;send();
  assert(d.getTotalAirTime()==300&&d.failed==2&&pool.getFreeCount()==8);
  // Long uncertain TX saturates budget; queued traffic cannot bypass the gate.
  r.startOK=true;send();c.now+=2000;d.loop();assert(d.getRemainingTxBudget()==0);
  int starts=r.starts;send();assert(r.starts==starts&&d.isTxBudgetWaiting());
  c.now+=1501;d.loop();assert(r.starts==starts+1);
  r.complete=true;c.now+=100;d.loop();
  // Parsing checks exact small buffers under ASan, including transport header truncation.
  mesh::Packet p;
  assert(!d.tryParsePacket(&p,nullptr,2));
  for(int n=0;n<6;++n){
    auto raw=new uint8_t[n ? n : 1]();raw[0]=ROUTE_TYPE_TRANSPORT_DIRECT;
    assert(!d.tryParsePacket(&p,raw,n));delete[] raw;
  }
  uint8_t valid[]={ROUTE_TYPE_DIRECT,0,42};
  assert(d.tryParsePacket(&p,valid,sizeof(valid))&&p.payload_len==1);
  uint8_t badPath[]={ROUTE_TYPE_DIRECT,1};assert(!d.tryParsePacket(&p,badPath,sizeof(badPath)));
  // Real queue retains priority and scheduled time; no remove/reinsert downgrade.
  static PacketQueue q(3);mesh::Packet low,high,later;
  assert(q.add(&low,5,100)&&q.add(&high,0,100)&&q.add(&later,0,500));
  assert(q.get(100)==&high&&q.get(100)==&low&&q.get(100)==nullptr&&q.get(500)==&later);
  // A profile whose reserve exceeds the complete window must remain blocked.
  r.complete=false;r.estimate=1000;starts=r.starts;send();c.now+=20000;d.loop();
  assert(r.starts==starts&&d.isTxBudgetWaiting()&&pool.getOutboundTotal()==1);
  assert(pool.peekNextOutbound(c.now)==pool.getOutboundByIdx(0));
  assert(d.millisHasNowPassed(c.now));
  c.now=5;assert(d.millisHasNowPassed(0xfffffff0UL));
  c.now=0xfffffff0UL;assert(!d.millisHasNowPassed(5));
  assert(d.futureMillis(32)==16);
  puts("dispatcher: timeout, success, failed start, budget gate, transport length, parser and queue passed");
}
