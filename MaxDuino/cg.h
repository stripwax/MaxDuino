#ifndef CG_H_INCLUDED
#define CG_H_INCLUDED

#include "configs.h"

#ifdef Use_CG

#define CG_ONE_PULSE 428
#define CG_ZERO_PULSE 865

bool cgcas_detect_and_init();
void cgcas_process();

#endif // Use_CG

#endif // CG_H_INCLUDED
