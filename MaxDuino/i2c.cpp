#ifndef CLI
#include "configs.h"
#include "Arduino.h"
#include "i2c.h"

#if (I2C_Library_Preference == _I2C_Impl_AceWire) && defined(ARDUINO_ARCH_AVR)
  #warning I2c using ACEWIRE interface with fast AVR optimisations
  #include <digitalWriteFast.h>
  #include <ace_wire/SimpleWireFastInterface.h>
  using ace_wire::SimpleWireFastInterface;
  SimpleWireFastInterface<SDA, SCL, I2C_DELAY_MICROS> i2cAceWire;
#elif (I2C_Library_Preference == _I2C_Impl_AceWire)
  #warning I2c using ACEWIRE interface without fast optimisations
  #include <AceWire.h>
  SimpleWireInterface i2cAceWire(SDA, SCL, I2C_DELAY_MICROS);
#elif (I2C_Library_Preference == _I2C_Impl_SoftI2CMaster)
  #warning I2C using SOFTI2CMaster interface
  #undef USE_SOFT_I2C_MASTER_H_AS_PLAIN_INCLUDE
  #undef _SOFTI2C_HPP
  #include <SoftI2CMaster.h>
#elif (I2C_Library_Preference == _I2C_Impl_SoftWire)
  #warning I2C using SoftWire interface
  #include <SoftWire.h>
  SoftWire Wire = SoftWire();  
#else
  #warning I2C using standard Wire interface
  #include <Wire.h>
#endif
#endif
