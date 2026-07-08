#ifndef PINSETUP_H_INCLUDED
#define PINSETUP_H_INCLUDED

#include "Arduino.h"

#ifdef __AVR_ATmega2560__
#define outputPin           23 
#define INIT_OUTPORT        DDRA |=  _BV(1)         // El pin23 es el bit1 del PORTA
#define WRITE_LOW           PORTA &= ~_BV(1)         // El pin23 es el bit1 del PORTA
#define WRITE_HIGH          PORTA |=  _BV(1)         // El pin23 es el bit1 del PORTA

#elif defined(__AVR_ATmega4809__)
  #define outputPin           9
  //#define INIT_OUTPORT         DDRB |=  _BV(1)         // El pin9 es el bit1 del PORTB
  //#define INIT_OUTPORT          pinMode(outputPin,OUTPUT)  
  #define INIT_OUTPORT         VPORTB.DIR |=  _BV(0)         // El pin9 es PB0
  //#define WRITE_LOW           PORTB &= ~_BV(1)         // El pin9 es el bit1 del PORTB
  //#define WRITE_LOW             digitalWrite(outputPin,LOW)
  #define WRITE_LOW           VPORTB.OUT &= ~_BV(0)         // El pin9 es PB0
  //#define WRITE_HIGH          PORTB |=  _BV(1)         // El pin9 es el bit1 del PORTB
  //#define WRITE_HIGH            digitalWrite(outputPin,HIGH)
  #define WRITE_HIGH          VPORTB.OUT |=  _BV(0)         // El pin9 es PB0

#elif defined(__AVR_ATmega4808__)
  #define outputPin           9
  //#define INIT_OUTPORT          pinMode(outputPin,OUTPUT)  
  #define INIT_OUTPORT         VPORTA.DIR |=  PIN7_bm         // El pin9 es PA7
  //#define WRITE_LOW             digitalWrite(outputPin,LOW)
  #define WRITE_LOW            VPORTA.OUT &= ~PIN7_bm         // El pin9 es PA7
  //#define WRITE_HIGH            digitalWrite(outputPin,HIGH)
  #define WRITE_HIGH           VPORTA.OUT |=  PIN7_bm         // El pin9 es PA7

#elif defined(__arm__) && defined(__STM32F1__)
  #define outputPin           PA9    // this pin is 5V tolerant and PWM output capable
  #define INIT_OUTPORT            pinMode(outputPin,OUTPUT)
  //#define INIT_OUTPORT            pinMode(outputPin,OUTPUT); GPIOA->regs->CRH |=  0x00000030  
  #define WRITE_LOW               digitalWrite(outputPin,LOW)
  //#define WRITE_LOW               GPIOA->regs->ODR &= ~0b0000001000000000
  //#define WRITE_LOW               gpio_write_bit(GPIOA, 9, LOW)
  #define WRITE_HIGH              digitalWrite(outputPin,HIGH)
  //#define WRITE_HIGH              GPIOA->regs->ODR |=  0b0000001000000000
  //#define WRITE_HIGH              gpio_write_bit(GPIOA, 9, HIGH)

#elif defined(__AVR_ATmega32U4__) 
#define outputPin           7    // this pin is 5V tolerant and PWM output capable
//#define INIT_OUTPORT            pinMode(outputPin,OUTPUT)
  #define INIT_OUTPORT         DDRE |=  _BV(6)         // El pin PE6 es el bit6 del PORTE
//#define WRITE_LOW               digitalWrite(outputPin,LOW)
  #define WRITE_LOW           PORTE &= ~_BV(6)         // El pin PE6 es el bit6 del PORTE
//#define WRITE_HIGH              digitalWrite(outputPin,HIGH)
  #define WRITE_HIGH          PORTE |=  _BV(6)         // El pin PE6 es el bit6 del PORTE
  
#elif defined(SEEED_XIAO_M0)
  #define outputPin           A0
  #define INIT_OUTPORT            pinMode(outputPin,OUTPUT)
  #define WRITE_LOW               digitalWrite(outputPin,LOW)
  #define WRITE_HIGH              digitalWrite(outputPin,HIGH)

#elif defined(ARDUINO_XIAO_ESP32C3)
  #define outputPin         D0
  #define INIT_OUTPORT            pinMode(outputPin,OUTPUT)
  #define WRITE_LOW               GPIO.out_w1tc.val = (1UL << outputPin)
  #define WRITE_HIGH              GPIO.out_w1ts.val = (1UL << outputPin)

#elif defined(ARDUINO_ESP8266_WEMOS_D1MINI)
  #define outputPin           16 // D0
  #define INIT_OUTPORT            pinMode(outputPin,OUTPUT)
  #define WRITE_LOW               digitalWrite(outputPin,LOW)
  #define WRITE_HIGH              digitalWrite(outputPin,HIGH)

#elif defined(ARDUINO_RASPBERRY_PI_PICO)
  #define outputPin           9
  #define INIT_OUTPORT            pinMode(outputPin,OUTPUT_12MA)
  #define WRITE_LOW               digitalWriteFast(outputPin,LOW)
  #define WRITE_HIGH              digitalWriteFast(outputPin,HIGH)

#elif defined(ARDUINO_SEEED_XIAO_RP2040) || defined(ARDUINO_SEEED_XIAO_RP2350)
  #define outputPin           D0
  #define INIT_OUTPORT            pinMode(outputPin,OUTPUT_12MA)
  #define WRITE_LOW               digitalWriteFast(outputPin,LOW)
  #define WRITE_HIGH              digitalWriteFast(outputPin,HIGH)

  #elif defined(WEMOS_D1_MINI32)
  #define outputPin         26 // D0
  #define INIT_OUTPORT      pinMode(outputPin,OUTPUT)
  #define WRITE_LOW         GPIO.out_w1tc = (1UL << outputPin)
  #define WRITE_HIGH        GPIO.out_w1ts = (1UL << outputPin)

#elif defined(__AVR_ATmega328P__) || defined(__AVR_ATmega4808__) || defined(__AVR_ATmega4809__)
  //#define MINIDUINO_AMPLI     // For A.Villena's Miniduino new design . You can define this in platformio.ini
  #define outputPin           9
  #ifdef MINIDUINO_AMPLI
    #define INIT_OUTPORT         DDRB |= B00000011                              // pin8+ pin9 es el bit0-bit1 del PORTB 
    #define WRITE_LOW           (PORTB &= B11111101) |= B00000001               // pin8+ pin9 , bit0- bit1 del PORTB
    #define WRITE_HIGH          (PORTB |= B00000010) &= B11111110               // pin8+ pin9 , bit0- bit1 del PORTB  
  //  #define WRITE_LOW           PORTB = (PORTB & B11111101) | B00000001         // pin8+ pin9 , bit0- bit1 del PORTB
  //  #define WRITE_HIGH          PORTB = (PORTB | B00000010) & B11111110         // pin8+ pin9 , bit0- bit1 del PORTB 
  #else
    #define INIT_OUTPORT         DDRB |=  _BV(1)         // El pin9 es el bit1 del PORTB
    #define WRITE_LOW           PORTB &= ~_BV(1)         // El pin9 es el bit1 del PORTB
    #define WRITE_HIGH          PORTB |=  _BV(1)         // El pin9 es el bit1 del PORTB
  #endif

// pin 0-7 PortD0-7, pin 8-13 PortB0-5, pin 14-19 PortC0-5

/*
#ifdef rpolarity 
  #define WRITE_LOW           PORTB &= ~_BV(1)        // El pin9 es el bit1 del PORTB
  #define WRITE_HIGH          PORTB |= _BV(1)         // El pin9 es el bit1 del PORTB
  // pin 0-7 PortD0-7, pin 8-13 PortB0-5, pin 14-19 PortC0-5
#endif

#ifndef rpolarity 
  #define WRITE_HIGH           PORTB &= ~_BV(1)        // El pin9 es el bit1 del PORTB
  #define WRITE_LOW          PORTB |= _BV(1)         // El pin9 es el bit1 del PORTB
  // pin 0-7 PortD0-7, pin 8-13 PortB0-5, pin 14-19 PortC0-5
#endif

*/

#else
#error Unknown device type or missing definition in pinSetup.h
#endif 


/////////////////////////////////////////////////////////////////////////////////////////////
  //General Pin settings
  //Setup buttons with internal pullup

#if defined(__AVR_ATmega2560__)

  const byte chipSelect = 53;          //Sd card chip select pin
  
  #define btnUp         A0            //Up button
  #define btnDown       A1            //Down button
  #define btnPlay       A2            //Play Button
  #define btnStop       A3            //Stop Button
  #define btnRoot       A4            //Return to SD card root
  // #define btnDelete     A5         //Not implemented this button is for an optional function
  #define btnMotor      6             //Motor Sense (connect pin to gnd to play, NC for pause)

#elif defined(__arm__) && defined(__STM32F1__)
//
// Pin definition for Blue Pill boards
//

#define chipSelect    PB12            //Sd card chip select pin

#define btnPlay       PA0           //Play Button
#define btnStop       PA1           //Stop Button
#define btnUp         PA2           //Up button
#define btnDown       PA3           //Down button
#define btnMotor      PA8     //Motor Sense (connect pin to gnd to play, NC for pause)
#define btnRoot       PA4           //Return to SD card root

#elif defined(__AVR_ATmega32U4__) 
  #define NO_MOTOR  
  const byte chipSelect = SS;          //Sd card chip select pin

  #define btnPlay       4            //Play Button
  #define btnStop       30            //Stop Button
  #define btnUp         6            //Up button
  #define btnDown       12            //Down button
  #define btnMotor      0             //Motor Sense (connect pin to gnd to play, NC for pause)
  #define btnRoot       1             //Return to SD card root
#elif defined(SEEED_XIAO_M0)
//
// Pin definition for Seeeduino Xiao M0 boards
//

#if defined(RICKY_TEST_BOARD)
  #define chipSelect    D6
  #define BUTTON_ADC
  #define btnADC        A3
  #define btnMotor      A1
#else
  #define chipSelect    12            //Sd card chip select pin - map to LED (on assumption that SD CS is actually just tied directly to GND)
  #define BUTTON_ADC
  #define btnADC        A2 // analog input pin for ADC buttons
  #define NO_MOTOR    // because no spare gpio
#endif

#elif defined(ARDUINO_XIAO_ESP32C3)
//
// Pin definition for Seeeduino Xiao ESP32C3 boards
//

#if defined(RICKY_TEST_BOARD)
  #define chipSelect    D6
  #define BUTTON_ADC
  #define btnADC        A3
  #define btnMotor      D1
#else
  #define chipSelect    D7
  #define BUTTON_ADC
  #define btnADC        A2 // analog input pin for ADC buttons // CHANGED!!!
  #define NO_MOTOR    // because no spare gpio
#endif

#elif defined(ARDUINO_ESP8266_WEMOS_D1MINI)
//
// Pin definition for Wemos D1 Mini (ESP8266) boards
//
#define chipSelect    15
#define BUTTON_ADC
#define btnADC        A0 
#define btnMotor      2

#elif defined(WEMOS_D1_MINI32)
//
// Pin definition for Wemos D1 Mini32 (ESP32) boards
//
#define chipSelect    SS
#define BUTTON_ADC
#define btnADC        A0
#define btnMotor      D4

#elif defined(ARDUINO_RASPBERRY_PI_PICO)
//
// Pin definition for Raspberry Pi Pico boards
//
  #define btnDown       0
  #define btnUp         1
  #define btnStop       2
  #define btnPlay       3
  #define btnMotor      6
  #define btnRoot       7

  #define RP2040_I2C_SDA_PIN 4
  #define RP2040_I2C_SCL_PIN 5
  #define RP2040_SD_SCK_PIN 10
  #define RP2040_SD_MOSI_PIN 11
  #define RP2040_SD_MISO_PIN 12
  #define chipSelect 13

#elif defined(ARDUINO_SEEED_XIAO_RP2040) || defined(ARDUINO_SEEED_XIAO_RP2350)
//
// Pin definition for Seeed Xiao RP2040 boards
//
  #if defined(RICKY_TEST_BOARD)
    #define chipSelect    D6
    #define BUTTON_ADC
    #if defined(ARDUINO_SEEED_XIAO_RP2350)
    #define btnADC        A2 /* there isn't an A3 on this board!  You need to bodge a jumper between pin 3 and pin 4 */
    #else
    #define btnADC        A3
    #endif
    #define btnMotor      D1
  #else
    #define chipSelect    D6
    #define BUTTON_ADC
    #define btnADC        A2 // analog input pin for ADC buttons
    #define NO_MOTOR    // because no spare gpio
  #endif

  #define RP2040_I2C_SDA_PIN D4
  #define RP2040_I2C_SCL_PIN D5
  #define RP2040_SD_SCK_PIN D8
  #define RP2040_SD_MOSI_PIN D10
  #define RP2040_SD_MISO_PIN D9

  #elif defined(__AVR_ATmega328P__) || defined(__AVR_ATmega4808__) || defined(__AVR_ATmega4809__)
  const byte chipSelect = 10;          //Sd card chip select pin
  
  #define btnPlay       17            //Play Button
  #define btnStop       16            //Stop Button
  #define btnUp         15            //Up button
  #define btnDown       14            //Down button
  #define btnMotor      6             //Motor Sense (connect pin to gnd to play, NC for pause)
  #define btnRoot       7             //Return to SD card root
  #define btnRec        8             // only relevant #if defined(RECORD)

  #else
#error Unknown device type or missing definition in pinSetup.h
#endif

void pinsetup();

#ifdef BUTTON_ADC
// Each button acts as a voltage divider between 10k and the following resistors:
// 0 Ohm  i.e. 100%
// 2.2k Ohm i.e. 82% (10 : 12.2)
// 4.7k Ohm i.e. 68% (10 : 14.7)
// 10k Ohm i.e. 50% (10 : 20)
// 20k Ohm i.e. 33% (10 : 30)

// For a 10-bit ADC, each button is calibrated to the band between this value and the next value above
// (or 1023 for upper limit).
// For ESP32 (both XTENSA-based e.g. ESP32-WROOM and RISC-V based e.g. ESP32-C3) we ask the Espressif
// framework to return calibrated ADC values in millivolts. Range is therefore 0-3200
// The bands are intentionally set very wide, and far apart
// However note that ESP ADC is nonlinear and not full-scale, so the resistor
// values must be chosen to avoid ranges at the extreme top (100%) end.
// The resistor values above are compatible with ESP devices

// Taking measurements with D1Mini32 vs ESP8266 in the original board:
// (Note: D1Mini32 values shown here are the calibrated mv ; ESP8266 are the raw ADC as the api doesn't return mv)
//            | D1_MINI32 | D1_ESP8266 |
// no button: |   142     |     12     |
// Play:      |   3139    |    1024    |
// Stop:      |   3035    |    986     |
// Root:      |   2680    |    875     |
// Down:      |   1670    |    543     |
// Up:        |   1114    |    364     |
// Rec:       |   n/a     |    n/a     |

// Taking measurements with different devices in the ricky test board:
// (Note: D1Mini32 and ESP32C3 values shown here are the calibrated mv
// ESP8266 and SAMD21 are the raw ADC as the api doesn't return mv)
//            | D1_MINI32 | ESP32C3 | D1_ESP8266 |  SAMD21  |
// no button: |   142     |    0    |     12     |     4    |
// Play:      |   2740    |  2667   |    895     |   848    |
// Stop:      |   2490    |  2412   |    813     |   770    |
// Root:      |   2190    |  2131   |    718     |   680    |
// Down:      |   1370    |  1330   |    450     |   420    |
// Up:        |   900     |  875    |    300     |   280    |
// Rec:       |   450     |  433    |    153     |   140    |

#if defined(ESP32_XTENSA) || defined(ESP32_RISCV)
  #if defined(RICKY_TEST_BOARD)
    #define btnADCPlayLow 2600
    #define btnADCStopLow 2300
    #define btnADCRootLow 1900
    #define btnADCDownLow 1200
    #define btnADCUpLow 700
    #define btnADCRecLow 350
  #else
    #define btnADCPlayLow 3100
    #define btnADCStopLow 2750
    #define btnADCRootLow 2000
    #define btnADCDownLow 1400
    #define btnADCUpLow 700
  #endif
#elif defined(ESP8266)
// ESP ADC is nonlinear, and also not full scale, so the values are different!
// because not full scale, a 1k:10k voltage divider (i.e. 90%) is undetectable
// and reads as 1023 still, so resistor values have been altered to create better spacing
  #if defined(RICKY_TEST_BOARD)
    #define btnADCPlayLow 850
    #define btnADCStopLow 750
    #define btnADCRootLow 600
    #define btnADCDownLow 375
    #define btnADCUpLow 225
    #define btnADCRecLow 90
  #else
    #define btnADCPlayLow 1020 // 0 ohm reading 1023 due to saturation
    #define btnADCStopLow 900 // 2.2k ohm reading around 960
    #define btnADCRootLow 700 // 4.7k ohm reading around 800
    #define btnADCDownLow 460 // 10k ohm reading around 590
    #define btnADCUpLow 200 // 20k ohm reading around 390
  #endif
#else
  #if defined(RICKY_TEST_BOARD)
  #define btnADCPlayLow 800
  #define btnADCStopLow 730
  #define btnADCRootLow 500
  #define btnADCDownLow 350
  #define btnADCUpLow 220
  #define btnADCRecLow 100
  #else
  #define btnADCPlayLow 950 // 0 ohm reading around 1000, ideally 1023
  #define btnADCStopLow 800 // 2.2k ohm reading around 840
  #define btnADCRootLow 600 // 4.7k ohm reading around 695
  #define btnADCDownLow 420 // 10k ohm reading around 510
  #define btnADCUpLow 200 // 20k ohm reading around 340
  #endif
#endif

#endif // BUTTON_ADC

#endif // #define PINSETUP_H_INCLUDED
