#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

#include "isr.h"

#if defined(ESP32_XTENSA)
typedef void (*timerCallback)(void);
#endif

class TimerCounter
{


  public:
    TimerCounter();
    static void initialize();
    static void ISR_ATTR setPeriod(unsigned long microseconds);
    static void stop();
  private:
    static void _attachInterrupt();
    static unsigned long currentMicroseconds;
    static void _initialize();
    static void ISR_ATTR _setPeriod(unsigned long microseconds);
#ifdef CLI
  public:
    static unsigned long getCurrentMicroseconds() { return currentMicroseconds; }
#endif
};
extern TimerCounter& Timer;
extern const unsigned long MAXPAUSE_PERIOD;

#if defined(__SAMD21G18A__) && defined(RECORD)
// Sets which handler the (single) timer interrupt dispatches to: playback isr or recording isr
// (SAMD21 only, see TimerCounter.cpp).
void setTimerIsrCallback(void (*fn)(void));
#endif

#endif // TIMER_H_INCLUDED