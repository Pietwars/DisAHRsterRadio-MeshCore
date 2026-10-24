#pragma once
#include <stdint.h>
#include <string.h>

namespace disaster {
// Nominal centre in kHz, bandwidth in Hz (Companion protocol units).
struct RadioPreset { uint32_t freq, bw; uint8_t sf, cr; };
#ifdef DISASTER_STANDALONE
constexpr unsigned DAR_RADIO_PROFILES = 4;
constexpr unsigned DAR_DEFAULT_RADIO_PROFILE = 3; // MeshCore EU/UK Narrow, fresh preferences
#else
constexpr unsigned DAR_RADIO_PROFILES = 3;
constexpr unsigned DAR_DEFAULT_RADIO_PROFILE = 1; // Mittel, fresh preferences only
#endif
constexpr int8_t DAR_MAX_TX_DBM = 10; // conducted-power cap, not measured antenna ERP

inline const char* radioProfileId(unsigned index) {
  return index == 0 ? "nah" : index == 1 ? "mittel" : index == 2 ? "fern" :
         index == 3 && index < DAR_RADIO_PROFILES ? "narrow" : nullptr;
}
inline const char* radioProfileName(unsigned index) {
  return index == 0 ? "Nah" : index == 1 ? "Mittel" : index == 2 ? "Fern" :
         index == 3 && index < DAR_RADIO_PROFILES ? "EU/UK Narrow" : "Nicht freigegeben";
}
inline bool radioProfileAt(unsigned index, RadioPreset& p) {
  if (index >= DAR_RADIO_PROFILES) return false;
  if (index == 3) { p = {869618, 62500, 8, 8}; return true; }
  p = {869525, 250000, uint8_t(index == 0 ? 7 : index == 1 ? 9 : 11), 5};
  return true;
}
// Nominal bandwidth containment in the reviewed German 869.4–869.65 MHz subband.
// This is NOT an occupied-spectrum/ERP measurement or a blanket EU permission.
inline bool withinDeBand54(uint32_t freqKhz, uint32_t bwHz) {
  uint64_t centre2 = uint64_t(freqKhz) * 2000;
  return bwHz && centre2 >= 2ULL * 869400000 + bwHz &&
         centre2 + bwHz <= 2ULL * 869650000;
}
inline bool dePreset(const char* id, RadioPreset& p) {
  if (!id) return false;
  for (unsigned i = 0; i < DAR_RADIO_PROFILES; ++i) {
    if (!strcmp(id, radioProfileId(i))) return radioProfileAt(i, p);
  }
  return false;
}
inline int radioProfileIndex(uint32_t freqKhz, uint32_t bwHz, uint8_t sf, uint8_t cr) {
  if (!withinDeBand54(freqKhz, bwHz)) return -1;
  for (unsigned i = 0; i < DAR_RADIO_PROFILES; ++i) {
    RadioPreset p; radioProfileAt(i, p);
    if (freqKhz == p.freq && bwHz == p.bw && sf == p.sf && cr == p.cr) return int(i);
  }
  return -1;
}
// Match only canonical stored float values. No truncation of NaN, infinity,
// near-miss frequencies or unsupported bandwidths into an accepted profile.
inline int radioProfileIndex(float freqMhz, float bwKhz, uint8_t sf, uint8_t cr) {
  for (unsigned i = 0; i < DAR_RADIO_PROFILES; ++i) {
    RadioPreset p; radioProfileAt(i, p);
    if (freqMhz == float(p.freq) / 1000.0f && bwKhz == float(p.bw) / 1000.0f &&
        sf == p.sf && cr == p.cr) return int(i);
  }
  return -1;
}
inline bool darTxPowerAllowed(int64_t power) { return power >= -9 && power <= DAR_MAX_TX_DBM; }
}
