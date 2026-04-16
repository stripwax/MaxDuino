#ifndef ISR_H_INCLUDED
#define ISR_H_INCLUDED

#include "Arduino.h"

#if defined(ESP32)
void ARDUINO_ISR_ATTR wave2();
#else
void wave2();
#endif

//ISR Variables
extern volatile byte isStopped;
extern volatile byte pinState;
extern volatile bool isPauseBlock;
extern volatile bool wasPauseBlock;

void reset_output_state();

#endif // ISR_H_INCLUDED
