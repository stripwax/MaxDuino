// __AVR_ATmega32U4__

#include "Arduino.h"

#define outputPin           7    // this pin is 5V tolerant and PWM output capable
#define INIT_OUTPORT         DDRE |=  _BV(6)         // El pin PE6 es el bit6 del PORTE
#define WRITE_LOW           PORTE &= ~_BV(6)         // El pin PE6 es el bit6 del PORTE
#define WRITE_HIGH          PORTE |=  _BV(6)         // El pin PE6 es el bit6 del PORTE

#define NO_MOTOR  
const byte chipSelect = SS;          //Sd card chip select pin

#define btnPlay       4            //Play Button
#define btnStop       30            //Stop Button
#define btnUp         6            //Up button
#define btnDown       12            //Down button
#define btnMotor      0             //Motor Sense (connect pin to gnd to play, NC for pause)
#define btnRoot       1             //Return to SD card root
