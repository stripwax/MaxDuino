#include "configs.h"
#include "Arduino.h"

#if defined(RECORD)

#include "record_buffers.h"

volatile pagepos_t pagePos;
volatile uint8_t activePage;
volatile bool pageReadyA;
volatile bool pageReadyB;
uint32_t droppedBytes;
uint32_t dataBytesWritten;

uint8_t* active_page_ptr() {
  return (uint8_t *)(wbuffer[activePage]);
}

#endif //defined(RECORD)
