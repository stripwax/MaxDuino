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

  #if defined(RECORD)
  void EEPROM_read_record_configbyte(byte &data) {
    EEPROM_get(EEPROM_RECORD_CONFIG_BYTEPOS, data);
  }

  void EEPROM_write_record_configbyte(byte data) {
    EEPROM_put(EEPROM_RECORD_CONFIG_BYTEPOS, data);
  }
  #endif

  void EEPROM_read_logo_byte(uint16_t address, byte& data) {
    EEPROM_get(address, data);
  }

  void EEPROM_write_logo_byte(uint16_t address, byte data) {
    // protect config bytes, don't overwrite them
    // should optimise to a no-op for devices where EEPROM_CONFIG_BYTES doesn't overlap with logo storage
    #if defined(RECORD)
    if (address%1023 != EEPROM_CONFIG_BYTEPOS && address%1023 != EEPROM_RECORD_CONFIG_BYTEPOS)
    #else
    if (address%1023 != EEPROM_CONFIG_BYTEPOS)
    #endif
    {
      EEPROM_put(address, data);
    }
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
    // protect config bytes, don't overwrite them
    // should optimise to a no-op for devices where EEPROM_CONFIG_BYTES doesn't overlap with logo storage
    #if defined(RECORD)
    if (address%1023 != EEPROM_CONFIG_BYTEPOS && address%1023 != EEPROM_RECORD_CONFIG_BYTEPOS)
    #else
    if (address%1023 != EEPROM_CONFIG_BYTEPOS)
    #endif
    {
      EEPROM_put(address, data);
    }
  }

#elif defined(__SAMD21__) || defined(__SAMD21G18A__) || defined(ESP8266) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2350)

  // All these platforms provide a flash-backed EEPROM emulation library
  // with a common EEPROM.get/put/commit API, even if the include differs.
  #if defined(__SAMD21__) || defined(__SAMD21G18A__)
    #define EEPROM_EMULATION_SIZE 1025
    #include <FlashStorage_SAMD.h>
  #elif defined(ESP8266)
    #include <ESP_EEPROM.h>
  #else
    #include <EEPROM.h>
  #endif

  #if defined(__SAMD21__) || defined(__SAMD21G18A__)
    // EEPROM locator for extra_script.py — bookended markers
    // that encode the runtime flash address and size of the
    // EEPROM backing store (_data_eeprom_storage).  extra_script.py
    // searches for "MAXD" in the bossac dump, reads addr+size,
    // verifies "DXAM", then extracts/embeds the EEPROM data at
    // the correct offset — no hardcoded flash address needed.
    static const struct {
      uint32_t magic1;   // "MAXD" = 0x4458414D
      uint32_t addr;     // flash address of _data_eeprom_storage
      uint32_t size;     // flash-aligned byte count
      uint32_t magic2;   // "DXAM" = 0x4D415844
    } eeprom_locator __attribute__((used)) = {
      0x4458414D,
      (uint32_t)(uintptr_t)&_dataeeprom_storage[0],
      (uint32_t)sizeof(_dataeeprom_storage),
      0x4D415844
    };
  #endif

  void EEPROM_init() {
    #if defined(__SAMD21__) || defined(__SAMD21G18A__)
      EEPROM.setCommitASAP(false);
      (void)*((volatile uint32_t*)&eeprom_locator);
    #else
      EEPROM.begin(1025);
    #endif
  }

  static void EEPROM_get(uint16_t address, byte &data) {
    EEPROM.get(address, data);
  }

  static void EEPROM_put(uint16_t address, byte data) {
    EEPROM.put(address, data);
  }

  void EEPROM_write_logo_begin() {}

  void EEPROM_write_logo_end() {
    EEPROM.commit();
  }

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
    // protect config bytes, don't overwrite them
    // should optimise to a no-op for devices where EEPROM_CONFIG_BYTES doesn't overlap with logo storage
    #if defined(RECORD)
    if (address%1023 != EEPROM_CONFIG_BYTEPOS && address%1023 != EEPROM_RECORD_CONFIG_BYTEPOS)
    #else
    if (address%1023 != EEPROM_CONFIG_BYTEPOS)
    #endif
    {
      EEPROM_put(address, data);
    }
  }

#elif defined(ESP32_XTENSA) || defined(ESP32_RISCV)
  #include <Preferences.h>
  Preferences prefs;
  
  #if defined(HAS_EEPROM_LOGO)
  byte EEPROM_logo_bytes[1024];
  #endif

  void EEPROM_init() {
  #if defined(HAS_EEPROM_LOGO)
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

  #if defined(HAS_EEPROM_LOGO)
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
