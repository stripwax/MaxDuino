# RECORDING

## ENABLING THE RECORDING FEATURE

This feature is introduced in v4.00 onwards, and is initially enabled only for the ATMEGA4808/ATMEGA4809 MCUs (Thinary Nano Every or Arduino Nano Every) .
To enable this, include `#define RECORD` in your userconfig or `-D RECORD` in your platformio.ini build_flags.
You also need to enable one or more recording filetypes, also via `#define` / `-D` , from the following list:

* `RECORD_TZX_ID15` : generic TZX direct-recording mode
* `RECORD_ZX_SPECTRUM` : Spectrum-tuned TZX recording mode
* `RECORD_CAS_MSX` : native MSX .cas recording mode
* `RECORD_SHARP_MZF` : native Sharp MZ .mzf recording mode

You can choose which to enable - you may wish to enable only one or two, to reduce your firmware size and make space available for other things - or you may wish to enable all of them.  The default build has these all enabled.

## HARDWARE SUPPORT

Support for recording requires dedicated external components, not least an additional audio input and amplifier, but also requires available ADC pins on the MCU board.
Currently the firmware support is only implemented for the ATMEGA4808 and ATMEGA4809 boards. It is likely to be added to other boards in the future - but only those
that have sufficient firmware and ram capacity (It is extremely unlikely that we will ever add support for recording to the ATMEGA328P firmware builds).

## ADDITIONAL MENU

The Recording feature includes an additional menu, available via the standard Menu (ROOT) button, to select one of the supported recording file types.  It behaves like a simple list picker, with the currently selected format shown with an asterisk beside it.

If LOAD_EEPROM_SETTINGS is enabled, the Recording settings are written to an additional config byte in the device's EEPROM (or emulated EEPROM, for devices without dedicated EEPROM).  As with the other Menu items, settings are only written when you exit the menu.

