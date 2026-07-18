# MaxDuino Devices

Various MaxDuino and TZXDuino devices have been manufactured over the last 15 years or so, and these have been constructed with different combinations of hardware components (microcontroller type, LCD type, etc).  We provide MaxDuino software intended to be compatible with all of them and more.

Our configuration-based build process enables you to customise a configuration that matches your particular hardware.  The below sections outline the components for which we include support.


## Microcontroller type

MaxDuino can be run on custom PCBs built using any of the following microcontrollers, as well as on standard dev boards listed in parentheses:

*  Atmel (Microchip) ATMEGA328P (Arduino Nano / Arduino Uno v1-3)
*  Atmel (Microchip) ATMEGA32u4 (Arduino Leonardo)
*  Atmel (Microchip) ATMEGA4809 (Arduino Nano Every)
*  Atmel (Microchip) ATMEGA4808 (Thinary Nano Every)
*  Atmel (Microchip) ATMEGA2560 (Arduino Mega 2560)
*  RaspberryPi RP2040 (Raspberry Pi Pico, also SEEED XIAO RP2040 (untested))
*  RaspberryPi RP2350 (SEEED XIAO RP2350)
*  Espressif ESP8266 (Wemos D1 Mini)
*  Espressif ESP32-WROOM (D1 Mini32)
*  Atmel (Microchip) SAMD21 (SEEED XIAO M0 SAMD21)
*  Espressif ESP32C3 (SEEED XIAO ESP32-C3)
*  STMicroelectronics STM32F (possibly incomplete, needs testing)
*  .. and others

## LCD type

* 128x64 GRAPHICAL OLED (I2C devices, compatible with SSD1306, SSD1309, or SH1106)
  * 0.96" - typically SSD1306
  * 1.3" - typically SH1106
  * 2.42" - typically SSD1309 (configured using same settings as 0.96" SSD1306)
* 16x2 TEXT LCD (I2C PCF8574T-compatible)
* LCD (Nokia P8544-compatible)  (see [Note 1](#notes))

## Output type

*  Audio output (typically connected to jack socket)

## Inputs type

*  5 buttons : Up, Down, Play/Pause, Stop, Menu (aka 'Root')   (See [Note 2](#notes))
  * Most devices support multi-press combinations
  * Some devices only support one-button-at-a-time due to lack of independent GPIO or insufficient pins e.g. Seeed Xiao (ESP32C3, SAMD21, D1 Mini)
*  Power button, aka "soft-off" (for supported devices only e.g. ESP32C3) (See [Note 3](#notes))
*  (Optional) Motor control sense
*  (Optional) Audio input (typically connected to jack socket)
*  (Optional) Record button  (See [Note 4](#notes))

## Prebuilt configurations

Several combinations are available 'prebuilt' and included in GitHub releases: https://github.com/stripwax/MaxDuino/releases

You can of course create your own custom firmware by editing the configuration files and compiling yourself - see [BUILDING](./BUILDING.md)

# Notes
1. Support for P8544 has been removed due to lack of devices for testing and development purposes
2. Devices may have a reset button but this directly resets the device via a hardware reset signal, and is not handled by the firmware
3. Currently, soft-off is implemented as a long-press on the Stop button, but future hardware iterations might include a separate GPIO power button
4. Experimental record functionality is included and enabled for certain devices, check sourcecode for more details


## Commercial devices

Many TZXDuino and MaxDuino have been produced by a variety of individual suppliers and businesses.  The MaxDuino firmware project is not directly affiliated with any of the manufacturers of these products.
MaxDuino firmware is itself an open-source project and remains free for everybody.

### ATMEGA328P - based

* TZXDuino, ArduiTape, CASDuino, TSXDuino
  * This is the 'classic' original device with a variety of different supporting hardware
  * Arduino Nano (ATMEGA328P), with either 128x64 GRAPHICAL OLED or 16x2 TEXT LCD, from a variety of suppliers and self-build kits.
  * Due to variety of configurations, different build configurations are necessary and you may need to simply build your own.
  * Sometimes these devices include PCD8544 LCD controller instead. This is no longer supported by MaxDuino but support can be added back if there is sufficient demand!!
  * Release builds available: `Nano328p_CF0` thru `Nano328p_CF7`; `Nano328p_LCD161` if you have a 16x2 TEXT LCD; `Nano328p_NO_BOOTLOADER` if you wish to have maximum features and don't mind losing bootloader functionality and USB reflashing.  **SEE NOTE BELOW ABOUT DEVICES WITH USB**

* NextDuino/MaxDuino/ZXUITape/(MSXUITape/etc) from You Make Robots
  * Various stylings and namings but based on ATMEGA328P
  * Current revisions use a custom PCB without USB connector.  **SEE NOTE BELOW ABOUT DEVICES WITHOUT USB**.
  * Release build available: same as above

* ZXTape by Retro-Spektro
  * https://www.retro-spektro.com/projects/zxtape-for-zx-spectrum/
  * Standard Arduino Nano-based MaxDuino, uses 1.3" OLED
  * Latest hardware revision uses a custom board with no USB.  **SEE NOTE BELOW ABOUT DEVICES WITHOUT USB**.
  * Release build available: same as above

* MaxDuino and Miniduino - ATMEGA328P based (from Antonio Villena)
  * Understood to use a custom board with no USB.  **SEE NOTE BELOW ABOUT DEVICES WITHOUT USB**.
  * (This entry is missing details, can you supply more info?)

### ATMEGA4808 and ATMEGA4809 - based

* MaxDuino Ultimate and MaxDuino Every from You Make Robots
  * Thinary Nano Every or Arduino Nano Every, respectively.  128x64 OLED (could be 0.96" or 1.3" variant) + tape motor control
  * Release builds available: `ThinaryNanoEvery_MaxduinoUltimate` and `NanoEvery_MaxduinoUltimate`
* MaxDuino Everstore (from You Make Robots)
  * Identical to MaxDuino Ultimate/Every but additionally includes the recording circuit and additional audio input.
  * The above firmware release builds (Thinary or NanoEvery) are already compatible with MaxDuino Everstore and include the Recording functionality

### ATMEGA32U4 - based

* MaxDuino Easy Upgrade and Miniduino Easy Upgrade (from Antonio Villena)
  * ATMEGA32u4 + 0.96" OLED + tape motor control
  * Release build available: `ATMEGA32U4_Miniduino` (confirmed working for Miniduino Easy Upgrade, can someone test with MaxDuino Easy Upgrade?)
  * (This entry is missing details, can you supply more info?)

### STM32 - based

* MaxDuino and Miniduino - STM32 based (from Antonio Villena)
  * STM32 + 0.96" 128x64 OLED + tape motor control
  * Release build available: `STM32_MapleMiniDuino` (confirmed working but not yet optimised for OTLA)
* MaxDuino Mini from Lotharek.pl
  * https://lotharek.pl/productdetail.php?id=409
  * Not much currently known about this product from the MaxDuino project team, so compatibility is unclear. Tester and volunteers would be appreciated here!

### ATMEGA2560 - based
* MaxDuino Mega
  * (This entry is missing details, can you supply more info?)
* MegaDuino aka TSXDuino Mega
  * either Mega2560 or dual Mega2560/STM32 (? need details)
  * ???
  * ref: https://github.com/capsule5000/TSXduino-MEGA - MaxDuino support needs to be confirmed!!!
  * firmware fork: https://github.com/merlinkv/MegaDuino_Firmware - would be great to unfork this and get back to mainline support
  * (This entry is missing details, can you supply more info?)

### OTHERS

Hobby devices have been developed using a variety of different MCUs, all of which are tested and supported by the MaxDuino project:
* ESP8166 (D1-Mini)
* ESP32-WROOM (D1-Mini32)
* ESP32-C3 (Seeed Xiao)
* RP2040 (Raspberry Pi Pico, Seeed Xiao)
* RP2350 (Seeed Xiao)
* SAMD21 (Seeed Xiao)

Commercial devices using other MCUs are either not available, or unknown to the MaxDuino project team at this current time.

There are some experimental forks available for various ESP and other devices, even though MaxDuino now officially supports these and other MCUs.  MaxDuino cannot support for other people's experimental forks of course!  Get in touch if you have features that can be ported back into our mainline firmware for everybody to use.

## Note about devices without USB
 
Several commercial devices uses custom PCBs without USB connectivity for uploading new firmware (even if they use USB for DC power).  For these you will need a USBASP or similar ICSP programmer to upload MaxDuino onto the device.

In this case, you may prefer to use one of the `Nano328p_NO_BOOTLOADER` envs (`platformio.ini`) and corresponding configuration file (e.g. see `userconfig11.h`) - optimised to cram in more features and filetypes by freeing up firmware space by removing the bootloader entirely.  Since the bootloader is typically used for USB programming, no functionality is lost on devices without USB programmability.  This frees up the full 32KB flash for MaxDuino.  You can still customize these builds in the usual way if you so wish.

**IMPORTANT**  To successfully flash and run one of the "NO_BOOTLOADER" images on ATMEGA328P, you will need to ensure you have set the e-fuses on the board.  You can do this from PlatformIO (`Project Tasks` -> `(env name)` -> `Platform` -> `Set Fuses`) or by a separate utility such as AVRDUDESS.  The correct values to set are listed in the `platformio.ini` file.

