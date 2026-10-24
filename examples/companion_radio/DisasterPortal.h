// SPDX-License-Identifier: MIT
#pragma once
// Web layer on top of the web-free local companion client (LocalCompanion.h, Issue #35): HTTP routes, open WLAN,
// captive DNS and per-phone send budgets. Companion session, history, local send rules and room login are shared
// with the T-Deck Plus standalone build.
#include "LocalCompanion.h"
// 0.9.1 (Peter): private key export/import is compiled in. The portal never sends these commands;
// OpenAccess.h allows them only over USB and PIN-paired Bluetooth, and MultiSerialInterface returns
// the export reply only to the requesting USB/Bluetooth link (never WiFi, Ethernet or the portal).
#include <WebServer.h>
#include <DNSServer.h>
#include <WiFi.h>
#include <Preferences.h>
#include <esp_system.h>
#include <esp_timer.h>
#include "PortalPage.h"
#ifndef HEAP_WARN_BYTES
#define HEAP_WARN_BYTES 40960  // Warnschwelle freier Heap (Weboberfläche rot, serielles [HEAP]-Log mit WARNUNG)
#endif
#include "PortalAssets.h"
#include "AdminSession.h"  // nur noch dePreset(); Web-Admin entfernt (0.9.1)

// A single local companion owns the protocol. HTTP clients share its history.
// Raw, arbitrary companion commands and private keys are never exposed to HTTP.
class DisasterPortal : public LocalCompanion {
  uint32_t txFlood=0, txDirect=0, rxFlood=0, rxDirect=0;
  bool txBudgetWaiting=false, txStorageBlocked=false;
  uint32_t txStartupWaitSeconds=0;
  bool gpsEnabled=false,gpsFix=false;double gpsLat=0,gpsLon=0;
  uint32_t txBudgetWaitEvents=0, txBudgetWaitSeconds=0;
  int txQueuePackets=0;
  struct ClientBudget { char id[16]; uint32_t lastMs=0, windowStart=0; uint8_t count=0; bool used=false; };
  ClientBudget clientBudgets[8]{};
  // Aggregate budget is independent of caller-controlled X-Disaster-Client IDs.
  uint32_t globalWindowStart=0, globalLastMs=0;
  uint8_t globalSendCount=0;
  unsigned long txAirMs=0, rxAirMs=0, txBudgetMs=0;
  WebServer http{80};
  bool requireAdmin() {
    if(!permitted())return false;
    // Fail closed until a secure local provisioning channel is implemented.
    http.send(403,"text/plain","Einstellungen nur ueber sicheren lokalen Kanal");
    return false;
  }

  unsigned long heapLog=0;
  bool apOk=false; unsigned long apRetry=0, apPrint=0;
  DNSServer dns;
  String apName;
  bool permitted() {
    // Custom header prevents cross-origin form submissions; no CORS is enabled.
    String host=http.hostHeader();
    if (host!="192.168.4.1" && host!="192.168.4.1:80" && host!="disaster.radio") {
      http.send(403,"text/plain","Unbekannter Host"); return false;
    }
    if(http.header("X-Disaster-Request")!="1") {
      http.send(403,"text/plain","Lokale Anfrage erforderlich"); return false;
    }
    http.sendHeader("Cache-Control","no-store");
    return true;
  }
  // One phone must not fill the channel. Missing or invalid ids share one bucket.
  bool allowClientSend() {
    String raw=http.header("X-Disaster-Client");
    char key[16]; memset(key,'0',sizeof(key));
    bool ok=raw.length()==16;
    if(ok)for(int i=0;i<16;++i){char c=raw[i];if(!((c>='0'&&c<='9')||(c>='a'&&c<='f')))ok=false;}
    if(ok)memcpy(key,raw.c_str(),16);
    uint32_t now=millis(); int slot=-1, empty=-1, oldest=0;
    for(int i=0;i<8;++i){
      if(!clientBudgets[i].used){if(empty<0)empty=i;continue;}
      if(memcmp(clientBudgets[i].id,key,16)==0){slot=i;break;}
      if(uint32_t(now-clientBudgets[i].lastMs)>uint32_t(now-clientBudgets[oldest].lastMs))oldest=i;
    }
    if(slot<0){slot=empty>=0?empty:oldest;memcpy(clientBudgets[slot].id,key,16);clientBudgets[slot].used=true;clientBudgets[slot].count=0;clientBudgets[slot].windowStart=now;clientBudgets[slot].lastMs=0;}
    ClientBudget &b=clientBudgets[slot];
    if(b.lastMs&&uint32_t(now-b.lastMs)<15000){http.send(429,"text/plain","Dieses Handy muss 15 Sekunden warten");return false;}
    if(!b.windowStart||uint32_t(now-b.windowStart)>=600000){b.windowStart=now;b.count=0;}
    if(b.count>=6){http.send(429,"text/plain","Sendebudget dieses Handys erschoepft: 6 Nachrichten in 10 Minuten");return false;}
    // Client IDs are advisory, not authentication: rotating IDs must not evade
    // the aggregate portal cap. Apply it to all HTTP send/advert/outbox paths.
    if(globalLastMs && uint32_t(now-globalLastMs)<2000){
      http.send(429,"text/plain","Funkportal: zwei Sekunden globale Sendepause");return false;
    }
    if(!globalWindowStart || uint32_t(now-globalWindowStart)>=600000){
      globalWindowStart=now;globalSendCount=0;
    }
    if(globalSendCount>=24){
      http.send(429,"text/plain","Funkportal: globales Sendebudget erschoepft");return false;
    }
    globalLastMs=now;++globalSendCount;
    b.lastMs=now;b.count++;return true;
  }
  bool room() {
    if (!permitted()) return false;
    if(busy()) {
      http.send(409,"text/plain","Knoten beschaeftigt. Bitte erneut versuchen."); return false;
    }
    return true;
  }
  void accept(const uint8_t *p,size_t n) {
    queueCommand(p,n);
    http.send(202,"text/plain","In Warteschlange; noch keine Zustellbestaetigung.");
  }
  void history() {
    if(!permitted()) return;
    http.setContentLength(CONTENT_LENGTH_UNKNOWN); http.send(200,"text/plain","");
    // Physical slot order; client sorts by monotonically increasing sequence.
    File f=disaster::openSlotFileRead(SPIFFS,"/dr-history.bin"); Record r;
    while(f && f.read((uint8_t*)&r,sizeof(r))==sizeof(r)) {
      if(disaster::valid(r))
        http.sendContent(String(r.seq)+":"+hex(r.data,r.len)+"\n");
    }
    f.close(); http.sendContent("");
  }
public:
  // Snapshot existing MeshCore counters; does not issue Companion commands.
  void setGps(bool enabled,bool fix,double lat,double lon){gpsEnabled=enabled;gpsFix=fix&&(lat!=0||lon!=0);gpsLat=lat;gpsLon=lon;}
  void setRadioDiagnostics(uint32_t tf, uint32_t td, uint32_t rf, uint32_t rd,
                           unsigned long ta, unsigned long ra, unsigned long budget,
                           bool waiting, uint32_t events, uint32_t seconds, int queued,
                           uint32_t startupSeconds=0, bool storageBlocked=false) {
    txFlood=tf;txDirect=td;rxFlood=rf;rxDirect=rd;
    txAirMs=ta;rxAirMs=ra;txBudgetMs=budget;
    txStartupWaitSeconds=startupSeconds;txStorageBlocked=storageBlocked;
    txBudgetWaiting=waiting;txBudgetWaitEvents=events;txBudgetWaitSeconds=seconds;txQueuePackets=queued;
  }
  void begin() {
    beginHistory();
    Preferences prefs; prefs.begin("disaster",false);
    // Remove old AP credentials and the former web-admin password (removed in 0.9.1) on upgrade.
    prefs.remove("ap-pass");prefs.remove("admin-salt");prefs.remove("admin-hash");
    prefs.end();
    uint64_t mac=ESP.getEfuseMac(); char suffix[7]; snprintf(suffix,sizeof(suffix),"%06X",(unsigned)(mac&0xffffff));
    apName=String("DisAHRsterRadio-")+suffix;
    WiFi.mode(WIFI_AP); WiFi.setSleep(false);
#ifdef DISASTER_WIFI_TX
    WiFi.setTxPower(DISASTER_WIFI_TX); Serial.println("[WLAN] reduzierte Sendeleistung"); Serial.flush(); delay(200);
#endif
    apOk=startAP();
    dns.start(53,"*",WiFi.softAPIP());
    Serial.printf("\nWLAN: %s\nOffenes WLAN - kein Passwort\nBrowser: http://192.168.4.1\n",apName.c_str());
    const char *headers[]={"X-Disaster-Request","X-Disaster-Client"}; http.collectHeaders(headers,2);
    http.on("/",HTTP_GET,[this](){
      http.sendHeader("Content-Security-Policy","default-src 'self'; script-src 'self' 'unsafe-inline'; style-src 'self' 'unsafe-inline'; connect-src 'self'; frame-ancestors 'none'; base-uri 'none'");
      http.sendHeader("X-Content-Type-Options","nosniff");
      http.sendHeader("Content-Encoding","gzip");
      http.send_P(200,"text/html; charset=utf-8",(const char*)PORTAL_PAGE_GZ,sizeof(PORTAL_PAGE_GZ));
    });
    http.on("/app.webmanifest",HTTP_GET,[this](){http.send_P(200,"application/manifest+json",PORTAL_MANIFEST);});
    http.on("/app-icon.png",HTTP_GET,[this](){http.send_P(200,"image/png",(const char*)PORTAL_ICON,sizeof(PORTAL_ICON));});
#ifdef PORTAL_HAS_BG
    http.on("/bg.webp",HTTP_GET,[this](){http.sendHeader("Cache-Control","max-age=86400");http.send_P(200,"image/webp",(const char*)PORTAL_BG_WEBP,sizeof(PORTAL_BG_WEBP));});
    for(const auto &m:PORTAL_MAPS){
      String path=String("/map/")+m.name+".json"; const PortalMap *mp=&m;
      http.on(path.c_str(),HTTP_GET,[this,mp](){http.sendHeader("Content-Encoding","gzip");http.sendHeader("Cache-Control","max-age=86400");http.send_P(200,"application/json",(const char*)mp->data,mp->len);});
    }
#endif
    http.on("/api/admin-login",HTTP_POST,[this](){
      if(!permitted())return;
      // Never accept PINs or issue bearer tokens over the open HTTP portal.
      http.send(503,"text/plain","Admin-Anmeldung ueber offenes WLAN deaktiviert");
    });
    http.on("/api/position",HTTP_POST,[this](){
      if(!room())return;
      long lat=strtol(http.arg("lat").c_str(),nullptr,10),lon=strtol(http.arg("lon").c_str(),nullptr,10);
      if(lat<-90000000L||lat>90000000L||lon<-180000000L||lon>180000000L){http.send(400,"text/plain","Ungueltige Koordinaten");return;}
      int32_t la=lat,lo=lon;uint8_t p[9]={14};memcpy(p+1,&la,4);memcpy(p+5,&lo,4);accept(p,9);
    });
    http.on("/api/state",HTTP_GET,[this](){
      if(!permitted())return;
      http.setContentLength(CONTENT_LENGTH_UNKNOWN);
      http.send(200,"application/json","");
      String s="{\"ready\":"+String(startup>=12?"true":"false")+",\"storage\":"+String(storageOK?"true":"false")+",\"heap\":"+String(ESP.getFreeHeap())+",\"seq\":"+String(seq)+",\"busy\":"+String((active||pendingLen||loginLen)?"true":"false")+",\"result\":\""+outcome+"\",\"info\":\""+info+"\",\"channels\":[";
      http.sendContent(s);s="";
      for(int i=0;i<8;i++){http.sendContent((i?",\"":"\"")+channels[i]+"\"");}
      http.sendContent("],\"contacts\":["); bool first=true;
      for(auto &c:contacts)if(c.length()){http.sendContent((first?"\"":",\"")+c+"\"");first=false;}
      s+="],\"archive\":"+String(archive.ready()?"true":"false")+",\"queueReady\":"+String(delivery.ready()?"true":"false")+",\"queueStorage\":"+String(delivery.storageOK()?"true":"false")+",\"room\":{\"target\":\""+roomTarget+"\",\"rights\":"+String(roomRights)+",\"pending\":"+String(roomPending?"true":"false")+"}}";
      // Fixed keys and numeric values only; no passwords, contacts or chat texts.
      s.remove(s.length()-1);
      s+=",\"diagnostics\":{\"firmware\":\""+String(FIRMWARE_VERSION)+"\",\"uptimeSeconds\":"+String((uint32_t)(esp_timer_get_time()/1000000ULL));
      s+=",\"resetReason\":"+String((int)esp_reset_reason())+",\"minHeapBytes\":"+String(ESP.getMinFreeHeap())+",\"largestHeapBlockBytes\":"+String(ESP.getMaxAllocHeap())+",\"heapWarn\":"+String((unsigned)HEAP_WARN_BYTES);
      s+=",\"wifiClients\":"+String(WiFi.softAPgetStationNum())+",\"txFlood\":"+String(txFlood)+",\"txDirect\":"+String(txDirect)+",\"rxFlood\":"+String(rxFlood)+",\"rxDirect\":"+String(rxDirect);
      s+=",\"txAirMs\":"+String(txAirMs)+",\"rxAirMs\":"+String(rxAirMs)+",\"txBudgetMs\":"+String(txBudgetMs)+",\"txBudgetWaiting\":"+String(txBudgetWaiting?"true":"false")+",\"txBudgetWaitEvents\":"+String(txBudgetWaitEvents)+",\"txBudgetWaitSeconds\":"+String(txBudgetWaitSeconds)+",\"txQueuePackets\":"+String(txQueuePackets);
      s+=",\"txStartupWaitSeconds\":"+String(txStartupWaitSeconds)+",\"txStorageBlocked\":"+String(txStorageBlocked?"true":"false")+"}";
      // Optional board GPS: position only when the module reports a valid fix.
      s+=",\"gps\":{\"enabled\":"+String(gpsEnabled?"true":"false")+",\"fix\":"+String(gpsFix?"true":"false");
      if(gpsFix)s+=",\"lat\":"+String(gpsLat,5)+",\"lon\":"+String(gpsLon,5);
      s+="}}";
      http.sendContent(s);
      http.sendContent("");
    });
    http.on("/api/history",HTTP_GET,[this](){history();});
    // Verlauf loeschen ohne Anmeldung (Peter, 0.9.1): jeder im offenen WLAN darf das.
    http.on("/api/history-wipe",HTTP_POST,[this](){
      if(!requireAdmin())return;if(busy()){http.send(409,"text/plain","Knoten beschaeftigt");return;}
      if(active||pendingLen){http.send(409,"text/plain","Knoten beschaeftigt. Bitte erneut versuchen.");return;}
      if(SPIFFS.exists("/dr-history.bin")&&!SPIFFS.remove("/dr-history.bin")){storageOK=false;http.send(500,"text/plain","Verlauf konnte nicht geloescht werden");return;}
      storageOK=true;http.send(200,"text/plain","Lokaler Verlauf geloescht. Schluessel und optionales SD-Archiv bleiben.");
    });
    http.on("/api/archive",HTTP_GET,[this](){
      if(!permitted())return;
      File f=archive.open(http.arg("part")=="previous");
      if(!f){http.send(404,"text/plain","Archiv nicht verfuegbar");return;}
      http.sendHeader("Content-Disposition","attachment; filename=disahrsterradio-archive.bin");
      http.streamFile(f,"application/octet-stream");f.close();
    });
    http.on("/api/outbox",HTTP_GET,[this](){
      if(!permitted())return;String s="[";bool first=true;
      for(size_t i=0;i<disaster::OUTBOX_SLOTS;++i){const auto &j=delivery.get(i);if(!j.id)continue;if(!first)s+=",";first=false;
       s+="{\"id\":"+String(j.id)+",\"target\":\"d"+hex(j.recipient,6)+"\",\"textHex\":\""+hex((const uint8_t*)j.text,strlen(j.text))+"\",\"attempts\":"+String(j.attempts)+",\"state\":"+String(j.state)+",\"expires\":"+String(j.expires)+"}";
      }http.send(200,"application/json",s+"]");
    });
    http.on("/api/outbox-cancel",HTTP_POST,[this](){
      if(!permitted())return;
      bool ok=delivery.cancel(strtoul(http.arg("id").c_str(),nullptr,10));http.send(ok?200:409,"text/plain",ok?"Abgebrochen; bereits gesendete Versuche koennen noch ankommen":"Auftrag nicht abbrechbar / Speicherfehler");
    });
    http.on("/api/outbox",HTTP_POST,[this](){
      if(!room())return;
      String target=http.arg("target"),body=http.arg("text"),ttl=http.arg("hours");
      uint32_t stamp=strtoul(http.arg("time").c_str(),nullptr,10);
      if(body.length()<1||body.length()>100||hasNull(body)||stamp<1700000000u||stamp>4102358400u||(ttl!="1"&&ttl!="6"&&ttl!="24")){http.send(400,"text/plain","Nachricht, Ablaufzeit oder Uhr pruefen");return;}
      const String *contact=nullptr;
      for(const auto &c:contacts)if(c.length()>=264&&target==String("d")+c.substring(2,14)){
        if(contact){http.send(400,"text/plain","Mehrdeutiger Kontakt");return;}contact=&c;
      }
      if(!contact||contact->substring(66,68)!="01"){http.send(400,"text/plain","Nur bekannte private Chat-Kontakte");return;}
      if(stamp<=lastStamp)stamp=lastStamp+1;
      if(!allowClientSend())return;
      uint8_t key[32];for(int i=0;i<32;++i)key[i]=(nibble((*contact)[2+2*i])<<4)|nibble((*contact)[3+2*i]);
      int slot=delivery.enqueue(key,body.c_str(),stamp,stamp+ttl.toInt()*3600);
      if(slot<0){http.send(slot==-1?409:503,"text/plain",slot==-1?"Warteschlange voll (8 Auftraege)":"Speicherstatus unklar; Auftrag vor erneutem Anlegen kontrollieren");return;}
      lastStamp=stamp;http.send(202,"text/plain","Dauerhaft vorgemerkt: maximal vier Versuche, Absender muss eingeschaltet bleiben");
    });
    http.on("/api/room-login",HTTP_POST,[this](){
      if(!room())return;
      String target=http.arg("target");
      const String *contact=roomContact(target);
      uint32_t stamp=strtoul(http.arg("time").c_str(),nullptr,10);
      if(!contact||stamp<1700000000u||stamp>4102444800u){http.send(400,"text/plain","Raum und Uhrzeit pruefen");return;}
      // First synchronize the companion clock; the next serialized command logs in (shared with the device).
      uint8_t clock[5];prepareRoomLogin(*contact,target,stamp,clock);accept(clock,5);
    });
    http.on("/api/send",HTTP_POST,[this](){
      if(!room())return;
      String text=http.arg("text"), target=http.arg("target");
      if(text.length()<1 || text.length()>100 || hasNull(text)) {http.send(400,"text/plain","1 bis 100 UTF-8-Bytes erforderlich");return;}
      if(sendPaused()) {http.send(429,"text/plain","Bitte fuenf Sekunden Sendepause einhalten");return;}
      uint8_t p[MAX_FRAME_SIZE]={}; size_t n;
      uint32_t stamp=(uint32_t)strtoul(http.arg("time").c_str(),nullptr,10);
      if(!disaster::validStamp(stamp)) {http.send(400,"text/plain","Handy-Uhr pruefen");return;}
      stamp=disaster::nextStamp(stamp,lastStamp);
      if(target.length()==2 && target[0]=='c' && target[1]>='0' && target[1]<='7') {
        if(!channels[target[1]-'0'].length()){http.send(400,"text/plain","Kanal nicht eingerichtet");return;}
        p[0]=3;p[2]=target[1]-'0';memcpy(p+3,&stamp,4);n=7;
      } else if(target.length()==13 && target[0]=='d') {
        if(roomContact(target)&&(roomTarget!=target||roomRights<2)){http.send(403,"text/plain","Zuerst am Raum mit Schreibrecht anmelden");return;}
        p[0]=2;memcpy(p+3,&stamp,4);n=13;
        for(int i=0;i<6;i++){int a=nibble(target[1+i*2]),b=nibble(target[2+i*2]);if(a<0||b<0){http.send(400,"text/plain","Ungueltiger Kontakt");return;}p[7+i]=(a<<4)|b;}
      } else {http.send(400,"text/plain","Ungueltiges Ziel");return;}
      memcpy(p+n,text.c_str(),text.length());n+=text.length();
      if(!allowClientSend())return;
      commitOutgoing(p,n,stamp);
      outcome="Warteschlange";accept(p,n);
    });
    // Knotenname ohne Adminanmeldung (Peter, 0.9.1); gleiche Pruefung wie bisher.
    http.on("/api/name",HTTP_POST,[this](){
      if(!requireAdmin())return;if(busy()){http.send(409,"text/plain","Knoten beschaeftigt");return;}String name=http.arg("name");
      if(name.length()<1||name.length()>24||hasNull(name)){http.send(400,"text/plain","Name: 1 bis 24 UTF-8-Bytes");return;}
      uint8_t p[26]={8};memcpy(p+1,name.c_str(),name.length());accept(p,name.length()+1);
    });
    http.on("/api/time",HTTP_POST,[this](){
      if(!room())return;
      uint32_t t=(uint32_t)strtoul(http.arg("time").c_str(),nullptr,10);
      if(t<1700000000u||t>4102444800u){http.send(400,"text/plain","Ungueltige Zeit");return;}
      delivery.syncClock(t);uint8_t p[5]={6};memcpy(p+1,&t,4);accept(p,5);
    });
    // Kanaele anlegen/aendern ohne Anmeldung (Peter, 0.9.1).
    http.on("/api/channel",HTTP_POST,[this](){
      if(!requireAdmin())return;if(busy()){http.send(409,"text/plain","Knoten beschaeftigt");return;}String slot=http.arg("slot"),name=http.arg("name"),key=http.arg("key");
      if(slot.length()!=1||slot[0]<'0'||slot[0]>'7'||name.length()<1||name.length()>24||hasNull(name)||key.length()!=32){http.send(400,"text/plain","Kanalangaben pruefen");return;}
      uint8_t p[50]={32,(uint8_t)(slot[0]-'0')};memcpy(p+2,name.c_str(),name.length());
      for(int i=0;i<16;i++){int a=nibble(key[2*i]),b=nibble(key[2*i+1]);if(a<0||b<0){http.send(400,"text/plain","Schluessel muss Hex sein");return;}p[34+i]=(a<<4)|b;}
      accept(p,50);
    });
    // Funkprofil ohne Adminanmeldung (Peter, 0.9.1): nur die festen Deutschland-Profile aus dePreset().
    http.on("/api/radio",HTTP_POST,[this](){
      if(!requireAdmin())return;if(busy()){http.send(409,"text/plain","Knoten beschaeftigt");return;}
      disaster::RadioPreset preset;
      if(!disaster::dePreset(http.arg("preset").c_str(),preset)){http.send(400,"text/plain","Unbekanntes Deutschland-Funkprofil");return;}
      uint8_t p[12]={11};memcpy(p+1,&preset.freq,4);memcpy(p+5,&preset.bw,4);p[9]=preset.sf;p[10]=preset.cr;p[11]=0;accept(p,12);
    });
    http.on("/api/advert",HTTP_POST,[this](){if(!room())return;if(sendPaused()){http.send(429,"text/plain","Bitte fuenf Sekunden Sendepause einhalten");return;}if(!allowClientSend())return;const uint8_t p[]={7,1};accept(p,2);});
    http.on("/api/refresh",HTTP_POST,[this](){if(!room())return;const uint8_t p[]={4};accept(p,1);});
    http.onNotFound([this](){
      // Diagnose: which client asks for unknown paths (e.g. iOS captive-portal probes)?
      static uint32_t lastLog=0,skipped=0;
      if(!lastLog||millis()-lastLog>=10000){Serial.printf("[HTTP] unbekannt: %s%s (Host %s) -> Umleitung%s\n",http.method()==HTTP_POST?"POST ":"",http.uri().c_str(),http.hostHeader().c_str(),skipped?(String(", +")+String(skipped)+" weitere seit letzter Meldung").c_str():"");lastLog=millis()|1;skipped=0;}else skipped++;
      http.sendHeader("Location","http://192.168.4.1/");http.send(302,"text/plain","");});
    http.begin();
  }
  bool startAP() {
    bool ok=WiFi.softAP(apName.c_str(),nullptr,1,false,4);
#ifdef DISASTER_WIFI_TX
    WiFi.setTxPower(DISASTER_WIFI_TX);
#endif
    Serial.printf("[WLAN] softAP(\"%s\", Kanal 1) -> %s, TX %d (0,25 dBm)\n",apName.c_str(),ok?"OK":"FEHLER",(int)WiFi.getTxPower());Serial.flush();
    return ok;
  }
  const String &ssid() const {return apName;}
  bool apRunning() const {return apOk;}
  // Alle 30 s Zugangsdaten seriell ausgeben, bei Fehlschlag alle 5 s neu versuchen
  void serviceAP() {
    if(!apOk&&millis()-apRetry>5000){apRetry=millis();apOk=startAP();}
    if(millis()-apPrint>30000||!apPrint){apPrint=millis()|1;
      Serial.printf("[WLAN] %s | SSID: %s | offen, ohne Passwort | http://192.168.4.1 | Clients: %d\n",apOk?"AP aktiv":"AP FEHLER",apName.c_str(),WiFi.softAPgetStationNum());
      }
  }
  void loop() override {
    dns.processNextRequest();http.handleClient();
    if(millis()-heapLog>=30000){heapLog=millis();uint32_t f=ESP.getFreeHeap();
      Serial.printf("[HEAP] frei %u | Minimum %u | groesster Block %u%s\n",(unsigned)f,(unsigned)ESP.getMinFreeHeap(),(unsigned)ESP.getMaxAllocHeap(),f<HEAP_WARN_BYTES?" | WARNUNG: wenig freier Speicher":"");}
    serviceRoom();
  }
};

