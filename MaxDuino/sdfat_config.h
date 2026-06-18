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

#if defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040)
#define SD_CONFIG SdSpiConfig(chipSelect, SHARED_SPI, SD_SPI_CLOCK_SPEED, &SPI1)
#else
#define SD_CONFIG SdSpiConfig(chipSelect, SHARED_SPI, SD_SPI_CLOCK_SPEED)
#endif

#endif // SDFAT_CONFIG_H_INCLUDED
