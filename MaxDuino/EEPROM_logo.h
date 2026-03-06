#ifndef EEPROM_logo_h_included
#define EEPROM_logo_h_included

#ifdef RECORD_EEPROM_LOGO
void write_logo_to_eeprom(const byte* logoptr);
  #ifdef SDCARD_RECORD_EEPROM_LOGO
  #error We do not support writing to eeprom from sd AND firmware in the same image
  #endif
#elif defined(SDCARD_RECORD_EEPROM_LOGO)
void write_sdcard_logo_to_eeprom();
#endif

#endif // EEPROM_logo_h_included