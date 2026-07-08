#include "configs.h"
#include "Arduino.h"
#include "current_settings.h"
#include "casProcessing.h"
#include "EEPROM_wrappers.h"

word BAUDRATE = DEFAULT_BAUDRATE;

#if defined(RECORD)
#include "record.h"

RecordFormat recordFormat = defaultRecordFormat();

static void encodeRecordFormat(byte &settings)
{
  settings &= ~RECORD_FORMAT_MASK;
  settings |= static_cast<byte>(recordFormat);
}

static bool decodeRecordFormat(const byte settings_byte, RecordFormat &format)
{
  byte record_format = settings_byte & RECORD_FORMAT_MASK;

  switch (record_format) {
    case static_cast<byte>(RecordFormat::TZX_ID15):
      format = RecordFormat::TZX_ID15;
      return true;
    case static_cast<byte>(RecordFormat::CAS_MSX):
      format = RecordFormat::CAS_MSX;
      return true;
    case static_cast<byte>(RecordFormat::ZX_SPECTRUM):
      format = RecordFormat::ZX_SPECTRUM;
      return true;
    case static_cast<byte>(RecordFormat::SHARP_MZF):
      format = RecordFormat::SHARP_MZF;
      return true;
    default:
      return false;
  }
}
#endif

#ifndef NO_MOTOR
bool mselectMask = DEFAULT_MSELECTMASK;
#endif
bool TSXCONTROLzxpolarityUEFSWITCHPARITY = DEFAULT_TSXzxpUEF;
bool skip2A = DEFAULT_SKIP2A;

#ifdef LOAD_EEPROM_SETTINGS

void updateEEPROM()
{
    /* Setting Byte: 
    *  bit 0: 1200
    *  bit 1: 2400
    *  bit 2: 3150
    *  bit 3: 3600
    *  bit 4: 3850
    *  bit 5: BLK_2A control
    *  bit 6: TSXCONTROLzxpolarityUEFSWITCHPARITY
    *  bit 7: Motor control
    */
    byte settings=0;

    switch(BAUDRATE) {
      case 1200:
      settings |=1;
      break;
      case 2400:
      settings |=2;
      break;
      case 3150:
      settings |=4;
      break;    
      case 3600:
      settings |=8;  
      break;      
      case 3850:
      settings |=16;
      break;     
    }

    #ifndef NO_MOTOR
      if(mselectMask) settings |=128;
    #endif

    if(TSXCONTROLzxpolarityUEFSWITCHPARITY) settings |=64;
    
    #ifdef MenuBLK2A
      if(skip2A) settings |=32;
    #endif

    EEPROM_write_configbyte(settings);

    #if defined(RECORD)
    byte recordsettings;
    encodeRecordFormat(recordsettings);
    EEPROM_write_record_configbyte(recordsettings);
    #endif

    #ifdef Use_CAS
    setCASBaud();
    #endif
}

void loadEEPROM()
{
    byte settings=0;
    EEPROM_read_configbyte(settings);
        
    if(!settings) return;
    
    #ifndef NO_MOTOR
      mselectMask=bitRead(settings,7);
    #endif

    TSXCONTROLzxpolarityUEFSWITCHPARITY=bitRead(settings,6);
    
    #ifdef MenuBLK2A
      skip2A=bitRead(settings,5);
    #endif
    
    if(bitRead(settings,0)) {
      BAUDRATE=1200;
    }
    if(bitRead(settings,1)) {
      BAUDRATE=2400;
    }
    if(bitRead(settings,2)) {
      BAUDRATE=3150;  
    }
    if(bitRead(settings,3)) {
      BAUDRATE=3600;  
    }
    if(bitRead(settings,4)) {
      BAUDRATE=3850;  
    }
    
    #if defined(RECORD)
    byte rawRecordSettings;
    EEPROM_read_record_configbyte(rawRecordSettings);
    RecordFormat loadedRecordFormat;
    if (decodeRecordFormat(rawRecordSettings, loadedRecordFormat) &&
        isRecordFormatSupported(loadedRecordFormat)) {
      recordFormat = loadedRecordFormat;
    }
    else
    {
      recordFormat = defaultRecordFormat();
    }
    #endif

    #if defined(Use_CAS)
      setCASBaud();
    #endif
}
#endif
