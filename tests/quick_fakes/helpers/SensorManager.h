#pragma once
#include <stdint.h>
class LocationProvider {
public:
  bool fresh = false;
  long latitude = 50542600, longitude = 7176100, satellites = 7;
  bool hasFreshFix(uint32_t age) { return age == 15000 && fresh; }
  long getLatitude() { return latitude; }
  long getLongitude() { return longitude; }
  long satellitesCount() { return satellites; }
};
class SensorManager {
public:
  LocationProvider gps;
  bool detected = true, enabled = true;
  bool isGPSDetected() const { return detected; }
  const char* getSettingByKey(const char*) { return enabled ? "1" : "0"; }
  LocationProvider* getLocationProvider() { return &gps; }
};
