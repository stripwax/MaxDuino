#ifndef TRS80CAS_H_INCLUDED
#define TRS80CAS_H_INCLUDED

#include "configs.h"

#if defined(Use_CAS) && defined(Use_TRS80)

bool trs80cas_detect_and_init();
void trs80cas_process();

#endif // defined(Use_CAS) && defined(Use_TRS80)

#endif // TRS80CAS_H_INCLUDED
