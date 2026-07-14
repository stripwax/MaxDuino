// platform-independent wrapper for common eeprom interface

#ifndef EEPROM_H_INCLUDED
#define EEPROM_H_INCLUDED

#include "Arduino.h"
#include "configs.h"

#if defined(RECORD_EEPROM_LOGO) || defined(LOAD_EEPROM_LOGO) || defined(LOAD_EEPROM_LOGO_MEM_FALLBACK) || defined(EEPROM_LOGO_BMP_LOADER)
#define HAS_EEPROM_LOGO
#endif

#if defined(LOAD_EEPROM_SETTINGS) || defined(HAS_EEPROM_LOGO)
#define USES_EEPROM
#endif

#ifdef USES_EEPROM
void EEPROM_init();
void EEPROM_write_logo_begin();
void EEPROM_write_logo_end();
void EEPROM_read_configbyte(byte &data);
void EEPROM_write_configbyte(byte data);
void EEPROM_read_logo_byte(uint16_t address, byte& data);
void EEPROM_write_logo_byte(uint16_t address, byte data);
#if defined(LOAD_EEPROM_LOGO_MEM_FALLBACK)
bool EEPROM_check_logo_valid();
#endif
#if defined(RECORD)
void EEPROM_read_record_configbyte(byte &data);
void EEPROM_write_record_configbyte(byte data);
#endif
#endif

#endif // EEPROM_H_INCLUDED
