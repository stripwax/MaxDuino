#ifndef CURRENT_SETTINGS_H_INCLUDED
#define CURRENT_SETTINGS_H_INCLUDED

#include "Arduino.h" // for types

enum class RecordFormat : byte {
  TZX_ID15 = 0,
  CAS_MSX = 1,
  ZX_SPECTRUM = 2,
  SHARP_MZF = 3,
};

extern word BAUDRATE;
extern RecordFormat recordFormat;
extern bool mselectMask;
extern bool TSXCONTROLzxpolarityUEFSWITCHPARITY;
extern bool skip2A;

#ifdef LOAD_EEPROM_SETTINGS
void updateEEPROM();
void loadEEPROM();
#endif

#endif // CURRENT_SETTINGS_H_INCLUDED
