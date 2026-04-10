#include "configs.h"

#include "record.h"

#ifdef REC_TZX

#if defined(REC_CAS_MSX) && !defined(Use_CAS)
#error REC_CAS_MSX requires Use_CAS
#endif

#include <Arduino.h>
#include "sdfat_config.h"
#include <SdFat.h>

#include "Display.h"
#include "file_utils.h"
#include "current_settings.h"

#if defined(REC_CAS_MSX) && defined(Use_CAS)
#include "casProcessing.h"
#endif

extern SdFat sd;

static constexpr uint16_t kRecordPageSize = 512;
static constexpr uint16_t kMsxHeaderMinDurationMs = 500;
static uint8_t pageA[kRecordPageSize];
static uint8_t pageB[kRecordPageSize];
static volatile uint16_t pagePos = 0;
static volatile uint8_t activePage = 0;
static volatile bool pageReadyA = false;
static volatile bool pageReadyB = false;
static volatile uint32_t droppedBytes = 0;

static volatile bool gRecording = false;
static volatile bool gRecordPaused = false;
static char gRecName[32] = {0};
static volatile byte gActiveRecordFormat = static_cast<byte>(RecordFormat::TZX_ID15);

static SdBaseFile recFile;
static uint32_t filePos_usedBits = 0;
static uint32_t filePos_len3 = 0;
static uint32_t dataBytesWritten = 0;

static inline bool active_recording_is_cas()
{
  #if defined(REC_CAS_MSX) && defined(Use_CAS)
    return gActiveRecordFormat == static_cast<byte>(RecordFormat::CAS_MSX);
  #else
    return false;
  #endif
}

static inline bool active_recording_is_mzf()
{
  #if defined(Use_MZF)
    return gActiveRecordFormat == static_cast<byte>(RecordFormat::SHARP_MZF);
  #else
    return false;
  #endif
}

static inline bool active_recording_is_zx_spectrum()
{
  return gActiveRecordFormat == static_cast<byte>(RecordFormat::ZX_SPECTRUM);
}

static RecordFormat select_record_format()
{
  #if defined(REC_CAS_MSX) && defined(Use_CAS)
    if (recordFormat == RecordFormat::CAS_MSX) {
      return RecordFormat::CAS_MSX;
    }
  #endif
  #if defined(Use_MZF)
    if (recordFormat == RecordFormat::SHARP_MZF) {
      return RecordFormat::SHARP_MZF;
    }
  #endif
  if (recordFormat == RecordFormat::ZX_SPECTRUM) {
    return RecordFormat::ZX_SPECTRUM;
  }
  return RecordFormat::TZX_ID15;
}

static void drawRecordingScreenOnce(const char* filename, RecordFormat format)
{
  char l1[17];
  char l2[17];
  memset(l1, ' ', 16);
  memset(l2, ' ', 16);
  l1[16] = '\0';
  l2[16] = '\0';

  const char* msg = "Recording";
  for (uint8_t i = 0; msg[i] && i < 16; ++i) l1[i] = msg[i];
  for (uint8_t i = 0; filename && filename[i] && i < 16; ++i) l2[i] = filename[i];

  #if defined(OLED1306) && defined(XY2)
    sendStrXY(l1, 0, 0);
    sendStrXY(l2, 0, lineaxy);
  #else
    printtext(l1, 0);
    printtext(l2, lineaxy);
  #endif
}

static bool has_ext_ci(const char *name, const char *ext3) {
  if (!name || !ext3) return false;
  const char *dot = strrchr(name, '.');
  if (!dot) return false;
  if (!dot[1] || !dot[2] || !dot[3] || dot[4]) return false;

  auto up = [](char c) -> char {
    if (c >= 'a' && c <= 'z') return (char)(c - 'a' + 'A');
    return c;
  };

  return (up(dot[1]) == up(ext3[0]) && up(dot[2]) == up(ext3[1]) && up(dot[3]) == up(ext3[2]));
}

static uint16_t count_files_with_ext_in_current_dir(const char *ext3) {
  if (!currentDir) return 0;

  const uint32_t savedPos = currentDir->curPosition();
  currentDir->rewind();

  uint16_t count = 0;
  SdBaseFile tmp;
  char name[64];

  while (tmp.openNext(currentDir, O_RDONLY)) {
    if (tmp.isFile()) {
      name[0] = 0;
      tmp.getName(name, sizeof(name));
      if (has_ext_ci(name, ext3)) {
        count++;
      }
    }
    tmp.close();
  }

  currentDir->seekSet(savedPos);
  return count;
}

static bool file_exists_in_current_dir(const char *name) {
  if (!currentDir || !name || !name[0]) return false;

  const uint32_t savedPos = currentDir->curPosition();
  SdBaseFile tmp;
  const bool exists = tmp.open(currentDir, name, O_RDONLY);
  if (exists) {
    tmp.close();
  }
  currentDir->seekSet(savedPos);
  return exists;
}

static void format_recording_name(char *out, size_t outSize, uint16_t index, const char *ext3) {
  if (!out || outSize == 0) return;

  static const char prefix[] = "MaxSave";
  size_t pos = 0;
  for (; pos < sizeof(prefix) - 1 && pos + 1 < outSize; ++pos) {
    out[pos] = prefix[pos];
  }

  char digits[5];
  uint8_t digitCount = 0;
  do {
    digits[digitCount++] = (char)('0' + (index % 10));
    index /= 10;
  } while (index && digitCount < sizeof(digits));

  while (digitCount && pos + 1 < outSize) {
    out[pos++] = digits[--digitCount];
  }

  if (pos + 5 < outSize) {
    out[pos++] = '.';
    out[pos++] = ext3[0];
    out[pos++] = ext3[1];
    out[pos++] = ext3[2];
  }

  out[pos] = '\0';
}

static uint16_t next_recording_index(const char *ext3) {
  char name[32];
  for (uint16_t index = 0; index < 10000; ++index) {
    format_recording_name(name, sizeof(name), index, ext3);
    if (!file_exists_in_current_dir(name)) {
      return index;
    }
  }

  return count_files_with_ext_in_current_dir(ext3);
}

static inline uint8_t* active_page_ptr() {
  return (activePage == 0) ? pageA : pageB;
}

static inline bool other_page_ready() {
  return (activePage == 0) ? pageReadyB : pageReadyA;
}

static inline void mark_active_ready_and_swap() {
  if (activePage == 0) pageReadyA = true;
  else pageReadyB = true;
  activePage ^= 1;
  pagePos = 0;
}

static inline void queue_output_byte(uint8_t value) {
  uint16_t pos = pagePos;
  uint8_t* p = active_page_ptr();
  if (pos < kRecordPageSize) {
    p[pos] = value;
  }
  pos++;

  if (pos >= kRecordPageSize) {
    if (other_page_ready()) {
      droppedBytes++;
      pagePos = kRecordPageSize - 1;
      return;
    }
    pagePos = pos;
    mark_active_ready_and_swap();
    return;
  }

  pagePos = pos;
}

static void write_ready_page(uint8_t which) {
  if (!recFile.isOpen()) return;
  if (which == 0) {
    recFile.write(pageA, kRecordPageSize);
  } else {
    recFile.write(pageB, kRecordPageSize);
  }
  dataBytesWritten += kRecordPageSize;
}

#if defined(__AVR_ATmega4808__) || defined(__AVR_ATmega4809__)

static constexpr uint16_t kMsxSampleRate = 50000;
static constexpr uint16_t kTzxSampleRate = 44100;
#if defined(__AVR_ATmega4809__)
static constexpr uint8_t kWeakZxFilterShift = 2;
static constexpr uint8_t kWeakZxEnvelopeTrackShift = 4;
static constexpr uint8_t kWeakZxCenterTrackShift = 4;
static constexpr uint8_t kWeakZxMinHysteresis = 2;
static constexpr uint8_t kWeakZxMaxHysteresis = 8;
#else
static constexpr uint8_t kWeakZxFilterShift = 2;
static constexpr uint8_t kWeakZxEnvelopeTrackShift = 5;
static constexpr uint8_t kWeakZxCenterTrackShift = 3;
static constexpr uint8_t kWeakZxMinHysteresis = 2;
static constexpr uint8_t kWeakZxMaxHysteresis = 8;
#endif
#if defined(REC_CAS_MSX) && defined(Use_CAS)

static constexpr uint8_t kMsxRecordCenterTrackShift = 6;
static constexpr uint8_t kMsxRecordHysteresis = 4;
static constexpr uint8_t kMsxMinAcceptedEdgeSamples = 2;
static constexpr uint8_t kMsxMinShortCycle = 2;
static constexpr uint8_t kMsxMaxHeaderShortCycle = 40;
static constexpr uint8_t kMsxMinBitCellsBeforeHeader = 50;

static volatile uint16_t msxRecordCenter = 512;
static volatile uint8_t msxRecordLevel = 0;
static volatile uint8_t msxSamplesSinceEdge = 0;
static volatile uint16_t msxHeaderShortRun = 0;
static volatile uint16_t msxHeaderShortSum = 0;
static volatile uint8_t msxShortCycleAvg = 0;
static volatile bool msxHeaderArmed = false;
static volatile bool msxHeaderPending = false;
static volatile bool msxBlockOpen = false;
static volatile bool msxInByte = false;
static volatile uint8_t msxCurrentByte = 0;
static volatile uint8_t msxFrameBitIndex = 0;
static volatile uint8_t msxBitCellCount = 0;
static volatile uint8_t msxBitEdgeCount = 0;
static volatile uint8_t msxExpectedShortMin = 0;
static volatile uint8_t msxExpectedShortMax = 0;
static volatile uint8_t msxExpectedLongMin = 0;
static volatile uint8_t msxExpectedLongMax = 0;
static volatile uint8_t msxSilenceSamples = 0;
static volatile uint16_t msxHeaderShortSamplesNeeded = 4000;

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

static inline void msx_reset_capture_state() {
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
    queue_output_byte(pgm_read_byte(&HEADER[i]));
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
#endif

#if defined(Use_MZF)
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

static byte mzfRecordHeader[128];
static volatile MzfRecStage mzfRecordStage = MzfRecStage::SEEK_LTM;
static volatile uint8_t mzfRecordLevel = 0;
static volatile uint8_t mzfSamplesSinceEdge = 0;
static volatile uint8_t mzfPendingHalfKind = static_cast<uint8_t>(MzfPulseKind::INVALID);
static volatile bool mzfHavePendingHalf = false;
static volatile uint16_t mzfGapShortRun = 0;
static volatile uint8_t mzfMarkerCount = 0;
static volatile uint8_t mzfSeekPhase = 0;
static volatile uint8_t mzfBitMask = 0x80;
static volatile uint8_t mzfCurrentByte = 0;
static volatile bool mzfExpectLeadLong = true;
static volatile uint8_t mzfHeaderIndex = 0;
static volatile uint16_t mzfHeaderChecksumCalc = 0;
static volatile uint16_t mzfChecksumRead = 0;
static volatile uint8_t mzfChecksumBytesRead = 0;
static volatile bool mzfHeaderAccepted = false;
static volatile bool mzfHeaderOutputPending = false;
static volatile bool mzfHeaderWritten = false;
static volatile uint16_t mzfFileLength = 0;
static volatile uint16_t mzfFileBytesDecoded = 0;
static volatile uint16_t mzfFileChecksumCalc = 0;
static volatile bool mzfDecodeComplete = false;

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

static inline void mzf_reset_capture_state() {
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
      mzfChecksumRead = (uint16_t)((mzfChecksumRead << 8) | value);
      mzfChecksumBytesRead++;
      if (mzfChecksumBytesRead >= 2) {
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

    case MzfRecStage::CHKF1:
      mzfChecksumRead = (uint16_t)((mzfChecksumRead << 8) | value);
      mzfChecksumBytesRead++;
      if (mzfChecksumBytesRead >= 2) {
        mzf_finish_file_checksum();
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

static void mzf_flush_pending_header() {
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
#endif

static constexpr uint16_t kTStatesPerSample = 79;
static constexpr uint16_t kPauseAfterMs = 1000;

static const uint8_t kTzxHeader[10] = {
  'Z','X','T','a','p','e','!',
  0x1A,
  0x01, 0x20
};

static volatile uint8_t tzxBitByte = 0;
static volatile uint8_t tzxBitCount = 0;
static volatile uint16_t tzxRecordCenter = 512;
static volatile uint16_t tzxRecordFiltered = 512;
static volatile uint8_t tzxRecordDeviation = 8;
static volatile uint16_t tzxRecordFloor = 512;
static volatile uint16_t tzxRecordCeil = 512;
static volatile uint8_t tzxRecordLevel = 0;
static volatile bool tzxRecordCenterPrimed = false;

static inline void tzx_reset_capture_state() {
  tzxBitByte = 0;
  tzxBitCount = 0;
  tzxRecordCenter = 512;
  tzxRecordFiltered = 512;
  tzxRecordDeviation = 8;
  tzxRecordFloor = 512;
  tzxRecordCeil = 512;
  tzxRecordLevel = 0;
  tzxRecordCenterPrimed = false;
}

static void adc_start_freerun_record_pin() {
  ADC0.CTRLA = 0;
  ADC0.CTRLB = 0;
  ADC0.CTRLC = ADC_PRESC_DIV16_gc;
  ADC0.CTRLD = 0;
  ADC0.SAMPCTRL = 0;

  #if defined(__AVR_ATmega4808__)
    ADC0.MUXPOS = ADC_MUXPOS_AIN15_gc;
  #elif defined(__AVR_ATmega4809__)
    ADC0.MUXPOS = ADC_MUXPOS_AIN5_gc;
  #endif

  ADC0.CTRLA = ADC_ENABLE_bm | ADC_FREERUN_bm;
  ADC0.COMMAND = ADC_STCONV_bm;
}

static void adc_stop() {
  ADC0.CTRLA &= ~(ADC_ENABLE_bm);
}

static void timer_start_recording() {
#if defined(TCB1)
#  define REC_TCB TCB1
#  define REC_TCB_INT_vect TCB1_INT_vect
#else
#  define REC_TCB TCB0
#  define REC_TCB_INT_vect TCB0_INT_vect
#endif

  const uint16_t sampleRate =
      active_recording_is_cas() ? kMsxSampleRate :
      active_recording_is_mzf() ? kMzfSampleRate :
      kTzxSampleRate;
  REC_TCB.CTRLA = 0;
  REC_TCB.CTRLB = TCB_CNTMODE_INT_gc;
  REC_TCB.CCMP  = (uint16_t)(F_CPU / sampleRate);
  REC_TCB.CNT   = 0;
  REC_TCB.INTFLAGS = TCB_CAPT_bm;
  REC_TCB.INTCTRL  = TCB_CAPT_bm;
  REC_TCB.CTRLA = TCB_CLKSEL_CLKDIV1_gc | TCB_ENABLE_bm;
}

static void timer_stop_recording() {
  REC_TCB.INTCTRL = 0;
  REC_TCB.CTRLA = 0;
  REC_TCB.INTFLAGS = TCB_CAPT_bm;
}

ISR(REC_TCB_INT_vect) {
  REC_TCB.INTFLAGS = TCB_CAPT_bm;

  if (!gRecording) return;

#if defined(Use_MZF)
  if (active_recording_is_mzf()) {
    const uint16_t sample = ADC0.RES;
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
    return;
  }
#endif

#if defined(REC_CAS_MSX) && defined(Use_CAS)
  if (active_recording_is_cas()) {
    const uint16_t sample = ADC0.RES;
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
    return;
  }
#endif

  const uint16_t sample = ADC0.RES;
  uint8_t bit;

  if (!active_recording_is_zx_spectrum()) {
    bit = (sample >= 512) ? 1 : 0;
  } else {
    int16_t filtered = (int16_t)tzxRecordFiltered;
    int16_t floor = (int16_t)tzxRecordFloor;
    int16_t ceil = (int16_t)tzxRecordCeil;
    uint8_t hysteresis = tzxRecordDeviation;
    bit = tzxRecordLevel;

    filtered += (((int16_t)sample) - filtered) >> kWeakZxFilterShift;

    if (!tzxRecordCenterPrimed) {
      floor = filtered;
      ceil = filtered;
      hysteresis = kWeakZxMinHysteresis;
      tzxRecordCenterPrimed = true;
      tzxRecordCenter = (uint16_t)filtered;
      bit = (sample >= 512) ? 1 : 0;
    } else {
      if (filtered < floor) {
        floor = filtered;
      } else {
        floor += (filtered - floor) >> kWeakZxEnvelopeTrackShift;
      }

      if (filtered > ceil) {
        ceil = filtered;
      } else {
        ceil += (filtered - ceil) >> kWeakZxEnvelopeTrackShift;
      }

      const uint16_t span = (ceil >= floor) ? (uint16_t)(ceil - floor) : 0;
      hysteresis = (uint8_t)(span >> 7);
      if (hysteresis < kWeakZxMinHysteresis) hysteresis = kWeakZxMinHysteresis;
      if (hysteresis > kWeakZxMaxHysteresis) hysteresis = kWeakZxMaxHysteresis;

      const int16_t centerTarget = (floor + ceil) >> 1;
      int16_t center = (int16_t)tzxRecordCenter;
      center += (centerTarget - center) >> kWeakZxCenterTrackShift;
      const int16_t decisionDelta = (int16_t)sample - center;

      if (decisionDelta >= hysteresis) {
        bit = 1;
      } else if (decisionDelta <= -((int16_t)hysteresis)) {
        bit = 0;
      }
      tzxRecordCenter = (uint16_t)center;
    }

    if (floor < 0) floor = 0;
    else if (floor > 1023) floor = 1023;
    if (ceil < 0) ceil = 0;
    else if (ceil > 1023) ceil = 1023;
    tzxRecordFloor = (uint16_t)floor;
    tzxRecordCeil = (uint16_t)ceil;
    tzxRecordFiltered = (uint16_t)filtered;
    tzxRecordDeviation = hysteresis;
    tzxRecordLevel = bit;
  }

  uint8_t bb = tzxBitByte;
  uint8_t bc = tzxBitCount;
  if (bit) bb |= (uint8_t)(0x80 >> bc);
  bc++;

  if (bc >= 8) {
    uint16_t pos = pagePos;
    uint8_t* p = active_page_ptr();
    p[pos] = bb;
    pos++;

    bb = 0;
    bc = 0;

    if (pos >= kRecordPageSize) {
      if (other_page_ready()) {
        droppedBytes++;
        pos = kRecordPageSize - 1;
      } else {
        pagePos = pos;
        tzxBitByte = bb;
        tzxBitCount = bc;
        mark_active_ready_and_swap();
        return;
      }
    }
    pagePos = pos;
  }

  tzxBitByte = bb;
  tzxBitCount = bc;
}

#else

static void adc_start_freerun_record_pin() {}
static void adc_stop() {}
static void timer_start_recording() {}
static void timer_stop_recording() {}

#endif

bool is_recording() {
  return gRecording;
}

bool is_recording_paused() {
  return gRecordPaused;
}

void pause_recording() {
  if (!gRecording || gRecordPaused) return;
  timer_stop_recording();
  gRecordPaused = true;
  printtext2F(PSTR("Paused       "),0);
}

void resume_recording() {
  if (!gRecording || !gRecordPaused) return;
  gRecordPaused = false;
  timer_start_recording();
  printtext2F(PSTR("Recording    "),0);
}

static void tzx_write_u16_le(SdBaseFile &f, uint16_t v) {
  uint8_t b[2] = { (uint8_t)(v & 0xFF), (uint8_t)(v >> 8) };
  f.write(b, 2);
}

static void tzx_write_u24_le(SdBaseFile &f, uint32_t v) {
  uint8_t b[3] = { (uint8_t)(v & 0xFF), (uint8_t)((v >> 8) & 0xFF), (uint8_t)((v >> 16) & 0xFF) };
  f.write(b, 3);
}

bool start_recording() {
  if (gRecording) return true;

  const RecordFormat activeFormat = select_record_format();
  const bool activeCas = (activeFormat == RecordFormat::CAS_MSX);
  const bool activeMzf = (activeFormat == RecordFormat::SHARP_MZF);
  const char *ext3 = activeCas ? "cas" : (activeMzf ? "mzf" : "tzx");

  const uint16_t filecount = next_recording_index(ext3);

  char recName[32];
  format_recording_name(recName, sizeof(recName), filecount, ext3);
  strncpy(gRecName, recName, sizeof(gRecName) - 1);
  gRecName[sizeof(gRecName) - 1] = 0;

  recFile.close();
  if (!recFile.open(currentDir, recName, O_RDWR | O_CREAT | O_TRUNC)) {
    printtextF(PSTR("SD open fail"), 0);
    return false;
  }

  noInterrupts();
  pagePos = 0;
  activePage = 0;
  pageReadyA = false;
  pageReadyB = false;
  droppedBytes = 0;
  gActiveRecordFormat = static_cast<byte>(activeFormat);
  if (activeCas) {
    #if defined(REC_CAS_MSX) && defined(Use_CAS)
    msx_reset_capture_state();
    #endif
  } else if (activeMzf) {
    #if defined(Use_MZF)
    mzf_reset_capture_state();
    #endif
  } else {
    tzx_reset_capture_state();
  }
  interrupts();

  dataBytesWritten = 0;
  gRecordPaused = false;

  drawRecordingScreenOnce(recName, activeFormat);

  if (!activeCas && !activeMzf) {
    recFile.write(kTzxHeader, sizeof(kTzxHeader));
    recFile.write((uint8_t)0x15);
    tzx_write_u16_le(recFile, kTStatesPerSample);
    tzx_write_u16_le(recFile, kPauseAfterMs);
    filePos_usedBits = recFile.curPosition();
    recFile.write((uint8_t)8);
    filePos_len3 = recFile.curPosition();
    tzx_write_u24_le(recFile, 0);
    recFile.flush();
  }

  adc_start_freerun_record_pin();
  gRecording = true;
  timer_start_recording();
  return true;
}

static void service_record_output() {
  #if defined(Use_MZF)
    if (active_recording_is_mzf()) {
      mzf_flush_pending_header();
    }
  #endif

  if (pageReadyA) {
    noInterrupts();
    pageReadyA = false;
    interrupts();
    write_ready_page(0);
  }

  if (pageReadyB) {
    noInterrupts();
    pageReadyB = false;
    interrupts();
    write_ready_page(1);
  }
}

void recording_loop() {
  if (!gRecording) return;
  service_record_output();
}

void stop_recording() {
  if (!gRecording) return;

  timer_stop_recording();
  gRecording = false;
  gRecordPaused = false;
  adc_stop();

  service_record_output();

  uint16_t pos;
  uint8_t bb = 0;
  uint8_t bc = 0;
  uint8_t which;
  const bool activeCas = active_recording_is_cas();
  const bool activeMzf = active_recording_is_mzf();
  noInterrupts();
  pos = pagePos;
  which = activePage;
  if (!activeCas && !activeMzf) {
    bb = tzxBitByte;
    bc = tzxBitCount;
  }
  pagePos = 0;
  interrupts();

  uint8_t usedBitsLast = 8;
  if (!activeCas && !activeMzf) {
    if (bc != 0) {
      usedBitsLast = bc;
      uint8_t* p = (which == 0) ? pageA : pageB;
      if (pos < kRecordPageSize) {
        p[pos] = bb;
        pos++;
      }
    }
  }

  if (pos) {
    uint8_t* p = (which == 0) ? pageA : pageB;
    recFile.write(p, pos);
    dataBytesWritten += pos;
  }

  if (!activeCas && !activeMzf) {
    recFile.seekSet(filePos_usedBits);
    recFile.write(usedBitsLast);

    recFile.seekSet(filePos_len3);
    tzx_write_u24_le(recFile, dataBytesWritten);
  }

  recFile.flush();
  recFile.close();

  printtextF(PSTR("Saved"), 0);
  printtext(gRecName, lineaxy);
}

#endif // REC_TZX



