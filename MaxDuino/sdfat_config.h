#ifndef SDFAT_CONFIG_H_INCLUDED
#define SDFAT_CONFIG_H_INCLUDED

// Optional size-saving switch for small AVR targets that only need FAT16/FAT32.
// This disables exFAT support in SdFat and avoids linking the hybrid FsFile path.
#ifdef FAT16_32_Only
  #ifndef SDFAT_FILE_TYPE
    #define SDFAT_FILE_TYPE 1
  #endif
#endif

// default SPI clock speed
#ifndef SD_SPI_CLOCK_SPEED
  #if defined(ESP32) || defined(ESP8266)
    #ifndef SD_SPI_CLOCK_SPEED
      #define SD_SPI_CLOCK_SPEED SD_SCK_MHZ(4)
    #endif
  #endif

  #ifndef SD_SPI_CLOCK_SPEED
    #define SD_SPI_CLOCK_SPEED SPI_FULL_SPEED
  #endif
#endif

#endif // SDFAT_CONFIG_H_INCLUDED
