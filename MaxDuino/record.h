#ifndef RECORD_H_INCLUDED
#define RECORD_H_INCLUDED

#include <Arduino.h>
#include "configs.h"

// The recorder supports multiple runtime-selectable formats.
// Compile-time feature flags decide which record types appear in the menu.

// NOTE:
// The rest of MaxDuino can be built with RECORD disabled.
// Provide stubs in that case to avoid undefined references at link time.

#ifdef RECORD

// Returns true when recording is active.
bool is_recording();

// Start recording into a generated file in the current directory.
// Returns true if recording was started successfully.
bool start_recording();

// Service routine to be called frequently from loop() while recording.
// Handles SD writes for filled buffers.
void recording_loop();

// Recording pause state helpers used by motor-control integration.
bool is_recording_paused();
void pause_recording();
void resume_recording();

// Stop recording, finalize the active file format and close the file.
void stop_recording();

// For config, settings, menu, etc
enum class RecordFormat : byte {
  TZX_ID15 = 0,
  CAS_MSX = 1,
  ZX_SPECTRUM = 2,
  SHARP_MZF = 3,
  _COUNT,  /* put this after the end, so it's always equals one after the last one */
  _NONE
};
#define RECORD_FORMAT_MASK 0x07  /* bit mask for record format settings within eeprom stored byte */
bool isRecordFormatSupported(const RecordFormat format);
RecordFormat defaultRecordFormat();

#else

static inline bool is_recording() { return false; }

#endif

#endif // RECORD_H_INCLUDED
