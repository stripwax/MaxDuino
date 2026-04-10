////////////////                 CONFIG FOR RASPBERRY PI PICO / RP2040                 ////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*                 Add // at the beginning of lines to comment and remove selected option                                */

// Conservative SPI clock for first RP2040 bring-up.
#define SD_SPI_CLOCK_SPEED SD_SCK_MHZ(4)

//**************************************  OPTIONAL USE TO SAVE SPACE  ***************************************************//
#define Use_MENU                          // removing menu saves space
#define AYPLAY
#define MenuBLK2A
#define ID11CDTspeedup
#define ZX81SPEEDUP
#define Use_MZF
#define Use_MTX
#define Use_CAQ
#define Use_c64                         // Commodore C64/C16 .tap files with native C64-TAPE-RAW/C16-TAPE-RAW headers
#define c64_invert                      // invert Commodore C64/C16 .tap playback pulse polarity
#define tapORIC
    #define ORICSPEEDUP
#define Use_CAS                           // .cas files playback on MSX / Dragon / CoCo Tandy computers
    #define Use_TRS80                     // TRS-80 .cas files playback
    #define Use_DRAGON
        #define Use_Dragon_sLeader        // short Leader of 0x55 allowed for loading TOSEC files
            #define Expand_All            // Expand short Leaders in ALL file header blocks.
#define Use_UEF                           // .uef files playback on BBC Micro / Electron / Atom computers
    #define Use_c112                      // integer gap chunk for .uef
    #define Use_hqUEF                     // .hq.uef files playback on BBC Micro / Electron / Atom computers
        #define Use_c104                  // defined tape format data block: data bits per packet/parity/stop bits
        #define Use_c117                  // data encoding format change for 300 bauds
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//         UEF file instructions: UEF files are compressed and can not be executed directly in MAXDUINO,                 //
/*         for that you have to decompress these files manually.                                                         */
/*         linux / mac os: gunzip -c game.uef> game.uef.tmp && mv game.uef.tmp game.uef                                  */
/*         windows os: add .gz to file name, then click to extract with winrar                                           */
//***********************************************************************************************************************//
//                                       Set Acorn UEF default speed                                                     //
#define TURBOBAUD1500                 // default setting, 25% faster than 1200 baudios standard speed
//#define TURBOBAUD1550
//#define TURBOBAUD1600

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//** If Use_MENU commented to remove Menu, then vars need setting preference cause no EEPROM for read/write **//
//** These are also the initial first-time defaults when you haven't saved preferences yet **//
#define DEFAULT_BAUDRATE 3850
#define DEFAULT_MSELECTMASK 0   // Motor control state 1=on 0=off
#define DEFAULT_TSXzxpUEF 0     // Multiple flag: rpolarity needed for zx games: Basil the Great Mouse Detective,
                                //            Mask // SpeedControl for .tsx // UEF Switch Parity
#define DEFAULT_SKIP2A 0        // Pause on for BLK:2A

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*                                   Configure your screen settings here                                                  */
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Set defines for various types of screen

//#define SERIALSCREEN              // For testing and debugging
//#define FREERAM                   // Changing filenameLength from 255 to 190
//#define LARGEBUFFER               // small buffer size used by default to free RAM

#define LCD_I2C_ADDR    0x27        // Set the i2c address of your 1602LCD usually 0x27
//#define LCDSCREEN16x2             // Set if you are using a 1602 LCD screen

//#define OLED_SETCONTRAS   0xcf    // Override default value inside Diplay.ino, bigger to increase output current per segment
#define OLED_ROTATE180
#define OLED_address   0x3C         //0x3C or 0x3D
#define OLED1306                    // Set if you are using OLED 1306 display
      #define OLED1306_128_64       // 128x64 resolution with 8 rows
      //#define OLED1106_1_3        // Use this line as well if you have a 1.3" OLED screen
      //#define video64text32
//#define btnRoot_AS_PIVOT
  #define SHOW_DIRPOS
      //#define SHOW_STATUS_LCD
      //#define SHOW_DIRNAMES

  #define SHOW_BLOCKPOS_LCD

//#define XY                         // use original settings for Oled line 0,1 and status for menu
#define XY2                          // use double size font without status line for menu
#define XY2force                     // Use with care: delay interrupts and crash with other options, needs I2CFAST

#define SHOW_CNTR
#define SHOW_PCT
#define CNTRBASE 100                // 100 for sss, 60 for m:ss (sorry, no space for separator)
#define MAXPAUSE_PERIOD   8191      // millis
//#define ONPAUSE_POLCHG
#define BLOCKMODE                   // REW or FF a block when in pause and Play to select it
#define BLKSJUMPwithROOT            // use menu button in pause mode to switch blocks to jump
#define BM_BLKSJUMP 20              // when menu pressed in pause mode, how may blocks to jump with REW OR FF
#define BLKBIGSIZE                  // max number of block > 255
#define OLEDBLKMATCH                // Match block numbers with REW/FF
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#define SPLASH_SCREEN   1         // Displays the logo and welcome text at the initialization and remains until a button is pressed.
#define TIMEOUT_RESET   60          // Timeout for reset tzxduino (without pause or play activated), comment to not reset.
//#define BLOCK_EEPROM_PUT          // EEPROM-backed block storage disabled for first RP2040 bring-up
//#define BLOCKID_INTO_MEM          // enable for blockid recording and later rewinding if EEPROM_PUT is disabled.
#define BLOCKID_NOMEM_SEARCH        // Loop and search for a block
#define maxblock 99                 // maxblock if not using EEPROM
//#define BLOCKID15_IN
#define BLOCKID19_IN                // trace id19 block for zx81 .tzx to be rewinded
#define BLOCKID21_IN
#define BLOCKTAP_IN
#define OLEDPRINTBLOCK
#define OSTATUSLINE
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// RP2040 flash-backed EEPROM emulation is used for menu settings only.
// Keep logo/block storage disabled unless a larger EEPROM layout is planned.
#define LOAD_EEPROM_SETTINGS
#define EEPROM_CONFIG_BYTEPOS 255
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//#define COMPRESS_REPEAT_ROW
//#define EEPROM_LOGO_COMPRESS
#define LOAD_MEM_LOGO
//#define RECORD_EEPROM_LOGO
//#define LOAD_EEPROM_LOGO

// for list of logos, see filenames in "logos" folder, and remove the logo_ prefix from the filename
// either use the below defines, or use -DLOGO
#define LOGO_128_64 maxevery
#define LOGO_128_32 LOGOMAXDUINO2
#define LOGO_84_48 LOGOMAXDUINO2

/////////////////////
//      FONTS      //
/////////////////////
//#define DoubleFont

#define FONT8x8 cartoonFont
#define FONT8x16 cartoon8x16

