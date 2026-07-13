// includes the corresponding user config.h file based on target platform

// The configs are used for multiple things and there is currently overlap:
//  * whether EEPROM logo is supported by target hardware or not;
//  * what prefernces you might have for EEPROM logo and fonts, etc
//  * what hardware capabilities are supported by a standard circuit using this board (e.g. is there a motor output)
//  * which different file types can all be enabled simultaneously and still fit into firmware
//  * how much RAM can be devoted to the various buffer

// Some of these are related to the specific MCU, some related to the board that uses a certain MCU
// (e.g. Seeed form-factor has fewer available GPIOs), some related to the overall MaxDuino device circuit
// using the board (e.g. what specific components like OLED or LCD16x2), some related to just personal
// preferences.

// As a result, there's a lot of copy+pasting across all the config files which is not good practice
// My hope is to improve MaxDuino to separate more cleanly to simplify these distinct kinds
// of 'configuration'

#ifndef CONFIGS_H_INCLUDED
#define CONFIGS_H_INCLUDED

#include "defines_config.h"
#include "defines_board_arch.h"

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
  #elif defined(ESP32_RISCV)
    #ifndef CONFIGFILE
    #define CONFIGFILE _CONFIG_FILE_DEFAULT_XIAO_ESP32C3
    #endif
    #define CONFIG_PATH userSEEEDUINO_XIAO_ESP32C3config
  #elif defined(ARDUINO_ESP8266_WEMOS_D1MINI)
    #ifndef CONFIGFILE
    #define CONFIGFILE _CONFIG_FILE_DEFAULT_WEMOS_D1MINI_ESP8266
    #endif
    #define CONFIG_PATH userARDUINO_ESP8266_WEMOS_D1MINIconfig
  #elif defined(__AVR_ATmega32U4__)
    #ifndef CONFIGFILE
    #define CONFIGFILE _CONFIG_FILE_DEFAULT_ATMEGA32U4
    #endif
    #define CONFIG_PATH userATMEGA32U4config
  #elif defined(__AVR_ATmega328P__)
    #include "userconfig.h" // legacy
    #ifndef CONFIGFILE
    #define CONFIGFILE _CONFIG_FILE_DEFAULT_ATMEGA328P
    #endif
    #define CONFIG_PATH userconfig
  #elif defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2350)
    #ifndef CONFIGFILE
    #define CONFIGFILE _CONFIG_FILE_DEFAULT_RP2040
    #endif
    #define CONFIG_PATH userRPI_PICOconfig
  #elif defined(ESP32_XTENSA)
    #ifndef CONFIGFILE
    #define CONFIGFILE _CONFIG_FILE_DEFAULT_D1_MINI32
    #endif
    #define CONFIG_PATH userD1_MINI32config
  #else
    #if not defined(CONFIGFILE) or not defined(CONFIG_PATH)
    #error Unknown platform for default configs
    #endif
  #endif

  #include CONFIG_header(CONFIG_PATH, CONFIGFILE)

#endif

#include "maxduino_prefs.h"

#include "configs_sanity.h"

#endif // CONFIGS_H_INCLUDED
