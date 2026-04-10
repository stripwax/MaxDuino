# ESP32 DevKit Pinout Proposal

This is a proposed full-size ESP32 DevKit pin layout for MaxDuino that mirrors the Arduino Nano / Nano Every style approach:

- no resistor ladder for buttons
- separate GPIO buttons
- standard SPI SD card
- standard I2C OLED
- dedicated ADC input for recording
- dedicated output pin
- dedicated REM / motor-control input

## Recommended Pin Map

### SD Card
- `SCK` = `GPIO18`
- `MISO` = `GPIO19`
- `MOSI` = `GPIO23`
- `CS` = `GPIO13`

### OLED / I2C Display
- `SDA` = `GPIO21`
- `SCL` = `GPIO22`

### Buttons
- `Play` = `GPIO32`
- `Stop` = `GPIO33`
- `Up` = `GPIO27`
- `Down` = `GPIO14`
- `Root / Menu` = `GPIO16`

### Control / Tape
- `REM / Motor sense` = `GPIO17`
- `Audio output` = `GPIO26`
- `Audio ADC input` = `GPIO35`
- `Record button` = `GPIO25`  if recording is added later

## Why This Layout

- It avoids the ESP32 boot strap pins for the normal button set.
- It avoids the resistor ladder entirely.
- It keeps the recorder input on an `ADC1` pin.
- It keeps output on `GPIO26`, which already matches the existing ESP32 output approach used in this codebase.
- It uses the normal ESP32 hardware SPI pins.
- It uses the standard ESP32 I2C pair.

## Pins To Avoid

Avoid these for buttons or motor control where possible:

- `GPIO0`
- `GPIO2`
- `GPIO4`
- `GPIO5`
- `GPIO12`
- `GPIO15`

These are ESP32 boot-related strap pins and can cause awkward startup behavior.

Also avoid these for normal buttons:

- `GPIO34`
- `GPIO35`
- `GPIO36`
- `GPIO39`

These are input-only pins and do not provide internal pullups.

## ADC Note

Use `ADC1` for the recording input. `GPIO35` is a good fit for that.

Do not feed cassette or audio directly into the ESP32 ADC pin. The signal should be conditioned into the `0V .. 3.3V` range and biased around about `1.65V`.

## Summary

If a full-size ESP32 DevKit board is added, this is the cleanest mapping for MaxDuino while staying close to the Nano / Every style of wiring:

- separate buttons
- dedicated motor line
- dedicated audio output
- dedicated ADC input
- standard SPI SD
- standard I2C display