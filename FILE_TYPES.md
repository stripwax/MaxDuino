# MaxDuino Filetypes support

## ZX Spectrum

### .TAP

We support standard .TAP files with standard timings

### .TZX

We support most block types for standard .TZX files . Due to differences in interpretations of the .TZX spec, we support some additional flags to change polarity behaviours.

### .AY

.AY files containing z80 music player routines are supported.  We generate .TAP headers on-the-fly when loading a .AY file , so that these can simply be loaded like any other file.  These can then be loaded into an AY module player application running on the target device (your ZX Spectrum).

Support is enabled via configuration flag: `AYPLAY`.  This is enabled in most configurations by default, and there should be very little reason to disable it.

For more information about this file type, please see:
* https://worldofspectrum.net/projectay/tech.htm
* https://worldofspectrum.net/projectay/ayplayers.htm
* https://www.timf-tinkering.co.uk/spectrum/specay/

## ZX80

### .O

.O (O80 format) is supported. 

### .P

.P (P81 format) is supported for ZX80 (using 8K ROM conversions).

## ZX81

### .P

.P (P81 format) is supported for ZX81 files.

## ORIC-1

### .TAP

We support standard ORIC-1 .TAP files, and can playback at standard 300 BAUD or fast 2400 BAUD.

Support is enabled via configuration flag: `tapORIC`.  This is enabled in most configurations by default, and there should be very little reason to disable it.

Fast playback support is enabled via configuration flag: `ORICSPEEDUP`.  This is enabled everywhere by default.

## BBC MICRO / ACORN ATOM / ACORN ELECTRON

### .UEF

We support many of the .UEF features, but not all are enabled by default for every device type (due to firmware size limitations) - you may wish to customize your build to turn on or off options you care about.

* `Use_c112` - enables integer gap chunk for .uef
* `Use_hqUEF` - support `.hq.uef` files playback (see note)
* `Use_c104` - support defined tape format data block: data bits per packet/parity/stop bits    
* `Use_c114` - security cycles replaced with carrier tone
* `Use_c116` - floating point gap chunk for .hq.uef
* `Use_c117` - data encoding format change for 300 baud
        
We also support a customized turbo baud rate for .UEF files.  The standard speed is 1200 baud and the default 'turbo' speed is 1500 baud.  To turn on the 'turbo' mode, set the MaxDuino BAUDRATE (in the Menu) to anything other than 1200.  To turn off 'turbo' mode for UEF, set the MaxDuino BAUDRATE back to 1200.  You can customize the 'turbo' speed, at *compile time*, by changing the value of `UEF_TURBOBAUD` in `maxduino_prefs.h` or in your `userconfig.h` file - supported values are 1500, 1550 or 1600.  If `UEF_TURBOBAUD` is set to 1200, or is undefined completely, then .UEF will always play back at 1200 baud and the MaxDuino BAUDRATE setting will have no effect on .UEF playback.

#### Instructions for preparing .UEF files

* UEF files are compressed and can not be executed directly in MAXDUINO
* You have to decompress these files manually first.
* For example: on linux / mac os: `gunzip -c game.uef > game.uef.tmp && mv game.uef.tmp game.uef`
* For example: windows os: add .gz to file name, then click to extract with winrar


## JUPITER ACE

### .TAP

## COMMODORE C64 / C16

### .TAP

We support Commodore `.tap` images that use the native `C64-TAPE-RAW` or `C16-TAPE-RAW` header.

Support is enabled via configuration flag: `Use_c64`. When enabled, `.tap` files are identified by header first, so Commodore TAP files are routed to the Commodore handler while ZX Spectrum / Jupiter Ace / Oric `.tap` files keep their existing extension-based behaviour.

Playback follows Commodore TAP timing semantics directly from the file data. TAP v0/v1 entries are treated as full edge-to-edge pulse periods and split into two half-waves for MaxDuino's output engine; TAP v2 entries are used as half-wave timings directly. Long overflow pulses are preserved using a dedicated extended-pulse path so large tape gaps are not truncated.

## MATTEL AQUARIUS

### .CAQ

We handle .CAQ encoded at 600 baud (playback is fixed to this baud rate, meaning that baud rate customizations are ignored for .CAQ files)

## MSX

### .CAS

We support .CAS files for MSX computers.  CAS support is enabled via configuration flag: `Use_CAS`.  Due to several recent optimisations, it should be possible to enable `Use_CAS` for all devices now without running out of firmware space, so all devices should be able to include CAS support by default now.

### .TSX

.TSX format is a variant of .TZX format with a specific additional block type.  We support .TSX files.

In addition, we have included support for enabling the user to choose their own baud rate, as an alternative to using the timing parameters from the .TSX file itself.  The custom baud rates we support here are 1200 , 2400, 3150, 3600, and 3850 .  To turn on the custom baud rate when playing a .TSX file, toggle the `TSXCzxpUEFSW` option in the menu to 'ON'. (Note that this option has a different meaning for certain other file types).  Turning this option to OFF uses the standard parameters from the .TSX file .

For more information about .TSX format, take a look at https://github.com/nataliapc/makeTSX/wiki

### .TZX

Because of how MaxDuino operates, MSX files are supported regardless of extension (we don't check that the extension is specifically .TSX and can handle MSX file saved as .TZX too)

## DRAGON 32/64

### .CAS

We support .CAS files for DRAGON computers.  Support is enabled via configuration flag: `Use_DRAGON` **in addition to** `Use_CAS`.  Due to several recent optimisations, it should be possible to enable `Use_CAS` and `Use_DRAGON` for all devices now without running out of firmware space, so all devices should be able to include CAS support by default now.

There are several additional customizations for .CAS support for DRAGON computers:

* `Use_Dragon_sLeader` - a short Leader of 0x55 allowed for loading TOSEC files
* `Expand_All` - expand short Leaders in ALL file header blocks. 

## TANDY COLOR COMPUTER (COCO)

### .CAS

We support TRS Color Computer .CAS files.  Essentially the same as the Dragon 32 CAS format

## TRS-80 MC-10

### .C10

We support TRS-80 MC-10 .C10 files.  Essentially the same as the COCO/Dragon .CAS format with a different extension.

## TRS-80 Model 1,2,3 and 4

### .CAS

We support TRS-80 .CAS files.

TRS-80 .cas is a separate family of cassette formats: BASIC_L1, BASIC_L2,
SYSTEM_L1, SYSTEM_L2, and HIGHSPEED, detected from file markers or raw sync
patterns. Although it shares the .cas extension with CoCo, it is not the same
format: while CoCo uses the generic Dragon/CoCo CAS byte-stream path, TRS-80
uses TRS-80-specific timing and playback rules.

## AMSTRAD CPC

### .CDT

.CDT images are essentially the same as .TZX images, for Amstrad CPC computers.
.CDT support is enabled via configuration flag: `ID11CDTspeedup` .  Without this flag enabled, files with a .cdt extension will not be recognised.  This is enabled in most configurations by default, and there should be very little reason to disable it.
In addition, we have included support for enabling the user to choose their own baud rate, as an alternative to using the timing parameters  from the .CDT file itself.  The custom baud rates we support here are 1000 (same as ROM default), 2000, 3500, and 4000 .  The menu options currently don't match exactly, so you will choose `1200`, `2400`, `3150` (or `3600`), and `3850`, respectively.  To turn on the custom baud rate when playing a .CDT file, toggle the `TSXCzxpUEFSW` option in the menu to 'ON'. (Note that this option has a different meaning for certain other file types).  Turning this option to OFF uses the standard parameters from the .CDT file .

## SHARP MZ (MZ-700, MZ-800)

### .MZF / .MZT / .M12

We support Sharp MZ tape images stored as `.mzf`, `.mzt`, or `.m12`.

`.mzt` / `.m12` files are routed through the same playback engine and use the same pulse timings as `.mzf`. If a file contains a repeated Sharp block, MaxDuino plays the first header+data copy, which preserves the existing `.mzf` behaviour.

## MEMOTECH MTX

### .MTX

We support MTX files but only test in a emulator as of now. 

## EACA EG2000 Colour Genie

### .CAS / CGC

We support Color Genie .CAS files (which are not related to other .CAS files), which can also have .CGC extension.  These have a fixed 1200 baud.  Accuracy and reliability tested via converting MaxDuino output to .wav and loading into Genieous emulator.
