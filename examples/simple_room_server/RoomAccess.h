// SPDX-License-Identifier: MIT
#pragma once
// Zugangsregeln des Notfall-Room-Servers (nur DISASTER_ROOM) und des Portals.
// Das Gast-Passwort ist fest der oeffentliche MeshCore-Standard "hello", damit es jeder kennt:
// mit "hello" lesen und schreiben, ohne/mit anderem Passwort nur lesen.
// Verwaltung nur ueber das Admin-Passwort (unveraendert).
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Oeffentlich bekannter MeshCore-Standard, kein Geheimnis.
#define DISASTER_ROOM_GUEST_PASSWORD "hello"

namespace disaster {

// Rollen wie in src/helpers/ClientACL.h.
static const uint8_t ROOM_PERM_GUEST = 0;       // PERM_ACL_GUEST: nur lesen
static const uint8_t ROOM_PERM_READ_WRITE = 2;  // PERM_ACL_READ_WRITE
static const uint8_t ROOM_PERM_ADMIN = 3;       // PERM_ACL_ADMIN

// Rolle fuer eine neue Anmeldung: Admin-Passwort -> Admin, Gast-Passwort -> Lesen/Schreiben,
// alles andere (leer oder falsch) -> Gast mit Leserecht. Leere Passwoerter passen nie.
inline bool roomAdminPasswordSafe(const char *admin, const char *guest) {
  return admin && admin[0] && strcmp(admin, DISASTER_ROOM_GUEST_PASSWORD) != 0 &&
         (!guest || !guest[0] || strcmp(admin, guest) != 0);
}
inline uint8_t roomLoginPermission(const char *given, const char *admin, const char *guest) {
  if (!given) return ROOM_PERM_GUEST;
  if (roomAdminPasswordSafe(admin, guest) && strcmp(given, admin) == 0) return ROOM_PERM_ADMIN;
  if (guest && guest[0] && strcmp(given, guest) == 0) return ROOM_PERM_READ_WRITE;
  return ROOM_PERM_GUEST;
}

// Both guest (0) and explicit read-only (1) are non-writing roles.
inline bool roomCanPost(uint8_t permissions) {
  return (permissions & 3) == ROOM_PERM_READ_WRITE || (permissions & 3) == ROOM_PERM_ADMIN;
}
inline bool roomPostRetryAllowed(uint8_t permissions, bool sameOwner, bool accepted,
                                 uint32_t previous, uint32_t incoming) {
  return roomCanPost(permissions) && sameOwner && accepted && previous == incoming;
}

// Beim Start (auch nach Update): Gast-Passwort auf "hello" setzen (ersetzt leere oder
// zufaellige Werte aelterer Versionen) und Gast-Lesezugriff einschalten.
// Liefert true, wenn die Einstellungen gespeichert werden muessen.
inline bool roomGuestPrefs(char *guest_password, size_t len, uint8_t &allow_read_only) {
  bool changed = false;
  if (guest_password && len > strlen(DISASTER_ROOM_GUEST_PASSWORD) &&
      strncmp(guest_password, DISASTER_ROOM_GUEST_PASSWORD, len) != 0) {
    memset(guest_password, 0, len);
    memcpy(guest_password, DISASTER_ROOM_GUEST_PASSWORD, strlen(DISASTER_ROOM_GUEST_PASSWORD));
    changed = true;
  }
  if (!allow_read_only) { allow_read_only = 1; changed = true; }
  return changed;
}

// Simple per-client posting budget. Timestamp is the receiver's monotonic
// millis() clock; callers keep one state per authenticated room client.
// The first accepted post starts a ten-minute window. Retries must not call this.
struct RoomPostBudget {
  uint32_t window_start_ms = 0;
  uint32_t last_post_ms = 0;
  uint8_t count = 0;
  bool started = false;
};
inline bool roomPostAllowed(RoomPostBudget &budget, uint32_t now) {
  // A late post still needs its full pause when the ten-minute window ends.
  // Check before resetting state so a rejected request cannot erase the pause.
  if (budget.started && budget.count && uint32_t(now - budget.last_post_ms) < 15000UL) return false;
  if (!budget.started || uint32_t(now - budget.window_start_ms) >= 600000UL) {
    budget.window_start_ms = now;
    budget.last_post_ms = 0;
    budget.count = 0;
    budget.started = true;
  }
  if (budget.count >= 6) return false;
  budget.last_post_ms = now;
  ++budget.count;
  return true;
}

// CLI-Befehle, die das oeffentliche Gast-Passwort aendern oder den Gast-Lesezugriff abschalten
// wuerden. Liefert die Antwort fuer gesperrte Befehle, sonst nullptr.
inline const char *roomCliBlocked(const char *command) {
  if (!command) return nullptr;
  if (strncmp(command, "set guest.password", 18) == 0)
    return "ERR - Gast-Passwort ist fest 'hello' (oeffentlicher Standard)";
  if (strncmp(command, "set allow.read.only", 19) == 0)
    return "ERR - Gast-Lesezugriff ist immer an";
  return nullptr;
}

}  // namespace disaster
