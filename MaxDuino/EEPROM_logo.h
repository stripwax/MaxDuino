#ifndef EEPROM_logo_h_included
#define EEPROM_logo_h_included

#include "configs.h"
#include "Arduino.h" // for types

#if defined(LOAD_EEPROM_LOGO) || defined(RECORD_EEPROM_LOGO) || defined(RECORD_EEPROM_LOGO_FROM_SDCARD)
#include "EEPROM_wrappers.h"
#endif

#ifdef RECORD_EEPROM_LOGO
void write_logo_to_eeprom(void);
  #ifdef RECORD_EEPROM_LOGO_FROM_SDCARD
  #error We do not support writing start logo to eeprom from both firmware-embedded image and from sd card, in the same firmware type
  #endif
#endif
#if defined(RECORD_EEPROM_LOGO_FROM_SDCARD)
bool handle_load_logo_file();
void read_display_sdcard_logo(byte invert, bool eeprom_write, bool compress);
#endif

#ifdef LOAD_EEPROM_LOGO
#ifdef EEPROM_LOGO_COMPRESS
byte EEPROM_get_compressed(int x, int y);
#endif
#endif

#endif // EEPROM_logo_h_included