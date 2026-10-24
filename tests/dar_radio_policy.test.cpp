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
  assert(!radioProfileAt(DAR_RADIO_PROFILES,p)&&!radioProfileId(DAR_RADIO_PROFILES));
#ifdef DAR_ORIGINAL_MESHCORE_TX
  AppliedRadioConfig applied;
  assert(radioParamsValid(868.1f,125.0f,12,8)); // custom app settings, not a named preset
  assert(!radioParamsValid(std::numeric_limits<float>::quiet_NaN(),125.0f,8,8));
  assert(!radioParamsValid(869.618f,std::numeric_limits<float>::infinity(),8,8));
  assert(!radioParamsValid(149.0f,125.0f,8,8)&&!radioParamsValid(869.618f,0,8,8));
  assert(!radioParamsValid(869.618f,125.0f,4,8)&&!radioParamsValid(869.618f,125.0f,8,9));
  assert(!applied.matches(868.1f,125.0f,12,8));
  applied.remember(868.1f,125.0f,12,8); assert(applied.matches(868.1f,125.0f,12,8));
  assert(!applied.matches(868.2f,125.0f,12,8));
  applied.invalidate(); assert(!applied.matches(868.1f,125.0f,12,8)); // partial driver failure
  assert(DAR_DEFAULT_RADIO_PROFILE==3&&DAR_RADIO_PROFILES==4);
  assert(radioProfileAt(DAR_DEFAULT_RADIO_PROFILE,p));
  assert(p.freq==869618&&p.bw==62500&&p.sf==8&&p.cr==8);
  assert(withinDeBand54(p.freq,p.bw));
  assert(dePreset("narrow",p));
  assert(radioProfileIndex(p.freq,p.bw,p.sf,p.cr)==3);
  assert(radioProfileIndex(869.618f,62.5f,8,8)==3);
  assert(radioProfileIndex(869618u,62500u,8,5)<0);
  assert(radioProfileIndex(869.6181f,62.5f,8,8)<0);
  assert(radioProfileIndex(869618u,62501u,8,8)<0);
#else
  assert(DAR_DEFAULT_RADIO_PROFILE==1&&DAR_RADIO_PROFILES==3);
  assert(!dePreset("narrow",p));
#endif
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
#ifndef DAR_ORIGINAL_MESHCORE_TX
  assert(radioProfileIndex(869.618f,62.5f,8,8)<0);
#endif
  assert(radioProfileIndex(std::numeric_limits<float>::quiet_NaN(),250.0f,9,5)<0);
  assert(radioProfileIndex(869.525f,std::numeric_limits<float>::infinity(),9,5)<0);
  unsigned accepted=0;
  for (unsigned sf=0;sf<256;++sf) for (unsigned cr=0;cr<256;++cr)
    if (radioProfileIndex(869525u,250000u,uint8_t(sf),uint8_t(cr))>=0) ++accepted;
  assert(accepted==3);
  assert(darTxPowerAllowed(-9)&&darTxPowerAllowed(10));
  assert(!darTxPowerAllowed(-10)&&!darTxPowerAllowed(11)&&!darTxPowerAllowed(255));
  assert(!darTxPowerAllowed(4294967306LL)&&!darTxPowerAllowed(-4294967286LL));
  puts("dar_radio_policy: target profiles and defaults, canonical floats, band edges and power cap passed");
}
