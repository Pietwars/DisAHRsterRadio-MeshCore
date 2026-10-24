#!/usr/bin/env python3
"""Static regression checks for DAR 0.9.6 security and TX policy.

These checks intentionally do NOT claim that the firmware builds, that the
open-WLAN login is secure, or that regional duty-cycle rules are satisfied.
Run from repository root: python3 tools/check_096_static.py
"""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
portal = (ROOT / "examples/companion_radio/DisasterPortal.h").read_text(encoding="utf-8")
dispatcher = (ROOT / "src/Dispatcher.cpp").read_text(encoding="utf-8")
mesh = (ROOT / "examples/companion_radio/MyMesh.cpp").read_text(encoding="utf-8")
policy = (ROOT / "examples/companion_radio/DarBuild.h").read_text(encoding="utf-8")
page = (ROOT / "web/index.html").read_text(encoding="utf-8")
access = (ROOT / "examples/companion_radio/OpenAccess.h").read_text(encoding="utf-8")
main = (ROOT / "examples/companion_radio/main.cpp").read_text(encoding="utf-8")
ui_task = (ROOT / "examples/companion_radio/ui-brand/UITask.cpp").read_text(encoding="utf-8")
ui_page = (ROOT / "examples/companion_radio/ui-brand/StandaloneUI.cpp").read_text(encoding="utf-8")

def check(ok: bool, message: str) -> None:
    if not ok:
        raise AssertionError(message)
    print("OK:", message)

def route_body(name: str) -> str:
    # Capture only up to the next registered route. This is a source-level
    # approximation; it is not a C++ parser.
    marker = 'http.on("/api/' + name + '",HTTP_POST'
    start = portal.find(marker)
    check(start >= 0, f"route /api/{name} exists")
    end = portal.find('http.on("', start + len(marker))
    return portal[start:end if end >= 0 else len(portal)]

for route in ("name", "channel", "radio", "history-wipe"):
    check("requireAdmin()" in route_body(route), f"/api/{route} is admin-gated")

check("allowClientSend()" in route_body("send"), "message sending remains rate-limited")
check("uint32_t globalWindowStart=0, globalLastMs=0;" in portal and
      "uint8_t globalSendCount=0;" in portal,
      "portal has a shared send budget independent of client identifiers")
check("globalSendCount>=24" in portal and "now-globalLastMs)<2000" in portal,
      "portal enforces aggregate ten-minute cap and minimum spacing")
check("globalLastMs=now;++globalSendCount;" in portal,
      "accepted sends consume the shared budget")
check("allowClientSend()" in route_body("outbox") and "allowClientSend()" in route_body("advert"),
      "outbox and advert endpoints also use the shared budget")
check("loadAdmin()" not in portal and "hashPin(" not in portal,
      "unused PIN provisioning is absent from open portal")
check('"dar-admin"' not in portal and "adminDigest" not in portal,
      "open portal does not create or load PIN hashes")
check('sessionToken.c_str()' not in portal, "unused HTTP session-token validation is removed")
check('http.send(403,"text/plain","Einstellungen nur ueber sicheren lokalen Kanal");' in portal,
      "privileged HTTP settings fail closed")
check('http.send(503,"text/plain","Admin-Anmeldung ueber offenes WLAN deaktiviert");' in portal,
      "open-WLAN PIN login is disabled")
check("adminSession.valid(" not in portal, "open HTTP portal has no session authentication bypass")
check("adminSession.issue(" not in portal, "open HTTP portal never issues bearer tokens")
check("#define DAR_MESH_POLICY 1" in policy, "DAR mesh policy is defined")
check("return 9.0f" in mesh, "DAR airtime factor is fixed at 9")
check("required_airtime" in dispatcher, "scheduler checks TX budget")
check("TX_BUDGET_AIRTIME_MARGIN_NUM" in dispatcher, "TX reserve margin is present")

# All outgoing mesh routes must converge on the budget-aware dispatcher.
mesh_core = (ROOT / "src/Mesh.cpp").read_text(encoding="utf-8")
for signature, next_signature in (
    ("void Mesh::sendFlood(Packet* packet, uint32_t", "void Mesh::sendFlood(Packet* packet, uint16_t*"),
    ("void Mesh::sendFlood(Packet* packet, uint16_t*", "void Mesh::sendDirect(Packet*"),
    ("void Mesh::sendDirect(Packet*", "void Mesh::sendZeroHop(Packet* packet, uint32_t"),
    ("void Mesh::sendZeroHop(Packet* packet, uint32_t", "void Mesh::sendZeroHop(Packet* packet, uint16_t*"),
):
    route = mesh_core.split(signature, 1)[1].split(next_signature, 1)[0]
    check("sendPacket(packet," in route, f"{signature} uses shared TX scheduler")
last_zero_hop = mesh_core.split("void Mesh::sendZeroHop(Packet* packet, uint16_t*", 1)[1]
check("sendPacket(packet," in last_zero_hop,
      "transport zero-hop packets use shared TX scheduler")
check("sendPacket(a1," in mesh_core and "sendPacket(a2," in mesh_core,
      "multipart forwarding also enters shared TX scheduler")

# Budget checks must precede dequeue and the physical radio TX call.
check_send = dispatcher.split("void Dispatcher::checkSend()", 1)[1].split("void Dispatcher::", 1)[0]
check("peekNextOutbound(" in check_send and "getNextOutbound(" in check_send and
      check_send.index("peekNextOutbound(") < check_send.index("getNextOutbound("),
      "TX candidate is inspected before dequeue")
check("tx_budget_ms < required_airtime" in check_send and
      check_send.index("tx_budget_ms < required_airtime") < check_send.index("getNextOutbound("),
      "airtime reserve is enforced before dequeue")
check("reserve64 > max_budget" in check_send and
      check_send.index("reserve64 > max_budget") < check_send.index("getNextOutbound("),
      "oversized TX packets cannot bypass the budget")
check("startSendRaw(raw, len)" in check_send and
      check_send.index("getNextOutbound(") < check_send.index("startSendRaw(raw, len)"),
      "physical radio send happens only after budget checks")
check("accountTxAirtime();" in dispatcher,
      "dispatcher charges elapsed TX time to shared airtime budget")

check("tx_window.start(" in check_send and
      check_send.index("tx_window.start(") < check_send.index("getNextOutbound("),
      "durable rolling-window admission precedes dequeue and radio TX")
check("peekNextOutbound(candidate_time)" in check_send and "getNextOutbound(candidate_time)" in check_send,
      "flash commit cannot substitute a newly due packet after reserve inspection")
check("tx_window.complete(" in dispatcher and "tx_window.tick(" in dispatcher,
      "TX outcomes charge the rolling window and monotonic time advances in the loop")
check("the_mesh.setTxWindowStore(&tx_window_store);" in main,
      "DAR ESP32 startup attaches persistent TX activity storage")
tx_store = (ROOT / "src/helpers/esp32/Esp32TxWindowStore.h").read_text()
check("nvs_commit(handle) == ESP_OK" in tx_store and "nvs_erase" not in tx_store,
      "TX activity marker requires explicit successful NVS commit without erase-on-error")

# SD configuration is inert at boot; only a physical local action imports it.
standalone = (ROOT / "examples/companion_radio/StandaloneFrontend.cpp").read_text()
loader = standalone.split("void loadConfig()", 1)[1].split("bool saveConfig(", 1)[0]
check("pending_config = c;" in loader and "nvs.put" not in loader and "localSetName(" not in loader,
      "boot SD loader stages settings without changing device preferences")
check("config_name" not in standalone and "readConfigName" not in standalone,
      "no deferred automatic SD name application remains")
check("api->importConfig(" in ui_page and "api->configPreview(c)" in ui_page,
      "local SD import has a device preview and explicit confirmation")

# Public transports must never become privileged merely because a BLE PIN is configured.
permission = access.split("inline bool allowCompanionCommand(", 1)[1].split("\n}", 1)[0]
check("return !publicTransport || publicCompanionCommand(cmd);" in permission,
      "BLE and open WLAN have the same explicit public-command allowlist")
check("if (bluetooth && blePin != 0) return true;" not in permission,
      "configured BLE PIN cannot grant privileged companion commands")
check("validBlePin(uint32_t pin)" in access and "effectiveBlePin(uint32_t saved)" in access,
      "six-digit BLE pairing PIN handling remains present")
# Guard the public allowlist itself: privileged commands must never leak into BLE/guest WLAN.
public_body = access.split("inline bool publicCompanionCommand(", 1)[1].split("default: return false;", 1)[0]
public_cases = {int(value) for value in re.findall(r"case\s+(\d+)\s*:", public_body)}
privileged_cases = {8, 11, 19, 24, 25, 32, 36, 51}
check(len(public_cases) >= 20, "public command allowlist is actually parsed (non-vacuous)")
check(not (public_cases & privileged_cases),
      "public BLE/WLAN allowlist excludes name/radio changes, key management, reboot and reset")
check("default: return false;" in access.split("inline bool publicCompanionCommand(", 1)[1].split("inline bool allowCompanionCommand(", 1)[0],
      "unknown companion commands remain denied by default")
check("allowCompanionCommand(cmd_frame[0]" in mesh,
      "companion command dispatcher still enforces access policy")

check("esp_random() % 900000UL" in main, "ESP32 provisions a per-device BLE PIN")
check("the_mesh.savePrefs();" in main, "BLE PIN is persisted in device preferences")
check('Serial.printf("[BLE] Bluetooth-PIN %06lu' not in main,
      "BLE PIN is not printed to serial logs")
check("disaster::needsBlePinProvisioning(pin)" in main,
      "ESP32 migrates invalid and shared test PINs")

# The existing T-Deck pairing overlay already displays the actual active PIN.
# Keep it scoped to pairing rather than exposing a permanent PIN in Device Info.
check("bool pairing = ble_pairing_active();" in ui_task,
      "T-Deck observes active Bluetooth pairing state")
check('snprintf(pin, sizeof(pin), "%06lu", (unsigned long)the_mesh.getBLEPin())' in ui_task,
      "pairing overlay displays the actual configured BLE PIN")
check("if (!_pairing) { _sui.render(d); return; }" in ui_task,
      "PIN overlay is not shown during ordinary standalone navigation")
check("if (pairing) wake();" in ui_task,
      "display wakes for Bluetooth pairing")
device_info = ui_page.split("void UI::drawDeviceInfo(", 1)[1].split("void UI::render(", 1)[0]
check("getBLEPin()" not in device_info,
      "Device Info does not reveal Bluetooth PIN digits")

# Room-server guest must never inherit moderator/admin rights.
room_access = (ROOT / "examples/simple_room_server/RoomAccess.h").read_text(encoding="utf-8")
check('ROOM_PERM_READ_WRITE = 2' in room_access and 'ROOM_PERM_ADMIN = 3' in room_access,
      "room guest writer and administrator permissions remain distinct")
check('strcmp(given, admin) == 0) return ROOM_PERM_ADMIN;' in room_access and
      'strcmp(given, guest) == 0) return ROOM_PERM_READ_WRITE;' in room_access,
      "room login grants admin rights only for matching admin password")
check('return ROOM_PERM_GUEST;' in room_access,
      "unknown room credentials default to read-only guest")

# Room moderation must be restricted at the actual packet dispatch point,
# not just in the login helper.
room_mesh = (ROOT / "examples/simple_room_server/MyMesh.cpp").read_text(encoding="utf-8")
room_rx = room_mesh.split("void MyMesh::onPeerDataRecv(", 1)[1].split("bool MyMesh::onPeerPathRecv(", 1)[0]
room_commands = room_rx.split("if (flags == TXT_TYPE_CLI_DATA || flags == TXT_TYPE_CLI_COMMAND)", 1)[1].split("} else { // TXT_TYPE_PLAIN", 1)[0]
check("if (client->isAdmin())" in room_commands and "handleCommand(sender_timestamp" in room_commands,
      "room moderation commands require administrator role at packet dispatch")
check("send_ack = false; // and no ACK" in room_commands,
      "non-admin room moderation commands are rejected without acknowledgment")
room_posts = room_rx.split("} else { // TXT_TYPE_PLAIN", 1)[1].split("uint32_t delay_millis;", 1)[0]
check("(client->permissions & PERM_ACL_ROLE_MASK) == PERM_ACL_GUEST" in room_posts and
      "addPost(client, (const char *)&data[5]);" in room_posts,
      "read-only room guests cannot publish posts")
check("if (!is_retry)" in room_posts,
      "room post retries do not duplicate stored messages")

room_header = (ROOT / "examples/simple_room_server/MyMesh.h").read_text(encoding="utf-8")
check("room_budget_owners[MAX_CLIENTS]" in room_header and
      "room_budget_owned[MAX_CLIENTS]" in room_header,
      "room post budget tracks the owner of each recycled ACL slot")
check("memcmp(room_budget_owners[i].pub_key, client->id.pub_key, PUB_KEY_SIZE) != 0" in room_posts and
      "room_post_budgets[i] = disaster::RoomPostBudget{};" in room_posts,
      "recycled ACL slots reset the previous client's room budget")
check("room_post_budgets[MAX_CLIENTS]" in room_header,
      "room post budgets are bounded by authenticated ACL slots")
check("roomPostAllowed(room_post_budgets[i], millis())" in room_posts and
      "addPost(client, (const char *)&data[5]);" in room_posts,
      "disaster room checks the posting budget before storing messages")
check("if (!is_retry)" in room_posts and "#ifdef DISASTER_ROOM" in room_posts,
      "room budget does not charge message retries and is scoped to disaster room")
check("budget.count >= 6" in room_access and "600000UL" in room_access and "15000UL" in room_access,
      "room post policy limits each client to six posts per ten minutes with 15-second spacing")

check("send_ack = post_accepted;" in room_posts,
      "room post ACK is sent only after accepted storage")
check("room_last_accepted_timestamp[i] == sender_timestamp" in room_posts and
      "room_has_accepted_post[i] = true;" in room_posts,
      "room post retries are acknowledged only if original post was accepted")
check("room_has_accepted_post[i] = false;" in room_posts,
      "recycled ACL slots cannot acknowledge the previous owner's posts")

# Local PIN editor regression checks (source-level only; hardware pairing remains unverified).
frontend = (ROOT / "examples/companion_radio/StandaloneFrontend.cpp").read_text(encoding="utf-8")
check("bool changeBlePin(uint32_t pin" in frontend,
      "standalone frontend implements local PIN persistence")
check("disaster::validBlePin(pin)" in frontend and "bluetooth_interface.isConnected()" in frontend,
      "PIN updates validate six digits and refuse active Bluetooth sessions")
check("getNodePrefs()->ble_pin = pin;" in frontend and "the_mesh.savePrefs();" in frontend,
      "local PIN change saves to persistent node preferences")
check("Neustart erforderlich" in frontend,
      "PIN change instructs restart before use")
check("case BLE_PIN_EDIT:" in ui_page and "api->changeBlePin(" in ui_page,
      "T-Deck PIN editor is wired to the local API")
check("strcmp(blePinFirst, blePinDraft)" in ui_page,
      "PIN editor requires matching confirmation")
check('memset(blePinFirst, 0, sizeof(blePinFirst));' in ui_page,
      "PIN editor clears confirmation buffer on cancel or mismatch")

check("case BLE_PIN_EDIT:" in ui_page and re.search(r"case BLE_PIN_EDIT:\s*return true;\s*// PIN entry accepts keyboard only", ui_page) is not None,
      "PIN editor ignores unrelated touch actions")
check("Never retain a rejected PIN or its confirmation in the UI buffers." in ui_page and
      re.search(r"else \{\s*// Never retain a rejected PIN.*?memset\(blePinDraft, 0, sizeof\(blePinDraft\)\);.*?memset\(blePinFirst, 0, sizeof\(blePinFirst\)\);.*?blePinConfirm = false;", ui_page, re.S) is not None,
      "rejected PIN updates clear both local input buffers")
check("back(); // Clear both PIN buffers before any tab navigation." in ui_page,
      "switching tabs cancels and clears PIN entry")
# Chat and board navigation must not produce invalid indices for empty lists.
check("if (n == 0) { top = 0; sel = 0; return false; }" in ui_page,
      "empty chat/board list swipes keep selection and scroll at zero")
check("if (sel >= n) sel = n - 1;" in ui_page,
      "chat/board selection stays within current target count")

# Chat sending must fail safely without silently clearing a draft.
check('if (!conv[0]) {' in ui_page and 'setStatus("Kein Chat ausgewaehlt", 8000);' in ui_page,
      "chat send requires an explicit destination")
check('if (ok) { compose[0] = 0; scroll = 0; }' in ui_page,
      "chat draft is cleared only after successful API handoff")
check('"Nachricht uebergeben" : "Senden fehlgeschlagen"' in ui_page,
      "chat send displays a fallback result when API returns no status")

# Explicitly record known unresolved blockers, without giving a false pass.
print("BLOCKED: privileged web operations and PIN login intentionally disabled on open HTTP")
print("BLOCKED: privileged settings require a separate secure local provisioning channel")
print("UNVERIFIED: PlatformIO compilation and hardware tests")
print("UNVERIFIED: physical RF airtime and regional/profile compliance")
print("STATIC CHECKS PASSED (limited scope)")
