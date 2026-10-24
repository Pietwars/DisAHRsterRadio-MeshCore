#pragma once
#include <algorithm>
#include <map>
#include <string>
#include <vector>
#include <cstring>
#include <cstdio>
#include "SPI.h"
#define FILE_READ "r"
#define FILE_APPEND "a"
#define CARD_NONE 0
struct Entry { bool dir=false;std::vector<uint8_t> bytes; };
extern size_t writeLimit,readLimit;
struct File {
 Entry *entry=nullptr;size_t pos=0;
 explicit operator bool() const {return entry;}
 bool isDirectory()const{return entry&&entry->dir;}
 size_t size()const{return entry?entry->bytes.size():0;}
 void close(){entry=nullptr;}
 size_t write(const uint8_t *p,size_t n){
  if(!entry||entry->dir)return 0;
  n=std::min(n,writeLimit);entry->bytes.insert(entry->bytes.end(),p,p+n);return n;
 }
 size_t read(uint8_t *p,size_t n){
  if(!entry||entry->dir)return 0;
  n=std::min(n,std::min(readLimit,entry->bytes.size()-pos));
  memcpy(p,entry->bytes.data()+pos,n);pos+=n;return n;
 }
};
struct FakeSd {
 std::map<std::string,Entry> files;
 bool beginOK=true,mkdirOK=true,openOK=true,renameOK=true,removeOK=true;
 unsigned type=1,beginCalls=0,endCalls=0,frequency=0;bool format=true;
 bool begin(int,SPIClass&,unsigned hz,const char*,int,bool erase){++beginCalls;frequency=hz;format=erase;return beginOK;}
 unsigned cardType(){return type;}
 void end(){++endCalls;}
 bool exists(const char *p){return files.count(p);}
 bool mkdir(const char *p){if(!mkdirOK)return false;files[p].dir=true;return true;}
 bool remove(const char *p){return removeOK&&files.erase(p);}
 bool rename(const char *a,const char *b){if(!renameOK||!files.count(a))return false;files[b]=files[a];files.erase(a);return true;}
 File open(const char *p,const char *mode){
  File f;if(!openOK||(!files.count(p)&&mode[0]=='r'))return f;
  f.entry=&files[p];return f;
 }
};
extern FakeSd SD;
struct SerialStub {void println(const char*){}};
extern SerialStub Serial;
