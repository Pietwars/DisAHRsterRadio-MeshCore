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
check("uint8_t input[32];" in portal and "memcpy(input+16,pin.c_str(),16);" in portal,
      "setup PIN hash includes all 16 digits")
check('sessionToken.c_str()' in portal, "session validation receives a C string")
check("adminSession.valid(" in portal, "privileged requests check session")
check("adminSession.locked(" in portal, "login lockout is present")
check("#define DAR_MESH_POLICY 1" in policy, "DAR mesh policy is defined")
check("return 9.0f" in mesh, "DAR airtime factor is fixed at 9")
check("required_airtime" in dispatcher, "scheduler checks TX budget")
check("TX_BUDGET_AIRTIME_MARGIN_NUM" in dispatcher, "TX reserve margin is present")

# Explicitly record known unresolved blockers, without giving a false pass.
print("UNVERIFIED: Web PIN/session travel over open HTTP; not secure for deployment")
print("UNVERIFIED: Embedded web UI login integration")
print("UNVERIFIED: PlatformIO compilation and hardware tests")
print("UNVERIFIED: Airtime enforcement for exact serialized packet lengths")
print("STATIC CHECKS PASSED (limited scope)")
