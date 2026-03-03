// __AVR_ATmega328P__
// For A.Villena's Miniduino new design

#include "Arduino.h"

#define outputPin           9
#define INIT_OUTPORT         DDRB |= B00000011                              // pin8+ pin9 es el bit0-bit1 del PORTB 
#define WRITE_LOW           (PORTB &= B11111101) |= B00000001               // pin8+ pin9 , bit0- bit1 del PORTB
#define WRITE_HIGH          (PORTB |= B00000010) &= B11111110               // pin8+ pin9 , bit0- bit1 del PORTB  

const byte chipSelect = 10;          //Sd card chip select pin

#define btnPlay       17            //Play Button
#define btnStop       16            //Stop Button
#define btnUp         15            //Up button
#define btnDown       14            //Down button
#define btnMotor      6             //Motor Sense (connect pin to gnd to play, NC for pause)
#define btnRoot       7             //Return to SD card root
