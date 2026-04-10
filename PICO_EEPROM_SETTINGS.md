# Pico EEPROM Settings

This note describes the RP2040/Pico menu-settings persistence that now works in this repo.

## Problem

The Raspberry Pi Pico has no real EEPROM.
MaxDuino's existing settings code assumed either:
- real EEPROM on AVR, or
- an existing wrapper implementation on other MCUs.

In this repo the Pico config originally had EEPROM settings disabled, and the EEPROM wrapper had no RP2040 branch.
That meant menu settings always fell back to their compile-time defaults after reboot.

## What Did Not Work Reliably

The first simple idea was to treat Pico like the old byte-addressed EEPROM users and store settings at high byte positions such as `255`, `254`, and `253`.
That matched the older MaxDuino layout, but it was a poor fit for the RP2040 emulated EEPROM path and was harder to validate.

## Working Approach

The final working approach was:

1. Enable menu settings for Pico only.
2. Keep logo EEPROM and block EEPROM disabled.
3. Use Arduino-Pico's flash-backed EEPROM emulation via `<EEPROM.h>`.
4. Store one compact settings blob at EEPROM address `0`.
5. Include a magic byte, version byte, and checksum so blank or stale flash is rejected cleanly.
6. Save immediately when a menu choice is committed, instead of only depending on final exit from the whole menu.

## Files Involved

### `MaxDuino/userRPI_PICOconfig.h`

Pico now enables settings persistence only:
- `LOAD_EEPROM_SETTINGS`
- `EEPROM_CONFIG_BYTEPOS 255`

The config byte position is kept for compatibility with the wider codebase, but the RP2040 path now uses a dedicated settings blob starting at EEPROM address `0`.

### `MaxDuino/current_settings.cpp`

This is the main fix.

For RP2040 it now:
- includes `<EEPROM.h>` directly
- defines a small `PersistedSettings` struct
- writes that struct at EEPROM address `0`
- read-validates with:
  - `magic`
  - `version`
  - `checksum`

### `MaxDuino/menu.cpp`

Settings are now saved when the user presses `Play` to commit:
- baud rate
- record type
- boolean menu toggles

The old save-on-menu-exit path remains as a final fallback.

### `MaxDuino/EEPROM_wrappers.h`

A Pico/RP2040 branch was added so the generic EEPROM byte helpers work on RP2040 too.

## Why This Works Better

This design matches Arduino-Pico's EEPROM emulation model more closely:
- one small flash-backed EEPROM area
- begin/read or begin/write/commit usage
- contiguous data storage

Using one validated blob is also more robust than scattering individual bytes and then guessing whether flash contents are valid.

## Current Scope

Enabled on Pico:
- menu settings persistence

Still intentionally disabled on Pico:
- EEPROM logo storage
- EEPROM block storage

That keeps flash wear and layout complexity low.

## If This Breaks Later

The first places to check are:
- `MaxDuino/current_settings.cpp`
- `MaxDuino/menu.cpp`
- `MaxDuino/userRPI_PICOconfig.h`
- `MaxDuino/EEPROM_wrappers.h`

Do not assume the old AVR-style high-byte EEPROM layout is the right model for Pico.
The working Pico path is the blob-at-address-0 approach.
