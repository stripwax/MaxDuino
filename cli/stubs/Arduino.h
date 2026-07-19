// Arduino.h stub for CLI builds on PC
#ifndef ARDUINO_H_CLISTUB
#define ARDUINO_H_CLISTUB

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cctype>

typedef uint8_t byte;
typedef uint16_t word;

#define LOW  0
#define HIGH 1

#define PROGMEM
#define PSTR(s) (s)

#define pgm_read_byte(addr) (*(const uint8_t *)(addr))
#define strcasecmp_P(a, b) strcasecmp((a), (b))
#define memcmp_P(a, b, n) memcmp((a), (b), (n))

#define lowByte(w) ((uint8_t)((w) & 0xff))
#define highByte(w) ((uint8_t)((w) >> 8))

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))

inline void delay(unsigned long) {}
inline void noInterrupts() {}
inline void interrupts() {}

#endif // ARDUINO_H_CLISTUB
