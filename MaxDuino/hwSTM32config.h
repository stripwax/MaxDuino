// defined(__arm__) && defined(__STM32F1__)
//
// Pin definition for Blue Pill boards
//

#include "Arduino.h"

#define outputPin           PA9    // this pin is 5V tolerant and PWM output capable
#define INIT_OUTPORT            pinMode(outputPin,OUTPUT)
#define WRITE_LOW               digitalWrite(outputPin,LOW)
#define WRITE_HIGH              digitalWrite(outputPin,HIGH)

#define chipSelect    PB12            //Sd card chip select pin

#define btnPlay       PA0           //Play Button
#define btnStop       PA1           //Stop Button
#define btnUp         PA2           //Up button
#define btnDown       PA3           //Down button
#define btnMotor      PA8     //Motor Sense (connect pin to gnd to play, NC for pause)
#define btnRoot       PA4           //Return to SD card root
