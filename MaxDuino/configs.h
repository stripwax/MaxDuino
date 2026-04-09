// includes the corresponding user config.h file based on target platform

#ifndef CONFIGS_H_INCLUDED
#define CONFIGS_H_INCLUDED

#include "defines_config.h"

// IF YOU REALLY WANT TO DEFINE YOUR CONFIG VERSION IN THIS HEADER FILE,
// YOU CAN, BY UNCOMMENTING AND CHANGING THE FOLLOWING LINE
// BUT DOING IT VIA BUILD SETTINGS MIGHT BE EASIER
// #define CONFIGFILE 7
// ALTERNATIVELY YOU CAN DEFINE A DEFAULT FOR A PLATFORM WITHOUT CHANGING
// THE DEFAULTS FOR OTHER PLATFORMS
//#define _CONFIG_FILE_DEFAULT_ATMEGA328P 7

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*   OPTION TO HAVE ALL CONFIG PARAMETERS DEFINED ENTIRELY VIA -D / platformio.ini                                      */
#if defined(CONFIGFILE) && CONFIGFILE == -1

  #include "userconfig_blank.h"

#else
  #if defined(__AVR_ATmega2560__)
    #ifndef CONFIGFILE
    #define CONFIGFILE _CONFIG_FILE_DEFAULT_ATMEGA2560
    #endif
    #define CONFIG_PATH userMAXconfig
  #elif defined(__AVR_ATmega4809__) || defined(__AVR_ATmega4808__)
    #ifndef CONFIGFILE
    #define CONFIGFILE _CONFIG_FILE_DEFAULT_EVERY
    #endif
    #define CONFIG_PATH userEVERYconfig
  #elif defined(__arm__) && defined(__STM32F1__)
    #ifndef CONFIGFILE
    #define CONFIGFILE _CONFIG_FILE_DEFAULT_STM32
    #endif
    #define CONFIG_PATH userSTM32config
  #elif defined(SEEED_XIAO_M0)
    #ifndef CONFIGFILE
    #define CONFIGFILE _CONFIG_FILE_DEFAULT_XIAO_M0
    #endif
    #define CONFIG_PATH userSEEEDUINO_XIAO_M0config
  #elif defined(ARDUINO_XIAO_ESP32C3)
    #ifndef CONFIGFILE
    #define CONFIGFILE _CONFIG_FILE_DEFAULT_XIAO_ESP32C3
    #endif
    #define CONFIG_PATH userSEEEDUINO_XIAO_ESP32C3
  #elif defined(ARDUINO_ESP8266_WEMOS_D1MINI)
    #ifndef CONFIGFILE
    #define CONFIGFILE _CONFIG_FILE_DEFAULT_WEMOS_D1MINI_ESP8266
    #endif
    #define CONFIG_PATH userARDUINO_ESP8266_WEMOS_D1MINI
  #elif defined(ARDUINO_D1_MINI32)
    #ifndef CONFIGFILE
    #define CONFIGFILE _CONFIG_FILE_DEFAULT_WEMOS_D1_MINI32_ESP32
    #endif
    #define CONFIG_PATH userARDUINO_D1_MINI32
  #elif defined(MAXDUINO_RP2040)
    #ifndef CONFIGFILE
    #define CONFIGFILE _CONFIG_FILE_DEFAULT_RP2040
    #endif
    #define CONFIG_PATH userRPI_PICOconfig
  #else //__AVR_ATmega328P__
    #include "userconfig.h" // legacy
    #ifndef CONFIGFILE
    #define CONFIGFILE _CONFIG_FILE_DEFAULT_ATMEGA328P
    #endif
    #define CONFIG_PATH userconfig
  #endif

  #include CONFIG_header(CONFIG_PATH, CONFIGFILE)

#endif

// Canonical recorder feature flags:
//   RECORD                 master on/off switch for the recorder subsystem
//   RECORD_TZX_ID15        generic TZX direct-recording mode
//   RECORD_ZX_SPECTRUM     Spectrum-tuned TZX recording mode
//   RECORD_CAS_MSX         native MSX .cas recording mode
//   RECORD_SHARP_MZF       native Sharp MZ .mzf recording mode
//
// Legacy compatibility aliases are kept so older configs continue to build.

// Old recorder master flag.
#if defined(Use_Rec) && !defined(RECORD)
  #define RECORD
#endif

// Old TZX recorder flag enabled both TZX-based record types.
#if defined(REC_TZX)
  #ifndef RECORD
    #define RECORD
  #endif
  #ifndef RECORD_TZX_ID15
    #define RECORD_TZX_ID15
  #endif
  #ifndef RECORD_ZX_SPECTRUM
    #define RECORD_ZX_SPECTRUM
  #endif
#endif

// Legacy MSX experimental / renamed flag.
#if defined(REC_MSX_EXPERIMENTAL) && !defined(RECORD_CAS_MSX)
  #define RECORD_CAS_MSX
#endif
#if defined(REC_CAS_MSX) && !defined(RECORD_CAS_MSX)
  #define RECORD_CAS_MSX
#endif

// Older recorder builds used Use_MZF to gate the native MZF recorder too.
#if defined(Use_MZF) && (defined(REC_TZX) || defined(Use_Rec)) && !defined(RECORD_SHARP_MZF)
  #define RECORD_SHARP_MZF
#endif

// If any recorder format is selected, enable the recorder master switch.
#if !defined(RECORD) && \
    (defined(RECORD_TZX_ID15) || defined(RECORD_ZX_SPECTRUM) || \
     defined(RECORD_CAS_MSX) || defined(RECORD_SHARP_MZF))
  #define RECORD
#endif

// If recording is enabled but no explicit format was selected, fall back to
// the generic TZX ID15 recorder so the build still has a valid default.
#if defined(RECORD) && \
    !defined(RECORD_TZX_ID15) && !defined(RECORD_ZX_SPECTRUM) && \
    !defined(RECORD_CAS_MSX) && !defined(RECORD_SHARP_MZF)
  #define RECORD_TZX_ID15
#endif

// Shared internal helper for the TZX-based recorders.
#if defined(RECORD_TZX_ID15) || defined(RECORD_ZX_SPECTRUM)
  #define RECORD_TZX
#endif

// Keep the legacy names available for any untouched downstream code.
#if defined(RECORD_TZX) && !defined(REC_TZX)
  #define REC_TZX
#endif
#if defined(RECORD_CAS_MSX) && !defined(REC_CAS_MSX)
  #define REC_CAS_MSX
#endif

#endif // CONFIGS_H_INCLUDED
