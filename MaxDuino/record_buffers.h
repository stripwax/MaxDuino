#ifndef RECORD_BUFFERS_H_INCLUDED
#define RECORD_BUFFERS_H_INCLUDED

#if defined(RECORD)
#include "buffer.h"

constexpr uint16_t kRecordPageSize = 2 * buffsize; // in BYTES (since buffsize is in UINT16_t)
#if buffsize>128
typedef uint16_t pagepos_t;
#else
typedef uint8_t pagepos_t;
#endif
extern volatile pagepos_t pagePos;
extern volatile uint8_t activePage;
extern volatile bool pageReadyA;
extern volatile bool pageReadyB;
extern uint32_t droppedBytes;
extern uint32_t dataBytesWritten;

uint8_t* active_page_ptr();

#endif //defined(RECORD)

#endif //RECORD_BUFFERS_H_INCLUDED
