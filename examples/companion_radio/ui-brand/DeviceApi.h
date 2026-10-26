#pragma once
// Interface between the T-Deck Plus standalone display (StandaloneUI) and the companion core (Issue #35).
// Implemented by StandaloneFrontend.cpp on top of LocalCompanion.h, MyMesh, the GPS and the SD card; host tests
// and tools/standalone_preview.py implement it with fixed demo data. The display never talks to the radio or the
// app directly and never sees PINs or keys.
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "../HistoryRecord.h"
#include "../OutboxRecord.h"
#include "../DeviceConfig.h"
#include "../DarRadioPolicy.h"

namespace dev {
enum class Kind : uint8_t { Channel = 0, Chat = 1, Repeater = 2, Room = 3, Sensor = 4 };
struct Target {
  char id[14];      // "c0".."c7" or "d" + 12 hex digits (same ids as the web UI)
  char name[33];    // UTF-8
  Kind kind;
  bool hasPos;
  double lat, lon;  // advertised position (contacts), degrees
};
struct Gps {
  bool detected = false, receiverOn = false, fresh = false, share = false;
  long sats = 0; double lat = 0, lon = 0;
};
struct Sos {
 uint32_t id=0,expires=0;uint8_t state=0,attempts=0;bool storageOK=true;
 char token[17]="",text[101]="",target[14]="";
};
struct Room { char id[14]; uint8_t rights; bool pending; };
struct Info {
  char name[33] = ""; const char *firmware = "";
  unsigned channels = 0, contacts = 0, history = 0; bool historyOK = true;
  bool bleConnected = false, defaultPin = true, clockOK = false;
  uint32_t heapFree = 0, heapMin = 0;
  bool sdCard = false, sdArchive = false, sdConfig = false, sdLog = false;
  char sdNote[64] = "";
  uint32_t txFlood = 0, txDirect = 0, rxFlood = 0, rxDirect = 0;
  uint32_t txStartupWaitSeconds = 0;
  bool txStorageBlocked = false, txBudgetWaiting = false, txProfileBlocked = false;
};
class Api {
public:
  virtual ~Api() {}
  virtual bool ready() const = 0;                                  // companion startup finished
  virtual uint32_t targetRevision() const { return 0; }
  virtual uint32_t privateHistoryFloor() const { return 0; }
  virtual unsigned targetCount() const = 0;                        // channels first, then contacts
  virtual bool target(unsigned index, Target &out) const = 0;
  virtual uint32_t historySeq() const = 0;                         // changes with every stored record
  virtual bool historyRecord(size_t slot, disaster::HistoryRecord &r) const = 0;
  virtual void historyScan(bool start) const { (void)start; }       // around a full historyRecord() pass
  virtual uint32_t now() const = 0;                                // unix time of the device clock
  virtual bool send(const char *id, const char *text, char *status, size_t size) = 0;
  virtual Sos sos() const {return {};}
  virtual bool sosStart(const char *,char *,size_t){return false;}
  virtual bool sosCancel(uint32_t,char *,size_t){return false;}
  virtual const char *sendStatus() const = 0;
  virtual bool roomLogin(const char *id, char *status, size_t size) = 0;
  virtual Room room() const = 0;
  virtual Gps gps() const = 0;
  virtual void setGpsReceiver(bool on) = 0;
  virtual void setGpsShare(bool on) = 0;
  virtual bool advert(char *status, size_t size) = 0;               // flood advert (with position if shared)
  virtual bool refresh() = 0;                                      // re-read contacts
  virtual void info(Info &out) const = 0;
  // Physical device UI only; never expose these through public transports.
  virtual bool changeName(const char *, char *, size_t) { return false; }
  virtual bool importConfig(char *, size_t) { return false; }
  virtual bool configPreview(devcfg::Config &) const { return false; }
  virtual bool changeBlePin(uint32_t pin, char *status, size_t size) { (void)pin; (void)status; (void)size; return false; }
  virtual bool saveConfig(char *status, size_t size) = 0;          // settings to SD (no PINs, no keys)
  virtual int mapIndex() const = 0;
  virtual void setMapIndex(int index) = 0;
};
Api &api();   // StandaloneFrontend.cpp (firmware) or the host test / preview harness
inline bool findTarget(const Api &a, const char *id, Target &out) {
  for (unsigned i = 0, n = a.targetCount(); i < n; ++i)
    if (a.target(i, out) && !strcmp(out.id, id)) return true;
  return false;
}
}
