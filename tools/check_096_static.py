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

# Public transports must never become privileged merely because a BLE PIN is configured.
permission = access.split("inline bool allowCompanionCommand(", 1)[1].split("\\n}", 1)[0]
check("return !publicTransport || publicCompanionCommand(cmd);" in permission,
      "BLE and open WLAN have the same explicit public-command allowlist")
check("if (bluetooth && blePin != 0) return true;" not in permission,
      "configured BLE PIN cannot grant privileged companion commands")
check("validBlePin(uint32_t pin)" in access and "effectiveBlePin(uint32_t saved)" in access,
      "six-digit BLE pairing PIN handling remains present")
check("allowCompanionCommand(cmd_frame[0]" in mesh,
      "companion command dispatcher still enforces access policy")

check("esp_random() % 900000UL" in main, "ESP32 provisions a per-device BLE PIN")
check("the_mesh.savePrefs();" in main, "BLE PIN is persisted in device preferences")
check('Serial.printf("[BLE] Bluetooth-PIN %06lu' not in main,
      "BLE PIN is not printed to serial logs")
check("disaster::validBlePin(pin)" in main,
      "existing valid user-configured BLE PIN is preserved")

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

# Explicitly record known unresolved blockers, without giving a false pass.
print("BLOCKED: privileged web operations and PIN login intentionally disabled on open HTTP")
print("BLOCKED: privileged settings require a separate secure local provisioning channel")
print("UNVERIFIED: PlatformIO compilation and hardware tests")
print("UNVERIFIED: Airtime enforcement for exact serialized packet lengths")
print("STATIC CHECKS PASSED (limited scope)")
