// wrappers around different i2c implementations to expose the same interface to library code

#ifndef I2C_H_INCLUDED
#define I2C_H_INCLUDED

#include "configs.h"
#include "Arduino.h"
#include "i2c_config.h"

#if defined(I2CFAST)
  #define I2C_FASTMODE  1
  #if !defined(I2CCLOCK)
  #define I2CCLOCK  400000L   //100000L for StandardMode, 400000L for FastMode and 1000000L for FastModePlus
  #endif
#else
  #define I2C_FASTMODE  0
  #if !defined(I2CCLOCK)
  #define I2CCLOCK  100000L   //100000L for StandardMode, 400000L for FastMode and 1000000L for FastModePlus
  #endif
#endif


#if (I2C_Library_Preference == _I2C_Impl_SoftI2CMaster)
  // these #defines in the section are ONLY used by the SoftI2CMaster library
  #if (defined(__AVR_ATmega2560__) || defined(__AVR_ATmega32U4__))
    #define SDA_PORT PORTD
    #define SDA_PIN 1 
    #define SCL_PORT PORTD
    #define SCL_PIN 0 
  #else
    #define SDA_PORT PORTC
    #define SDA_PIN 4 
    #define SCL_PORT PORTC
    #define SCL_PIN 5
  #endif
#endif


#if (I2C_Library_Preference == _I2C_Impl_AceWire)
  #ifndef I2C_DELAY_MICROS
    #define I2C_DELAY_MICROS ((1000000UL / (2UL * I2CCLOCK)) > 0 ? (1000000UL / (2UL * I2CCLOCK)) : 1)
  #endif
#endif


#if (I2C_Library_Preference == _I2C_Impl_AceWire) && defined(ARDUINO_ARCH_AVR)
  #include <digitalWriteFast.h>
  #include <ace_wire/SimpleWireFastInterface.h>
  using ace_wire::SimpleWireFastInterface;
  extern SimpleWireFastInterface<SDA, SCL, I2C_DELAY_MICROS> i2cAceWire;
  #define mx_i2c_init() i2cAceWire.begin()
  #define mx_i2c_start(address) i2cAceWire.beginTransmission(address)
  #define mx_i2c_write(byte) i2cAceWire.write(byte)
  #define mx_i2c_end() i2cAceWire.endTransmission()

#elif (I2C_Library_Preference == _I2C_Impl_SoftI2CMaster)

// ideally I'd do this:
  //     #define USE_SOFT_I2C_MASTER_H_AS_PLAIN_INCLUDE
  //     #include <SoftI2CMaster.h>
  // but as of 2.1.9 it appears it still doesn't work properly (at least not with platform io)
  // so I'll forward-declare everything myself:
  bool __attribute__ ((noinline)) i2c_init(void) __attribute__ ((used));
  bool __attribute__ ((noinline)) i2c_start(uint8_t addr) __attribute__ ((used));
  void __attribute__ ((noinline)) i2c_stop(void) asm("ass_i2c_stop") __attribute__ ((used));
  bool __attribute__ ((noinline)) i2c_write(uint8_t value) asm("ass_i2c_write") __attribute__ ((used));

  #ifndef I2C_WRITE
  #define I2C_WRITE 0
  #endif

  #define mx_i2c_init() i2c_init()
  #define mx_i2c_start(address) i2c_start((address<<1)|I2C_WRITE)
  #define mx_i2c_write(byte) i2c_write(byte)
  #define mx_i2c_end() i2c_stop()

#else
  // Wire, SoftWire, or AceWire (non-AVR)
  #if (I2C_Library_Preference == _I2C_Impl_SoftWire)
    extern SoftWire Wire;
  #elif (I2C_Library_Preference == _I2C_Impl_Wire)
    #include <Wire.h>
  #elif (I2C_Library_Preference == _I2C_Impl_AceWire)
    #include <AceWire.h>
    using ace_wire::SimpleWireInterface;
    extern SimpleWireInterface i2cAceWire;
  #else
    #error Unknown I2C library configuration
  #endif

    
  #if defined(ARDUINO_SEEED_XIAO_RP2350)
    // XIAO RP2350 is the awekward one here:
    // XIAO RP2350: D4/D5 = GPIO 6/7 are on i2c1 (Wire1), not i2c0 (Wire).
    #define I2C_WIRE_CLASS Wire1
  #else
    // Everything else: Wire defaults to working configuration
    // Pico and other RP2040 boards: Wire defaults to i2c0 which is fine.
    // XIAO RP2040: D4/D5 = GPIO 6/7 are on i2c1, same as RP2350,
    // but the earlephilhower core maps Wire→i2c1 . So Wire "just works".
    #define I2C_WIRE_CLASS Wire
  #endif

  #if (I2C_Library_Preference == _I2C_Impl_Wire) || (I2C_Library_Preference == _I2C_Impl_SoftWire)
    #define mx_i2c_init() I2C_WIRE_CLASS.begin();\
                          I2C_WIRE_CLASS.setClock(I2CCLOCK)
    #define mx_i2c_start(address) I2C_WIRE_CLASS.beginTransmission(address)
    #define mx_i2c_write(byte) I2C_WIRE_CLASS.write(byte)
    #define mx_i2c_end() I2C_WIRE_CLASS.endTransmission()
  #else // (I2C_Library_Preference == _I2C_Impl_AceWire)
    #define mx_i2c_init() i2cAceWire.begin()
    #define mx_i2c_start(address) i2cAceWire.beginTransmission(address)
    #define mx_i2c_write(byte) i2cAceWire.write(byte)
    #define mx_i2c_end() i2cAceWire.endTransmission()
  #endif

#endif

#if defined(P8544)
// this really does not belong here
  #include <pcd8544.h>
  #define ADMAX 1023
  #define ADPIN 0
  #include <avr/pgmspace.h>
  byte dc_pin = 5;
  byte reset_pin = 3;
  byte cs_pin = 4;
  #define backlight_pin 2
#endif

#endif // I2C_H_INCLUDED
