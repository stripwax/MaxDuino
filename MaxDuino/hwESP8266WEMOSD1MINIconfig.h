// ARDUINO_ESP8266_WEMOS_D1MINI
//
// Pin definition for Wemos D1 Mini (ESP8266) boards
//

#include "Arduino.h"

#define outputPin           16 // D0
#define INIT_OUTPORT            pinMode(outputPin,OUTPUT)
#define WRITE_LOW               digitalWrite(outputPin,LOW)
#define WRITE_HIGH              digitalWrite(outputPin,HIGH)

#define chipSelect    15
#define BUTTON_ADC
#define btnADC        A0 
#define btnMotor      2

