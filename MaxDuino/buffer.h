#ifndef BUFFER_H_INCLUDED
#define BUFFER_H_INCLUDED

#include "Arduino.h" // for types
#include "configs.h"

/* buffsize can be any number now, the larger the better .
It is defined in terms of number of words i.e. uint16_t
(so, the number of bytes needed is twice that, per buffer.  And there are two
buffers, so in total you need this number x 4 in free ram)
However, some loops and optimizations make assumpttions that buffsize >= 8
so 8 is the absolute minimum so let's say we only support >= 32 words buffer.
I can't imagine anyone wanting (or needing) the buffer tot be that small though.  */

#if defined(BUFFER_SIZE)
  #define buffsize BUFFER_SIZE  // custom number of words
#else
  #if defined(VERY_LARGE_BUFFER)
    #define buffsize 512  // 1024 bytes
  #elif defined(LARGEBUFFER)
    #define buffsize 128  // 256 bytes
  #else
    #define buffsize 88  // 176 bytes
  #endif
#endif

#if buffsize < 32
#error buffsize needs be at least 32 words (64 bytes)! And you probably want it to be as large as possible and greater than 64 words (128 bytes)!
#endif

// we need to use an appropriate type to index this array.  uint8_t can index into an up to and including 256 entries (0-255).  if buffsize>256 then you need a larger type.
#if buffsize <= 256  
  typedef uint8_t buffsize_t;
#else
  typedef uint16_t buffsize_t;
#endif

extern volatile bool morebuff;
extern buffsize_t readpos;
extern buffsize_t writepos;
extern bool write_buffer_full;
extern volatile uint16_t wbuffer[2][buffsize];
extern volatile uint16_t * volatile writeBuffer;
extern volatile uint16_t * readBuffer;
void clearBuffer(void);
void advance_write_word(void);

#endif // BUFFER_H_INCLUDED
