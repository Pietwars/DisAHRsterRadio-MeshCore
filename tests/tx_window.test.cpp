#include <TxWindowLimiter.h>
#include <cassert>
#include <cstdio>
#include <vector>

struct Store : mesh::TxWindowStore {
  bool clean = true, fail = false, clearFail = false;
  int dirtyWrites = 0, cleanWrites = 0;
  bool loadClean() override { return clean; }
  bool saveDirty() override { ++dirtyWrites; if (fail) return false; clean=false; return true; }
  bool saveClean() override { ++cleanWrites; clean=true; return !clearFail; }
};
using Window = mesh::TxWindowLimiter;
static_assert(sizeof(Window)<=600,"rolling TX history must remain bounded in RAM");
int main() {
  Store store; Window w;
  w.begin(100, &store);
  assert(w.available(1000)==1000&&store.dirtyWrites==0);
  assert(w.start(100,600,1000)&&!store.clean);
  assert(!w.start(100,1,1000)); // one radio attempt at a time
  w.complete(110,10); // reserve remains charged even if the driver was fast
  assert(w.used()==600&&w.available(1000)==400);
  w.tick(1800100); // half an hour does NOT refill the sliding window
  assert(!w.start(1800100,401,1000)&&w.start(1800100,400,1000));
  w.complete(1800110,10);
  assert(w.used()==1000&&store.dirtyWrites==1); // no per-packet flash write
  w.tick(3600099); assert(!w.start(3600099,1,1000));
  w.tick(3660099); assert(w.used()==1000); // full conservative first bucket
  w.tick(3660100); assert(w.used()==400&&w.available(1000)==600);
  assert(w.available(300)==0); // reducing the quota never erases history
  w.tick(5460100); assert(w.used()==0&&store.clean&&store.cleanWrites==1);

  // Dirty/unknown boot: observe a continuous silent hour, without wall time.
  store.clean=false; w.begin(0xfffffff0u,&store);
  assert(w.startupSeconds()==3600&&!w.start(0xfffffff0u,1,1000));
  uint32_t now=uint32_t(0xfffffff0u+Window::WINDOW_MS-1);
  w.tick(now); assert(w.startupSeconds()==1&&!w.start(now,1,1000));
  ++now; assert(w.start(now,100,1000));
  // Reset between durable marker and radio completion cannot restore free quota.
  Window reboot; reboot.begin(42,&store);
  assert(reboot.startupSeconds()==3600&&!reboot.start(42,1,1000));
  w.complete(now,2000); assert(w.available(1000)==0&&w.used()==2000);

  // Store failure leaves admission closed and retries at most once per second.
  Store broken; broken.fail=true; w.begin(100,&broken);
  assert(!w.start(100,1,1000)&&w.storageBlocked());
  assert(!w.start(1099,1,1000)&&broken.dirtyWrites==1);
  assert(!w.start(1100,1,1000)&&broken.dirtyWrites==2);
  broken.fail=false; assert(w.start(2100,1,1000)); w.complete(2100,1);
  assert(!w.storageBlocked());
  broken.clearFail=true; w.tick(3660100);
  assert(broken.clean); // model a clear which persisted but returned an error
  assert(w.start(3660100,1,1000)&&!broken.clean&&broken.dirtyWrites==4);
  w.complete(3660100,1);

  // Long in-flight attempts cannot be declared clean while history ages out.
  Store longStore; w.begin(10,&longStore); assert(w.start(10,100,1000));
  w.tick(2*Window::WINDOW_MS); assert(!longStore.clean&&longStore.cleanWrites==0);
  w.complete(2*Window::WINDOW_MS,2*Window::WINDOW_MS-10);
  assert(w.available(1000)==0&&!longStore.clean);

  // Without a backend, even an hour of silence cannot authorize a transmission.
  w.begin(0,nullptr); w.tick(Window::WINDOW_MS);
  assert(!w.start(Window::WINDOW_MS,1,1000)&&w.storageBlocked());
  Store cancel; w.begin(0,&cancel); assert(w.start(0,100,1000));
  w.cancelBeforeRadio(); w.tick(1); assert(cancel.clean&&w.used()==0);
  // Deterministic adversarial traffic across multiple millis() wraps. Compare
  // every admission with an independent exact history, not the bucket layout.
  Store stress; w.begin(0,&stress);
  struct Event { uint64_t at; uint32_t charge; };
  std::vector<Event> history;
  uint64_t monotonic=0; uint32_t random=17;
  for (int i=0;i<10000;++i) {
    random=random*1664525u+1013904223u;
    monotonic+=random%3000000u;
    uint32_t tick=uint32_t(monotonic); w.tick(tick);
    uint64_t exact=0;
    for (const auto &e:history) if (monotonic-e.at<Window::WINDOW_MS) exact+=e.charge;
    assert(w.used()>=exact);
    uint32_t reserve=1+(random%1000);
    if (w.start(tick,reserve,10000)) {
      assert(exact+reserve<=10000);
      monotonic+=reserve; w.complete(uint32_t(monotonic),reserve);
      history.push_back({monotonic,reserve});
    }
    while (!history.empty()&&monotonic-history.front().at>=Window::WINDOW_MS)
      history.erase(history.begin());
  }
  puts("tx_window: sliding quota, rollover, reset, durable admission and storage failures passed");
}
