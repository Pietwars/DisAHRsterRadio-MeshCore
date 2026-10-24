#include "../examples/companion_radio/LocalRadioProfileEditor.h"
#include <cassert>
#include <cstdio>
using Editor = disaster::LocalRadioProfileEditor;
int main() {
  Editor e;
  assert(e.key('1',1)==Editor::None&&!e.active());
  assert(e.key('\r',1)==Editor::None);
  assert(e.key('p',2)==Editor::Changed&&e.selected()==2&&!e.confirming());
  assert(e.key('1',2)==Editor::Changed&&e.selected()==0);
  assert(e.key('\r',2)==Editor::Changed&&e.confirming());
  e.key('3',2); assert(e.selected()==2&&!e.confirming());
  assert(e.key('\r',2)==Editor::Changed&&e.key('\r',2)==Editor::Apply&&!e.confirming());
  e.key('\r',2); e.resetConfirmation(); // waking the display invalidates stale confirmation
  assert(e.key('\r',2)==Editor::Changed);
  e.key(27,2); assert(!e.active());
  e.key('P',-1); assert(e.selected()==1); // unsupported old prefs select Mittel for review only
  e.key('\r',-1); e.cancel();
  assert(e.key('\r',-1)==Editor::None&&!e.active()); // pairing/cancellation cannot apply
  e.key('P',99); e.key('\r',99); e.key('4',99);
  assert(!e.confirming()&&e.selected()==1&&e.key('\r',99)==Editor::Changed);
  e.key(8,99); assert(!e.active());
  puts("radio_profile_editor: three choices, physical opening, confirmation, cancellation and wake passed");
}
