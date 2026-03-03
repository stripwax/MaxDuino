// __AVR_ATmega4808__

#include "Arduino.h"

#define outputPin           9
#define INIT_OUTPORT         VPORTA.DIR |=  PIN7_bm         // El pin9 es PA7
#define WRITE_LOW            VPORTA.OUT &= ~PIN7_bm         // El pin9 es PA7
#define WRITE_HIGH           VPORTA.OUT |=  PIN7_bm         // El pin9 es PA7

const byte chipSelect = 10;          //Sd card chip select pin

#define btnPlay       17            //Play Button
#define btnStop       16            //Stop Button
#define btnUp         15            //Up button
#define btnDown       14            //Down button
#define btnMotor      6             //Motor Sense (connect pin to gnd to play, NC for pause)
#define btnRoot       7             //Return to SD card root
