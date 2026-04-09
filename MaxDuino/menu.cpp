/**************************************************************
 *                  Casduino Menu Code:
 *  Menu Button (was motor control button) opens menu
 *  up/down move through menu, play select, stop back
 *  Menu Options:
 *  Baud:
 *    1200
 *    2400
 *    3150
 *    3600
 *    3850
 *  
 *  MotorControl:
 *    On
 *    Off
 *  
 *  Save settings to eeprom on exit. 
 */

#include "configs.h"
#include "Arduino.h"
#include "compat.h"
#include "buttons.h"
#include "Display.h"
#include "product_strings.h"
#include "current_settings.h"

#if defined(lineaxy)
#define M_LINE2 lineaxy
#else
#define M_LINE2 1
#endif

enum MenuItems{
  VERSION,
  BAUD_RATE,
#ifdef RECORD
  RECORD_TYPE,
#endif
#ifndef NO_MOTOR
  MOTOR_CTL,
#endif
  TSX_POL_UEFSW,
#ifdef MenuBLK2A
  BLK2A,
#endif
  _Num_Menu_Items
};

const char MENU_ITEM_VERSION[] PROGMEM = "Version...";
const char MENU_ITEM_BAUD_RATE[] PROGMEM = "Baud Rate ?";
#ifdef RECORD
const char MENU_ITEM_RECORD_TYPE[] PROGMEM = "Record Type ?";
#endif
#ifndef NO_MOTOR
const char MENU_ITEM_MOTOR_CTRL[] PROGMEM = "Motor Ctrl ?";
#endif
const char MENU_ITEM_TSX[] PROGMEM = "TSXCzxpUEFSW ?";
#ifdef MenuBLK2A
const char MENU_ITEM_BLK2A[] PROGMEM = "Skip BLK:2A ?";
#endif
const char* const MENU_ITEMS[] PROGMEM = {
  MENU_ITEM_VERSION,
  MENU_ITEM_BAUD_RATE,
#ifdef RECORD
  MENU_ITEM_RECORD_TYPE,
#endif
#ifndef NO_MOTOR
  MENU_ITEM_MOTOR_CTRL,
#endif
  MENU_ITEM_TSX,
#ifdef MenuBLK2A
  MENU_ITEM_BLK2A,
#endif
};

const word BAUDRATES[] PROGMEM = {1200, 2400, 3150, 3600, 3850};

#ifdef RECORD
static byte recordTypeIndexFromFormat(const RecordFormat format)
{
  byte index = 0;

  #if defined(RECORD_TZX_ID15)
    if (format == RecordFormat::TZX_ID15) {
      return index;
    }
    index++;
  #endif

  #if defined(RECORD_ZX_SPECTRUM)
    if (format == RecordFormat::ZX_SPECTRUM) {
      return index;
    }
    index++;
  #endif

  #if defined(RECORD_CAS_MSX) && defined(Use_CAS)
    if (format == RecordFormat::CAS_MSX) {
      return index;
    }
    index++;
  #endif

  #if defined(RECORD_SHARP_MZF)
    if (format == RecordFormat::SHARP_MZF) {
      return index;
    }
  #endif

  return 0;
}

static byte recordTypeCount()
{
  byte count = 0;
  #if defined(RECORD_TZX_ID15)
    count++;
  #endif
  #if defined(RECORD_ZX_SPECTRUM)
    count++;
  #endif
  #if defined(RECORD_CAS_MSX) && defined(Use_CAS)
    count++;
  #endif
  #if defined(RECORD_SHARP_MZF)
    count++;
  #endif
  return count;
}

static RecordFormat recordTypeFormatFromIndex(const byte index)
{
  byte next = 0;

  #if defined(RECORD_TZX_ID15)
    if (index == next) {
      return RecordFormat::TZX_ID15;
    }
    next++;
  #endif

  #if defined(RECORD_ZX_SPECTRUM)
    if (index == next) {
      return RecordFormat::ZX_SPECTRUM;
    }
    next++;
  #endif

  #if defined(RECORD_CAS_MSX) && defined(Use_CAS)
    if (index == next) {
      return RecordFormat::CAS_MSX;
    }
    next++;
  #endif

  #if defined(RECORD_SHARP_MZF)
    if (index == next) {
      return RecordFormat::SHARP_MZF;
    }
  #endif

  #if defined(RECORD_TZX_ID15)
    return RecordFormat::TZX_ID15;
  #elif defined(RECORD_ZX_SPECTRUM)
    return RecordFormat::ZX_SPECTRUM;
  #elif defined(RECORD_CAS_MSX) && defined(Use_CAS)
    return RecordFormat::CAS_MSX;
  #elif defined(RECORD_SHARP_MZF)
    return RecordFormat::SHARP_MZF;
  #else
    return RecordFormat::TZX_ID15;
  #endif
}

static void doRecordTypeSubmenu()
{
  byte subItem = recordTypeIndexFromFormat(recordFormat);
  const byte maxSubItem = recordTypeCount() - 1;
  bool updateScreen = true;
  lastbtn = true;
  while(!button_stop() || lastbtn) {
    if(button_down() && !lastbtn) {
      if(subItem < maxSubItem) subItem += 1;
      lastbtn = true;
      updateScreen = true;
    }
    if(button_up() && !lastbtn) {
      if(subItem > 0) subItem += -1;
      lastbtn = true;
      updateScreen = true;
    }

    const RecordFormat format = recordTypeFormatFromIndex(subItem);

    if(button_play() && !lastbtn) {
      recordFormat = format;
      #if defined(LOAD_EEPROM_SETTINGS) && !defined(ESP32) && !defined(ARDUINO_ARCH_ESP32)
        updateEEPROM();
      #endif
      lastbtn = true;
      updateScreen = true;
    }

    if(updateScreen) {
      switch (format) {
        case RecordFormat::ZX_SPECTRUM:
          strcpy_P((char *)input, PSTR("ZX Spectrum"));
          break;
        case RecordFormat::CAS_MSX:
          strcpy_P((char *)input, PSTR("CAS MSX"));
          break;
        case RecordFormat::SHARP_MZF:
          strcpy_P((char *)input, PSTR("Sharp MZF"));
          break;
        case RecordFormat::TZX_ID15:
        default:
          strcpy_P((char *)input, PSTR("TZX ID15"));
          break;
      }
      if(recordFormat == format) {
        strcat_P((char *)input, PSTR(" *"));
      }
      printtext((char *)input, M_LINE2);
      updateScreen = false;
    }

    checkLastButton();
  }
}
#endif

void doOnOffSubmenu(bool& refVar)
{
  bool updateScreen=true;
  lastbtn=true;
  while(!button_stop() || lastbtn) {
    if(updateScreen) {
      if(refVar==0) printtextF(PSTR("off *"), M_LINE2);
      else  printtextF(PSTR("ON *"), M_LINE2);
      updateScreen=false;
    }
    
    if(button_play() && !lastbtn) {
      refVar = !refVar;
      #if defined(LOAD_EEPROM_SETTINGS) && !defined(ESP32) && !defined(ARDUINO_ARCH_ESP32)
        updateEEPROM();
      #endif
      lastbtn=true;
      updateScreen=true;
      #if defined(OLED1306) && defined(OSTATUSLINE) 
        OledStatusLine();
      #endif              
    }
    checkLastButton();
  }
}

void menuMode()
{ 
  byte menuItem=0;
  byte subItem=0;
  bool updateScreen=true;
  
  while(!button_stop() || lastbtn)
  {
    if(updateScreen) {
      printtextF(PSTR("Menu"),0);
      printtextF((char *)(pgm_read_ptr(&(MENU_ITEMS[menuItem]))), M_LINE2);
      updateScreen=false;
    }
    if(button_down() && !lastbtn){
      if(menuItem<MenuItems::_Num_Menu_Items-1) menuItem+=1;
      lastbtn=true;
      updateScreen=true;
    }
    if(button_up() && !lastbtn) {
      if(menuItem>0) menuItem+=-1;
      lastbtn=true;
      updateScreen=true;
    }
    if(button_play() && !lastbtn) {
      printtextF((char *)(pgm_read_ptr(&(MENU_ITEMS[menuItem]))), 0);
      switch(menuItem){
        case MenuItems::VERSION:
          printtextF(P_VERSION, M_LINE2);
          lastbtn=true;
          while(!button_stop() || lastbtn) {
            checkLastButton();
          }
        break;

        case MenuItems::BAUD_RATE:
          subItem=0;
          updateScreen=true;
          lastbtn=true;
          while(!button_stop() || lastbtn) {
            if(button_down() && !lastbtn){
              if(subItem<4) subItem+=1;
              lastbtn=true;
              updateScreen=true;
            }
            if(button_up() && !lastbtn) {
              if(subItem>0) subItem+=-1;
              lastbtn=true;
              updateScreen=true;
            }

            const word baudrate = pgm_read_word(&(BAUDRATES[subItem]));

            if(button_play() && !lastbtn) {
              BAUDRATE = baudrate;
              #if defined(LOAD_EEPROM_SETTINGS) && !defined(ESP32) && !defined(ARDUINO_ARCH_ESP32)
                updateEEPROM();
              #endif
              updateScreen=true;
              #if defined(OLED1306) && defined(OSTATUSLINE) 
                OledStatusLine();
              #endif
              lastbtn=true;
            }

            if(updateScreen) {
              utoa(baudrate, (char *)input, 10);
              if(BAUDRATE == baudrate) {
                strcat_P((char *)input, PSTR(" *"));
              }
              printtext((char *)input, M_LINE2);
              updateScreen=false;
            }
                    
            checkLastButton();
          }
        break;

        #ifdef RECORD
          case MenuItems::RECORD_TYPE:
            doRecordTypeSubmenu();
            break;
        #endif

        #ifndef NO_MOTOR
          case MenuItems::MOTOR_CTL:
            doOnOffSubmenu(mselectMask);
            break;
        #endif

        case MenuItems::TSX_POL_UEFSW:
          doOnOffSubmenu(TSXCONTROLzxpolarityUEFSWITCHPARITY);
          break;
          
        #ifdef MenuBLK2A
          case MenuItems::BLK2A:
            doOnOffSubmenu(skip2A);
            break;
        #endif     
      }
      lastbtn=true;
      updateScreen=true;
    }
    checkLastButton();
  }
  #ifdef LOAD_EEPROM_SETTINGS
    updateEEPROM();
  #endif
  debounce(button_stop);
}


