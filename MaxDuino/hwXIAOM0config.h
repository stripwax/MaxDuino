// SEEED_XIAO_M0
//
// Pin definition for Seeeduino Xiao M0 boards
//

#include "Arduino.h"

#define outputPin           A0
#define INIT_OUTPORT            pinMode(outputPin,OUTPUT)
#define WRITE_LOW               digitalWrite(outputPin,LOW)
#define WRITE_HIGH              digitalWrite(outputPin,HIGH)

#define chipSelect    12            //Sd card chip select pin - map to LED (on assumption that SD CS is actually just tied directly to GND)
#define BUTTON_ADC
#define btnADC        A2 // analog input pin for ADC buttons
#define NO_MOTOR    // because no spare gpio

