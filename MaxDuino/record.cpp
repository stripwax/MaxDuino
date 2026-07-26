#include "configs.h"

#ifdef RECORD

#include "record.h"
#include "buffer.h"
#include <Arduino.h>
#include "sdfat_config.h"
#include <SdFat.h>
#include "record_TimerADC.h"
#include "record_buffers.h"
#include "Display.h"
#include "file_utils.h"
#include "current_settings.h"

#include "record_msx.h"
#include "record_mzf.h"
#include "record_tzx.h"

#if defined(RECORD_CAS_MSX)
#include "casProcessing.h"
#endif

static bool gRecording = false;
static bool gRecordPaused = false;
static const char prefix[] PROGMEM = "MaxSave";
static char gRecName[17];  // "MaxSavennnnn.ext" needs only 17 bytes incl NUL terminator. Be careful if you change prefix of course!
static const char * ext3;  // required file extension for a given recording format. This is set when beginning recording.

static SdBaseFile recFile;
static uint32_t filePos_usedBits = 0;
static uint32_t filePos_len3 = 0;

bool isRecordFormatSupported(const RecordFormat format)
{
  switch (format) {
    #if defined(RECORD_TZX_ID15)
      case RecordFormat::TZX_ID15:
        return true;
    #endif
    #if defined(RECORD_CAS_MSX)
      case RecordFormat::CAS_MSX:
        return true;
    #endif
    #if defined(RECORD_ZX_SPECTRUM)
      case RecordFormat::ZX_SPECTRUM:
        return true;
    #endif
    #if defined(RECORD_SHARP_MZF)
      case RecordFormat::SHARP_MZF:
        return true;
    #endif
      default:
        return false;
  }
}

RecordFormat defaultRecordFormat()
{
  #if defined(RECORD_TZX_ID15)
    return RecordFormat::TZX_ID15;
  #elif defined(RECORD_ZX_SPECTRUM)
    return RecordFormat::ZX_SPECTRUM;
  #elif defined(RECORD_CAS_MSX)
    return RecordFormat::CAS_MSX;
  #elif defined(RECORD_SHARP_MZF)
    return RecordFormat::SHARP_MZF;
  #else
    // this check here means we don't need to duplicate such check in configs_sanity.h
    #error Either missing case in defaultRecordFormat or you are building with no record formats defined
  #endif
}

const char TXT_RECORDING[] PROGMEM = "Recording";
static void drawRecordingScreenOnce()
{
  printtextF(TXT_RECORDING, 0);
  printtext(gRecName, lineaxy);
}

static bool has_ext_ci() {
  filenameExt = strrchr(fileName,'.') + 1;
  return (strcasecmp_P(filenameExt, ext3) == 0);
}

static uint16_t count_files_with_ext_in_current_dir() {
  currentDir->rewind();

  uint16_t count = 0;
  SdBaseFile tmp;

  while (tmp.openNext(currentDir, O_RDONLY)) {
    if (tmp.isFile()) {
      tmp.getName(fileName,filenameLength);
      if (has_ext_ci()) {
        count++;
      }
    }
    tmp.close();
  }

  return count;
}

static bool file_exists_in_current_dir(const char *name) {
  SdBaseFile tmp;
  const bool exists = tmp.open(currentDir, name, O_RDONLY);
  if (exists) {
    tmp.close();
  }
  return exists;
}

static void format_recording_name(char *out, uint16_t index) {
  strncpy_P(out, prefix, sizeof(prefix) - 1);
  out += sizeof(prefix) - 1;
  ultoa(index, out, 10);
  strcat_P(out, PSTR("."));
  strcat_P(out, ext3);
}

static uint16_t next_recording_index() {
  for (uint16_t index = 0; index < 10000; ++index) {
    format_recording_name(fileName, index);
    if (!file_exists_in_current_dir(fileName)) {
      return index;
    }
  }

  return count_files_with_ext_in_current_dir();
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

void queue_output_byte(uint8_t value) {
  uint16_t pos = pagePos;
  volatile uint8_t* p = active_page_ptr();
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
    recFile.write((uint8_t *)pageA, kRecordPageSize);
  } else {
    recFile.write((uint8_t *)pageB, kRecordPageSize);
  }
  dataBytesWritten += kRecordPageSize;
}

uint16_t default_sample_rate_for_format() {
    return
#if defined(RECORD_CAS_MSX)
    active_recording_is_cas() ? kMsxSampleRate :
#endif
#if defined(RECORD_SHARP_MZF)
    active_recording_is_mzf() ? kMzfSampleRate :
#endif
#if defined(RECORD_TZX_ID15) || defined(RECORD_ZX_SPECTRUM)
    kTzxSampleRate;
#else
    44100; // just some default, we shouldn't get here
#endif
}

void recorder_isr()
{
  if (!gRecording) return;

  #if defined(RECORD_SHARP_MZF)
    if (recordFormat==RecordFormat::SHARP_MZF)
    {
      isr_mzf();
      return;
    }
  #endif
  
  #if defined(RECORD_CAS_MSX)
    if (recordFormat==RecordFormat::CAS_MSX)
    {
      isr_cas();
      return;
    }
  #endif
  
  #if defined(RECORD_TZX_ID15) || defined(RECORD_ZX_SPECTRUM)
  isr_tzx();
  #endif
}


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
  printtextF(TXT_PAUSED,0);
}

void resume_recording() {
  if (!gRecording || !gRecordPaused) return;
  gRecordPaused = false;
  timer_start_recording();
  printtextF(TXT_RECORDING, 0);
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

  const bool activeCas = (recordFormat == RecordFormat::CAS_MSX);
  const bool activeMzf = (recordFormat == RecordFormat::SHARP_MZF);
  ext3 = activeCas ? PSTR("cas") : activeMzf ? PSTR("mzf") : PSTR("tzx");

  const uint16_t filecount = next_recording_index();

  format_recording_name(gRecName, filecount);

  recFile.close();
  if (!recFile.open(currentDir, gRecName, O_RDWR | O_CREAT | O_TRUNC)) {
    printtextF(PSTR("SD open fail"), 0);
    return false;
  }

  noInterrupts();
  pagePos = 0;
  activePage = 0;
  pageReadyA = false;
  pageReadyB = false;
  droppedBytes = 0;
  
  if (activeCas) {
    #if defined(RECORD_CAS_MSX)
    msx_reset_capture_state();
    #endif
  } else if (activeMzf) {
    #if defined(RECORD_SHARP_MZF)
    mzf_reset_capture_state();
    #endif
  } else {
    #if defined(RECORD_TZX_ID15) || defined(RECORD_ZX_SPECTRUM)
    tzx_reset_capture_state();
    #endif
  }
  interrupts();

  dataBytesWritten = 0;
  gRecordPaused = false;

  drawRecordingScreenOnce();

  #if defined(RECORD_TZX_ID15) || defined(RECORD_ZX_SPECTRUM)
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
  #endif

  adc_start_freerun_record_pin();
  gRecording = true;
  timer_start_recording();
  return true;
}

static void service_record_output() {
  #if defined(RECORD_SHARP_MZF)
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
  #if defined(RECORD_TZX_ID15) || defined(RECORD_ZX_SPECTRUM)
  if (!activeCas && !activeMzf) {
    bb = tzxBitByte;
    bc = tzxBitCount;
  }
  #endif
  pagePos = 0;
  interrupts();

  uint8_t usedBitsLast = 8;
  if (!activeCas && !activeMzf) {
    if (bc != 0) {
      usedBitsLast = bc;
      volatile uint8_t* p = (which == 0) ? pageA : pageB;
      if (pos < kRecordPageSize) {
        p[pos] = bb;
        pos++;
      }
    }
  }

  if (pos) {
    volatile uint8_t* p = (which == 0) ? pageA : pageB;
    recFile.write((uint8_t *)p, pos);
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

#endif // RECORD
