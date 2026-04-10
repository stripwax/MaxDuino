# TZX ID15 Recording

## Goal
Keep generic tape recording available as `TZX ID15`, while making playback of recorded files reliable again.

## What `TZX ID15` Means Here
`ID15` is a direct-recording TZX block.
In this project it is used as a packed 1-bit sampled waveform.

The normal recorder path writes:
- TZX header
- block `0x15`
- sample period `79` T-states per sample
- pause after block `1000 ms`
- packed sample bytes, `MSB` first

The normal TZX record path is in `MaxDuino/record.cpp`.

## Recorder Side
The normal `TZX ID15` recorder intentionally stays simple.

For standard `TZX ID15` mode it uses:
- `44.1 kHz` sample rate
- fixed `79` T-states per sample in the file header
- fixed threshold slicing: `sample >= 512`

That simplicity is deliberate. A previous attempt to blend in newer experimental slicers changed the sound of normal recordings and made working recordings worse.

So the final normal path keeps the archive-style behavior for plain `TZX ID15`.

## Playback Side: The Real Root Cause
The biggest regression was not actually the file header or file naming. It was the playback contract for `ID15`.

The recorder assumed the archive-style `ID15` path, but playback had drifted away from that.

The fix was to restore the older contract:
- convert `ID15` sample length to microseconds up front in `MaxDuino/MaxProcessing.cpp`
- let the ISR consume that direct sample period plainly in `MaxDuino/isr.cpp`

That restored compatibility between:
- how the recorder writes direct-recording blocks
- how playback consumes them

This was the fix that finally made recorded TZX files work again without harming CAS recording.

## Why This Was Sensitive
`ID15` is a very timing-sensitive path.
It is not like normal TZX pulse regeneration. The ISR is effectively replaying recorded sample levels at a fixed sample period.

Because of that, the recorder and playback sides must agree exactly on:
- how sample period is stored
- when the sample period is updated
- how the direct sample bits are shifted out

## D1 Mini32 / ESP32 Notes
The Wemos D1 Mini32 integration added a second layer of work for `ID15` playback on ESP32.

Those changes are separate from the AVR recorder fixes, but they matter for reliable `ID15` playback on that board.

The important D1/ESP32 fixes are:
- hardware timer uses APB clock via `getApbFrequency() / 1000000`
- GPIO26 is forced out of DAC/RTC mode before use as tape output
- direct register writes are used for GPIO26 output
- cached SD reads reduce refill overhead
- buffer depth is enlarged only for OTLA / direct-recording playback
- OTLA optimizations are enabled only when playback actually enters `ID15`
- the main loop keeps the OTLA fast path short but still checks `Stop`

Those parts live across:
- `MaxDuino/TimerCounter.cpp`
- `MaxDuino/pinSetup.h`
- `MaxDuino/pinSetup.cpp`
- `MaxDuino/file_utils.cpp`
- `MaxDuino/buffer.h`
- `MaxDuino/buffer.cpp`
- `MaxDuino/processing_state.h`
- `MaxDuino/processing_state.cpp`
- `MaxDuino/MaxProcessing.cpp`
- `MaxDuino/MaxDuino.ino`

## Important Files
Normal TZX recorder/playback:
- `MaxDuino/record.cpp`
- `MaxDuino/MaxProcessing.cpp`
- `MaxDuino/isr.cpp`

Shared runtime record-type selection:
- `MaxDuino/current_settings.h`
- `MaxDuino/current_settings.cpp`
- `MaxDuino/menu.cpp`

ESP32 / D1 Mini32 `ID15` playback support:
- `MaxDuino/TimerCounter.cpp`
- `MaxDuino/pinSetup.h`
- `MaxDuino/pinSetup.cpp`
- `MaxDuino/file_utils.cpp`
- `MaxDuino/buffer.h`
- `MaxDuino/buffer.cpp`
- `MaxDuino/MaxDuino.ino`

## Things Not To Undo
- Do not replace the normal recorder with a different thresholding scheme globally.
- Do not move `ID15` sample timing back to a mismatched playback contract.
- Do not force the large D1 playback buffer for all TZX playback.
- Do not enable OTLA optimizations from file start; enable them when entering `ID15`.

## Practical Summary
`TZX ID15` works because the current branch again treats recorded direct data the same way end-to-end:
- archive-style simple recording
- matching `ID15` playback semantics
- board-specific high-speed playback optimizations only where they are actually needed