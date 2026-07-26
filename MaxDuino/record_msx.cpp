#include "Arduino.h"
#include "configs.h"

#if defined(RECORD) && defined(RECORD_CAS_MSX)
#include "current_settings.h"
#include "record_msx.h"
#include "record_TimerADC.h"
#include "casProcessing.h"

static uint16_t msxRecordCenter = 512;
static uint8_t msxRecordLevel = 0;
static uint8_t msxSamplesSinceEdge = 0;
static uint16_t msxHeaderShortRun = 0;
static uint16_t msxHeaderShortSum = 0;
static uint8_t msxShortCycleAvg = 0;
static bool msxHeaderArmed = false;
static bool msxHeaderPending = false;
static bool msxBlockOpen = false;
static bool msxInByte = false;
static uint8_t msxCurrentByte = 0;
static uint8_t msxFrameBitIndex = 0;
static uint8_t msxBitCellCount = 0;
static uint8_t msxBitEdgeCount = 0;
static uint8_t msxExpectedShortMin = 0;
static uint8_t msxExpectedShortMax = 0;
static uint8_t msxExpectedLongMin = 0;
static uint8_t msxExpectedLongMax = 0;
static uint8_t msxSilenceSamples = 0;
static uint16_t msxHeaderShortSamplesNeeded = 4000;

static inline void msx_restore_header_detection() {
  msxShortCycleAvg = 0;
  msxExpectedShortMin = kMsxMinShortCycle;
  msxExpectedShortMax = kMsxMaxHeaderShortCycle;
  msxExpectedLongMin = kMsxMaxHeaderShortCycle + 1u;
  msxExpectedLongMax = 80u;
  msxSilenceSamples = 160u;
}

static inline void msx_reset_header_state() {
  msxHeaderShortRun = 0;
  msxHeaderShortSum = 0;
  msxHeaderArmed = false;
}

static inline void msx_reset_byte_state() {
  msxInByte = false;
  msxCurrentByte = 0;
  msxFrameBitIndex = 0;
  msxBitCellCount = 0;
  msxBitEdgeCount = 0;
}

static inline void msx_abort_block() {
  msxBlockOpen = false;
  msxHeaderPending = false;
  msx_restore_header_detection();
  msx_reset_header_state();
  msx_reset_byte_state();
}

static inline void msx_resync_open_block() {
  msx_reset_byte_state();
}

static inline void msx_set_cycle_expectations(uint16_t shortCycleSamples) {
  uint8_t shortMin = (shortCycleSamples * 3u) / 4u;
  uint8_t shortMax = (shortCycleSamples * 5u) / 4u + 1u;
  const uint16_t longCycleSamples = shortCycleSamples * 2u;
  uint8_t longMin = (longCycleSamples * 3u) / 4u;
  uint8_t longMax = (longCycleSamples * 5u) / 4u + 2u;

  if (shortMin < kMsxMinShortCycle) shortMin = kMsxMinShortCycle;
  if (shortMax <= shortMin) shortMax = shortMin + 1u;
  if (longMin <= shortMax) longMin = shortMax + 1u;
  if (longMax <= longMin) longMax = longMin + 1u;

  msxExpectedShortMin = shortMin;
  msxExpectedShortMax = shortMax;
  msxExpectedLongMin = longMin;
  msxExpectedLongMax = longMax;

  uint16_t silence = longMax * 8u;
  if (silence < 32u) silence = 32u;
  if (silence > 250u) silence = 250u;
  msxSilenceSamples = (uint8_t)silence;
}

void msx_reset_capture_state() {
  msxRecordCenter = 512;
  msxRecordLevel = 0;
  msxSamplesSinceEdge = 0;
  msxHeaderPending = false;
  msxBlockOpen = false;
  msx_reset_header_state();
  msx_reset_byte_state();
  msx_restore_header_detection();
  msxHeaderShortSamplesNeeded =
      (uint16_t)(((uint32_t)kMsxSampleRate * kMsxHeaderMinDurationMs + 999u) / 1000u);
}

static inline bool msx_is_short_cycle(uint8_t cycleSamples) {
  return cycleSamples >= msxExpectedShortMin && cycleSamples <= msxExpectedShortMax;
}

static inline bool msx_is_long_cycle(uint8_t cycleSamples) {
  return cycleSamples >= msxExpectedLongMin && cycleSamples <= msxExpectedLongMax;
}

static inline void msx_write_cas_header() {
  for (uint8_t i = 0; i < 8; ++i) {
    queue_output_byte(pgm_read_byte(&CAS_HEADER[i]));
  }
}

static inline void msx_start_byte() {
  msxInByte = true;
  msxCurrentByte = 0;
  // The first long interval after the header is the first half of the start bit.
  msxFrameBitIndex = 0;
  msxBitCellCount = 2;
  msxBitEdgeCount = 1;
}

static inline void msx_process_data_cycle(uint8_t cycleSamples) {
  uint8_t intervalCells = 0;
  if (msx_is_short_cycle(cycleSamples)) {
    intervalCells = 1;
  } else if (msx_is_long_cycle(cycleSamples)) {
    intervalCells = 2;
  } else {
    // Ignore obvious glitch edges that are much shorter than a real cell.
    if (cycleSamples < msxExpectedShortMin && cycleSamples + 1u < msxExpectedShortMin) {
      return;
    }
    msx_resync_open_block();
    return;
  }

  msxBitCellCount = (uint8_t)(msxBitCellCount + intervalCells);
  msxBitEdgeCount++;
  if (msxBitCellCount < 4) {
    return;
  }

  if (msxBitCellCount > 4) {
    msx_resync_open_block();
    return;
  }

  const uint8_t edges = msxBitEdgeCount;
  msxBitCellCount = 0;
  msxBitEdgeCount = 0;

  uint8_t bit;
  if (edges == 2) {
    bit = 0;
  } else if (edges == 4) {
    bit = 1;
  } else {
    msx_resync_open_block();
    return;
  }

  if (msxFrameBitIndex == 0) {
    if (bit != 0) {
      msx_resync_open_block();
      return;
    }
    msxFrameBitIndex = 1;
    return;
  }

  if (msxFrameBitIndex <= 8) {
    if (bit) {
      msxCurrentByte |= (uint8_t)(1u << (msxFrameBitIndex - 1u));
    }
    msxFrameBitIndex++;
    return;
  }

  if (bit != 1) {
    msx_resync_open_block();
    return;
  }

  msxFrameBitIndex++;
  if (msxFrameBitIndex <= 10) {
    return;
  }

  if (msxHeaderPending) {
    msx_write_cas_header();
    msxHeaderPending = false;
  }
  queue_output_byte(msxCurrentByte);
  msx_reset_byte_state();
  msxBlockOpen = true;
}

static inline void msx_process_sync_cycle(uint8_t cycleSamples) {
  if (msxBlockOpen) {
    msx_reset_header_state();
    if (msx_is_long_cycle(cycleSamples)) {
      msx_start_byte();
    }
    return;
  }

  if (msx_is_short_cycle(cycleSamples)) {
    if (msxHeaderShortRun < 0xFFFF) msxHeaderShortRun++;
    if ((uint16_t)(0xFFFFu - msxHeaderShortSum) < cycleSamples) {
      msxHeaderShortSum = 0xFFFFu;
    } else {
      msxHeaderShortSum += cycleSamples;
    }

    if (msxHeaderShortRun >= kMsxMinBitCellsBeforeHeader &&
        msxHeaderShortSum >= msxHeaderShortSamplesNeeded) {
      if (!msxHeaderArmed) {
        uint16_t avg = (msxHeaderShortSum + (msxHeaderShortRun / 2u)) / msxHeaderShortRun;
        if (avg < kMsxMinShortCycle) avg = kMsxMinShortCycle;
        if (avg > kMsxMaxHeaderShortCycle) avg = kMsxMaxHeaderShortCycle;
        msxShortCycleAvg = (uint8_t)avg;
        msx_set_cycle_expectations(avg);
      }
      msxHeaderArmed = true;
    }
    return;
  }

  if (msx_is_long_cycle(cycleSamples)) {
    if (msxHeaderArmed) {
      msxHeaderPending = true;
      msxBlockOpen = true;
      msx_reset_header_state();
      msx_start_byte();
      return;
    }
  }

  msx_restore_header_detection();
  msx_reset_header_state();
}

static inline void msx_process_cycle(uint8_t cycleSamples) {
  if (msxInByte) {
    msx_process_data_cycle(cycleSamples);
  } else {
    msx_process_sync_cycle(cycleSamples);
  }
}

void isr_cas()
{
    const uint16_t sample = get_input_value();
    int16_t center = (int16_t)msxRecordCenter;
    uint8_t level = msxRecordLevel;

    if ((int16_t)sample >= center + kMsxRecordHysteresis) {
      level = 1;
    } else if ((int16_t)sample <= center - kMsxRecordHysteresis) {
      level = 0;
    }

    center += (((int16_t)sample) - center) >> kMsxRecordCenterTrackShift;
    if (center < 0) center = 0;
    else if (center > 1023) center = 1023;
    msxRecordCenter = (uint16_t)center;

    if (msxSamplesSinceEdge < 0xFF) msxSamplesSinceEdge++;

    if (level == msxRecordLevel) {
      if (msxSamplesSinceEdge >= msxSilenceSamples) {
        msx_abort_block();
      }
      return;
    }

    const uint8_t halfCycleSamples = msxSamplesSinceEdge;
    if (halfCycleSamples == 0) return;
    if (halfCycleSamples < kMsxMinAcceptedEdgeSamples) return;

    msxRecordLevel = level;
    msxSamplesSinceEdge = 0;
    msx_process_cycle(halfCycleSamples);
}

bool active_recording_is_cas()
{
  return recordFormat == RecordFormat::CAS_MSX;
}

#else

bool active_recording_is_cas()
{
  return false;
}

#endif // defined(RECORD) && defined(RECORD_CAS_MSX)
