#include "buffer.h"

volatile bool morebuff = false;
buffsize_t readpos = 0; // only used within the ISR, never accessed outside, so doesn't need to be volatile
buffsize_t writepos = 0; // only used within the main loop, never accessed by ISR, so doesn't need to be volatile
bool write_buffer_full = false;
volatile uint16_t wbuffer[2][buffsize];
volatile uint16_t * volatile writeBuffer=wbuffer[0]; // the pointer itself is volatile (since the ISR can swap readBuffer/writeBuffer)
volatile uint16_t * readBuffer=wbuffer[1]; // this pointer is not volatile since this pointer is only manipulated by the ISR, not code outside the ISR

void clearBuffer(void)
{
  noInterrupts();
  buffsize_t i=buffsize-1;
  do
  {
    wbuffer[0][i]=0;
    wbuffer[1][i]=0;
  } while (i--);
  interrupts();
}

void advance_write_word()
{
  writepos++;
  if (writepos==(buffsize_t)(buffsize))  // this handles overflow correctly i.e. if buffsize==256 and writepos has wrapped from 255 to 0, (buffsize_t)(buffsize) == 0 too.
    write_buffer_full=true;
}
