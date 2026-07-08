#ifndef CURRENT_SETTINGS_H_INCLUDED
#define CURRENT_SETTINGS_H_INCLUDED

#include "Arduino.h" // for types
#include "configs.h"

#if defined(RECORD)
enum class RecordFormat : byte {
  TZX_ID15 = 0,
  CAS_MSX = 1,
  ZX_SPECTRUM = 2,
  SHARP_MZF = 3,
  _COUNT,  /* put this after the end, so it's always equals one after the last one */
  _NONE
};
#define RECORD_FORMAT_MASK 0x07  /* bit mask for record format settings within eeprom stored byte */
extern RecordFormat recordFormat;
#endif

extern word BAUDRATE;
extern bool mselectMask;
extern bool TSXCONTROLzxpolarityUEFSWITCHPARITY;
extern bool skip2A;

#ifdef LOAD_EEPROM_SETTINGS
void updateEEPROM();
void loadEEPROM();
#endif

#endif // CURRENT_SETTINGS_H_INCLUDED
