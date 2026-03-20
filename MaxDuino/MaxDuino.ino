// ---------------------------------------------------------------------------------
// USE CLASS-4 AND CLASS-10 CARDS on this project WITH SDFAT2 FOR BETTER ACCESS SPEED
// ---------------------------------------------------------------------------------
/*
 *                                    TZXduino
 *                             Written and tested by
 *                          Andrew Beer, Duncan Edwards
 *                          www.facebook.com/Arduitape/
 *                          
 *              Designed for TZX files for Spectrum (and more later)
 *              Load TZX files onto an SD card, and play them directly 
 *              without converting to WAV first!
 *              
 *              Directory system allows multiple layers,  to return to root 
 *              directory ensure a file titles ROOT (no extension) or by 
 *              pressing the Menu Select Button.
 *              
 *              Written using info from worldofspectrum.org
 *              and TZX2WAV code by Francisco Javier Crespo
 *              
 *              ***************************************************************
 *              Menu System:
 *                TODO: add ORIC and ATARI tap support, clean up code, sleep                 
 *                
 *              V1.0
 *                Motor Control Added.
 *                High compatibility with Spectrum TZX, and Tap files
 *                and CPC CDT and TZX files.
 *                
 *                V1.32 Added direct loading support of AY files using the SpecAY loader 
 *                to play Z80 coded files for the AY chip on any 128K or 48K with AY 
 *                expansion without the need to convert AY to TAP using FILE2TAP.EXE. 
 *                Download the AY loader from http://www.specay.co.uk/download 
 *                and load the LOADER.TAP AY file loader on your spectrum first then
 *                simply select any AY file and just hit play to load it. A complete
 *                set of extracted and DEMO AY files can be downloaded from
 *                http://www.worldofspectrum.org/projectay/index.htm
 *                Happy listening!
 *                
 *                V1.8.1 TSX support for MSX added by Natalia Pujol
 *                
 *                V1.8.2 Percentage counter and timer added by Rafael Molina Chesserot along with a reworking of the OLED1306 library. 
 *                Many memory usage improvements as well as a menu for TSX Baud Rates and a refined directory controls.
 *                
 *                V1.8.3 PCD8544 library changed to use less memory. Bitmaps added and Menu system reduced to a more basic level. 
 *                Bug fixes of the Percentage counter and timer when using motor control/
 *
 *                V1.8u1 (kernel@kernelcrash.com)  
 *                Mods to V1.8 TZXDuino to enable UEF playback. Only supports gunzip'd
 *                UEF files though (that still end in .uef). Plays back UEF's as square
 *                wave not sine waves.
 */ 
 //               MAXDUINO http://github.com/rcmolina/maxduino is a reworking from casduino and tzxduino 
 //               to support both at the same time on Arduino Nano. Original idea from Rafael Molina Chasserot, 
 //               testing and help from Davide Barlotti, @ManuFerHi, @BCH, Alfredo Retrocant, @jgilcas and @Spirax
 //               who also provided modifications to support Oled double line with small font.
 //
 //               V1.28 Better speeed loading tzx files, more than 5000 baudios using PORT instructions
 //               V1.29 3-digits counter can be configured to display m:ss with #define CNTRBASE
 //               V1.30 max TSX speed 3850 vs  cas speed 3675. Also changed in Menu.
 //               V1.31 Modified ID20,2A to support automatic pausing(@spirax). Also swapped REW-FW for block navigation(@acf76es).
 //               V1.32 Deprecated old pskipPause and new block 2A pause control option in Menu.
 //               V1.33 Blocks for manual REW/FF expanded from 10 upto 20, used as circular buffer. On Oled screen prints upto 99 blocks,
 //                     overflowing to 00.  Selecting a block in pause mode traverse last 20 blocks.
 //               V1.34 alias "EEPROM version". Now blocks can be directly read/written from EEPROM.
 //                     Also logo can be copied to eeprom using #define option. After that, you can select option
 //                     for loading logo directly from EEPROM.. this saves 512 bytes in the sketch.
 //               V1.35 Uncompressed UEF support for Acorn computers, code imported from v1.8u1
 //               V1.35t Acorn speed for loading now 1500 baud, 25% faster (standard is 1200).
 //               V1.36 Polarity handling
 //               V1.37 Minor optimizations for counter format, acorn standard/turbo speed, simplication in polarity handling and 
 //                     now controlled from menu using the same option for tsx acceleration: tsxSPEEDzxPOL.
 //               V1.38 Code reformatted, cas is optional with new #def Use_CAS.Solved a bug when back with stop button from 4th subdir.
 //               V1.39 Optional hq.uef support for Acorn using define (implemented chunks 111 and 116).
 //                     Code better organized, and new userconfig.h with help for easier configuration -Alfredo Crespo
 //               V1.40 LCD sketch savings, tsx control/polarity/UEF turbo mode can be changed while paused with menu button. 
 //                     Reworked acorn chunk 116 floating point conversion to integer.
 //               V1.41 New logos from project Sugarless (@spark2k06/@yoxxxoy). New SPLASH_SCREEN configuration (@spark2k06).
 //                     Better .tsx start/stopbits support: Now working with SVI-3x8 machines!
 //               V1.42 Bugs solved: Rewinding block not showing first digit correctly (@acf76es). 
 //                     LCD printing when block ID not recognized again in hex. First support of 1,3" oled SH1106 but using 128x32 
 //                     and not 128x64
 //               V1.43 aka "Mar Menor". Implemented half-interval (logarithmic) search for dirs and block selection. 
 //                     Now root button used as pivot. Menu funcions now activated with ROOT + STOP, new half-interval search with 
 //                     ROOT + UP/DOWN. (Frank Schröder)
 //               V1.44 New define btnRoot_AS_PIVOT in userconfig.h so it can be deactivated by user. New #define in Maxduino.h to support 
 //                     Antonio Villena's MINIDUINO new design with amplifier and new define OLED_SETCONTRAS in userconfig.h for contrast 
 //                     value override. New Miniduino logo. It's posible to select RECORD_EEPROM and LOAD_EEPROM both for better testing 
 //                     when selecting new logos in userconfig.h, pressing MENU simulates a reset to show logo again.
 //               V1.45 New ID15 testing code, can be deactivated from userconfig.h to save space. Amstrad bugs solved: 
 //                     Deflektor and other cdts now loading.
 //               V1.46 OLED 128x64 mode with 8 rows, logo can also be loaded with full resolution (@geloalex, @merlinkv, @acf76es)
 //               V1.47 Optional BLOCKID_INTO_MEM if loading many turbo short blocks. BLOCK_EEPROM_PUT must be disabled when loading 
 //                     Amstrad cpc Breaking Baud demo. Bug fixed: block counter should not be incremented when pausing.
 //               V1.48 New chunks for Acorn computers: Implemented parity handling when loading protected games (@acf76es).
 //                     Remove warnings when compilation fron IDE (@llopis).
 //               V1.49 ID15 adjustment for slow Amstrad musical loaders.
 //               V1.50 aka "Maria". File auto-scrolling when holding down UP or DOWN buttons (Frank Schröder).
 //               V1.51 Dragon TOSEC files with short leader loading now.
 //               V1.52 New arduino nano EVERY micro supported. Need work for turbo loading.
 //                     Different config files for new high capacity chips (Riccardo Repetto).
 //                     Compressed logo where only even columns are used for load/save with EEPROM.
 //               V1.53 Some rework for Arduino nano every and timers TCA / TCB0 and using last SDFat for better speed.
 //               V1.54 Maintenance release. Final byte patched again for Dragon and Coco, got switched when testing in v1.51. 
 //               V1.55 support for BBC chunk 0x117 (@vanekp)
 //               V1.56 aka "baby Fran". ID19 short implementation to load zx81 tzx files. 
 //                     Now you can use TSXControl to speedup loadings
 //               V1.57 Changed zx81 TSXControl to Speed to avoid polarity problems. 
 //                     Better zx81 file loading (@El Viejoven FX). Removed Amstrad tweaking to slower ID15, 
 //                     Oricium game now can load in 4 minutes in Oric Atmos, file attached for testing.
 //               V1.58 STM32 testing support thanks to @Sildenafile.
 //               V1.59 Oric native tap support.
 //               V1.60 Oric tap speedup if speed greater than 2400.
 //               V1.61 Graphic 128x64 and text 128x32 combined mode for Oled screens. Optional 8x8 bold font.
 //                     Better XY2 thanks to @alferboy.
 //               V1.62 aka "Marble". New StandardFont (8x8) pack. DoubleFont (8x16) support. 
 //                     Improved speed loading in oled marble mode (XY2force) @alferboy. 
 //                     MenuBLK2A option in userconfig, no dependencies from UEF (@merlinkv).
 //               V1.63 New Logos. Fixed side-effect on MSX cas when implementing Oric native tap files loading.
 //               V1.64 V1.64 Thinary PCB support (cheap arduino nano every). 
 //                     Many thanks to Ricky El-Qasem for his help and his perseverance for this to work !!
 //               V1.65 Code rearrangement for esasier pinout modification. 
 //                     Support for the new Maple miniduino (STM32) from Antonio Villena.
 //               V1.66 aka "Beast Nano". New userconfig option for optional oled on screen print, so more space for nano 
 //                     to include oric and uef in the same upgrade. Fixed bug when dragon to msx switching without reset.
 //               V1.67 End of file adjustments, better loading in Camputerx Lynx tzx id15. 
 //                     Removed Oric final pause, for testing, forgot to remove it. Some changes in STM32 for handling the clock.
 //               V1.68 aka "ALLinOne". Maintenance release, new optimizations and changes for Acorn computers and Oric.
 //                     Solved a bug for zx polarity. Dragon 32 optimizations.
 //               V1.69 aka "Jumbo". New TZX Block Search with online Fast Forwarding, enjoy (still need testing).
 //               V1.70 aka "optiboot". To flash this complete relase in your Arduino 328, ask your seller to burn optiboot first !! 
 //               V1.71 aka "Clive". Better Blockmode: when menu pressed in pause mode, jump 20 blocks with REW OR FF
 //               V1.72 New suboption XY2shadedplay for XY2. Softwire working with both OLED and LCD to save extra 550 bytes, 
 //                     I2C can be configured in fastmode for better performance. 
 //               V1.73 aka "Christmas21 edition". Better config frontend, some adjustments in hwconfig.h
 //               V1.74 aka "Christmas21 Eve". Used SoftI2CMaster instead of Softwire to save more space, new adjusted configs.
 //               V1.75 New option to handle more than 255 block in Blockmode if needed. Decrease block count in Blockmode
 //                     to skip some blocks thus matching live oled block count.
 //               V1.76 New option to trace ID15 blocks #BLOCKID15_IN. Support variable baudrate on the fly 
 //                     for Amstrad CPC ID11 blocks (like TSXControl).
 //               V1.77 Started new Maxduino version, big sdfat savings by David Hooper @stripwax, solved buffer corruption al last
 //               V2.00 New maxduino stable and tested reference version. Dragon optimizations.
 //               V2.01 aka "San Fermin". Fixed oled corruption and new cartoon8x16 font
 //               V2.02 Oled driver optimizations
 //