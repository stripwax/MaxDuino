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

#endif //RECORD

#endif //RECORD_TIMERADC_H_INCLUDED