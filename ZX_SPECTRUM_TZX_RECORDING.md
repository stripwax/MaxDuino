# ZX Spectrum TZX Recording

## Goal
Improve recording from older ZX Spectrum machines with poor cassette output, without damaging normal `TZX ID15` recording.

## Main Discovery
The initial assumption was that weak Spectrum sources might need to be "recorded with more amplitude".
That turned out to be the wrong model.

`TZX ID15` stores only 1-bit sample levels. It does not store analog amplitude.
So the real problem was not file amplitude. It was input slicing quality while recording.

Diagnostics showed that the troublesome Spectrum source was not simply tiny at the ADC. It was badly biased and asymmetric. One useful capture showed a range roughly like:
- min `25`
- max `1023`

So the issue was: the recorder had to make correct 1/0 decisions from a clipped, offset waveform.

## Why We Did Not Switch To `.tap`
A dedicated Sinclair `.tap` recorder was considered, but not used.

Reason:
- `.tap` is a standard ROM-save representation
- `TZX ID15` can preserve arbitrary waveform structure and custom loaders better

So the final solution keeps the `TZX` container and improves the front end instead.

## Final Working Design
The Spectrum-specific solution is a separate runtime record type named `ZX Spectrum`.

That mode is part of the same `Record Type ?` setting as:
- `TZX ID15`
- `ZX Spectrum`
- `CAS MSX`

The menu and persisted setting live in:
- `MaxDuino/current_settings.h`
- `MaxDuino/current_settings.cpp`
- `MaxDuino/menu.cpp`

## How `ZX Spectrum` Mode Differs From Normal `TZX ID15`
Normal `TZX ID15` recording stays on the simple archive-style slicer:
- `44.1 kHz`
- `79` T-states per sample
- fixed threshold `sample >= 512`

`ZX Spectrum` mode still writes the same `TZX ID15` format, but changes only the front-end bit decision.

The Spectrum-specific path in `MaxDuino/record.cpp` now does this:
- smooths the incoming ADC stream slightly
- tracks low and high envelopes of the incoming waveform
- derives hysteresis from the current signal span
- clamps that hysteresis so it stays useful on real hardware
- tracks the decision center separately instead of snapping instantly to the raw midpoint
- writes the resulting 1-bit decisions into the same packed `ID15` stream as normal TZX mode

That means:
- format is unchanged
- playback path is unchanged
- only the way weak/clipped Spectrum audio is sliced during recording is different

## The Final Tuning That Worked
The working Spectrum path did not come from one change. It came from a sequence of narrower adjustments.

What worked in the end:
- Keep normal `TZX ID15` untouched
- Use a Spectrum-only front end
- Let the envelope follow the waveform a bit faster so data blocks do not get smeared
- Smooth the decision center separately so the long pilot/header tone stays steadier

This was the important balance:
- if the whole path was smoothed too much, the cyan/red pilot bars looked better but the blue/yellow data blocks failed
- if the center reacted too fast, the data could work but the pilot/header phase became less stable

The final compromise was:
- keep the faster data response
- smooth only the center threshold slightly

That is why the latest working code uses all three controls together:
- `kWeakZxFilterShift`
- `kWeakZxEnvelopeTrackShift`
- `kWeakZxCenterTrackShift`

## 4808 And 4809 Split
A later discovery was that the same Spectrum tuning did not behave identically on the two Every-class boards.

Observed behavior:
- `ATmega4808` Thinary worked well with the shared Spectrum tuning
- `ATmega4809` Nano Every still needed more help

The likely reason is not CPU speed. Both builds use the same recorder logic and sample rate. The more plausible difference is the analog path and pin mapping:
- `4808` record input uses `A7 = PF5 = AIN15`
- `4809` record input uses `A7 = PD5 = AIN5`

So the final code keeps the `4808` on the known-good shared values and gives the `4809` its own Spectrum constants.

Final split in `MaxDuino/record.cpp`:
- `4808`: keep `kWeakZxEnvelopeTrackShift = 5` and `kWeakZxCenterTrackShift = 3`
- `4809`: use `kWeakZxEnvelopeTrackShift = 4` and `kWeakZxCenterTrackShift = 4`

What that means in practice:
- `4808` stays on the tuning that already worked well
- `4809` gets a slightly faster envelope response but a slightly more damped center

This let the Nano Every keep the stronger Spectrum-data behavior without disturbing the Thinary path.

## Why A Separate Record Type Was Important
Trying to improve all TZX recording globally made normal recordings worse.

Normal TZX worked best when left alone.
The Spectrum problem was specific enough that it needed its own runtime mode instead of a global tweak.

That is why `ZX Spectrum` exists as a separate option rather than as a hidden threshold tweak.

## Important Files
- `MaxDuino/record.cpp`
- `MaxDuino/current_settings.h`
- `MaxDuino/current_settings.cpp`
- `MaxDuino/menu.cpp`
- `MaxDuino/pinSetup.cpp`

## Things Not To Undo
- Do not fold the Spectrum front end into normal `TZX ID15` mode.
- Do not go back to a plain fixed-threshold slicer for the Spectrum mode.
- Do not remove the separate center smoothing from the Spectrum path unless you want to retune pilot/header stability again.
- Do not collapse the `4808` and `4809` Spectrum constants back together unless you are deliberately retuning both boards again.
- Do not replace Spectrum recording with `.tap` unless the goal is specifically standard ROM-save conversion.

## Practical Summary
`ZX Spectrum` mode works because it solves the real problem:
- not the amplitude stored in the file
- but the correctness of the 1-bit decisions made from a weak, clipped, biased Spectrum input signal

The output is still `TZX ID15`, but the input slicer is tailored for older Spectrum cassette hardware and now balances both:
- steadier pilot/header detection
- cleaner data-block slicing
- separate 4808/4809 tuning where the boards need it
