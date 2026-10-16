// SPDX-License-Identifier: MIT
#pragma once
#include "HistoryRecord.h"
#include "ArchivePolicy.h"
#ifdef DISASTER_SD
#include <SD.h>
#include <SPI.h>
#include "ArchiveBusConfig.h"
#endif
class ArchiveSd {
#ifdef DISASTER_SD
 SPIClass bus{disaster::sdBus};
#endif
 bool mounted=false;
public:
 void begin(){
#ifdef DISASTER_SD
  bus.begin(disaster::sdSck,disaster::sdMiso,disaster::sdMosi,disaster::sdCs);
  mounted=SD.begin(disaster::sdCs,bus,4000000,"/sd",2,false);
  if(mounted&&!SD.exists("/disaster"))mounted=SD.mkdir("/disaster");
  Serial.println(mounted?"SD archive ready":"SD archive unavailable; internal history remains active");
#endif
 }
 bool ready()const{return mounted;}
 void append(const disaster::HistoryRecord &r){
#ifdef DISASTER_SD
  if(!mounted)return;
  auto f=SD.open("/disaster/current.bin",FILE_APPEND);
  if(!f){mounted=false;return;}
  size_t bytes=f.size();f.close();
  if(bytes>disaster::ARCHIVE_LIMIT){mounted=false;return;}
  // Preserve a torn tail in the previous segment; never append misaligned records.
  bool rotate=disaster::archiveRotate(bytes,sizeof(r));
  if(rotate){if(SD.exists("/disaster/previous.bin")&&!SD.remove("/disaster/previous.bin")){mounted=false;return;}
   if(!SD.rename("/disaster/current.bin","/disaster/previous.bin")){mounted=false;return;}}
  f=SD.open("/disaster/current.bin",FILE_APPEND);
  if(!f||f.write((const uint8_t*)&r,sizeof(r))!=sizeof(r))mounted=false;
  f.close();
#else
  (void)r;
#endif
 }
 File open(bool previous){
#ifdef DISASTER_SD
  if(mounted){File f=SD.open(previous?"/disaster/previous.bin":"/disaster/current.bin",FILE_READ);if(f&&f.size()<=disaster::ARCHIVE_LIMIT)return f;f.close();}
#else
  (void)previous;
#endif
  return File();
 }
};
