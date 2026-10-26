// SPDX-License-Identifier: MIT
#pragma once
// Keep original companion/app compatibility. The separately reviewed DE guard
// selects conservative dispatcher accounting for both DAR frontends (#47).
// Relay and room-server roles do not opt in through their UI or transport flags.
#if defined(DISASTER_STANDALONE) || defined(DISASTER_PORTAL)
#define DAR_ORIGINAL_MESHCORE_TX 1
#define DAR_REGIONAL_TX_GUARD 1
#endif
