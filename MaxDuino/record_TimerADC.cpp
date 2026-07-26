#include "configs.h"
#include "record_TimerADC.h"
#include "Arduino.h"

#ifdef RECORD

// Implements ADC and timer/interrupt logic for each supported MCU

#if defined(__AVR_ATmega4808__) || defined(__AVR_ATmega4809__)

void adc_start_freerun_record_pin() {
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

void adc_stop() {
  ADC0.CTRLA &= ~(ADC_ENABLE_bm);
}

void timer_start_recording() {
#if defined(TCB1)
#  define REC_TCB TCB1
#  define REC_TCB_INT_vect TCB1_INT_vect
#else
#  define REC_TCB TCB0
#  define REC_TCB_INT_vect TCB0_INT_vect
#endif

  const uint16_t sampleRate = default_sample_rate_for_format();
  REC_TCB.CTRLA = 0;
  REC_TCB.CTRLB = TCB_CNTMODE_INT_gc;
  REC_TCB.CCMP  = (uint16_t)(F_CPU / sampleRate);
  REC_TCB.CNT   = 0;
  REC_TCB.INTFLAGS = TCB_CAPT_bm;
  REC_TCB.INTCTRL  = TCB_CAPT_bm;
  REC_TCB.CTRLA = TCB_CLKSEL_CLKDIV1_gc | TCB_ENABLE_bm;
}

void timer_stop_recording() {
  REC_TCB.INTCTRL = 0;
  REC_TCB.CTRLA = 0;
  REC_TCB.INTFLAGS = TCB_CAPT_bm;
}

ISR(REC_TCB_INT_vect) {
  REC_TCB.INTFLAGS = TCB_CAPT_bm;
  recorder_isr();
}

uint16_t get_input_value(void)
{
  return ADC0.RES;
}

#else
#error Missing recording timer and isr definition for this MCU
#endif

#endif //RECORD
