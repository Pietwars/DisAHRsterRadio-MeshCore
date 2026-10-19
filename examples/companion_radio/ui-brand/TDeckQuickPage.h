#pragma once
#if defined(LILYGO_TDECK) && defined(DISASTER_PORTAL)
#include "QuickModel.h"
#include "QuickBridge.h"
#include <helpers/ui/DisplayDriver.h>
#include <helpers/SensorManager.h>

class TDeckQuickPage {
  quick::Model model;
  SensorManager* sensors = nullptr;
  char target[14] = "c0", targetName[64] = "# Oeffentlich";
  char status[96] = "ALT+C: Feld waehlen. ENTER: Aktion.";
  unsigned targetIndex = 0;
  bool submitted = false, lastFix = false;
  long lastSatellites = -1;
  uint32_t nextGps = 0, nextStatus = 0;
  void selectTarget(int direction);
  bool refreshTargetName();
  void draftChanged() { submitted = false; snprintf(status, sizeof(status), "Entwurf: ALT+C waehlt Feld, ENTER Aktion"); }
  void submit(bool position, uint32_t stamp);
  static void displayText(DisplayDriver* d, int x, int y, uint16_t color, const char* text, int limit = 52);
public:
  void begin(SensorManager* s) { sensors = s; }
  quick::Fix gpsFix() const;
  bool activate(uint32_t stamp); // true=request return to home page
  bool key(uint8_t key, uint32_t stamp);
  bool tick(uint32_t now);
  void render(DisplayDriver* d);
};
#endif
