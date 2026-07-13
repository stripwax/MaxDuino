////////////////                             CONFIG FOR STM32                                //////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*                 Add // at the beginning of lines to comment and remove selected option                                */
//**************************************  OPTIONAL USE TO SAVE SPACE  ***************************************************//
#define Use_MENU                          // removing menu saves space
#define AYPLAY
#define MenuBLK2A
#define ID11CDTspeedup
#define ZX81SPEEDUP
#define Use_MZF
#define Use_MTX
#define Use_CAQ
//#define Use_CG                      // Colour Genie .cas/.cgc files playback
#define Use_c64
#define tapORIC
    #define ORICSPEEDUP
#define Use_CAS                           // .cas files playback on MSX / Dragon / CoCo Tandy computers
    //#define Use_TRS80                   // TRS-80 .cas files playback
    #define Use_DRAGON
        #define Use_Dragon_sLeader        // short Leader of 0x55 allowed for loading TOSEC files
            #define Expand_All            // Expand short Leaders in ALL file header blocks.        
#define Use_UEF                           // .uef files playback on BBC Micro / Electron / Atom computers
    #define Use_c112                      // integer gap chunk for .uef
    #define Use_hqUEF                     // .hq.uef files playback on BBC Micro / Electron / Atom computers
        #define Use_c104                  // defined tape format data block: data bits per packet/parity/stop bits    
        //#define Use_c114                // security cycles replaced with carrier tone
        //#define Use_c116                // floating point gap chunk for .hq.uef
        #define Use_c117                // data encoding format change for 300 bauds

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*                                   Configure your screen settings here                                                  */
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Set defines for various types of screen

//#define SERIALSCREEN              // For testing and debugging
//#define FREERAM                   // Changing filenameLength from 255 to 190
//#define LARGEBUFFER               // small buffer size used by default to free RAM

//#define LCD_I2C_ADDR    0x27        // Set the i2c address of your 1602LCD usually 0x27
//#define LCD_I2C_ADDR    0x3f        // Set the i2c address of your 1602LCD usually 0x3f
//#define LCDSCREEN16x2             // Set if you are using a 1602 LCD screen

//#define OLED_SETCONTRAS   0xcf      // Override default value inside Diplay.ino, bigger to increase output current per segment
#define OLED_ROTATE180
#define OLED_address   0x3C           //0x3C or 0x3D
#define OLED1306                      // Set if you are using OLED 1306 display
    #define OLED1306_128_64         // 128x64 resolution with 8 rows
    //#define OLED1106_1_3            // Use this line as well if you have a 1.3" OLED screen
    //#define video64text32    
//#define P8544                       // Set if you are Display Nokia 5110 display

//#define btnRoot_AS_PIVOT
  #define SHOW_DIRPOS
      //#define SHOW_STATUS_LCD
      //#define SHOW_DIRNAMES
      
  #define SHOW_BLOCKPOS_LCD
  
//#define XY                         // use original settings for Oled line 0,1 and status for menu
#define XY2                      // use double size font wihtout status line for menu
#define XY2force                    // Use with care: delay interrupts and crash with other options, needs I2CFAST

#define SHOW_CNTR
#define SHOW_PCT
#define CNTRBASE 100                // 100 for sss, 60 for m:ss (sorry, no space for separator)
//#define ONPAUSE_POLCHG              // 
#define BLOCKMODE                   // REW or FF a block when in pause and Play to select it
#define BLKSJUMPwithROOT            // use menu button in pause mode to switch blocks to jump
#define BM_BLKSJUMP 20               // when menu pressed in pause mode, how may blocks to jump with REW OR FF
#define BLKBIGSIZE                   // max number of block > 255
#define OLEDBLKMATCH               // Match block numbers with REW/FF
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#define SPLASH_SCREEN   1  // Displays the logo and welcome text at the initialization and remains until a button is pressed.
#define TIMEOUT_RESET   60 // Timeout for reset tzxduino (without pause or play activated), comment to not reset.
//#define BLOCKID_INTO_MEM              // enable for blockid recording and later rewinding if EEPROM_PUT is disabled.
#define BLOCKID_NOMEM_SEARCH          // Loop and search for a block
#define maxblock 99                   // maxblock if not using EEPROM
//#define BLOCKID15_IN
#define BLOCKID19_IN                  // trace id19 block for zx81 .tzx to be rewinded
#define BLOCKID21_IN
#define BLOCKTAP_IN
#define OLEDPRINTBLOCK 
#define LOAD_EEPROM_SETTINGS
#define EEPROM_CONFIG_BYTEPOS  1023     // Byte position to save configuration
#define OSTATUSLINE
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// EEPROM LOGO. How to move to EEPROM, saving memory:
// Phase 1: Uncomment RECORD_EEPROM_LOGO define , this copies logo from memory to EEPROM. Compile the sketch.
// Phase 2:  Comment RECORD_EEPROM define, uncomment LOAD_EEPROM define. Complile the sketch again 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Also it's posible to select record and load both for better testing new logo activation, pressing MENU simulates a reset.
// And both can be deactivated also showing a black screen.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//#define COMPRESS_REPEAT_ROW
//#define EEPROM_LOGO_COMPRESS
#define LOAD_MEM_LOGO             // legacy, logo is not in EEPROM then wasting memory.
//#define RECORD_EEPROM_LOGO        // Uncommenting RECORD_EEPROM deactivates #define Use_MENU
//#define LOAD_EEPROM_LOGO 
#define LOGO_FADE_IN 2500 /* Number of milliseconds for logo animation */

// for list of logos, see filenames in "logos" folder, and remove the logo_ prefix from the filename
// either use the below defines, or use -DLOGO
#define LOGO_128_64 cablemax
#define LOGO_128_32 LOGOMAXDUINO2
#define LOGO_84_48 LOGOMAXDUINO2

/////////////////////
//      FONTS      //
/////////////////////
//#define DoubleFont

#define FONT8x8 cartoonFont
#define FONT8x16 cartoon8x16
