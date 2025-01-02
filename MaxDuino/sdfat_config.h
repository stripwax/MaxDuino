#ifndef SDFAT_CONFIG_H_INCLUDED
#define SDFAT_CONFIG_H_INCLUDED

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

/*
// Consider some or all of these, to the extend that they might reduce firmware size and/or improve performance on certain devices
// (Note however that if you are using Arduino IDE, these #defines would be ignored anyway. To achieve the same result you would need to
// create your own makefile/preferences or hack the SdFat library code, or make a copy of the SdFat library code...
// PlatformIO is much more flexible in this regard.)
#define SDFAT_FILE_TYPE 1
#define USE_SEPARATE_FAT_CACHE 0
#define USE_EXFAT_BITMAP_CACHE 0
#define USE_MULTI_SECTOR_IO 0
*/

#endif // SDFAT_CONFIG_H_INCLUDED
