/* This file contains inital preferences that are not device specific.
You can change these here and they will affect every device you build.
You can also override these in your userconfigs, or via platformio.ini using -D flags (e.g. -DDEFAULT_BAUDRATE 3850 etc),
since each flag below is guarded with #ifndef .  maxduino_prefs.h is included AFTER configs.h (which loads your userconfigs)
to ensure any custom overrides take precedence. */

#ifndef PREFERENCES_H_INCLUDED
#define PREFERENCES_H_INCLUDED

#ifndef scrollSpeed
#define scrollSpeed   250           //text scroll delay
#endif

#ifndef scrollWait  
#define scrollWait    3000          //Delay before scrolling starts
#endif

#ifndef UEF_TURBOBAUD
// Set Acorn UEF 'turbo' speed (when MaxDuino BAUDRATE is set greater than 1200)
// Supported values currently as below, 1200 (no turbo), 1500, 1550, 1600
//#define UEF_TURBOBAUD 1200               // effectively always standard speed, no turbo
#define UEF_TURBOBAUD 1500                 // recommended turbo setting, 25% faster than 1200 baud standard speed
//#define UEF_TURBOBAUD 1550
//#define UEF_TURBOBAUD 1600
#endif

// Initial first-time defaults when you haven't saved preferences to EEPROM yet and/or Use_MENU is disabled
#ifndef DEFAULT_BAUDRATE
#define DEFAULT_BAUDRATE 3850
#endif

#ifndef DEFAULT_MSELECTMASK
#define DEFAULT_MSELECTMASK 0   // Motor control state 1=on 0=off
#endif

#ifndef DEFAULT_TSXzxpUEF
#define DEFAULT_TSXzxpUEF 0     // Multiple flag: rpolarity needed for zx games: Basil the Great Mouse Detective, 
                                //            Mask // SpeedControl for .tsx // UEF Switch Parity
#endif

#ifndef DEFAULT_SKIP2A
#define DEFAULT_SKIP2A 0        // Pause on for BLK:2A
#endif

#endif // PREFERENCES_H_INCLUDED
