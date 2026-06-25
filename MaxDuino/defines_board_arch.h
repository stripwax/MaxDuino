#ifndef DEFINES_BOARD_ARCH_H_INCLUDED
#define DEFINES_BOARD_ARCH_H_INCLUDED

// just some helpers to differentiate different devices/architectures/layouts with a single #define
// rather than requiring multiple expressions each time
#if defined(__XTENSA__) && defined(ESP32)
#define ESP32_XTENSA
#endif
#if defined(__riscv) && defined(ESP32)
#define ESP32_RISCV
#endif

#endif // DEFINES_BOARD_ARCH_H_INCLUDED