# MaxDuino

This project contains the MaxDuino firmware, compatible with most MaxDuino and TZXDuino devices.  These devices are 'tape emulators' that output audio for loading into retro computers typically manufactured in the 1980s and 1990s.

By downloading and installing the latest MaxDuino firmware onto your MaxDuino or TZXDuino devices, you can add support for the largest range of filetypes for different retro computers as well as adding new usability and configuration options for your device.  **All supported filetypes can be enabled simultaneously** given sufficient system resources - no more switching firmwares on your TZXDuino!

For more information about MaxDuino and TZXDuino, check out the [DOCUMENTATION](./DOCS.md)

MaxDuino firmware is free and open-source.

## LATEST NEWS

* V3.05 Adds improvements to C64 pause handling, and TRS-80 .CAS support. Includes a lot of performance optimisations, some fixes for ESP32 and ESP8266, and a temporary new home and name change: https://github.com/stripwax/MaxDuino and "MaxDuiNeo 2026"
* V3.04 Adds C64 .TAP support, Sharp MZ-700 .MZT + .M12 file extension support, and Memotech .MTX implementation
* V3.03 (we skipped v3.03 due to a problem with C64 support)
* V3.02 Adds Sharp MZ-700 .MZF implementation, and Mattel Aquarius .CAQ implementation
* V3.01 Adds Native Jupiter tap (JTAP) implementation.
* V3.00 Adds 44.1kHz OTLA support and we now use "Maxduino OTLA" as reference name for this version. Check https://github.com/rcmolina/otla_tzx for some testing.

## FILETYPES SUPPORTED BY MAXDUINO

* ZX SPECTRUM: .TZX, .TAP, .AY
* ZX80: .P, .O
* ZX81: .P, .O
* ORIC-1: .TAP
* BBC MICRO / ACORN ATOM / ACORN ELECTRON: .UEF
* JUPITER ACE: .TAP (JTAP)
* COMMODORE C64/C16: .TAP
* MATTEL AQUARIUS: .CAQ
* MSX: .CAS, .TSX, .TZX
* DRAGON 32/64: .CAS
* TANDY COLOR COMPUTER: .CAS
* TANDY TRS-80 MC-10: .C10
* TRS-80 MODEL 1, 2, 3, and 4: .CAS
* AMSTRAD CPC: .CDT
* SHARP MZ (MZ-700, MZ-800): .MZF, .MZT, .M12
* MEMOTECH MTX: .MTX

**All supported filetypes can be enabled simultaneously** on any Maxduino device that has sufficient firmware and RAM capacity, such as:

* Thinary Every / Nano Every
* ESP32
* ESP8266
* SAMD21

More information at [FILE TYPES](./FILE_TYPES.md)

## SUPPORTED MAXDUINO/TZXDUINO DEVICES

Various MaxDuino and TZXDuino devices have been manufactured over the last 15 years or so, and these have been constructed with different combinations of hardware components (microcontroller type, LCD type, etc).  We provide MaxDuino software intended to be compatible with all of them and more.

*  Arduino Nano 328p
*  Arduino Nano Every
*  Thinary Nano Every
*  Arduino Mega 2560
*  Seeeduino Seeed Xiao M0 (SAMD21) (experimental)
*  Espressif ESP8266 (experimental)
*  Espressif ESP32C3 (experimental)
*  STMicroelectonics STM32 (experimental, needs testing)
*  .. and others

The above are included in GitHub releases: https://github.com/stripwax/MaxDuino/releases

More information at [MAXDUINO DEVICES](./MAXDUINO_DEVICES.md)

## BUILD INSTRUCTIONS

You can use PlatformIO along with the included environments in platformio.ini or create your own.

Or you can use Arduino IDE (via the MaxDuino/MaxDuino.ino file)

More information at [BUILDING](./BUILDING.md)

## INSTALLING PREBUILT RELEASES

You can download prebuilt releases from the github Releases page and upload them to your device
using the appropriate tools for your device (usually that's avrdude).  For example, on Arduino Nano you can
follow these instructions: https://forum.arduino.cc/t/using-avrdude-to-push-sketch-to-arduino/525745

The master branch will always have a release called "latest" which is automatically rebuilt on every commit.
We also aim to produce release packages with tagged versions whenever there is a major or minor version bump.

More information at [INSTALLATION](./INSTALLATION.md)

## PREVIOUS VERSIONS

More information at [HISTORY](./HISTORY.md)

## DOCS

See [DOCS](./DOCS.md)

## TECHNICAL ADVICE

    // ---------------------------------------------------------------------------------
    // USE CLASS-4 AND CLASS-10 CARDS on this project WITH SDFAT2 FOR BETTER ACCESS SPEED
    // ---------------------------------------------------------------------------------

## In memoriam

> To my father R.I.P. who bought my first ZX Spectrum and forced me to learn with his casio programming calc.
[Rafa Molina](https://github.com/rcmolina)
