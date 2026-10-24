// Room-Server: oeffentliches Gast-Passwort "hello" (Lesen/Schreiben), ohne Passwort nur Lesen,
// Admin-Anmeldung unveraendert.
#include <assert.h>
#include <string.h>
#include "../examples/simple_room_server/RoomAccess.h"
using namespace disaster;
int main() {
  const char *admin = "a1b2c3d4e5f6a7";
  assert(strcmp(DISASTER_ROOM_GUEST_PASSWORD, "hello") == 0);
  // Gast mit "hello" -> Lesen und Schreiben (wie Upstream)
  assert(roomLoginPermission("hello", admin, "hello") == ROOM_PERM_READ_WRITE);
  // Ohne oder mit falschem Passwort -> nur lesen
  assert(roomLoginPermission("", admin, "hello") == ROOM_PERM_GUEST);
  assert(roomLoginPermission("Hello", admin, "hello") == ROOM_PERM_GUEST);
  assert(roomLoginPermission("hell", admin, "hello") == ROOM_PERM_GUEST);
  assert(roomLoginPermission(nullptr, admin, "hello") == ROOM_PERM_GUEST);
  // Nur ein nichtoeffentliches, vom Gastkennwort verschiedenes Admin-Passwort vergibt Adminrechte.
  assert(roomLoginPermission(admin, admin, "hello") == ROOM_PERM_ADMIN);
  assert(roomLoginPermission("hello", "hello", "hello") == ROOM_PERM_READ_WRITE);
  assert(roomLoginPermission("shared", "shared", "shared") == ROOM_PERM_READ_WRITE);
  assert(roomLoginPermission("hello", "hello", nullptr) == ROOM_PERM_GUEST);
  assert(roomLoginPermission("hello", "hello", "different") == ROOM_PERM_GUEST);
  // Leere Passwoerter vergeben niemals Rechte
  assert(roomLoginPermission("", "", "") == ROOM_PERM_GUEST);
  // Start/Update: leeres oder zufaelliges Gast-Passwort wird "hello", Lesezugriff an
  char guest[16]; memset(guest, 0, sizeof(guest)); uint8_t ro = 0;
  assert(roomGuestPrefs(guest, sizeof(guest), ro) && strcmp(guest, "hello") == 0 && ro == 1);
  for (size_t i = 5; i < sizeof(guest); i++) assert(guest[i] == 0);
  assert(!roomGuestPrefs(guest, sizeof(guest), ro));  // zweiter Start: nichts zu speichern
  strcpy(guest, "0123456789abcd");
  assert(roomGuestPrefs(guest, sizeof(guest), ro) && strcmp(guest, "hello") == 0);
  for (size_t i = 5; i < sizeof(guest); i++) assert(guest[i] == 0);
  strcpy(guest, "hello2"); assert(roomGuestPrefs(guest, sizeof(guest), ro) && strcmp(guest, "hello") == 0);
  // CLI: Gast-Passwort und Gast-Lesezugriff nicht aenderbar, Admin-Befehle frei
  assert(roomCliBlocked("set guest.password geheim"));
  assert(roomCliBlocked("set allow.read.only off"));
  assert(!roomCliBlocked("get guest.password"));
  assert(!roomCliBlocked("set password neu"));
  assert(!roomCliBlocked("password neu"));
  assert(!roomCliBlocked(nullptr));
  // A fixed-window reset must not discard the pause after a late post.
  RoomPostBudget budget;
  assert(roomPostAllowed(budget,100));
  assert(roomPostAllowed(budget,599999));
  assert(!roomPostAllowed(budget,600100));
  assert(budget.window_start_ms==100&&budget.count==2&&budget.last_post_ms==599999);
  assert(roomPostAllowed(budget,614999)&&budget.count==1);
  RoomPostBudget zero;
  assert(roomPostAllowed(zero,0)&&!roomPostAllowed(zero,1));
  RoomPostBudget wrap;
  assert(roomPostAllowed(wrap,0xfffffff0u));
  assert(!roomPostAllowed(wrap,uint32_t(0xfffffff0u+14999)));
  assert(roomPostAllowed(wrap,uint32_t(0xfffffff0u+15000)));
  RoomPostBudget quota;
  for (unsigned i=0;i<6;++i) assert(roomPostAllowed(quota,i*15000));
  assert(!roomPostAllowed(quota,90000)&&quota.count==6);
  assert(roomPostAllowed(quota,600000)&&quota.count==1);
  return 0;
}
