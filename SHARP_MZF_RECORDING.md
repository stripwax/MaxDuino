# Sharp MZF Recording

## Goal
Add a native Sharp MZ recording mode that saves directly as `.mzf`, without breaking the existing `TZX ID15`, `ZX Spectrum`, or `CAS MSX` recorders.

## Problem We Had
Sharp MZ already recorded cleanly into raw `TZX ID15`, so the hardware was clearly capable of hearing the machine.

But the first native `.mzf` recorder attempts failed in two different ways:
- empty `0` byte `.mzf` files
- recordings that still were not usable as valid Sharp MZ files

That told us the problem was not SD writing or the container itself. The problem was the native Sharp MZ decode path.

## Important Discovery
The decisive clue came from a known-good `TZX ID15` recording of the same Sharp MZ save.

By analysing that working TZX capture, the real pulse structure on this hardware became clear:
- the recorder was seeing stable short half-cycles around `9-11` samples
- and stable long half-cycles around `18-20` samples

So the analog front end was not the main problem.
The real problem was how the native MZF recorder reconstructed Sharp MZ pulses from edges.

## The Real Decoder Bug
The first native MZF recorder paired half-cycles by simply summing every two consecutive edges.

That is unsafe for Sharp MZ PWM.
If capture starts one half-cycle out of phase, the recorder can combine:
- half of one pulse
- with half of the next pulse

That creates fake pulse lengths which then corrupt:
- tapemark detection
- header bytes
- checksums

This is why the recorder could fail even though the same signal produced a good raw `TZX ID15` recording.

## Final Working Design
The final solution is a dedicated runtime record type named `Sharp MZF`.

It is selected from `Record Type ?` and persisted through:
- `MaxDuino/current_settings.h`
- `MaxDuino/current_settings.cpp`
- `MaxDuino/menu.cpp`

The recorder engine lives in:
- `MaxDuino/record.cpp`

The Sharp MZ playback model in:
- `MaxDuino/mzf.cpp`

was used as the structural reference for the recorder.

## Capture Front End
The working MZF recorder uses a simple fixed-threshold front end rather than an adaptive one.

That choice was deliberate.
Sharp MZ was already known to record well into `TZX ID15` with a plain threshold, so the native recorder now follows the same simpler idea:
- sample at `50 kHz`
- slice the input with `sample >= 512`
- reject very short false edges

Important live-capture settings in the final design:
- minimum accepted edge length increased to ignore tiny glitches
- short and long half-cycle windows tuned from the working TZX capture
- shorter leader qualification than the very first over-strict version

## Phase-Safe Pulse Reconstruction
This was the real breakthrough.

The final recorder does not blindly combine every two half-cycles.
It now works like this:
- classify each half-cycle first as `SHORT`, `LONG`, or `INVALID`
- only form a full Sharp MZ pulse from `SHORT + SHORT` or `LONG + LONG`
- if the two halves do not match, discard the old pending half and treat the new one as the new start

That makes the decoder self-correcting if it slips half a pulse out of phase.

This is what finally made native MZF recording work.

## Decode Model
Once pulses are reconstructed properly, the native recorder follows the Sharp MZ tape structure:
- wait for long gap plus long tapemark
- decode header copy 1
- verify header checksum 1
- decode header copy 2
- verify header checksum 2
- wait for short gap plus short tapemark
- decode file bytes
- verify file checksum
- write native `.mzf` output

The header is stored first, and the file body is only written once a valid header has been accepted.

## Why The Working TZX Recording Mattered
The good `TZX ID15` recording of the same save was the truth source.

It showed two important things:
- the hardware and analog path were good enough already
- the native MZF recorder was failing in logic, not because Sharp MZ audio could not be captured

Without that TZX comparison it would have been easy to keep guessing at thresholds and still miss the actual framing bug.

## Build Notes
Adding native MZF recording pushed the Every builds close to flash limits.

To keep the feature fitting, the Every PlatformIO environments use the same AVR size-oriented flags already used elsewhere in the project.
That change lives in:
- `platformio.ini`

## Important Files
- `MaxDuino/record.cpp`
- `MaxDuino/mzf.cpp`
- `MaxDuino/current_settings.h`
- `MaxDuino/current_settings.cpp`
- `MaxDuino/menu.cpp`
- `platformio.ini`

## Things Not To Undo
- Do not go back to pairing Sharp MZ half-cycles in blind consecutive pairs.
- Do not assume Sharp MZ needs a more complex adaptive threshold just because it is a native decoder.
- Do not remove the stronger edge-glitch floor unless you are deliberately retuning the live MZF front end.
- Do not treat a failing native MZF decode as proof that the analog path is bad if a raw `TZX ID15` recording of the same source works.

## Practical Summary
Sharp MZ native recording works because the recorder now decodes the real Sharp MZ pulse structure correctly.

The key fix was not a new file format idea and not extra analog gain.
It was making pulse reconstruction phase-safe so the decoder stops building fake pulses from mismatched half-cycles.
