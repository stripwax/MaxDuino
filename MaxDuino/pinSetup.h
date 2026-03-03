#ifndef PINSETUP_H_INCLUDED
#define PINSETUP_H_INCLUDED

#include "hwconfig.h"

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
// The bands are intentionally set very wide, and far apart
// However note that ESP ADC is nonlinear and not full-scale, so the resistor
// values must be chosen to avoid ranges at the extreme top (100%) end.
// The resistor values and bands chosen here are compatible with ESP devices

#if defined(ESP32) || defined(ESP8266)
// ESP ADC is nonlinear, and also not full scale, so the values are different!
// because not full scale, a 1k:10k voltage divider (i.e. 90%) is undetectable
// and reads as 1023 still, so resistor values have been altered to create better spacing
#define btnADCPlayLow 1020 // 0 ohm reading 1023 due to saturation
#define btnADCStopLow 900 // 2.2k ohm reading around 960
#define btnADCRootLow 700 // 4.7k ohm reading around 800
#define btnADCDownLow 500 // 10k ohm reading around 590
#define btnADCUpLow 200 // 20k ohm reading around 390
#else
#define btnADCPlayLow 950 // 0 ohm reading around 1000, ideally 1023
#define btnADCStopLow 800 // 2.2k ohm reading around 840
#define btnADCRootLow 600 // 4.7k ohm reading around 695
#define btnADCDownLow 420 // 10k ohm reading around 510
#define btnADCUpLow 200 // 20k ohm reading around 340
#endif

#endif // BUTTON_ADC

#endif // #define PINSETUP_H_INCLUDED
