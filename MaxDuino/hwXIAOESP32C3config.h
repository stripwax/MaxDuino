// ARDUINO_XIAO_ESP32C3
//
// Pin definition for Seeeduino Xiao ESP32C3 boards
//

#include "Arduino.h"

#define outputPin         D0
#define INIT_OUTPORT            pinMode(outputPin,OUTPUT)
#define WRITE_LOW               digitalWrite(outputPin,LOW)
#define WRITE_HIGH              digitalWrite(outputPin,HIGH)

#define chipSelect    D7
#define BUTTON_ADC
#define btnADC        A2 // analog input pin for ADC buttons // CHANGED!!!
#define NO_MOTOR    // because no spare gpio
