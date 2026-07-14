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

#ifdef MENU_HAS_REBOOT
extern void (*resetFunc) (void);
#endif

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
#ifdef MENU_HAS_REBOOT
  REBOOT,
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
#ifdef MENU_HAS_REBOOT
const char MENU_ITEM_REBOOT[] PROGMEM = "REBOOT?";
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
#ifdef MENU_HAS_REBOOT
  MENU_ITEM_REBOOT
#endif
};

const word BAUDRATES[] PROGMEM = {1200, 2400, 3150, 3600, 3850};

#ifdef RECORD
/* we map between the 'supported' formats for this build, which will be the listed menu items,
   and the overall universal set of known recording formats which defines what byte value is stored in config
   The former is customizable (i.e. by choosing what formats to support) so the indexes into the menu
   items change based on config.  This is the 'order of the items in the menu'.
   The latter is not customizable : these are the fixed enum values that identify each format, which we hold constant across builds */
enum recordTypeIndex
{
  #if defined(RECORD_TZX_ID15)
  TZX_ID15,
  #endif
  #if defined(RECORD_CAS_MSX)
  CAS_MSX,
  #endif
  #if defined(RECORD_ZX_SPECTRUM)
  ZX_SPECTRUM,
  #endif
  #if defined(RECORD_SHARP_MZF)
  SHARP_MZF,
  #endif
  _NONE
};

// the below items must be same order as recordTypeIndex
static RecordFormat recordTypeFormatFromIndex[] =
{
    #if defined(RECORD_TZX_ID15)
    RecordFormat::TZX_ID15,
    #endif
    #if defined(RECORD_CAS_MSX)
    RecordFormat::CAS_MSX,
    #endif
    #if defined(RECORD_ZX_SPECTRUM)
    RecordFormat::ZX_SPECTRUM,
    #endif
    #if defined(RECORD_SHARP_MZF)
    RecordFormat::SHARP_MZF,
    #endif
    RecordFormat::_NONE  // ensure this array is at least size 1 to avoid compile errors
};

// the below items must be same order as RecordFormat enums
static const byte recordTypeIndexFromFormat[] = {
  #if defined(RECORD_TZX_ID15)
  recordTypeIndex::TZX_ID15
  #else
  recordTypeIndex::_NONE
  #endif
,
  #if defined(RECORD_CAS_MSX)
  recordTypeIndex::CAS_MSX
  #else
  recordTypeIndex::_NONE
  #endif
,
  #if defined(RECORD_ZX_SPECTRUM)
  recordTypeIndex::ZX_SPECTRUM
  #else
  recordTypeIndex::_NONE
  #endif
,
  #if defined(RECORD_SHARP_MZF)
  recordTypeIndex::SHARP_MZF
  #else
  recordTypeIndex::_NONE
  #endif
};

static void doRecordTypeSubmenu()
{
  byte subItem = recordTypeIndexFromFormat[static_cast<byte>(recordFormat)];
  const byte maxSubItem = sizeof(recordTypeFormatFromIndex) - 2;  // we want the largest index, not the count, so subtract 1, and subtract another 1 due to the dummy 'NONE' item that we need there
  bool updateScreen = true;
  lastbtn = true;
  while(!button_stop() || lastbtn) {
    if(button_down() && !lastbtn) {
      if(subItem < maxSubItem) ++subItem;
      lastbtn = true;
      updateScreen = true;
    }
    if(button_up() && !lastbtn) {
      if(subItem > 0) --subItem;
      lastbtn = true;
      updateScreen = true;
    }

    const RecordFormat format = recordTypeFormatFromIndex[subItem];

    if(button_play() && !lastbtn) {
      recordFormat = format;
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
      printtextF((const char *)(pgm_read_ptr(&(MENU_ITEMS[menuItem]))), M_LINE2);
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
      printtextF((const char *)(pgm_read_ptr(&(MENU_ITEMS[menuItem]))), 0);
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
              updateScreen=true;
              #if defined(OLED1306) && defined(OSTATUSLINE) 
                OledStatusLine();
              #endif
              lastbtn=true;
            }

            if(updateScreen) {
              ultoa(baudrate, (char *)input, 10);
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

        #ifdef MENU_HAS_REBOOT
          case MenuItems::REBOOT:
            clear_display();
            resetFunc();
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
