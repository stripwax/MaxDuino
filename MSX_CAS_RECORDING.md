# MSX CAS Recording

## Goal
Make MSX recording reliable without breaking the existing TZX recorder.

## Problem We Had
The generic `TZX ID15` recorder was too crude for MSX.

MSX cassette data is framed serial data with a leader, a start bit, 8 data bits, and stop bits. Recording it as a raw 1-bit sampled waveform caused repeated failures:
- pilot tones but almost no data
- tiny `.cas` files made of many short fragments
- empty recordings
- files that only contained the first block or two

The main lesson was that MSX needed decoding, not just raw sampling.

## Final Working Design
The final solution is a dedicated `CAS MSX` record type.

It is selected at runtime from `Record Type ?` and stored in EEPROM through:
- `MaxDuino/current_settings.h`
- `MaxDuino/current_settings.cpp`
- `MaxDuino/menu.cpp`

The recorder engine lives in `MaxDuino/record.cpp`.

### Capture front end
The MSX recorder samples at `50 kHz` and uses an adaptive midpoint with hysteresis instead of a fixed threshold.

Important parts of the front end:
- running center tracking
- Schmitt-style level decision
- minimum accepted edge length to ignore obvious glitches
- silence detection to end or resync a block cleanly

### Decode model
The recorder does not try to preserve the analog waveform. It decodes the incoming MSX tape structure into bytes.

The working decode path does this:
- waits for a sustained short-cycle leader
- requires about `500 ms` of valid header before arming a block
- measures the short cycle and derives the short/long timing windows from that measured average
- detects the first long interval after the header as the start of a byte
- decodes framed serial data as:
  - start bit `0`
  - 8 data bits, `LSB` first
  - stop bits `1`
- writes real `.cas` bytes, not a sampled waveform block

### CAS file writing rules
A block header is only written once the first byte of that block has actually been decoded.

That detail was important. Earlier versions wrote CAS headers too early, which created files like:
- header
- one byte
- header
- one byte
- header
- one byte

The final path keeps sync inside an open block and only aborts a block when it has a real reason to do so.

## Why It Finally Worked
The breakthrough was when recorder diagnostics showed that the decoder was actually reaching the expected MSX structure.

The useful counters were:
- `H` = headers seen
- `B` = blocks started
- `D` = decoded data bytes

A successful test produced:
- `H:6`
- `B:6`
- `D:33599`

That matched the source file structure exactly once the 8-byte CAS headers were accounted for.

## Important Files
- `MaxDuino/record.cpp`
- `MaxDuino/current_settings.h`
- `MaxDuino/current_settings.cpp`
- `MaxDuino/menu.cpp`

## Things Not To Undo
- Do not replace the MSX path with generic `ID15` recording.
- Do not reduce leader qualification back to the earlier short values.
- Do not write CAS headers before the first decoded byte of a block.
- Do not drop the in-block resync logic and go back to aborting a block on every bad byte.

## Practical Summary
MSX recording works because it is no longer treated as "record a waveform and hope playback is good enough".
It is treated as "decode MSX cassette framing while recording, then save bytes as `.cas`".