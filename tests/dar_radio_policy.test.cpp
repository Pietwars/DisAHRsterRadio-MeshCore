#include "../examples/companion_radio/DarRadioPolicy.h"
#include <cassert>
#include <cstdio>
#include <limits>
#include <initializer_list>
using namespace disaster;
int main() {
  const char* ids[] = {"nah", "mittel", "fern"};
  const uint8_t spread[] = {7,9,11};
  for (unsigned i=0;i<3;++i) {
    RadioPreset p; assert(dePreset(ids[i],p));
    assert(p.freq==869525&&p.bw==250000&&p.sf==spread[i]&&p.cr==5);
    assert(radioProfileIndex(p.freq,p.bw,p.sf,p.cr)==int(i));
    assert(radioProfileIndex(869.525f,250.0f,p.sf,p.cr)==int(i));
    assert(radioProfileIndex(float(p.freq)/1000.0f,float(p.bw)/1000.0f,p.sf,p.cr)==int(i));
  }
  RadioPreset p;
  assert(!radioProfileAt(3,p)&&!radioProfileId(3));
  for (const char* id : {"", "de-narrow", "ahr-legacy", "eu-legacy", "nah ", "Nah"})
    assert(!dePreset(id,p));
  assert(!dePreset(nullptr,p));
  assert(withinDeBand54(869525,250000));
  assert(!withinDeBand54(869524,250000)&&!withinDeBand54(869526,250000));
  assert(!withinDeBand54(869525,250001)&&!withinDeBand54(869525,0));
  assert(!withinDeBand54(0xffffffffu,0xffffffffu));
  assert(radioProfileIndex(868100u,125000u,9,5)<0);
  assert(radioProfileIndex(869525u,125000u,9,5)<0);
  assert(radioProfileIndex(869.5251f,250.0f,9,5)<0);
  assert(radioProfileIndex(869.618f,62.5f,8,8)<0);
  assert(radioProfileIndex(std::numeric_limits<float>::quiet_NaN(),250.0f,9,5)<0);
  assert(radioProfileIndex(869.525f,std::numeric_limits<float>::infinity(),9,5)<0);
  unsigned accepted=0;
  for (unsigned sf=0;sf<256;++sf) for (unsigned cr=0;cr<256;++cr)
    if (radioProfileIndex(869525u,250000u,uint8_t(sf),uint8_t(cr))>=0) ++accepted;
  assert(accepted==3);
  assert(darTxPowerAllowed(-9)&&darTxPowerAllowed(10));
  assert(!darTxPowerAllowed(-10)&&!darTxPowerAllowed(11)&&!darTxPowerAllowed(255));
  puts("dar_radio_policy: exactly three profiles, canonical floats, band edges and power cap passed");
}
