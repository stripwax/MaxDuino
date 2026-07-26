#ifndef RECORD_MSX_H_INCLUDED
#define RECORD_MSX_H_INCLUDED

#include "configs.h"
#include "Arduino.h"

#if defined(RECORD) && defined(RECORD_CAS_MSX)
static constexpr uint16_t kMsxHeaderMinDurationMs = 500;
static constexpr uint16_t kMsxSampleRate = 50000;
static constexpr uint8_t kMsxRecordCenterTrackShift = 6;
static constexpr uint8_t kMsxRecordHysteresis = 4;
static constexpr uint8_t kMsxMinAcceptedEdgeSamples = 2;
static constexpr uint8_t kMsxMinShortCycle = 2;
static constexpr uint8_t kMsxMaxHeaderShortCycle = 40;
static constexpr uint8_t kMsxMinBitCellsBeforeHeader = 50;

void isr_cas();
void msx_reset_capture_state();

#endif //defined(RECORD) && defined(RECORD_CAS_MSX)

bool active_recording_is_cas();

#endif //RECORD_MSX_H_INCLUDED