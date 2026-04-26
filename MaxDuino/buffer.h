#ifndef BUFFER_H_INCLUDED
#define BUFFER_H_INCLUDED

#include "Arduino.h" // for types
#include "configs.h"

/* buffsize can be any multiple of 2, the larger the better .
However, some loops and optimizations make assumpttions that buffsize >= 16
so let's say 32 bytes is the absolute minimum */

#if defined(VERY_LARGE_BUFFER)
  #define buffsize 1024
#elif defined(LARGEBUFFER)
  #define buffsize 256
#else
  #define buffsize 176
#endif

#if buffsize < 32
#error buffsize needs be at least 32 bytes. And you probably want it to be as large as possible and greater than 128 bytes!
#endif

#if buffsize <= 256
  typedef uint8_t buffsize_t;
#else
  typedef uint16_t buffsize_t;
#endif

// define a sentinel value which is an illegal position (an odd position, since all writes are aligned to even entries only)
#define write_buffer_full ((buffsize_t)(-1))

extern volatile bool morebuff;
extern buffsize_t readpos;
extern buffsize_t writepos;
extern volatile byte wbuffer[2][buffsize];
extern volatile byte * volatile writeBuffer;
extern volatile byte * readBuffer;
void clearBuffer(void);
void advance_write_word(void);

#endif // BUFFER_H_INCLUDED
