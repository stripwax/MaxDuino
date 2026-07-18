#ifndef ISR_H_INCLUDED
#define ISR_H_INCLUDED

#include "Arduino.h"

// we use ISR_ATTR for all functions called within isr.
// Some platforms need this or something like it - for these we set it appropriately based on what
// the platform has already defined, and for others we fall through and define ISR_ATTR to be empty.
// That way the same code using ISR_ATTR works correctly for all platforms.
#ifdef ISR_ATTR
#error unexpected - ISR_ATTR already defined
#endif

#ifndef ISR_ATTR
#if defined(ARDUINO_ISR_ATTR)
#define ISR_ATTR ARDUINO_ISR_ATTR
#elif defined(IRAM_ATTR)
#define ISR_ATTR IRAM_ATTR
#elif defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2350)
// earlephilhower core: linker puts .time_critical* in RAM, avoiding flash-cache-miss jitter
#define ISR_ATTR __attribute__((section(".time_critical")))
#else
#define ISR_ATTR
#endif
#endif

void ISR_ATTR isrCallback();

//ISR Variables
extern volatile byte isStopped;
extern volatile byte pinState;
extern volatile bool isPauseBlock;
extern volatile bool wasPauseBlock;

void reset_output_state();

#endif // ISR_H_INCLUDED
