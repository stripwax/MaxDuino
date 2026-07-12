#include "configs.h"

#ifdef Use_CG

#include "cg.h"
#include "Arduino.h"
#include "MaxDuino.h"
#include "processing_state.h"
#include "file_utils.h"
#include "compat.h"
#include "MaxProcessing.h"

static constexpr byte CG_CGC_SYNC_BYTE = 0x66;

static bool cgcas_init(uint16_t sync_pos) {
  if (readfile(1, 0) != 1) {
    return false;
  }

  uint16_t data_start;
  if (filebuffer[0] == 0xAA) {
    data_start = 0;
    pilotPulses = 0;
  } else {
    data_start = sync_pos;
    pilotPulses = 255;
  }

  bytesRead = data_start;
  currentID = BLOCKID::CGCAS;
  currentTask = TASK::PROCESSID;
  currentBit = 0;
  pass = 0;
  onePulse = CG_ONE_PULSE;
  zeroPulse = CG_ZERO_PULSE;
  return true;
}

bool cgcas_detect_and_init() {
  const byte probe_len = (filesize < FILEBUFFER_SIZE) ? (byte)filesize : FILEBUFFER_SIZE;
  if (probe_len == 0 || readfile(probe_len, 0) != probe_len) {
    return false;
  }

  // Check for text header: "Colour Genie - Virtual Tape File"
  if (filebuffer[0] == 'C' && filebuffer[1] == 'o' && filebuffer[2] == 'l') {
    uint16_t pos = 0;
    while (pos < probe_len && filebuffer[pos]) {
      pos++;
    }
    if (pos >= probe_len) {
      return false;
    }
    // pos is at null terminator. Next byte should be 0x66.
    if (pos + 1 < probe_len) {
      if (filebuffer[pos + 1] == CG_CGC_SYNC_BYTE) {
        return cgcas_init(pos + 1);
      }
    }
    return false;
  }

  // Check for bare 0x66 sync byte (no leader, no header)
  if (filebuffer[0] == CG_CGC_SYNC_BYTE) {
    return cgcas_init(0);
  }

  // Check for leader byte (0x00 or 0xAA) followed by 0x66 at byte 256
  if (filebuffer[0] == 0x00 || filebuffer[0] == 0xAA) {
    if (filesize > 256 && readfile(1, 256) == 1) {
      if (filebuffer[0] == CG_CGC_SYNC_BYTE) {
        return cgcas_init(256);
      }
    }
    return false;
  }

  return false;
}

void cgcas_process() {
  // CG file is so simple there's almost no need for a statemachine.
  // There's just a stream of bytes to render into pulses.
  // The file might or might not contain the pilot - if it doesn't contain
  // it, we detect this, and generate it (by initialising pilotPulses)
  // but if the file does contain the pilot then pilotPulses will be zero
  // Then it's just a case of shifting bits and setting currentPeriod until
  // we run out of file.
  if (pass == 0) {
    if (currentBit == 0) {
      if (pilotPulses > 0) {
        pilotPulses -= 1;
        currentByte = 0xAA;
        currentBit = 8;
      } else if (!ReadByte()) {
        currentID = BLOCKID::IDEOF;
        currentTask = TASK::PROCESSID;
        count_r = 255;
        currentPeriod = 0;
        return;
      } else {
        currentByte = outByte;
        currentBit = 8;
      }
    }
    bool bit = (currentByte & 0x80);
    currentByte <<= 1;
    currentBit -= 1;
    pass = bit ? 2 : 1;
    currentPeriod = bit ? onePulse : zeroPulse;
  } else {
    currentPeriod = onePulse;
  }

  pass -= 1;
}

#endif // Use_CG
