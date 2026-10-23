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

# Explicitly record known unresolved blockers, without giving a false pass.
print("BLOCKED: privileged web operations and PIN login intentionally disabled on open HTTP")
print("BLOCKED: privileged settings require a separate secure local provisioning channel")
print("UNVERIFIED: PlatformIO compilation and hardware tests")
print("UNVERIFIED: Airtime enforcement for exact serialized packet lengths")
print("STATIC CHECKS PASSED (limited scope)")
