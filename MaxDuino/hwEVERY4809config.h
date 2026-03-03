// __AVR_ATmega4808__

#include "Arduino.h"

#define outputPin           9
#define INIT_OUTPORT         VPORTB.DIR |=  _BV(0)         // El pin9 es PB0
#define WRITE_LOW           VPORTB.OUT &= ~_BV(0)         // El pin9 es PB0
#define WRITE_HIGH          VPORTB.OUT |=  _BV(0)         // El pin9 es PB0

const byte chipSelect = 10;          //Sd card chip select pin

#define btnPlay       17            //Play Button
#define btnStop       16            //Stop Button
#define btnUp         15            //Up button
#define btnDown       14            //Down button
#define btnMotor      6             //Motor Sense (connect pin to gnd to play, NC for pause)
#define btnRoot       7             //Return to SD card root
