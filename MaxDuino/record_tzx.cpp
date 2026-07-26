#include "configs.h"
#include "Arduino.h"

#if defined(RECORD_TZX_ID15) || defined(RECORD_ZX_SPECTRUM)

#include "current_settings.h"
#include "record.h"
#include "record_tzx.h"
#include "record_TimerADC.h"

volatile uint8_t tzxBitByte;
volatile uint8_t tzxBitCount;
#if defined(RECORD_ZX_SPECTRUM)
static uint16_t tzxRecordCenter = 512;
static uint16_t tzxRecordFiltered = 512;
static uint8_t tzxRecordDeviation = 8;
static uint16_t tzxRecordFloor = 512;
static uint16_t tzxRecordCeil = 512;
static uint8_t tzxRecordLevel = 0;
static bool tzxRecordCenterPrimed = false;
#endif

void tzx_reset_capture_state() {
  tzxBitByte = 0;
  tzxBitCount = 0;
#if defined(RECORD_ZX_SPECTRUM)
  tzxRecordCenter = 512;
  tzxRecordFiltered = 512;
  tzxRecordDeviation = 8;
  tzxRecordFloor = 512;
  tzxRecordCeil = 512;
  tzxRecordLevel = 0;
  tzxRecordCenterPrimed = false;
#endif
}

void isr_tzx()
{
  const uint16_t sample = get_input_value();
  uint8_t bit;

#if defined(RECORD_TZX_ID15) && defined(RECORD_ZX_SPECTRUM)
  if (!active_recording_is_zx_spectrum()) {
    bit = (sample >= 512) ? 1 : 0;
  } else
#endif
#if defined(RECORD_TZX_ID15)
    bit = (sample >= 512) ? 1 : 0;
#endif
#if defined(RECORD_ZX_SPECTRUM)
  {
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
#endif

  uint8_t bb = tzxBitByte;
  uint8_t bc = tzxBitCount;
  if (bit) bb |= (uint8_t)(0x80 >> bc);
  bc++;

  if (bc >= 8) {
    tzxBitByte = 0;
    tzxBitCount = 0;
    queue_output_byte(bb);
  } else {
    tzxBitByte = bb;
    tzxBitCount = bc;
  }
}

bool active_recording_is_zx_spectrum()
{
  return recordFormat == RecordFormat::ZX_SPECTRUM;
}

#else

bool active_recording_is_zx_spectrum()
{
  return false;
}

#endif //defined(RECORD_TZX_ID15) || defined(RECORD_ZX_SPECTRUM)
