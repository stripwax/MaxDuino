#ifndef I2C_CONFIG_H_INCLUDED
#define I2C_CONFIG_H_INCLUDED

#include "constants.h"
#include "configs.h"

// preferred I2C implementation
#ifndef I2C_Library_Preference
#if defined(__AVR_ATmega2560__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega328P__)
    #define I2C_Library_Preference _I2C_Impl_AceWire
//    #define I2C_Library_Preference _I2C_Impl_SoftI2CMaster
#else
    #define I2C_Library_Preference _I2C_Impl_Wire
#endif
#endif

// I2CFAST, if defined, I2C bus runs at 400kHz, instead of 100kHz default
#define I2CFAST

// I2C_DELAY_MICROS, only used by Acewire initialisation
// How many microseconds to wait between I2C requests
// Default is calculated based on I2CCLOCK (which is calculated based on I2CFAST, above)
// Defining here to be 0 means run as fast as possible (which is around 400kHz if using SimpleWireFastInterface on AVR)
// If using acewire and I2C is locking up, you may want to comment out the below, or set to some other value > 0 
#define I2C_DELAY_MICROS 0

#endif // I2C_CONFIG_H_INCLUDED
