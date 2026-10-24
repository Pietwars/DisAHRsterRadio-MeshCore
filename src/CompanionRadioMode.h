// SPDX-License-Identifier: MIT
#pragma once
// Both DAR companion frontends use the same original MeshCore radio/TX policy.
// Relay and room-server roles do not opt in through their UI or transport flags.
#if defined(DISASTER_STANDALONE) || defined(DISASTER_PORTAL)
#define DAR_ORIGINAL_MESHCORE_TX 1
#endif
