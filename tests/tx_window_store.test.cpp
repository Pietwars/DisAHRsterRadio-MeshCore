#include <helpers/esp32/Esp32TxWindowStore.h>
#include <cassert>
#include <cstdio>

int main() {
  fakeNvs::reset();
  mesh::Esp32TxWindowStore store;
  assert(!store.loadClean()); // missing marker is unknown history, never clean
  assert(store.saveClean()&&store.loadClean()&&fakeNvs::commits==1);
  fakeNvs::commitOK=false;
  assert(!store.saveDirty()&&fakeNvs::commits==2);
  assert(store.loadClean()); // staging without a successful commit is insufficient
  fakeNvs::commitOK=true;
  assert(store.saveDirty()&&!store.loadClean());
  fakeNvs::setOK=false;
  unsigned commits=fakeNvs::commits;
  assert(!store.saveClean()&&fakeNvs::commits==commits);
  fakeNvs::setOK=true; fakeNvs::readOK=false; assert(!store.loadClean());
  fakeNvs::readOK=true; fakeNvs::persisted=123; assert(!store.loadClean());

  fakeNvs::openOK=false;
  mesh::Esp32TxWindowStore unavailable;
  assert(!unavailable.loadClean()&&!unavailable.saveDirty());
  fakeNvs::openOK=true;
  assert(unavailable.saveClean());
  mesh::TxWindowLimiter window;
  window.begin(0,&unavailable);
  fakeNvs::commitOK=false;
  assert(!window.start(0,100,1000)&&window.storageBlocked());
  fakeNvs::commitOK=true;
  assert(window.start(1000,100,1000));
  mesh::Esp32TxWindowStore afterCrash;
  mesh::TxWindowLimiter reboot;
  reboot.begin(0,&afterCrash);
  assert(reboot.startupSeconds()==3600&&!reboot.start(0,1,1000));
  puts("tx_window_store: explicit NVS commit, unknown history and storage errors passed");
}
