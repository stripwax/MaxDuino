#include "configs.h"
#include "Arduino.h"

#if defined(RECORD) && defined(RECORD_SHARP_MZF)

#include "record.h"
#include "current_settings.h"
#include "record_mzf.h"
#include "record_TimerADC.h"
#include "record_buffers.h"

static byte mzfRecordHeader[128];
static MzfRecStage mzfRecordStage = MzfRecStage::SEEK_LTM;
static uint8_t mzfRecordLevel = 0;
static uint8_t mzfSamplesSinceEdge = 0;
static uint8_t mzfPendingHalfKind = static_cast<uint8_t>(MzfPulseKind::INVALID);
static bool mzfHavePendingHalf = false;
static uint16_t mzfGapShortRun = 0;
static uint8_t mzfMarkerCount = 0;
static uint8_t mzfSeekPhase = 0;
static uint8_t mzfBitMask = 0x80;
static uint8_t mzfCurrentByte = 0;
static bool mzfExpectLeadLong = true;
static uint8_t mzfHeaderIndex = 0;
static uint16_t mzfHeaderChecksumCalc = 0;
static uint16_t mzfChecksumRead = 0;
static uint8_t mzfChecksumBytesRead = 0;
static volatile bool mzfHeaderAccepted = false;
static volatile bool mzfHeaderOutputPending = false;
static bool mzfHeaderWritten = false;
static uint16_t mzfFileLength = 0;
static uint16_t mzfFileBytesDecoded = 0;
static uint16_t mzfFileChecksumCalc = 0;
static bool mzfDecodeComplete = false;

static inline uint8_t mzf_popcount8(byte v) {
  v = v - ((v >> 1) & 0x55);
  v = (v & 0x33) + ((v >> 2) & 0x33);
  return (uint8_t)((((v + (v >> 4)) & 0x0F) * 0x01) & 0x1F);
}

static inline uint16_t mzf_cksum_add(uint16_t acc, byte v) {
  return (uint16_t)(acc + mzf_popcount8(v));
}

static inline void mzf_reset_byte_decoder() {
  mzfBitMask = 0x80;
  mzfCurrentByte = 0;
  mzfExpectLeadLong = true;
}

static inline void mzf_reset_seek(const MzfRecStage stage) {
  mzfRecordStage = stage;
  mzfGapShortRun = 0;
  mzfMarkerCount = 0;
  mzfSeekPhase = 0;
  mzfHavePendingHalf = false;
  mzf_reset_byte_decoder();
}

static inline void mzf_accept_header() {
  mzfHeaderAccepted = true;
  mzfHeaderOutputPending = true;
  mzfFileLength = (uint16_t)mzfRecordHeader[18] | ((uint16_t)mzfRecordHeader[19] << 8);
}

void mzf_reset_capture_state() {
  mzfRecordLevel = 0;
  mzfSamplesSinceEdge = 0;
  mzfPendingHalfKind = static_cast<uint8_t>(MzfPulseKind::INVALID);
  mzfHavePendingHalf = false;
  mzfHeaderIndex = 0;
  mzfHeaderChecksumCalc = 0;
  mzfChecksumRead = 0;
  mzfChecksumBytesRead = 0;
  mzfHeaderAccepted = false;
  mzfHeaderOutputPending = false;
  mzfHeaderWritten = false;
  mzfFileLength = 0;
  mzfFileBytesDecoded = 0;
  mzfFileChecksumCalc = 0;
  mzfDecodeComplete = false;
  mzf_reset_seek(MzfRecStage::SEEK_LTM);
}

static inline MzfPulseKind mzf_classify_half_cycle(const uint8_t halfCycleSamples) {
  if (halfCycleSamples >= kMzfShortHalfMin && halfCycleSamples <= kMzfShortHalfMax) {
    return MzfPulseKind::SHORT;
  }
  if (halfCycleSamples >= kMzfLongHalfMin && halfCycleSamples <= kMzfLongHalfMax) {
    return MzfPulseKind::LONG;
  }
  return MzfPulseKind::INVALID;
}

static inline uint8_t mzf_seek_long_min(const MzfRecStage stage) {
  return (stage == MzfRecStage::SEEK_LTM) ? kMzfLtmLongMin : kMzfStmLongMin;
}

static inline uint8_t mzf_seek_short_min(const MzfRecStage stage) {
  return (stage == MzfRecStage::SEEK_LTM) ? kMzfLtmShortMin : kMzfStmShortMin;
}

static inline void mzf_begin_header_stage(const MzfRecStage stage) {
  mzfRecordStage = stage;
  mzfHeaderIndex = 0;
  mzfHeaderChecksumCalc = 0;
  mzfChecksumRead = 0;
  mzfChecksumBytesRead = 0;
  mzf_reset_byte_decoder();
}

static inline void mzf_begin_file_stage() {
  mzfRecordStage = MzfRecStage::FILE1;
  mzfFileBytesDecoded = 0;
  mzfFileChecksumCalc = 0;
  mzfChecksumRead = 0;
  mzfChecksumBytesRead = 0;
  mzf_reset_byte_decoder();
}

static inline void mzf_process_seek_pulse(const bool isLong) {
  switch (mzfSeekPhase) {
    case 0:
      if (!isLong) {
        if (mzfGapShortRun < 0xFFFF) mzfGapShortRun++;
      } else if (mzfGapShortRun >= kMzfGapMinShortPulses) {
        mzfSeekPhase = 1;
        mzfMarkerCount = 1;
      } else {
        mzfGapShortRun = 0;
      }
      return;

    case 1:
      if (isLong) {
        if (mzfMarkerCount < 0xFF) mzfMarkerCount++;
      } else if (mzfMarkerCount >= mzf_seek_long_min(mzfRecordStage)) {
        mzfSeekPhase = 2;
        mzfMarkerCount = 1;
      } else {
        mzfGapShortRun = 1;
        mzfSeekPhase = 0;
        mzfMarkerCount = 0;
      }
      return;

    case 2:
      if (!isLong) {
        if (mzfMarkerCount < 0xFF) mzfMarkerCount++;
      } else if (mzfMarkerCount >= mzf_seek_short_min(mzfRecordStage)) {
        mzfGapShortRun = 0;
        mzfMarkerCount = 0;
        mzfSeekPhase = 0;
        if (mzfRecordStage == MzfRecStage::SEEK_LTM) {
          mzf_begin_header_stage(MzfRecStage::HDR1);
        } else if (mzfHeaderAccepted) {
          mzf_begin_file_stage();
        } else {
          mzfRecordStage = MzfRecStage::DONE;
        }
      } else {
        mzfSeekPhase = 1;
        mzfMarkerCount = 1;
        mzfGapShortRun = 0;
      }
      return;
  }
}

static inline void mzf_finish_header_checksum() {
  if (!mzfHeaderAccepted && mzfChecksumRead == mzfHeaderChecksumCalc) {
    mzf_accept_header();
  }

  if (mzfRecordStage == MzfRecStage::CHKH1) {
    mzf_begin_header_stage(MzfRecStage::HDR2);
  } else {
    mzf_reset_seek(MzfRecStage::SEEK_STM);
  }
}

static inline void mzf_finish_file_checksum() {
  mzfDecodeComplete = (mzfChecksumRead == mzfFileChecksumCalc);
  mzfRecordStage = MzfRecStage::DONE;
}

static inline void mzf_process_decoded_byte(const byte value) {
  switch (mzfRecordStage) {
    case MzfRecStage::HDR1:
    case MzfRecStage::HDR2:
      if (!mzfHeaderAccepted || mzfRecordStage == MzfRecStage::HDR1) {
        mzfRecordHeader[mzfHeaderIndex] = value;
        mzfHeaderChecksumCalc = mzf_cksum_add(mzfHeaderChecksumCalc, value);
      }
      mzfHeaderIndex++;
      if (mzfHeaderIndex >= sizeof(mzfRecordHeader)) {
        mzfRecordStage = (mzfRecordStage == MzfRecStage::HDR1) ? MzfRecStage::CHKH1 : MzfRecStage::CHKH2;
        mzfChecksumRead = 0;
        mzfChecksumBytesRead = 0;
        mzf_reset_byte_decoder();
      }
      return;

    case MzfRecStage::CHKH1:
    case MzfRecStage::CHKH2:
    case MzfRecStage::CHKF1:
      mzfChecksumRead = (uint16_t)((mzfChecksumRead << 8) | value);
      mzfChecksumBytesRead++;
      if (mzfChecksumBytesRead >= 2) {
        if (mzfRecordStage == MzfRecStage::CHKF1)
          mzf_finish_file_checksum();
        else
          mzf_finish_header_checksum();
      }
      return;

    case MzfRecStage::FILE1:
      if (mzfHeaderWritten) {
        queue_output_byte(value);
      }
      mzfFileChecksumCalc = mzf_cksum_add(mzfFileChecksumCalc, value);
      mzfFileBytesDecoded++;
      if (mzfFileBytesDecoded >= mzfFileLength) {
        mzfRecordStage = MzfRecStage::CHKF1;
        mzfChecksumRead = 0;
        mzfChecksumBytesRead = 0;
        mzf_reset_byte_decoder();
      }
      return;

    default:
      return;
  }
}

static inline void mzf_process_byte_pulse(const bool isLong) {
  if (mzfExpectLeadLong) {
    if (!isLong) {
      return;
    }
    mzfExpectLeadLong = false;
    mzfBitMask = 0x80;
    mzfCurrentByte = 0;
    return;
  }

  if (isLong) {
    mzfCurrentByte |= mzfBitMask;
  }
  mzfBitMask >>= 1;

  if (mzfBitMask == 0) {
    const byte completedByte = mzfCurrentByte;
    mzf_reset_byte_decoder();
    mzf_process_decoded_byte(completedByte);
  }
}

static inline void mzf_process_pulse(const MzfPulseKind kind) {
  if (kind == MzfPulseKind::INVALID) {
    if (mzfRecordStage == MzfRecStage::SEEK_LTM || mzfRecordStage == MzfRecStage::SEEK_STM) {
      mzfGapShortRun = 0;
      mzfMarkerCount = 0;
      mzfSeekPhase = 0;
    } else if (mzfRecordStage != MzfRecStage::DONE) {
      mzf_reset_byte_decoder();
    }
    return;
  }

  if (mzfRecordStage == MzfRecStage::SEEK_LTM || mzfRecordStage == MzfRecStage::SEEK_STM) {
    mzf_process_seek_pulse(kind == MzfPulseKind::LONG);
    return;
  }

  if (mzfRecordStage == MzfRecStage::DONE) {
    return;
  }

  mzf_process_byte_pulse(kind == MzfPulseKind::LONG);
}

static inline void mzf_process_half_cycle(const uint8_t halfCycleSamples) {
  const MzfPulseKind kind = mzf_classify_half_cycle(halfCycleSamples);
  if (kind == MzfPulseKind::INVALID) {
    mzfHavePendingHalf = false;
    mzf_process_pulse(MzfPulseKind::INVALID);
    return;
  }

  if (!mzfHavePendingHalf) {
    mzfPendingHalfKind = static_cast<uint8_t>(kind);
    mzfHavePendingHalf = true;
    return;
  }

  const MzfPulseKind pendingKind = static_cast<MzfPulseKind>(mzfPendingHalfKind);
  if (pendingKind == kind) {
    mzfHavePendingHalf = false;
    mzf_process_pulse(kind);
    return;
  }

  mzfPendingHalfKind = static_cast<uint8_t>(kind);
}

void mzf_flush_pending_header() {
  if (!mzfHeaderOutputPending || mzfHeaderWritten || !mzfHeaderAccepted) return;
  if (pageReadyA || pageReadyB) return;
  if (dataBytesWritten != 0 || pagePos != 0) return;

  uint8_t* p = active_page_ptr();
  for (uint8_t i = 0; i < sizeof(mzfRecordHeader); ++i) {
    p[i] = mzfRecordHeader[i];
  }
  pagePos = sizeof(mzfRecordHeader);
  mzfHeaderWritten = true;
  mzfHeaderOutputPending = false;
}

void isr_mzf()
{
  const uint16_t sample = get_input_value();
  const uint8_t level = (sample >= 512) ? 1 : 0;

  if (mzfSamplesSinceEdge < 0xFF) mzfSamplesSinceEdge++;

  if (level == mzfRecordLevel) {
    if (mzfSamplesSinceEdge >= kMzfResetSilenceSamples) {
      mzfHavePendingHalf = false;
      if (mzfRecordStage == MzfRecStage::SEEK_LTM || mzfRecordStage == MzfRecStage::SEEK_STM) {
        mzfGapShortRun = 0;
        mzfMarkerCount = 0;
        mzfSeekPhase = 0;
      } else if (mzfRecordStage != MzfRecStage::DONE) {
        mzf_reset_byte_decoder();
      }
      mzfSamplesSinceEdge = kMzfResetSilenceSamples;
    }
    return;
  }

  const uint8_t halfCycleSamples = mzfSamplesSinceEdge;
  if (halfCycleSamples == 0) return;
  if (halfCycleSamples < kMzfMinAcceptedEdgeSamples) return;

  mzfRecordLevel = level;
  mzfSamplesSinceEdge = 0;
  mzf_process_half_cycle(halfCycleSamples);
}

bool active_recording_is_mzf()
{
  return recordFormat == RecordFormat::SHARP_MZF;
}

#else

bool active_recording_is_mzf()
{
  return false;
}

#endif //defined(RECORD) && defined(RECORD_SHARP_MZF)