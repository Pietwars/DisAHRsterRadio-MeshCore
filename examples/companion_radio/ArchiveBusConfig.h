// SPDX-License-Identifier: MIT
#pragma once
// Radio uses SPIClass() (FSPI on S3); SD must use the other physical controller.
namespace disaster {
#if defined(CONFIG_IDF_TARGET_ESP32S3) && defined(LILYGO_T3S3)
constexpr int sdBus = HSPI;
constexpr int sdSck = 14, sdMiso = 2, sdMosi = 11, sdCs = 13;
#elif defined(CONFIG_IDF_TARGET_ESP32)
constexpr int sdBus = VSPI;
constexpr int sdSck = 14, sdMiso = 2, sdMosi = 15, sdCs = 13;
#else
#error "SD archive requires a verified ESP32 TTGO V1.6 or ESP32-S3 T3-S3 profile"
#endif
static_assert(P_LORA_RESET != sdSck, "LoRa reset conflicts with SD clock");
static_assert(P_LORA_SCLK != sdSck && P_LORA_MOSI != sdMosi && P_LORA_NSS != sdCs,
              "Radio and SD pins must be separate");
}
