#include "configs.h"
#include "file_utils.h"
#include "sdfat_config.h"
#include <SdFat.h>

SdBaseFile entry;  // SD card file
unsigned long bytesRead=0;
unsigned long filesize;
byte lastByte;
_readout_type readout;

char fileName[filenameLength + 1];
const char * filenameExt = nullptr;

byte filebuffer[FILEBUFFER_SIZE]; // used for small reads from files (readfile, ReadByte, etc use this), sized for the largest header read
byte readfile(byte nbytes, unsigned long p)
{
  byte i=0;
  if(entry.seekSet(p)) {
    i=entry.read(filebuffer, nbytes);
  } 
  return i;
}

byte ReadByte() {
  //Read a byte from the file, and move file position on one if successful
  //Always reads from bytesRead, which is the current position in the file
  if(readfile(1, bytesRead)==1)
  {
    bytesRead += 1;
    outByte = filebuffer[0];
    return true;
  }
  return false;
}

byte ReadWord() {
  //Read 2 bytes from the file, and move file position on two if successful
  //Always reads from bytesRead, which is the current position in the file
  if(readfile(2, bytesRead)==2)
  {
    bytesRead += 2;
    outWord = ((uint16_t)filebuffer[1] << 8) | filebuffer[0];
    return true;
  }
  return false;
}

byte ReadLong() {
  //Read 3 bytes from the file, and move file position on three if successful
  //Always reads from bytesRead, which is the current position in the file
  if(readfile(3, bytesRead)==3)
  {
    bytesRead += 3;
    outLong = ((unsigned long)(((uint16_t)filebuffer[2] << 8) | filebuffer[1]) << 8) | filebuffer[0];
    return true;
  }
  return false;
}

byte ReadDword() {
  //Read 4 bytes from the file, and move file position on four if successful  
  //Always reads from bytesRead, which is the current position in the file
  if(readfile(4, bytesRead)==4)
  {
    bytesRead += 4;
    outLong = ((unsigned long)(((uint16_t)filebuffer[3] << 8) | filebuffer[2]) << 16) | ((uint16_t)filebuffer[1] << 8) | filebuffer[0];
    return true;
  }
  return false;
}
