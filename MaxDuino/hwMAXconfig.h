// __AVR_ATmega2560__

#include "Arduino.h"

#define outputPin           23 
#define INIT_OUTPORT        DDRA |=  _BV(1)         // El pin23 es el bit1 del PORTA
#define WRITE_LOW           PORTA &= ~_BV(1)         // El pin23 es el bit1 del PORTA
#define WRITE_HIGH          PORTA |=  _BV(1)         // El pin23 es el bit1 del PORTA

const byte chipSelect = 53;          //Sd card chip select pin

#define btnUp         A0            //Up button
#define btnDown       A1            //Down button
#define btnPlay       A2            //Play Button
#define btnStop       A3            //Stop Button
#define btnRoot       A4            //Return to SD card root
#define btnMotor      6             //Motor Sense (connect pin to gnd to play, NC for pause)

