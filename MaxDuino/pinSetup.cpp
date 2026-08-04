#ifndef CLI
#include "configs.h"
#include "Arduino.h"
#include "pinSetup.h"
 #include <digitalWriteFast.h>
#if defined(ARDUINO_XIAO_ESP32C3)
#include <driver/gpio.h>
#endif

#if defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2350)
#include <SPI.h>
#include "i2c.h"
#if (I2C_Library_Preference == _I2C_Impl_Wire) || (I2C_Library_Preference == _I2C_Impl_SoftWire)
#include <Wire.h>
#endif
#endif

// use the macro from digitalWriteFast instead of hardcoding pin-port mappings in this file
// Takes into account the fact that Input is already set by default so we don't need to BIT_CLEAR first
#define pinModePullupFast(P) { \
  BIT_SET(*__digitalPinToPortReg(P), __digitalPinToBit(P)); \
}

void pinsetup()
{
#if defined(__AVR_ATmega4808__)
  //pinMode(btnPlay,INPUT_PULLUP);  // Not needed, default is INPUT (0)
  //digitalWrite(btnPlay,HIGH); // 17 PD3
  VPORTD.DIR |= ~PIN3_bm;
  PORTD.PIN3CTRL |=PORT_PULLUPEN_bm; /* Enable the internal pullup */
  VPORTD.OUT |=  PIN3_bm;

  //pinMode(btnStop,INPUT_PULLUP);  // Not needed, default is INPUT (0)
  //digitalWrite(btnStop,HIGH); // 16 PD2
  VPORTD.DIR |= ~PIN2_bm;  
  PORTD.PIN2CTRL |=PORT_PULLUPEN_bm; /* Enable the internal pullup */
  VPORTD.OUT |=  PIN2_bm;    

  //pinMode(btnUp,INPUT_PULLUP);  // Not needed, default is INPUT (0)
  //digitalWrite(btnUp,HIGH); // 15 PD1
  VPORTD.DIR |= ~PIN1_bm;
  PORTD.PIN1CTRL |=PORT_PULLUPEN_bm; /* Enable the internal pullup */
  VPORTD.OUT |=  PIN1_bm;    

  //pinMode(btnDown,INPUT_PULLUP);  // Not needed, default is INPUT (0)
  //digitalWrite(btnDown,HIGH); // 14 PD0 also to enbale PULLUP if PINMODE is INPUT
  VPORTD.DIR |= ~PIN0_bm;
  PORTD.PIN0CTRL |=PORT_PULLUPEN_bm; /* Enable the internal pullup */ 
  VPORTD.OUT |=  PIN0_bm;   

  //pinMode(btnMotor, INPUT_PULLUP);  // Not needed, default is INPUT (0)
  //digitalWrite(btnMotor,HIGH); // 6 PA4
  VPORTA.DIR |= ~PIN4_bm;
  PORTA.PIN4CTRL |=PORT_PULLUPEN_bm; /* Enable the internal pullup */
  VPORTA.OUT |=  PIN4_bm;  
  
  //pinMode(btnRoot, INPUT_PULLUP);  // Not needed, default is INPUT (0)
  //digitalWrite(btnRoot, HIGH); // 7 PA5
  VPORTA.DIR |= ~PIN5_bm; 
  PORTA.PIN5CTRL |=PORT_PULLUPEN_bm; /* Enable the internal pullup */
  VPORTA.OUT |=  PIN5_bm;

  #if defined(RECORD)
    pinMode(btnRec, INPUT_PULLUP);

    // Reduce noise on the recording ADC pin (ATmega4808 Nano: A7 = PF5 = AIN15)
    // - disable digital input buffer on PF5
    // - ensure pullups are off
    #if defined(PORT_ISC_INPUT_DISABLE_gc)
      PORTF.PIN5CTRL = PORT_ISC_INPUT_DISABLE_gc;
    #endif
  #endif

      
#elif defined(__arm__) && defined(__STM32F1__)

  //General Pin settings
  //Setup buttons with internal pullup 
  pinMode(btnPlay,INPUT_PULLUP);
  digitalWrite(btnPlay,HIGH);
  pinMode(btnStop,INPUT_PULLUP);
  digitalWrite(btnStop,HIGH);
  pinMode(btnUp,INPUT_PULLUP);
  digitalWrite(btnUp,HIGH);
  pinMode(btnDown,INPUT_PULLUP);
  digitalWrite(btnDown,HIGH);
  pinMode(btnMotor, INPUT_PULLUP);
  digitalWrite(btnMotor,HIGH);
  pinMode(btnRoot, INPUT_PULLUP);
  digitalWrite(btnRoot, HIGH); 

   
#elif defined(ARDUINO_XIAO_ESP32C3)

  // GPIO output drive strength — try minimum (5mA) to reduce overshoot/ringing on edges
  gpio_set_drive_capability((gpio_num_t)outputPin, GPIO_DRIVE_CAP_0);

#elif defined(SEEED_XIAO_M0) || defined(ARDUINO_ESP8266_WEMOS_D1MINI)

  // BUTTON PIN CONFIGURATION
  // n.a.
  
#elif defined(ESP32_XTENSA)

  pinMode(btnMotor, INPUT_PULLUP);
  digitalWrite(btnMotor, HIGH);

#elif defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2350)

  #if defined(BUTTON_ADC)
  // nothing else required
  #else
  pinMode(btnPlay, INPUT_PULLUP);
  pinMode(btnStop, INPUT_PULLUP);
  pinMode(btnUp, INPUT_PULLUP);
  pinMode(btnDown, INPUT_PULLUP);
  pinMode(btnRoot, INPUT_PULLUP);
  #endif

  #if !defined(NO_MOTOR)
  pinMode(btnMotor, INPUT_PULLUP);
  #endif

  #if defined(RECORD)
  pinMode(btnRec, INPUT_PULLUP);
  #endif

#if (I2C_Library_Preference == _I2C_Impl_Wire) || (I2C_Library_Preference == _I2C_Impl_SoftWire)
  I2C_WIRE_CLASS.setSDA(RP2040_I2C_SDA_PIN);
  I2C_WIRE_CLASS.setSCL(RP2040_I2C_SCL_PIN);
#endif

  // XIAO boards use SPI0 (SD pins D8/D9/D10 = GPIO 2/4/3 match default SPI0 pins).
  // Pico and others use SPI1 with default pins (GPIO 10/11/12).
  // Don't call setCS at all — SdFat manages CS as a regular GPIO via
  // digitalWrite (not through the SPI peripheral's CS signal).
  #if defined(ARDUINO_SEEED_XIAO_RP2040) || defined(ARDUINO_SEEED_XIAO_RP2350)
    SPI.setSCK(RP2040_SD_SCK_PIN);
    SPI.setTX(RP2040_SD_MOSI_PIN);
    SPI.setRX(RP2040_SD_MISO_PIN);
  #else
    SPI1.setSCK(RP2040_SD_SCK_PIN);
    SPI1.setTX(RP2040_SD_MOSI_PIN);
    SPI1.setRX(RP2040_SD_MISO_PIN);
    SPI1.setCS(chipSelect);
  #endif
  
#elif defined(__AVR_ATmega328P__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega4809__) || defined(__AVR_ATmega2560__)

  //pinMode(btnPlay,INPUT_PULLUP);  // Not needed, default is INPUT (0)
  //digitalWrite(btnPlay,HIGH); // Wrte for INPUT_PULLUP if input type is only INPUT
  //PORTD |= _BV(4); // Not good practice, hardcodes the pin assignments.  Use a macro instead.
  pinModePullupFast(btnPlay);
  pinModePullupFast(btnStop);
  pinModePullupFast(btnUp);
  pinModePullupFast(btnDown);
  pinModePullupFast(btnMotor);
  pinModePullupFast(btnRoot);

  
  #if defined(RECORD)
    pinModePullupFast(btnRec);

    // Reduce noise on the recording ADC pin.
    // ATmega4809 Nano Every: A7 = PD5 = AIN5
    #if defined(PORT_ISC_INPUT_DISABLE_gc)
      PORTD.PIN5CTRL = PORT_ISC_INPUT_DISABLE_gc;
    #endif
  #endif

#else
#error Unknown device type or missing definition in pinSetup.h
#endif
}
#endif