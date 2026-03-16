#ifndef COMPAT_H_INCLUDED
#define COMPAT_H_INCLUDED

#if defined(__arm__) && defined(STM32F1)
  #include <itoa.h>  
  #define strncpy_P(a, b, n) strncpy((a), (b), (n))
  #define memcmp_P(a, b, n) memcmp((a), (b), (n)) 
  #define strcasecmp_P(a,b) strcasecmp((a), (b)) 
#endif

#endif // COMPAT_H_INCLUDED
