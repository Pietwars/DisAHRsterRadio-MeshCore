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
  bool complete=false, startOK=true, busy=false, configReady=true;
  bool isTxConfigurationValid() const override { return configReady; }
  int starts=0, finishes=0, length=0, estimatedLength=0, receives=0;
  uint32_t estimate=100, startDelay=0;
  Clock* clock=nullptr;
  int recvRaw(uint8_t*,int) override { ++receives; return 0; }
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
struct WindowStore : mesh::TxWindowStore {
  bool clean=true, fail=false;
  int dirtyWrites=0;
  Clock* clock=nullptr; uint32_t writeDelay=0;
  bool loadClean() override { return clean; }
  bool saveDirty() override {
    ++dirtyWrites;
    if (clock) clock->now=uint32_t(clock->now+writeDelay);
    if (fail) return false;
    clean=false; return true;
  }
  bool saveClean() override { clean=true; return true; }
};
struct WindowDispatcher : Dispatcher {
  bool profileAllowed=true;
  bool isTxProfileAllowed() const override { return profileAllowed && _radio->isTxConfigurationValid(); }
  using Dispatcher::Dispatcher;
  bool useTxWindowLimiter() const override { return true; }
};
static void testWindowDispatch() {
  // Unsupported profiles and failed hardware configuration cannot dequeue TX,
  // consume durable marker writes, or bypass the hold with top priority.
  static StaticPoolPacketManager blockedPool(2);
  Clock blockedClock; Radio blockedRadio; WindowStore blockedStore;
  WindowDispatcher blocked(blockedRadio,blockedClock,blockedPool);
  blocked.setTxWindowStore(&blockedStore); blocked.begin();
  auto blockedPacket=blocked.obtainNewPacket(); assert(blockedPacket);
  blockedPacket->header=ROUTE_TYPE_FLOOD; blockedPacket->payload_len=10;
  blocked.sendPacket(blockedPacket,0); blocked.profileAllowed=false; blocked.loop();
  assert(blocked.isTxProfileBlocked()&&blockedRadio.starts==0&&blockedPool.getOutboundTotal()==1&&blockedStore.dirtyWrites==0);
  blocked.profileAllowed=true; blockedRadio.configReady=false; blocked.loop();
  assert(blocked.isTxProfileBlocked()&&blockedRadio.starts==0&&blockedStore.dirtyWrites==0);
  blockedRadio.configReady=true; blocked.loop(); assert(blockedRadio.starts==1&&!blocked.isTxProfileBlocked());
  blockedRadio.complete=true; blockedClock.now+=100; blocked.loop(); assert(blockedPool.getFreeCount()==2);
  assert(blocked.getRemainingTxBudget()==850);
  blockedPacket=blocked.obtainNewPacket(); assert(blockedPacket);
  blockedPacket->header=ROUTE_TYPE_FLOOD; blockedPacket->payload_len=10; blocked.sendPacket(blockedPacket,0);
  blocked.profileAllowed=false; blockedClock.now+=10000; blocked.loop(); assert(blockedRadio.starts==1);
  blocked.profileAllowed=true; blocked.loop(); blockedClock.now+=100; blocked.loop();
  assert(blocked.getRemainingTxBudget()==700&&blockedPool.getFreeCount()==2); // reapproval does not reset the window

  // Refilling tokens is insufficient while rolling-hour quota is occupied.
  static StaticPoolPacketManager pool(3);
  Clock clock; Radio radio; radio.complete=true; WindowStore store;
  WindowDispatcher dispatcher(radio,clock,pool);
  dispatcher.setTxWindowStore(&store); dispatcher.begin();
  auto queue=[&](uint8_t priority=0,uint32_t delay=0){
    auto p=dispatcher.obtainNewPacket(); assert(p);
    p->header=ROUTE_TYPE_FLOOD; p->payload_len=10; dispatcher.sendPacket(p,priority,delay);
  };
  for (int i=0;i<6;++i) {
    queue(); dispatcher.loop(); clock.now+=100; dispatcher.loop();
  }
  assert(radio.starts==6&&store.dirtyWrites==1);
  assert(dispatcher.getRemainingTxBudget()==100); // 6 * 150ms reserve of 1000
  clock.now+=10000; queue(); dispatcher.loop();
  assert(radio.starts==6&&dispatcher.isTxBudgetWaiting()&&pool.getOutboundTotal()==1);
  clock.now=3660100; dispatcher.loop(); assert(radio.starts==7);
  clock.now+=100; dispatcher.loop(); assert(pool.getFreeCount()==3);

  // Dirty restart, storage errors and priority-zero traffic cannot bypass admission.
  static StaticPoolPacketManager retryPool(2);
  Clock restartClock; Radio restartRadio; WindowStore dirty; dirty.clean=false;
  WindowDispatcher restarted(restartRadio,restartClock,retryPool);
  restarted.setTxWindowStore(&dirty); restarted.begin();
  auto p=restarted.obtainNewPacket(); assert(p);
  p->header=ROUTE_TYPE_FLOOD; p->payload_len=10; restarted.sendPacket(p,0);
  restarted.loop(); assert(restartRadio.starts==0&&restartRadio.receives>0&&restarted.getTxStartupWaitSeconds()==3600);
  restartClock.now+=3599999; restarted.loop(); assert(restartRadio.starts==0);
  dirty.fail=true; ++restartClock.now; restarted.loop();
  assert(restarted.isTxStorageBlocked()&&retryPool.getOutboundTotal()==1);
  auto events=restarted.getTxBudgetWaitEvents(); restarted.loop();
  assert(restarted.getTxBudgetWaitEvents()==events); // no false wait-event inflation
  dirty.fail=false; restartClock.now+=1000; restarted.loop();
  assert(restartRadio.starts==1&&retryPool.getOutboundTotal()==0&&!restarted.isTxStorageBlocked());
  restartRadio.complete=true; restartClock.now+=100; restarted.loop();
  assert(retryPool.getFreeCount()==2);

  // Failed start and timeout both consume rolling quota and release their packet.
  static StaticPoolPacketManager errorPool(2);
  Clock errorClock; Radio errorRadio; WindowStore errorStore;
  WindowDispatcher errors(errorRadio,errorClock,errorPool);
  errors.setTxWindowStore(&errorStore); errors.begin();
  auto failed=errors.obtainNewPacket(); assert(failed);
  failed->header=ROUTE_TYPE_FLOOD; failed->payload_len=10;
  errorRadio.startOK=false; errors.sendPacket(failed,0); errors.loop();
  assert(errors.failed==1&&errors.getTotalAirTime()==0&&errors.getRemainingTxBudget()==850);
  assert(errorPool.getFreeCount()==2);
  failed=errors.obtainNewPacket(); assert(failed);
  failed->header=ROUTE_TYPE_FLOOD; failed->payload_len=10;
  errorRadio.startOK=true; errors.sendPacket(failed,0); errors.loop();
  errorClock.now+=200; errors.loop();
  assert(errors.failed==2&&errors.getRemainingTxBudget()==650&&errorPool.getFreeCount()==2);

  // A slow flash commit must not substitute a newly due, larger packet after peek.
  static StaticPoolPacketManager delayedPool(3);
  Clock delayedClock; Radio delayedRadio; WindowStore delayed;
  delayed.clock=&delayedClock; delayed.writeDelay=100;
  WindowDispatcher delayedDispatcher(delayedRadio,delayedClock,delayedPool);
  delayedDispatcher.setTxWindowStore(&delayed); delayedDispatcher.begin();
  auto first=delayedDispatcher.obtainNewPacket(); auto later=delayedDispatcher.obtainNewPacket();
  assert(first&&later); first->header=later->header=ROUTE_TYPE_FLOOD;
  first->payload_len=1; later->payload_len=100;
  delayedDispatcher.sendPacket(first,5); delayedDispatcher.sendPacket(later,0,50);
  delayedDispatcher.loop(); assert(delayedRadio.starts==1&&delayedRadio.length==3);
  assert(delayedPool.getOutboundTotal()==1);
  delayedRadio.complete=true; delayedClock.now+=100; delayedDispatcher.loop();
  assert(delayedRadio.starts==2&&delayedRadio.length==102);
  delayedClock.now+=100; delayedDispatcher.loop(); assert(delayedPool.getFreeCount()==3);
}
int main() {
  testWindowDispatch();
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
