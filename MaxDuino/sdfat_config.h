#ifndef SDFAT_CONFIG_H_INCLUDED
#define SDFAT_CONFIG_H_INCLUDED

#include "configs.h"

// default SPI clock speed
#ifndef SD_SPI_CLOCK_SPEED
  #if defined(ARDUINO_ESP32C3_DEV) || defined(CONFIG_IDF_TARGET_ESP32C3) || defined(ESP8266)
    #ifndef SD_SPI_CLOCK_SPEED
      #define SD_SPI_CLOCK_SPEED SD_SCK_MHZ(4)
    #endif
  #endif

  #ifndef SD_SPI_CLOCK_SPEED
    #define SD_SPI_CLOCK_SPEED SPI_FULL_SPEED
  #endif
#endif

#ifndef SDFAT_FILE_TYPE
// always build without exfat support (unless a hwconfig/userconfig has explicitly enabled it)
#define SDFAT_FILE_TYPE 1
#endif

// XIAO RP2040/RP2350 use SPI0 (pins D8=GPIO2 SCK, D10=GPIO3 MOSI, D9=GPIO4 MISO).
// Pico and other boards use SPI1 (pins GPIO10/11/12).
#if defined(ARDUINO_SEEED_XIAO_RP2040) || defined(ARDUINO_SEEED_XIAO_RP2350)
#define SD_CONFIG SdSpiConfig(chipSelect, SHARED_SPI, SD_SPI_CLOCK_SPEED, &SPI)
#elif defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040)
#define SD_CONFIG SdSpiConfig(chipSelect, SHARED_SPI, SD_SPI_CLOCK_SPEED, &SPI1)
#else
#define SD_CONFIG SdSpiConfig(chipSelect, SHARED_SPI, SD_SPI_CLOCK_SPEED)
#endif

#endif // SDFAT_CONFIG_H_INCLUDED
