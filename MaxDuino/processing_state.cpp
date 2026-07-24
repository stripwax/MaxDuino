#include "processing_state.h"

byte currentID;
TASK currentTask;
BLOCKTASK currentBlockTask;
byte pass;
long count_r;
byte currentBit;

// isStopped and writeFinished are used by UniLoop to manage
// the end-of-file tasks of stopping writing to the buffer
// while waiting for the ISR to finish reading from the buffer
volatile bool isStopped;
bool writeFinished;

// start and pauseOn are used by the main UI and related to user
// actions (starting, stopping, pausing playback)
byte start = 0;
bool pauseOn = false;
