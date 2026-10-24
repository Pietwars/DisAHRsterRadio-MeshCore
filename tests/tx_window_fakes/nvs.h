#pragma once
#include <stdint.h>
#include <string.h>
#include <cassert>
typedef int esp_err_t;
typedef unsigned nvs_handle_t;
enum { ESP_OK=0, ESP_FAIL=1, NVS_READWRITE=1 };
namespace fakeNvs {
static uint32_t persisted=0, staged=0;
static bool present=false, openOK=true, readOK=true, setOK=true, commitOK=true;
static unsigned sets=0, commits=0;
inline void reset() {
  persisted=staged=0; present=false;
  openOK=readOK=setOK=commitOK=true; sets=commits=0;
}
}
inline esp_err_t nvs_open(const char* name,int mode,nvs_handle_t* h) {
  assert(!strcmp(name,"dar-txwin")&&mode==NVS_READWRITE);
  *h=1; return fakeNvs::openOK ? ESP_OK : ESP_FAIL;
}
inline esp_err_t nvs_get_u32(nvs_handle_t,const char* key,uint32_t* out) {
  assert(!strcmp(key,"marker"));
  if (!fakeNvs::readOK||!fakeNvs::present) return ESP_FAIL;
  *out=fakeNvs::persisted; return ESP_OK;
}
inline esp_err_t nvs_set_u32(nvs_handle_t,const char* key,uint32_t value) {
  assert(!strcmp(key,"marker")); ++fakeNvs::sets;
  if (!fakeNvs::setOK) return ESP_FAIL;
  fakeNvs::staged=value; return ESP_OK;
}
inline esp_err_t nvs_commit(nvs_handle_t) {
  ++fakeNvs::commits;
  if (!fakeNvs::commitOK) return ESP_FAIL;
  fakeNvs::persisted=fakeNvs::staged; fakeNvs::present=true; return ESP_OK;
}
inline void nvs_close(nvs_handle_t) {}
