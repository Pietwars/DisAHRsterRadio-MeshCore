#include "../examples/companion_radio/PortalIdentity.h"
#include <cassert>
#include <cstring>
#include <set>
#include <string>

int main() {
  uint8_t mac[6] = {0x24, 0x6f, 0x28, 0x12, 0x34, 0x00};
  std::set<std::string> names;
  char name[33];
  for (unsigned i=0; i<256; ++i) {
    mac[5]=i;
    assert(disaster::formatPortalSsid(name, mac));
    assert(strlen(name)==disaster::PORTAL_SSID_PREFIX_LENGTH+12 && strlen(name)<=32);
    assert(names.insert(name).second);  // Same vendor prefix, distinct boards.
    char again[33];
    assert(disaster::formatPortalSsid(again, mac));
    assert(strcmp(name, again)==0);  // Reboot/reflash does not change the name.
  }
  assert(strcmp(name,"DisAHRsterRadio-246F281234FF")==0);
  mac[0]=0x30;  // Same final bytes, different vendor: retain the full hardware ID.
  assert(disaster::formatPortalSsid(name, mac) && names.insert(name).second);
  for (auto& byte : mac) byte=0;
  assert(!disaster::formatPortalSsid(name, mac) && name[0]==0);
  for (auto& byte : mac) byte=0xff;
  assert(!disaster::formatPortalSsid(name, mac) && name[0]==0);
}
