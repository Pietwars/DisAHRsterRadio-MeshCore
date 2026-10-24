// Standalone TX policy against the original MeshCore baseline; fake RF only.
#include <Dispatcher.h>
#include <helpers/StaticPoolPacketManager.h>
#include <cassert>
#include <cstdio>
struct Clock : mesh::MillisecondClock {
  unsigned long now=100;
  unsigned long getMillis() override { return now; }
};
struct Radio : mesh::Radio {
  bool complete=false, startOK=true;
  int starts=0, finishes=0;
  int recvRaw(uint8_t*,int) override { return 0; }
  uint32_t getEstAirtimeFor(int) override { return 1000; }
  float packetScore(float,int) override { return 0; }
  bool startSendRaw(const uint8_t*,int) override { ++starts; return startOK; }
  bool isSendComplete() override { return complete; }
  void onSendFinished() override { ++finishes; }
  bool isInRecvMode() const override { return true; }
  bool isReceiving() override { return false; }
};
struct Store : mesh::TxWindowStore {
  int accesses=0;
  bool loadClean() override { ++accesses; return false; }
  bool saveDirty() override { ++accesses; return false; }
  bool saveClean() override { ++accesses; return false; }
};
struct D : mesh::Dispatcher {
  float factor=1.0f;
  bool ready=true;
  D(Radio& r,Clock& c,StaticPoolPacketManager& p):mesh::Dispatcher(r,c,p) {}
  mesh::DispatcherAction onRecvPacket(mesh::Packet*) override { return ACTION_RELEASE; }
  float getAirtimeBudgetFactor() const override { return factor; }
  unsigned long getDutyCycleWindowMs() const override { return 10000; }
  bool isTxProfileAllowed() const override { return ready; }
};
static void queue(D& d) {
  auto p=d.obtainNewPacket(); assert(p);
  p->header=ROUTE_TYPE_FLOOD; p->payload_len=10; d.sendPacket(p,0);
}
int main() {
  static StaticPoolPacketManager pool(3), restartPool(2);
  Clock clock; Radio radio; Store store;
  D d(radio,clock,pool); d.factor=9; d.setTxWindowStore(&store); d.begin();
  assert(d.getRemainingTxBudget()==1000&&d.getTxStartupWaitSeconds()==0&&store.accesses==0);
  queue(d); d.ready=false; d.loop(); assert(radio.starts==0&&pool.getOutboundTotal()==1);
  d.ready=true; d.loop(); assert(radio.starts==1); // Original half-airtime admission, no DAR 150% reserve.
  clock.now+=200; radio.complete=true; d.loop();
  assert(d.getTotalAirTime()==200&&d.getRemainingTxBudget()==800&&pool.getFreeCount()==3);
  d.factor=1; clock.now+=1000; radio.startOK=false; queue(d); d.loop();
  assert(radio.starts==2&&d.getRemainingTxBudget()==1300&&d.getTotalAirTime()==200);
  radio.startOK=true; radio.complete=false; queue(d); d.loop();
  clock.now+=2000; d.loop(); // Original timeout handling, distinct from DAR failure/debt accounting.
  assert(radio.starts==3&&radio.finishes==2&&d.getTotalAirTime()==200&&pool.getFreeCount()==3);
  Clock rebootClock; Radio rebootRadio; D reboot(rebootRadio,rebootClock,restartPool);
  reboot.setTxWindowStore(&store); reboot.begin();
  assert(reboot.getRemainingTxBudget()==5000&&reboot.getTxStartupWaitSeconds()==0&&store.accesses==0);
  puts("original_meshcore_tx: configurable budget, upstream admission/outcomes and no DAR restart store passed");
}
