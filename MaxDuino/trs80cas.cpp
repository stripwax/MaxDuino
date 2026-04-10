#include "configs.h"

#if defined(Use_CAS) && defined(Use_TRS80)

#include "trs80cas.h"
#include "Arduino.h"
#include "MaxDuino.h"
#include "processing_state.h"
#include "MaxProcessing.h"
#include "file_utils.h"
#include "compat.h"

namespace {

enum class TRS80Mode : uint8_t {
  NONE = 0,
  BASIC_L1,
  BASIC_L2,
  SYSTEM_L1,
  SYSTEM_L2,
  HIGHSPEED,
};

static TRS80Mode trs80_mode = TRS80Mode::NONE;
static uint16_t trs80_payload_start = 0;
static bool trs80_sent_direct_header = false;
static uint16_t trs80_leading_silence = 0;
static uint16_t trs80_trailing_silence = 0;
static byte trs80_current_byte = 0;
static uint8_t trs80_bit_mask = 0;
static uint8_t trs80_current_bit_value = 0;
static uint8_t trs80_low_half_phase = 0;
static uint16_t trs80_low_half_pos = 0;
static uint16_t trs80_low_half_samples = 0;
static uint8_t trs80_high_phase = 0;
static uint16_t trs80_high_remaining = 0;
static uint16_t trs80_frac_error = 0;
static bool trs80_eof = false;

static constexpr uint16_t TRS80_SAMPLE_US = 45; // ~= 22.05 kHz direct sample clock
static constexpr uint16_t TRS80_SILENCE_SAMPLES = 11025; // 0.5s at 22050 Hz
static constexpr uint16_t TRS80_SAMPLES_NUM = 245;   // 22050 / gcd(22050, 2027520)
static constexpr uint16_t TRS80_SAMPLES_DEN = 22528; // 2027520 / gcd(22050, 2027520)
static constexpr byte TRS80_MARKER_PROBE_LEN = 20;
static constexpr uint16_t TRS80_RAW_SCAN_LIMIT = 512;

static void reset_byte_state();

static bool is_header_delim(char c) {
  return c == '\0' || c == '\r' || c == '\n' || c == ' ' || c == '\t' || c == ':' || c == ';' || c == ',' || c == ']';
}

static bool parse_marker(const char *s, TRS80Mode &mode, size_t &marker_len) {
  struct Marker { const char *name; TRS80Mode mode; };
  static const Marker markers[] = {
    {"basic-l1", TRS80Mode::BASIC_L1},
    {"basic-l2", TRS80Mode::BASIC_L2},
    {"system-l1", TRS80Mode::SYSTEM_L1},
    {"system-l2", TRS80Mode::SYSTEM_L2},
    {"highspeed", TRS80Mode::HIGHSPEED},
  };

  size_t start = 0;
  if ((uint8_t)s[0] == 0xEF && (uint8_t)s[1] == 0xBB && (uint8_t)s[2] == 0xBF) {
    start = 3;
  }
  if (s[start] == '[') {
    start += 1;
  }

  for (const auto &m : markers) {
    size_t n = strlen(m.name);
    if (strncasecmp(s + start, m.name, n) == 0 && is_header_delim(s[start + n])) {
      mode = m.mode;
      marker_len = start + n;
      while (is_header_delim(s[marker_len])) {
        marker_len += 1;
      }
      return true;
    }
  }

  return false;
}

static bool find_raw_sync(uint16_t &sync_offset) {
  const uint16_t limit = (filesize < TRS80_RAW_SCAN_LIMIT) ? (uint16_t)filesize : TRS80_RAW_SCAN_LIMIT;
  uint16_t offset = 0;

  while (offset < limit) {
    if (readfile(1, offset) != 1) {
      return false;
    }

    if (filebuffer[0] != 0) {
      break;
    }

    offset += 1;
  }

  if (offset >= limit || filebuffer[0] != 0xA5) {
    return false;
  }

  sync_offset = offset;
  return true;
}

static bool detect_raw_mode(TRS80Mode &mode, uint16_t &payload_start) {
  uint16_t sync_offset = 0;
  if (!find_raw_sync(sync_offset)) {
    return false;
  }

  const byte probe_len = (filesize - sync_offset >= 13UL) ? 13 : (byte)(filesize - sync_offset);
  if (probe_len < 5 || readfile(probe_len, sync_offset) != probe_len) {
    return false;
  }

  if (filebuffer[0] != 0xA5) {
    return false;
  }

  if (probe_len >= 10 && filebuffer[1] == 0x55) {
    if (filebuffer[8] != 0x3C && filebuffer[8] != 0x78) {
      return false;
    }

    if (filebuffer[8] == 0x3C) {
      const unsigned long data_len = (filebuffer[9] == 0) ? 256UL : (unsigned long)filebuffer[9];
      if (sync_offset + 12UL + data_len >= filesize) {
        return false;
      }
    }

    mode = TRS80Mode::SYSTEM_L2;
    payload_start = 0;
    return true;
  }

  if (probe_len >= 4 && filebuffer[1] == 0xD3 && filebuffer[2] == 0xD3 && filebuffer[3] == 0xD3) {
    mode = TRS80Mode::BASIC_L2;
    payload_start = 0;
    return true;
  }

  const uint16_t start = word(filebuffer[1], filebuffer[2]);
  const uint16_t end = word(filebuffer[3], filebuffer[4]);
  if (end < start) {
    return false;
  }

  const unsigned long data_len = (unsigned long)(end - start) + 1UL;
  const unsigned long checksum_offset = sync_offset + 5UL + data_len;
  if (checksum_offset >= filesize) {
    return false;
  }

  if (start == 0x4200) {
    mode = TRS80Mode::BASIC_L1;
    payload_start = 0;
    return true;
  }

  if (start >= 0x4000 && start < 0x4200) {
    mode = TRS80Mode::SYSTEM_L1;
    payload_start = 0;
    return true;
  }

  return false;
}

static bool init_mode(TRS80Mode mode, uint16_t payload_start) {
  trs80_mode = mode;
  trs80_payload_start = payload_start;
  trs80_sent_direct_header = false;
  trs80_leading_silence = TRS80_SILENCE_SAMPLES;
  trs80_trailing_silence = TRS80_SILENCE_SAMPLES;
  trs80_frac_error = 0;
  trs80_eof = false;
  bytesRead = trs80_payload_start;
  reset_byte_state();

  switch (trs80_mode) {
    case TRS80Mode::BASIC_L1:
    case TRS80Mode::SYSTEM_L1:
      trs80_low_half_samples = 44; // 88 samples/bit => 250 baud
      break;

    case TRS80Mode::BASIC_L2:
    case TRS80Mode::SYSTEM_L2:
      trs80_low_half_samples = 22; // 44 samples/bit => 500 baud
      break;

    case TRS80Mode::HIGHSPEED:
      trs80_low_half_samples = 0;
      break;

    default:
      return false;
  }

  currentID = BLOCKID::TRS80CAS;
  currentTask = TASK::PROCESSID;
  return true;
}

static uint16_t tcycles_to_samples(uint16_t tcycles) {
  uint32_t scaled = (uint32_t)tcycles * TRS80_SAMPLES_NUM + trs80_frac_error;
  uint16_t whole = (uint16_t)(scaled / TRS80_SAMPLES_DEN);
  trs80_frac_error = (uint16_t)(scaled - (uint32_t)whole * TRS80_SAMPLES_DEN);
  if (whole == 0) {
    return 1;
  }
  return whole;
}

static void reset_byte_state() {
  trs80_current_byte = 0;
  trs80_bit_mask = 0;
  trs80_current_bit_value = 0;
  trs80_low_half_phase = 0;
  trs80_low_half_pos = 0;
  trs80_high_phase = 0;
  trs80_high_remaining = 0;
}

static bool trs80_load_next_byte() {
  if (!ReadByte()) {
    trs80_eof = true;
    reset_byte_state();
    return false;
  }
  trs80_current_byte = outByte;
  trs80_bit_mask = 0x80;
  trs80_current_bit_value = 0;
  trs80_low_half_phase = 0;
  trs80_low_half_pos = 0;
  trs80_high_phase = 0;
  trs80_high_remaining = 0;
  return true;
}

static bool trs80_next_low_speed_level(uint8_t *level) {
  if (trs80_bit_mask == 0) {
    if (!trs80_load_next_byte()) {
      return false;
    }
  }

  if (trs80_low_half_pos == 0) {
    trs80_current_bit_value = (trs80_current_byte & trs80_bit_mask) ? 1 : 0;
  }

  if (trs80_low_half_phase == 0) {
    *level = (trs80_low_half_pos < 3) ? 1 : 0;
  } else {
    *level = (trs80_current_bit_value && trs80_low_half_pos < 3) ? 1 : 0;
  }

  trs80_low_half_pos += 1;
  if (trs80_low_half_pos >= trs80_low_half_samples) {
    trs80_low_half_pos = 0;
    if (trs80_low_half_phase == 0) {
      trs80_low_half_phase = 1;
    } else {
      trs80_low_half_phase = 0;
      trs80_bit_mask >>= 1;
    }
  }

  return true;
}

static bool trs80_next_high_speed_level(uint8_t *level) {
  if (trs80_bit_mask == 0) {
    if (!trs80_load_next_byte()) {
      return false;
    }
  }

  if (trs80_high_remaining == 0) {
    trs80_current_bit_value = (trs80_current_byte & trs80_bit_mask) ? 1 : 0;
    if (trs80_high_phase == 0) {
      trs80_high_remaining = trs80_current_bit_value ? tcycles_to_samples(378) : tcycles_to_samples(771);
    } else {
      trs80_high_remaining = trs80_current_bit_value ? tcycles_to_samples(381) : tcycles_to_samples(765);
    }
  }

  *level = (trs80_high_phase == 0) ? 1 : 0;

  trs80_high_remaining -= 1;
  if (trs80_high_remaining == 0) {
    if (trs80_high_phase == 0) {
      trs80_high_phase = 1;
    } else {
      trs80_high_phase = 0;
      trs80_bit_mask >>= 1;
    }
  }

  return true;
}

static bool trs80_next_level(uint8_t *level) {
  if (trs80_leading_silence) {
    trs80_leading_silence -= 1;
    *level = 0;
    return true;
  }

  if (!trs80_eof) {
    switch (trs80_mode) {
      case TRS80Mode::BASIC_L1:
      case TRS80Mode::SYSTEM_L1:
      case TRS80Mode::BASIC_L2:
      case TRS80Mode::SYSTEM_L2:
        if (trs80_next_low_speed_level(level)) {
          return true;
        }
        break;

      case TRS80Mode::HIGHSPEED:
        if (trs80_next_high_speed_level(level)) {
          return true;
        }
        break;

      default:
        break;
    }
  }

  if (trs80_trailing_silence) {
    trs80_trailing_silence -= 1;
    *level = 0;
    return true;
  }

  return false;
}

} // namespace

bool trs80cas_detect_and_init() {
  const byte header_len = (filesize < TRS80_MARKER_PROBE_LEN) ? (byte)filesize : TRS80_MARKER_PROBE_LEN;
  if (header_len == 0 || readfile(header_len, 0) != header_len) {
    return false;
  }

  char hdr[TRS80_MARKER_PROBE_LEN + 1];
  memset(hdr, 0, sizeof(hdr));
  memcpy(hdr, filebuffer, header_len);

  TRS80Mode mode = TRS80Mode::NONE;
  size_t marker_len = 0;
  uint16_t payload_start = 0;

  if (parse_marker(hdr, mode, marker_len)) {
    payload_start = marker_len;
  } else if (!detect_raw_mode(mode, payload_start)) {
    return false;
  }

  return init_mode(mode, payload_start);
}

void trs80cas_process() {
  if (!trs80_sent_direct_header) {
    trs80_sent_direct_header = true;
    currentPeriod = word(0x6000 | TRS80_SAMPLE_US);
    return;
  }

  uint8_t bits = 0;
  uint8_t count = 0;
  while (count < 8) {
    uint8_t level = 0;
    if (!trs80_next_level(&level)) {
      break;
    }
    bits = (bits << 1) | (level ? 1 : 0);
    count += 1;
  }

  if (count == 0) {
    currentID = BLOCKID::IDEOF;
    currentTask = TASK::PROCESSID;
    count_r = 255;
    currentPeriod = 0;
    return;
  }

  bits <<= (8 - count);
  currentPeriod = word(0x4000 | ((count - 1) << 8) | bits);
}

#endif // defined(Use_CAS) && defined(Use_TRS80)
