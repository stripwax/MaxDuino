#ifndef BUFFER_H_INCLUDED
#define BUFFER_H_INCLUDED

#include "Arduino.h" // for types
#include "configs.h"

/* With latest casprocessing logic, buffsize can be any multiple of 2.
*/
#if defined(ARDUINO_D1_MINI32)
  #ifdef LARGEBUFFER
    #define normal_buffsize 254
  #else
    #define normal_buffsize 176
  #endif
  // Keep extra capacity available on this board, but only use the deeper
  // buffer for OTLA-style direct-recording playback.
  #define buffsize 1024
#elif defined(LARGEBUFFER)
  #define buffsize 254
  #define normal_buffsize buffsize
#else
  #define buffsize 176
  #define normal_buffsize buffsize
#endif

extern volatile bool morebuff;
extern uint16_t readpos;
extern uint16_t writepos;
extern volatile uint16_t activeBuffsize;
extern volatile byte wbuffer[2][buffsize];
extern volatile byte * volatile writeBuffer;
extern volatile byte * readBuffer;
uint16_t getBufferSize(void);
void setOTLABufferMode(bool enabled);
void clearBuffer(void);

#endif // BUFFER_H_INCLUDED
