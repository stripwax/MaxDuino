#include "Arduino.h"
#include "configs.h"
#include "EEPROM_wrappers.h"

#ifdef USES_EEPROM

#if defined(__AVR__)
  #include <EEPROM.h>
  static void EEPROM_put(uint16_t address, byte data) {EEPROM.put(address,data);}
  static void EEPROM_get(uint16_t address, byte &data) {EEPROM.get(address,data);}
  void EEPROM_init() {};
  void EEPROM_write_logo_begin() {};
  void EEPROM_write_logo_end() {};

  void EEPROM_read_configbyte(byte &data) {
    EEPROM_get(EEPROM_CONFIG_BYTEPOS, data);
  }

  void EEPROM_write_configbyte(byte data) {
    EEPROM_put(EEPROM_CONFIG_BYTEPOS, data);
  }

  void EEPROM_read_logo_byte(uint16_t address, byte& data) {
    EEPROM_get(address, data);
  }

  void EEPROM_write_logo_byte(uint16_t address, byte data) {
    EEPROM_put(address, data);
  }


#elif defined(__arm__) && defined(__STM32F1__)
  #include <EEPROM.h>
  void EEPROM_init() {EEPROM.init();}
  static void EEPROM_get(uint16_t address, byte &data) {
    data = (byte)(EEPROM.read(address) & 0xff);  
  } 
  static void EEPROM_put(uint16_t address, byte data) {
    EEPROM.write(address, (uint16_t) data); 
  }

  void EEPROM_write_logo_begin() {};
  void EEPROM_write_logo_end() {};

  void EEPROM_read_configbyte(byte &data) {
    EEPROM_get(EEPROM_CONFIG_BYTEPOS, data);
  }

  void EEPROM_write_configbyte(byte data) {
    EEPROM_put(EEPROM_CONFIG_BYTEPOS, data);
  }

  void EEPROM_read_logo_byte(uint16_t address, byte& data) {
    EEPROM_get(address, data);
  }

  void EEPROM_write_logo_byte(uint16_t address, byte data) {
    EEPROM_put(address, data);
  }

#elif defined(ESP8266) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040)

  // an EEPROM library that emulates EEPROM using flash (which is not wiped when new firmware is flashed)
  // ESP8266 uses a differently-nmamed library, just to be awkward, even if it does the same thing
  #if defined(ESP8266)
  #include <ESP_EEPROM.h>
  #else
  #include <EEPROM.h>
  #endif
  
  void EEPROM_init() {
      EEPROM.begin(1025);
  }

  static void EEPROM_get(uint16_t address, byte &data) {
    EEPROM.get(address, data) ;  
  }
  static void EEPROM_put(uint16_t address, byte data) {
    EEPROM.put(address, data); 
  }

  void EEPROM_write_logo_begin() {}
  void EEPROM_write_logo_end() {
    EEPROM.commit();
  };

  void EEPROM_read_configbyte(byte &data) {
    EEPROM_get(EEPROM_CONFIG_BYTEPOS, data);
  }

  void EEPROM_write_configbyte(byte data) {
    EEPROM_put(EEPROM_CONFIG_BYTEPOS, data);
    EEPROM.commit();
  }

  void EEPROM_read_logo_byte(uint16_t address, byte& data) {
    EEPROM_get(address, data);
  }

  void EEPROM_write_logo_byte(uint16_t address, byte data) {
    EEPROM_put(address, data);
  }

#elif defined(ARDUINO_ESP32C3_DEV) || defined(CONFIG_IDF_TARGET_ESP32C3)
  #include <Preferences.h>
  Preferences prefs;
  
  #if defined(LOAD_EEPROM_LOGO) || defined(RECORD_EEPROM_LOGO)
  byte EEPROM_logo_bytes[1024];
  #endif

  void EEPROM_init() {
  #if defined(LOAD_EEPROM_LOGO) || defined(RECORD_EEPROM_LOGO)
    prefs.begin("maxduino", true);
    prefs.getBytes("logo", EEPROM_logo_bytes, 1024);
    prefs.end();
  #endif
  }

  void EEPROM_read_configbyte(byte &data) {
    prefs.begin("maxduino", true);
    data = prefs.getUChar("config", 0);
    prefs.end();
  }

  void EEPROM_write_configbyte(byte data) {
    prefs.begin("maxduino", false);
    prefs.putUChar("config", data);
    prefs.end();
  }

  #if defined(LOAD_EEPROM_LOGO) || defined(RECORD_EEPROM_LOGO)
  void EEPROM_write_logo_begin() {
  }

  void EEPROM_write_logo_end() {
    prefs.begin("maxduino", false);
    prefs.putBytes("logo", EEPROM_logo_bytes, 1024);
    prefs.end();
  }

  void EEPROM_read_logo_byte(uint16_t address, byte& data) {
    data = EEPROM_logo_bytes[address];
  }

  void EEPROM_write_logo_byte(uint16_t address, byte data) {
    // all writes buffered here so that we can write a single byte array to emulated eeprom
    EEPROM_logo_bytes[address] = data;
  }
  #endif

#else
  #error "EEPROM support required but no EEPROM library or compatibility layer has been implemented for this target"
#endif

#endif // USES_EEPROM
