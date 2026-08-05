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

const char TXT_PREPARING[] PROGMEM = "Preparing";
const char TXT_RECORDING[] PROGMEM = "Recording";

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
  pagepos_t pos = pagePos;
  uint8_t* p = active_page_ptr();
  p[pos] = value;
  pos++;

  if (pos == (pagepos_t)kRecordPageSize) {
    if (other_page_ready()) {
      droppedBytes++;
      pagePos = (pagepos_t)kRecordPageSize - 1;
      return;
    }
    mark_active_ready_and_swap();
    return;
  }

  pagePos = pos;
}

static inline void write_ready_page(uint8_t* buffer) {
  recFile.write((uint8_t *)(buffer), kRecordPageSize);
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

  #if defined(__SAMD21G18A__)
  // If the previous tick's button service re-pointed the ADC back at the
  // record channel, RESULT may still hold a stale button-channel conversion.
  // Discard it (and one more, to guarantee a fresh record sample) before any
  // format handler reads the sample.
  record_adc_flush_pending();
  #endif

  #if defined(RECORD_SHARP_MZF)
    if (recordFormat==RecordFormat::SHARP_MZF)
    {
      isr_mzf();
      goto done;
    }
  #endif
  
  #if defined(RECORD_CAS_MSX)
    if (recordFormat==RecordFormat::CAS_MSX)
    {
      isr_cas();
      goto done;
    }
  #endif
  
  #if defined(RECORD_TZX_ID15) || defined(RECORD_ZX_SPECTRUM)
  isr_tzx();
  #endif

  done:
  #if defined(__SAMD21G18A__)
  record_adc_service_button();
  #endif
  return;
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
  adc_stop();
  gRecordPaused = true;
  printtextF(TXT_PAUSED,0);
}

void resume_recording() {
  if (!gRecording || !gRecordPaused) return;
  gRecordPaused = false;
  // Re-initialise the record channel: analogRead (used by the buttons while
  // paused) leaves the ADC disabled, so the freerun configuration must be
  // restored before the timer restarts.
  adc_start_freerun_record_pin();
  timer_start_recording();
  printtextF(TXT_RECORDING, 0);
}

static void tzx_write_u24_le(SdBaseFile &f, uint32_t v) {
  uint8_t b[3] = { (uint8_t)(v & 0xFF), (uint8_t)((v >> 8) & 0xFF), (uint8_t)((v >> 16) & 0xFF) };
  f.write(b, 3);
}

bool start_recording() {
  if (gRecording) return true;

  printtextF(TXT_PREPARING, 0);
  printtextF(TXT_RECORDING, lineaxy);

  const bool activeCas = (recordFormat == RecordFormat::CAS_MSX);
  const bool activeMzf = (recordFormat == RecordFormat::SHARP_MZF);
  ext3 = activeCas ? PSTR("cas") : activeMzf ? PSTR("mzf") : PSTR("tzx");

  const uint16_t filecount = next_recording_index();
  format_recording_name(gRecName, filecount);
  printtext(gRecName, lineaxy);

  recFile.close();
  if (!recFile.open(currentDir, gRecName, O_RDWR | O_CREAT | O_TRUNC)) {
    printtextF(PSTR("SD open fail"), 0);
    return false;
  }

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

  dataBytesWritten = 0;
  gRecordPaused = false;

  // create file, this can take a while sometimes so we do it before we say we are really recording
  recFile.write((uint8_t)0);
  recFile.flush();
  recFile.seekSet(0);

  // now print 'RECORDING'
  // print this towards the end (in particular after the relatively slow SD file creation)
  printtextF(TXT_RECORDING, 0);

  #if defined(RECORD_TZX_ID15) || defined(RECORD_ZX_SPECTRUM)
  if (!activeCas && !activeMzf) {
    pagePos = 19;
    memcpy_P((void*)wbuffer[0], kTzxHeader, sizeof(kTzxHeader));
    // subtract 19 bytes (TZX header length plus length of ID15 block header) from dataBytesWritten
    // because the dataBytesWritten (part of the ID15 block) does not include the lengths of these...
    // but the sd write function does not know that
    dataBytesWritten-=19; 
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
    write_ready_page((uint8_t *)(wbuffer[0]));
    // Only signify page readiness is false after writing.
    // Don't need noInterrupts/interrupts here because
    // this flag is a single atomic byte variable
    // and isr will either see true or false (it will not change
    // while the isr is running)
    pageReadyA = false;
  }

  if (pageReadyB) {
    // as above for pageReadA
    write_ready_page((uint8_t *)(wbuffer[1]));
    pageReadyB = false;
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
      uint8_t* p = (uint8_t *)(wbuffer[which]);
      if (pos < kRecordPageSize) {
        p[pos] = bb;
        pos++;
      }
    }
  }

  if (pos) {
    uint8_t* p = (uint8_t *)(wbuffer[which]);
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
  currentFile = recFile.dirIndex();
  // update maxFile if necessary, but don't call getMaxFile since that also resets currentFile to 0
  if (currentFile>maxFile)
    maxFile=currentFile;
  recFile.close();

  printtextF(PSTR("Saved"), 0);
  printtext(gRecName, lineaxy);
}

#endif // RECORD
