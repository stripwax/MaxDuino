#ifndef RECORD_TZX_H_INCLUDED
#define RECORD_TZX_H_INCLUDED

#include "configs.h"
#include "Arduino.h"

#if defined(RECORD_TZX_ID15) || defined(RECORD_ZX_SPECTRUM)

static constexpr uint16_t kPauseAfterMs = 1000;
static constexpr uint16_t kTStatesPerSample = 79;
static constexpr uint16_t kTzxSampleRate = 44100;

#if defined(__AVR_ATmega4809__)
static constexpr uint8_t kWeakZxFilterShift = 2;
static constexpr uint8_t kWeakZxEnvelopeTrackShift = 4;
static constexpr uint8_t kWeakZxCenterTrackShift = 4;
static constexpr uint8_t kWeakZxMinHysteresis = 2;
static constexpr uint8_t kWeakZxMaxHysteresis = 8;
#elif defined(__AVR_ATmega4808__)
static constexpr uint8_t kWeakZxFilterShift = 2;
static constexpr uint8_t kWeakZxEnvelopeTrackShift = 5;
static constexpr uint8_t kWeakZxCenterTrackShift = 3;
static constexpr uint8_t kWeakZxMinHysteresis = 2;
static constexpr uint8_t kWeakZxMaxHysteresis = 8;
#else
//#error ZX Filter/Hysteresis not defined/calibrated for this MCU
static constexpr uint8_t kWeakZxFilterShift = 2;
static constexpr uint8_t kWeakZxEnvelopeTrackShift = 5;
static constexpr uint8_t kWeakZxCenterTrackShift = 3;
static constexpr uint8_t kWeakZxMinHysteresis = 2;
static constexpr uint8_t kWeakZxMaxHysteresis = 8;
#endif

static const uint8_t kTzxHeader[10] = {
  'Z','X','T','a','p','e','!',
  0x1A,
  0x01, 0x20
};

void isr_tzx();
void tzx_reset_capture_state();

extern volatile uint8_t tzxBitByte;
extern volatile uint8_t tzxBitCount;

#endif //defined(RECORD_TZX_ID15) || defined(RECORD_ZX_SPECTRUM)

bool active_recording_is_zx_spectrum();

#endif //RECORD_TZX_H_INCLUDED