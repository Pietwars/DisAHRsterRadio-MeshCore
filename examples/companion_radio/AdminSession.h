#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "DarRadioPolicy.h"
namespace disaster {
class AdminSession {
  char token_[33] = {};
  uint32_t issued_ = 0, failedAt_ = 0;
  uint8_t failures_ = 0;
public:
  static constexpr uint32_t LIFE_MS = 900000;
  bool locked(uint32_t now) const { return failures_ >= 5 && uint32_t(now-failedAt_) < 60000; }
  void failed(uint32_t now) { if(uint32_t(now-failedAt_)>=60000)failures_=0; if(failures_<5)++failures_;failedAt_=now; }
  void issue(const char* token, uint32_t now) { clear();if(strlen(token)!=32)return;memcpy(token_,token,32);issued_=now;failures_=0; }
  bool valid(const char* token, uint32_t now) const {
    if(!token_[0]||strlen(token)!=32||uint32_t(now-issued_)>=LIFE_MS)return false;
    uint8_t diff=0;for(size_t i=0;i<32;++i)diff|=uint8_t(token_[i]^token[i]);return diff==0;
  }
  void clear() { memset(token_,0,sizeof(token_)); }
};
}
