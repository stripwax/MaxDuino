#include "configs.h"

#ifdef RECORD

#include "record_TimerADC.h"
#include "Arduino.h"

#if defined(__SAMD21G18A__)
#include "TimerCounter.h"
#endif

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

#elif defined(__SAMD21G18A__)

#include "wiring_private.h"
#include "pinSetup.h"

// Cached button reading, refreshed by the record ISR's button service while
// recording. Declared here so the ADC setup below can reset it.
static volatile uint16_t latest_adc_reading = 0;
static volatile uint16_t sButtonRefreshTicks = 1;
// Set by the button service after it re-points the ADC back at the record
// channel; the next recording tick flushes the stale conversion before use.
static volatile bool sAdcNeedsFlush = false;

void adc_start_freerun_record_pin() {
  pinMode(A2, INPUT);
  pinPeripheral(A2, PIO_ANALOG);

  // Reset the button cache as part of (re)initialising the ADC. Ensures that no
  // stale old button value (especially STOP button) are read by then main loop when
  // starting a new recording.
  latest_adc_reading = 0;
  sButtonRefreshTicks = 1;
  sAdcNeedsFlush = false;

  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GCM_ADC) | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_CLKEN;
  while (GCLK->STATUS.bit.SYNCBUSY);

  ADC->CTRLA.reg = 0;
  while (ADC->STATUS.bit.SYNCBUSY);

  ADC->CTRLB.reg = ADC_CTRLB_PRESCALER_DIV32 | ADC_CTRLB_FREERUN | ADC_CTRLB_RESSEL_10BIT;
  while (ADC->STATUS.bit.SYNCBUSY);

  ADC->REFCTRL.reg = ADC_REFCTRL_REFSEL_INTVCC1;
  while (ADC->STATUS.bit.SYNCBUSY);

  ADC->AVGCTRL.reg = ADC_AVGCTRL_SAMPLENUM_1 | ADC_AVGCTRL_ADJRES(0);
  while (ADC->STATUS.bit.SYNCBUSY);

  ADC->SAMPCTRL.reg = 0;
  while (ADC->STATUS.bit.SYNCBUSY);

  ADC->INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_PIN18 | ADC_INPUTCTRL_MUXNEG_GND | ADC_INPUTCTRL_GAIN_DIV2;
  while (ADC->STATUS.bit.SYNCBUSY);

  ADC->CTRLA.reg = ADC_CTRLA_ENABLE;
  while (ADC->STATUS.bit.SYNCBUSY);

  ADC->SWTRIG.reg = ADC_SWTRIG_START;
}

void adc_stop() {
  ADC->CTRLA.reg &= ~ADC_CTRLA_ENABLE;
  while (ADC->STATUS.bit.SYNCBUSY);

  // analogRead() never touches CTRLB, so a FREERUN left behind here would
  // persist into the menu and change how the button reads behave after
  // recording stops. Put the ADC back in the plain one-shot mode the menu
  // expects (10-bit result to match setup_buttons' analogReadResolution(10)).
  ADC->CTRLB.reg = ADC_CTRLB_PRESCALER_DIV32 | ADC_CTRLB_RESSEL_10BIT;
  while (ADC->STATUS.bit.SYNCBUSY);

  sAdcNeedsFlush = false;
}

void timer_start_recording() {
  const uint16_t sampleRate = default_sample_rate_for_format();
  TcCount16* TC = (TcCount16*) TC3;

  noInterrupts();

  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GCM_TCC2_TC3) | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_CLKEN;
  while (GCLK->STATUS.bit.SYNCBUSY);

  TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while (TC->STATUS.bit.SYNCBUSY);

  TC->CTRLA.reg = TC_CTRLA_MODE_COUNT16 | TC_CTRLA_WAVEGEN_MFRQ | TC_CTRLA_PRESCALER_DIV1;
  while (TC->STATUS.bit.SYNCBUSY);

  TC->CC[0].reg = (uint16_t)(F_CPU / sampleRate);
  while (TC->STATUS.bit.SYNCBUSY);

  TC->COUNT.reg = 0;
  while (TC->STATUS.bit.SYNCBUSY);

  TC->INTFLAG.reg = TC_INTFLAG_MC0;
  TC->INTENSET.reg = TC_INTENSET_MC0;

  setTimerIsrCallback(&recorder_isr);
  NVIC_EnableIRQ(TC3_IRQn);

  TC->CTRLA.reg |= TC_CTRLA_ENABLE;
  while (TC->STATUS.bit.SYNCBUSY);

  interrupts();
}

void timer_stop_recording() {
  TcCount16* TC = (TcCount16*) TC3;

  noInterrupts();
  TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while (TC->STATUS.bit.SYNCBUSY);

  TC->INTENCLR.reg = TC_INTENCLR_MC0;
  NVIC_DisableIRQ(TC3_IRQn);
  TC->INTFLAG.reg = TC_INTFLAG_MC0;
  interrupts();
}

uint16_t get_input_value(void)
{
  return ADC->RESULT.reg;
}

uint8_t get_input_value_byte(void)
{
  return (uint8_t)(ADC->RESULT.reg >> 2);
}

uint16_t record_adc_get_button_value(void) {
  return latest_adc_reading;
}

void record_adc_service_button(void) {
  // The button input is a compile-time constant (btnADC), so this can be
  // called unconditionally from the record ISR. Refresh the cached value at
  // ~45ms intervals; the counter starts at zero so the first refresh happens
  // on the first recording tick.
  if (--sButtonRefreshTicks != 0) return;
  sButtonRefreshTicks = (uint16_t)(((uint32_t)default_sample_rate_for_format() * 45) / 1000);

  // The record sample for this tick has already been captured, so it is safe
  // to briefly re-point the shared ADC at the button input.
  pinPeripheral(btnADC, PIO_ANALOG);
  while (ADC->STATUS.bit.SYNCBUSY);

  ADC->INPUTCTRL.bit.MUXPOS = g_APinDescription[btnADC].ulADCChannelNumber;
  while (ADC->STATUS.bit.SYNCBUSY);

  // Discard the first result in case it completes an in-flight record-channel
  // conversion (or hasn't settled), then keep the second.
  ADC->INTFLAG.reg = ADC_INTFLAG_RESRDY;
  while (!ADC->INTFLAG.bit.RESRDY) {}
  (void)ADC->RESULT.reg;

  ADC->INTFLAG.reg = ADC_INTFLAG_RESRDY;
  while (!ADC->INTFLAG.bit.RESRDY) {}
  latest_adc_reading = (uint16_t)ADC->RESULT.reg;

  // Restore the recording input (A2 = AIN18).
  while (ADC->STATUS.bit.SYNCBUSY);
  ADC->INPUTCTRL.reg = ADC_INPUTCTRL_MUXPOS_PIN18 | ADC_INPUTCTRL_MUXNEG_GND | ADC_INPUTCTRL_GAIN_DIV2;
  while (ADC->STATUS.bit.SYNCBUSY);

  // The freerun ADC may have started a conversion on the button channel before
  // the mux restore took effect, so RESULT can still hold a button-channel
  // value when we return. If the next ISR read it as a record sample, that
  // spurious reading would become a fake bit in the .tzx stream every button
  // refresh (~45ms), which is enough to make every load fail.
  //
  // Don't flush here though: each flush is a full conversion wait (~10us), and
  // doing two of them inside this ISR would extend it to ~40us - two timer
  // periods - dropping samples (see the flag clearing order in TC3_Handler).
  // Instead defer the flush to the next recording tick, which is where the
  // stale result would actually be read.
  sAdcNeedsFlush = true;
}

void record_adc_flush_pending() {
  // Runs at the start of the next recording tick after a button refresh. The
  // ADC is back on the record channel by now; discard two completed results so
  // RESULT holds a clean record-channel sample before isr_*() reads it. The
  // first discard covers any conversion that began on the button channel
  // before the mux restore took effect; the second guarantees at least one
  // full record-channel conversion has completed. Waiting here also makes this
  // tick long, but TC3_Handler clears MC0 before the callback, so a timer
  // match that fires meanwhile is recovered rather than dropped.
  if (!sAdcNeedsFlush) return;
  sAdcNeedsFlush = false;

  for (uint8_t i = 0; i < 2; i++) {
    ADC->INTFLAG.reg = ADC_INTFLAG_RESRDY;
    while (!ADC->INTFLAG.bit.RESRDY) {}
    (void)ADC->RESULT.reg;
  }
}

#else
#error Missing recording timer and isr definition for this MCU
#endif

#endif //RECORD
