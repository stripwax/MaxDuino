#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

#include "isr.h"

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
};
extern TimerCounter& Timer;

#endif // TIMER_H_INCLUDED