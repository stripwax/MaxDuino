#ifndef RECORD_H_INCLUDED
#define RECORD_H_INCLUDED

#include <Arduino.h>

// The recorder defaults to TZX Direct Recording (ID15).
// When the MSX CAS recorder engine is compiled in, the runtime menu can switch
// recording between TZX ID15 and MSX .cas capture.

// NOTE:
// The rest of MaxDuino can be built with REC_TZX disabled.
// Provide stubs in that case to avoid undefined references at link time.

#ifdef REC_TZX

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

// Stop recording, finalize the TZX header/block and close the file.
void stop_recording();

#else

static inline bool is_recording() { return false; }
static inline bool start_recording() { return false; }
static inline void recording_loop() {}
static inline bool is_recording_paused() { return false; }
static inline void pause_recording() {}
static inline void resume_recording() {}
static inline void stop_recording() {}

#endif

#endif // RECORD_H_INCLUDED
