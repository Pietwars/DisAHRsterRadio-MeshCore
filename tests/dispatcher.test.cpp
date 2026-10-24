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
  uint32_t estimate=100, startDelay=0;
  Clock* clock=nullptr;
  int recvRaw(uint8_t*,int) override { return 0; }
  uint32_t getEstAirtimeFor(int n) override { estimatedLength=n;return estimate; }
  float packetScore(float,int) override { return 0; }
  bool startSendRaw(const uint8_t*,int n) override { ++starts;length=n;if(clock) clock->now=uint32_t(clock->now+startDelay);return startOK; }
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
  c.now+=1500;d.loop();assert(r.starts==starts&&d.getRemainingTxBudget()==0);
  c.now+=20000;d.loop();assert(r.starts==starts+1);
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
  // A one-millisecond airtime estimate needs two milliseconds of reserve.
  // Refill must retain sub-token elapsed time and unblock at the exact boundary.
  {
    static StaticPoolPacketManager edgePool(2);
    Clock clock; Radio radio; Dispatcher edge(radio,clock,edgePool); edge.begin();
    auto packet=edge.obtainNewPacket(); assert(packet);
    packet->header=ROUTE_TYPE_FLOOD; packet->payload_len=10;
    edge.sendPacket(packet,1); edge.loop(); assert(radio.starts==1);
    clock.now+=1000; edge.loop();
    assert(edge.getRemainingTxBudget()==0&&edge.failed==1);
    radio.estimate=1;
    packet=edge.obtainNewPacket(); assert(packet);
    packet->header=ROUTE_TYPE_FLOOD; packet->payload_len=10;
    edge.sendPacket(packet,1);
    for(int i=0;i<19;++i) { ++clock.now; edge.loop(); assert(radio.starts==1); }
    assert(edge.getRemainingTxBudget()==1&&edge.isTxBudgetWaiting());
    ++clock.now; edge.loop();
    assert(radio.starts==2&&edge.getRemainingTxBudget()==2);
    radio.complete=true; ++clock.now; edge.loop();
    assert(edge.getTotalAirTime()==1001&&edgePool.getFreeCount()==2);
  }
  // A failed radio start may still spend time in the driver. Charge that
  // uncertain interval, including clock wrap, without reporting a sent packet.
  {
    static StaticPoolPacketManager failPool(2);
    Clock clock; clock.now=0xfffffff0UL;
    Radio radio; radio.clock=&clock; radio.startDelay=200; radio.startOK=false;
    Dispatcher failed(radio,clock,failPool); failed.begin();
    auto packet=failed.obtainNewPacket(); assert(packet);
    packet->header=ROUTE_TYPE_FLOOD; packet->payload_len=10;
    failed.sendPacket(packet,0); failed.loop();
    assert(failed.failed==1&&failed.success==0&&failed.getNumSentFlood()==0);
    assert(failed.getTotalAirTime()==200&&failed.getRemainingTxBudget()==800);
    assert(failPool.getFreeCount()==2&&radio.finishes==0);
    // Repeated failures consume the same shared budget, even at top priority.
    radio.startDelay=1200;
    packet=failed.obtainNewPacket(); assert(packet);
    packet->header=ROUTE_TYPE_FLOOD; packet->payload_len=10;
    failed.sendPacket(packet,0); failed.loop();
    assert(failed.failed==2&&failed.getRemainingTxBudget()==0);
    packet=failed.obtainNewPacket(); assert(packet);
    packet->header=ROUTE_TYPE_FLOOD; packet->payload_len=10;
    failed.sendPacket(packet,0); failed.loop();
    assert(radio.starts==2&&failed.isTxBudgetWaiting());
    assert(failPool.getOutboundTotal()==1&&failPool.getFreeCount()==1);
  }
  // A failed TX longer than the complete bucket must retain its overrun as
  // debt. A fresh priority-zero packet cannot send after only the reserve wait.
  {
    static StaticPoolPacketManager debtPool(2);
    Clock clock; Radio radio; radio.clock=&clock;
    radio.startOK=false; radio.startDelay=2000;
    Dispatcher debt(radio,clock,debtPool); debt.begin();
    auto packet=debt.obtainNewPacket(); assert(packet);
    packet->header=ROUTE_TYPE_FLOOD; packet->payload_len=10;
    debt.sendPacket(packet,0); debt.loop();
    assert(debt.getTotalAirTime()==2000&&debt.getRemainingTxBudget()==0);
    radio.startOK=true; radio.startDelay=0;
    packet=debt.obtainNewPacket(); assert(packet);
    packet->header=ROUTE_TYPE_FLOOD; packet->payload_len=10;
    debt.sendPacket(packet,0);
    clock.now+=1500; debt.loop();
    assert(radio.starts==1&&debt.getRemainingTxBudget()==0&&debt.isTxBudgetWaiting());
    clock.now+=9990; debt.loop(); assert(radio.starts==1&&debt.getRemainingTxBudget()==149);
    clock.now+=10; debt.loop(); assert(radio.starts==2&&debt.getRemainingTxBudget()==150);
    radio.complete=true; clock.now+=100; debt.loop(); assert(debtPool.getFreeCount()==2);
  }
  // Missing airtime estimates fail closed, even when the initial bucket is full.
  {
    static StaticPoolPacketManager unknownPool(1);
    Clock clock; Radio radio; radio.estimate=0;
    Dispatcher unknown(radio,clock,unknownPool); unknown.begin();
    auto packet=unknown.obtainNewPacket(); assert(packet);
    packet->header=ROUTE_TYPE_FLOOD; packet->payload_len=10;
    unknown.sendPacket(packet,0); unknown.loop();
    assert(radio.starts==0&&unknown.isTxBudgetWaiting()&&unknownPool.getOutboundTotal()==1);
  }
  // An extreme delayed error must not overflow its next-refill deadline.
  {
    static StaticPoolPacketManager hugePool(2);
    Clock clock; Radio radio; radio.clock=&clock;
    radio.startOK=false; radio.startDelay=0xffffffffUL;
    Dispatcher huge(radio,clock,hugePool); huge.begin();
    auto packet=huge.obtainNewPacket(); assert(packet);
    packet->header=ROUTE_TYPE_FLOOD; packet->payload_len=10;
    huge.sendPacket(packet,0); huge.loop();
    assert(huge.getRemainingTxBudget()==0&&huge.getTotalAirTime()==0xffffffffUL);
    packet=huge.obtainNewPacket(); assert(packet);
    packet->header=ROUTE_TYPE_FLOOD; packet->payload_len=10;
    huge.sendPacket(packet,0); clock.now+=1000; huge.loop();
    assert(radio.starts==1&&huge.isTxBudgetWaiting()&&hugePool.getOutboundTotal()==1);
  }
  // Real TX timeout and budget subtraction across the embedded 32-bit clock wrap.
  {
    static StaticPoolPacketManager wrapPool(2);
    Clock clock; clock.now=0xfffffff0UL;
    Radio radio; Dispatcher wrapped(radio,clock,wrapPool); wrapped.begin();
    auto packet=wrapped.obtainNewPacket(); assert(packet);
    packet->header=ROUTE_TYPE_FLOOD; packet->payload_len=10;
    wrapped.sendPacket(packet,1); wrapped.loop(); assert(radio.starts==1);
    clock.now=uint32_t(clock.now+149); wrapped.loop();
    assert(wrapped.failed==0&&wrapPool.getFreeCount()==1);
    clock.now=uint32_t(clock.now+1); wrapped.loop();
    assert(wrapped.failed==1&&wrapped.success==0&&radio.finishes==1);
    assert(wrapped.getTotalAirTime()==150&&wrapped.getRemainingTxBudget()==850);
    assert(wrapPool.getFreeCount()==2);
  }
  puts("dispatcher: TX outcomes, exact budget boundary, clock wrap, parser and queue passed");
}
