#ifndef RECORD_TIMERADC_H_INCLUDED
#define RECORD_TIMERADC_H_INCLUDED

#include "configs.h"
#include "Arduino.h"

#ifdef RECORD

uint16_t get_input_value();
void adc_start_freerun_record_pin();
void adc_stop();
void timer_start_recording();
void timer_stop_recording();

void recorder_isr();
uint16_t default_sample_rate_for_format();
void queue_output_byte(uint8_t value);

// SAMD21: the recording ISR periodically samples the button ADC input (btnADC)
// and caches the last reading so the main loop can poll buttons while
// recording without disturbing the record channel.
#if defined(__SAMD21G18A__)
uint16_t record_adc_get_button_value();
void record_adc_service_button();
void record_adc_flush_pending();
#endif

#endif //RECORD

#endif //RECORD_TIMERADC_H_INCLUDED