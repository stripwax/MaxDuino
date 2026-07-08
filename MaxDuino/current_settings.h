#ifndef CURRENT_SETTINGS_H_INCLUDED
#define CURRENT_SETTINGS_H_INCLUDED

#include "Arduino.h" // for types
#include "configs.h"

#if defined(RECORD)
#include "record.h"
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
