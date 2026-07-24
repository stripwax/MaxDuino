#ifndef PROCESSING_STATE_H_INCLUDED
#define PROCESSING_STATE_H_INCLUDED

#include "configs.h"
#include "compat.h"
#include "constants.h"
#include "MaxDuino.h"


//Keep track of which ID, Task, and Block Task we're dealing with
//State is shared (readable and writeable) across various modules
extern byte currentID;
extern TASK currentTask;
extern BLOCKTASK currentBlockTask;
extern byte pass;
extern long count_r;
extern byte currentBit;
extern word currentPeriod;

// Flags for ISR -> main loop communication
extern volatile bool isStopped;
// Main-loop only flag: set by writeEnd(), checked by UniLoop()
extern bool writeFinished;

#endif // PROCESSING_STATE_H_INCLUDED
