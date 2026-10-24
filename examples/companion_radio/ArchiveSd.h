// SPDX-License-Identifier: MIT
#pragma once
#include "HistoryRecord.h"
#include "ArchivePolicy.h"
#ifdef DISASTER_SD
#include <SD.h>
#include <SPI.h>
#include "ArchiveBusConfig.h"
#ifdef DISASTER_SD_SHARED_BUS
extern SPIClass tdeck_shared_spi;   // variants/lilygo_tdeck/target.cpp
void tdeck_shared_spi_begin();
#endif
#endif
class ArchiveSd {
#ifdef DISASTER_SD
#ifdef DISASTER_SD_SHARED_BUS
 SPIClass &bus=tdeck_shared_spi;    // shared with LoRa and display; SD only uses its own chip select
#else
 SPIClass bus{disaster::sdBus};
#endif
#endif
 bool mounted=false, card_mounted=false;
 const char *error="SD nicht eingebunden: Karte/FAT32/Bus pruefen";
public:
 void begin(){
#ifdef DISASTER_SD
#ifdef DISASTER_SD_SHARED_BUS
  pinMode(disaster::sdCs,OUTPUT);digitalWrite(disaster::sdCs,HIGH);
  tdeck_shared_spi_begin();          // no-op if display/radio started it already
#else
  bus.begin(disaster::sdSck,disaster::sdMiso,disaster::sdMosi,disaster::sdCs);
#endif
  // Never format a user's card. SD.end() releases only the SD mount, not the shared SPI bus.
  if(!card_mounted){
   card_mounted=SD.begin(disaster::sdCs,bus,disaster::sdFrequency,"/sd",2,false);
   if(card_mounted&&SD.cardType()==CARD_NONE){SD.end();card_mounted=false;}
  }
  mounted=false;
  if(card_mounted){
   if(!SD.exists("/disaster")&&!SD.mkdir("/disaster"))error="SD da: Archivordner nicht schreibbar";
   else {
    File dir=SD.open("/disaster",FILE_READ);
    mounted=dir&&dir.isDirectory();dir.close();
    error=mounted?"":"SD da: /disaster ist kein Ordner";
   }
  }else error="SD nicht eingebunden: Karte/FAT32/Bus pruefen";
  Serial.println(mounted?"SD archive ready":"SD archive unavailable; internal history remains active");
#endif
 }
 bool ready()const{return mounted;}
 bool cardReady()const{return card_mounted;}
 const char *status()const{return error;}
 void append(const disaster::HistoryRecord &r){
#ifdef DISASTER_SD
  if(!mounted)return;
  auto f=SD.open("/disaster/current.bin",FILE_APPEND);
  if(!f){mounted=false;error="SD da: Archiv-Schreibfehler";return;}
  size_t bytes=f.size();f.close();
  if(bytes>disaster::ARCHIVE_LIMIT){mounted=false;error="SD da: Archiv-Schreibfehler";return;}
  // Preserve a torn tail in the previous segment; never append misaligned records.
  bool rotate=disaster::archiveRotate(bytes,sizeof(r));
  if(rotate){if(SD.exists("/disaster/previous.bin")&&!SD.remove("/disaster/previous.bin")){mounted=false;error="SD da: Archiv-Schreibfehler";return;}
   if(!SD.rename("/disaster/current.bin","/disaster/previous.bin")){mounted=false;error="SD da: Archiv-Schreibfehler";return;}}
  f=SD.open("/disaster/current.bin",FILE_APPEND);
  if(!f||f.write((const uint8_t*)&r,sizeof(r))!=sizeof(r)){mounted=false;error="SD da: Archiv-Schreibfehler";}
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
