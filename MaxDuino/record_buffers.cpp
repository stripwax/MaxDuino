#include "configs.h"
#include "Arduino.h"

#if defined(RECORD)

#include "record_buffers.h"

volatile uint8_t * pageA = (uint8_t *)(wbuffer[0]);
volatile uint8_t * pageB = (uint8_t *)(wbuffer[1]);
volatile uint16_t pagePos;
volatile uint8_t activePage;
volatile bool pageReadyA;
volatile bool pageReadyB;
uint32_t droppedBytes;
uint32_t dataBytesWritten;

volatile uint8_t* active_page_ptr() {
  return (activePage == 0) ? pageA : pageB;
}

#endif //defined(RECORD)
