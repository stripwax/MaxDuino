#ifndef RECORD_MZF_H_INCLUDED
#define RECORD_MZF_H_INCLUDED

#include "configs.h"
#include "Arduino.h"

#if defined(RECORD) && defined(RECORD_SHARP_MZF)
static constexpr uint16_t kMzfSampleRate = 50000;
static constexpr uint8_t kMzfMinAcceptedEdgeSamples = 6;
static constexpr uint8_t kMzfResetSilenceSamples = 120;
static constexpr uint8_t kMzfGapMinShortPulses = 32;
static constexpr uint8_t kMzfShortHalfMin = 8;
static constexpr uint8_t kMzfShortHalfMax = 16;
static constexpr uint8_t kMzfLongHalfMin = 18;
static constexpr uint8_t kMzfLongHalfMax = 30;
static constexpr uint8_t kMzfLtmLongMin = 30;
static constexpr uint8_t kMzfLtmShortMin = 30;
static constexpr uint8_t kMzfStmLongMin = 15;
static constexpr uint8_t kMzfStmShortMin = 15;

enum class MzfRecStage : uint8_t {
  SEEK_LTM,
  HDR1,
  CHKH1,
  HDR2,
  CHKH2,
  SEEK_STM,
  FILE1,
  CHKF1,
  DONE
};

enum class MzfPulseKind : uint8_t {
  INVALID,
  SHORT,
  LONG
};

void isr_mzf();
void mzf_reset_capture_state();
void mzf_flush_pending_header();

#endif //defined(RECORD) && defined(RECORD_SHARP_MZF)

bool active_recording_is_mzf();

#endif //RECORD_MZF_H_INCLUDED