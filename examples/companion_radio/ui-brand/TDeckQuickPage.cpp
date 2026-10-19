#if defined(LILYGO_TDECK) && defined(DISASTER_PORTAL)
#include "TDeckQuickPage.h"

namespace {
constexpr uint16_t BG = 0x08A3, WHITE = 0xFFFF, GREEN = 0x4EF0, GREY = 0x8410;
constexpr uint16_t HELP = 0x6808, INFO = 0x124D, OFFER = 0x1244, FOCUS = 0xFD20;
}

quick::Fix TDeckQuickPage::gpsFix() const {
  if (!sensors || !sensors->isGPSDetected()) return {};
  const char* enabled = sensors->getSettingByKey("gps");
  LocationProvider* gps = sensors->getLocationProvider();
  if (!enabled || strcmp(enabled, "1") || !gps || !gps->hasFreshFix(15000)) return {};
  // Read the provider's current fix directly, not node_lat/node_lon (which are
  // cached on the configured advertisement interval and may be older).
  return {true, gps->getLatitude() / 1000000.0, gps->getLongitude() / 1000000.0};
}

void TDeckQuickPage::selectTarget(int direction) {
  unsigned count = 0;
  char id[14], name[64];
  while (count < 72 && deviceQuickTarget(count, id, sizeof(id), name, sizeof(name))) ++count;
  if (!count) { snprintf(status, sizeof(status), "Noch keine Kanaele/Kontakte geladen"); return; }
  targetIndex = direction < 0 ? (targetIndex + count - 1) % count : (targetIndex + 1) % count;
  if (deviceQuickTarget(targetIndex, id, sizeof(id), name, sizeof(name))) {
    // Snapshot the ID as well as the label. A later contact-list refresh must
    // never silently change the recipient selected by the user.
    snprintf(target, sizeof(target), "%s", id);
    snprintf(targetName, sizeof(targetName), "%s", name);
    draftChanged();
  }
}

bool TDeckQuickPage::refreshTargetName() {
  char id[14], name[64];
  for (unsigned i = 0; i < 72 && deviceQuickTarget(i, id, sizeof(id), name, sizeof(name)); ++i) {
    if (strcmp(id, target)) continue;
    targetIndex = i;
    if (!strcmp(name, targetName)) return false;
    snprintf(targetName, sizeof(targetName), "%s", name); return true;
  }
  if (!strcmp(targetName, "[Ziel nicht verfuegbar]")) return false;
  snprintf(targetName, sizeof(targetName), "[Ziel nicht verfuegbar]"); return true;
}

void TDeckQuickPage::submit(bool position, uint32_t stamp) {
  if (submitted) {
    snprintf(status, sizeof(status), "Schon vorgemerkt: Text/Ort/Ziel aendern"); return;
  }
  char text[quick::TEXT_LIMIT + 1];
  quick::Result result = model.compose(position, gpsFix(), stamp, text, sizeof(text));
  switch (result) {
    case quick::Result::NoFix:
      snprintf(status, sizeof(status), "Kein frischer GPS-Fix. Warten / Ort waehlen."); return;
    case quick::Result::InvalidTime:
      snprintf(status, sizeof(status), "Uhr fehlt: GPS-Fix abwarten / App verbinden"); return;
    case quick::Result::Empty:
      snprintf(status, sizeof(status), "Zuerst Meldungstext eingeben"); return;
    case quick::Result::TooLong:
      snprintf(status, sizeof(status), "Zu lang: max. 100 Bytes inklusive Standort"); return;
    default: break;
  }
  submitted = deviceQuickSend(target, text, stamp, status, sizeof(status));
}

bool TDeckQuickPage::activate(uint32_t stamp) {
  switch (model.focus) {
    case quick::Presets: model.selectPreset(model.preset); model.focus = quick::Text; draftChanged(); break;
    case quick::Text: model.focus = quick::PlaceSelect; break;
    case quick::PlaceSelect: model.cyclePlace(1); draftChanged(); break;
    case quick::Target: selectTarget(1); break;
    case quick::Send: submit(false, stamp); break;
    case quick::Position: submit(true, stamp); break;
    case quick::Home: return true;
    case quick::Clear: model.text[0] = 0; model.focus = quick::Text; draftChanged(); break;
    default: break;
  }
  return false;
}

bool TDeckQuickPage::key(uint8_t key, uint32_t stamp) {
  // The original keyboard has no TAB/ESC keys. Its documented ALT+C sends 0x0C.
  if (key == 12 || key == '\t') { model.focus = quick::Focus((model.focus + 1) % quick::FOCUS_COUNT); return false; }
  if (key == 27) return true;
  if (key == '\r' || key == '\n') return activate(stamp);
  if (key == 21) { model.text[0] = 0; model.focus = quick::Text; draftChanged(); return false; } // optional Ctrl+U
  if (model.focus == quick::Text) {
    if (key == 8 || key == 127) { model.backspace(); draftChanged(); }
    else if (key >= 32 && key <= 126) {
      if (model.append(char(key))) draftChanged();
      else snprintf(status, sizeof(status), "Text voll: Backspace oder Leeren waehlen");
    }
    return false;
  }
  if (key == 8 || key == 127) return true; // Backspace outside text returns home
  if (model.focus == quick::Presets && key >= '1' && key <= '8') {
    model.selectPreset(key - '1'); model.focus = quick::Text; draftChanged(); return false;
  }
  // Space cycles selections only; transmission always needs Enter/trackball.
  if (key == ' ' && (model.focus == quick::PlaceSelect || model.focus == quick::Target)) return activate(stamp);
  if (key == '[' || key == ']' || key == '(' || key == ')') {
    int dir = key == '[' || key == '(' ? -1 : 1;
    if (model.focus == quick::Presets) {
      model.selectPreset((model.preset + quick::PRESET_COUNT + dir) % quick::PRESET_COUNT); draftChanged();
    } else if (model.focus == quick::PlaceSelect) { model.cyclePlace(dir); draftChanged(); }
    else if (model.focus == quick::Target) selectTarget(dir);
  }
  return false;
}

bool TDeckQuickPage::tick(uint32_t now) {
  bool changed = false;
  if (int32_t(now - nextGps) >= 0) {
    nextGps = now + 1000;
    bool fix = quick::validFix(gpsFix());
    LocationProvider* gps = sensors ? sensors->getLocationProvider() : nullptr;
    long sats = gps ? gps->satellitesCount() : 0;
    changed = fix != lastFix || sats != lastSatellites;
    if (refreshTargetName()) changed = true;
    lastFix = fix; lastSatellites = sats;
  }
  if (submitted && int32_t(now - nextStatus) >= 0) {
    nextStatus = now + 250;
    char next[96]; deviceQuickStatus(next, sizeof(next));
    if (strcmp(next, status)) { snprintf(status, sizeof(status), "%s", next); changed = true; }
  }
  return changed;
}

void TDeckQuickPage::displayText(DisplayDriver* d, int x, int y, uint16_t color, const char* text, int limit) {
  char out[128]; int n = 0;
  for (size_t i = 0; text[i] && n < limit && n < int(sizeof(out)) - 1; ++i) {
    uint8_t c = text[i];
    if (c < 128) out[n++] = c >= 32 ? c : ' ';
    else if (c == 0xC3 && text[i + 1]) {
      uint8_t next = text[++i];
      switch (next) {
        case 0xA4: out[n++] = char(0x84); break;
        case 0xB6: out[n++] = char(0x94); break;
        case 0xBC: out[n++] = char(0x81); break;
        case 0x84: out[n++] = char(0x8E); break;
        case 0x96: out[n++] = char(0x99); break;
        case 0x9C: out[n++] = char(0x9A); break;
        case 0x9F: out[n++] = char(0xE1); break;
        default: out[n++] = '?'; break;
      }
    } else if ((c & 0xC0) != 0x80) out[n++] = '?';
  }
  out[n] = 0; d->nativeText(x, y, 1, color, out);
}

void TDeckQuickPage::render(DisplayDriver* d) {
  d->startFrame(BG);
  d->nativeText(4, 2, 2, WHITE, "Schnellmeldung");
  d->nativeRect(257, 1, 60, 17, model.focus == quick::Home ? FOCUS : GREY);
  displayText(d, 265, 6, WHITE, "Zurueck");
  char line[128];
  bool detected = sensors && sensors->isGPSDetected();
  const char* enabled = sensors ? sensors->getSettingByKey("gps") : nullptr;
  const char* gpsState = !detected ? "GPS nicht erkannt" : !enabled || strcmp(enabled,"1") ? "GPS ausgeschaltet" : lastFix ? "GPS: frischer Fix" : "GPS: suche Fix";
  snprintf(line, sizeof(line), "%s (%ld Sat) | 1-8: Vorlage", gpsState, lastSatellites < 0 ? 0 : lastSatellites);
  displayText(d, 4, 19, lastFix ? GREEN : FOCUS, line);
  for (int i = 0; i < quick::PRESET_COUNT; ++i) {
    int x = 4 + (i % 2) * 158, y = 30 + (i / 2) * 23;
    const auto& p = quick::PRESETS[i];
    uint16_t bg = !strcmp(p.kind, "HILFE") ? HELP : !strcmp(p.kind, "INFO") ? INFO : OFFER;
    d->nativeFill(x, y, 154, 21, bg);
    d->nativeRect(x, y, 154, 21, model.focus == quick::Presets && model.preset == i ? FOCUS : GREY);
    snprintf(line, sizeof(line), "%d %s", i + 1, p.label);
    displayText(d, x + 4, y + 6, WHITE, line, 24);
  }
  snprintf(line, sizeof(line), "Text: %u/100 Bytes (ohne Ort)", unsigned(strlen(model.text)));
  displayText(d, 4, 124, model.focus == quick::Text ? FOCUS : GREY, line);
  d->nativeRect(268, 123, 49, 10, model.focus == quick::Clear ? FOCUS : GREY);
  displayText(d, 272, 124, WHITE, "Leeren");
  d->nativeRect(3, 134, 314, 27, model.focus == quick::Text ? FOCUS : GREY);
  // Wrap by UTF-8 character boundaries; never split an umlaut between lines.
  char textLine[104]; size_t pos = 0;
  for (int row = 0; row < 3 && model.text[pos]; ++row) {
    size_t n = 0; int chars = 0;
    while (model.text[pos] && chars < 50) {
      uint8_t c = model.text[pos];
      textLine[n++] = model.text[pos++];
      while (model.text[pos] && (uint8_t(model.text[pos]) & 0xC0) == 0x80) textLine[n++] = model.text[pos++];
      if ((c & 0xC0) != 0x80) ++chars;
    }
    textLine[n] = 0; displayText(d, 7, 136 + row * 8, WHITE, textLine, 50);
  }
  const char* place = model.place < 0 ? "Ohne Standort" : model.place == 0 ? "Mein Standort (Geraete-GPS)" : quick::PLACES[model.place - 1].name;
  snprintf(line, sizeof(line), "Ort: %s%s", model.place > 0 ? "manuell: " : "", place);
  displayText(d, 4, 165, model.focus == quick::PlaceSelect ? FOCUS : WHITE, line);
  snprintf(line, sizeof(line), "Ziel: %s", targetName);
  displayText(d, 4, 178, model.focus == quick::Target ? FOCUS : WHITE, line);
  d->nativeFill(4, 192, 153, 21, OFFER);
  d->nativeFill(163, 192, 153, 21, INFO);
  d->nativeRect(4, 192, 153, 21, model.focus == quick::Send ? FOCUS : GREY);
  d->nativeRect(163, 192, 153, 21, model.focus == quick::Position ? FOCUS : GREY);
  displayText(d, 12, 199, WHITE, "Meldung senden");
  displayText(d, 171, 199, WHITE, "Position senden");
  displayText(d, 4, 215, FOCUS, status);
  if (strlen(status) > 52) displayText(d, 4, 223, FOCUS, status + 52);
  displayText(d, 4, 232, GREY, "ALT+C: Feld  ENTER: Aktion  SPACE: Auswahl");
  d->endFrame();
}
#endif
