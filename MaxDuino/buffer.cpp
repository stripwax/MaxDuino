#include "buffer.h"

volatile bool morebuff = false;
uint16_t readpos = 0; // only used within the ISR, never accessed outside, so doesn't need to be volatile
uint16_t writepos = 0; // only used within the main loop, never accessed by ISR, so doesn't need to be volatile
volatile uint16_t activeBuffsize = normal_buffsize;
volatile byte wbuffer[2][buffsize];
volatile byte * volatile writeBuffer=wbuffer[0]; // the pointer itself is volatile (since the ISR can swap readBuffer/writeBuffer)
volatile byte * readBuffer=wbuffer[1]; // this pointer is not volatile since this pointer is only manipulated by the ISR, not code outside the ISR

uint16_t getBufferSize(void)
{
  return activeBuffsize;
}

void setOTLABufferMode(bool enabled)
{
#if defined(ARDUINO_D1_MINI32)
  activeBuffsize = enabled ? buffsize : normal_buffsize;
#else
  (void)enabled;
#endif
}

void clearBuffer(void)
{
  noInterrupts();
  for(uint16_t i=0;i<buffsize;i++)
  {
    wbuffer[0][i]=0;
    wbuffer[1][i]=0;
  }
  interrupts();
}
