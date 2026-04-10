#include "configs.h"
#include "file_utils.h"
#include "sdfat_config.h"
#include <SdFat.h>
#include <string.h>

SdBaseFile entry;  // SD card file
unsigned long bytesRead=0;
unsigned long filesize;
byte lastByte;
_readout_type readout;

byte filebuffer[20]; // used for small reads from files (readfile, ReadByte, etc use this), sized for the largest small header read
namespace {
  #if defined(ARDUINO_D1_MINI32)
    constexpr uint16_t FILE_READ_CACHE_SIZE = 512;
  #else
    constexpr uint16_t FILE_READ_CACHE_SIZE = 64;
  #endif

  byte fileReadCache[FILE_READ_CACHE_SIZE];
  unsigned long fileReadCacheStart = 0;
  uint16_t fileReadCacheLen = 0;
  bool fileReadCacheValid = false;

  uint16_t refillFileReadCache(unsigned long p)
  {
    const bool isSequentialRefill =
        fileReadCacheValid && (p == (fileReadCacheStart + fileReadCacheLen));

    if (!isSequentialRefill) {
      if (!entry.seekSet(p)) {
        fileReadCacheValid = false;
        fileReadCacheLen = 0;
        return 0;
      }
    }

    const int bytesReadIntoCache = entry.read(fileReadCache, FILE_READ_CACHE_SIZE);
    if (bytesReadIntoCache <= 0) {
      fileReadCacheValid = false;
      fileReadCacheLen = 0;
      return 0;
    }

    fileReadCacheStart = p;
    fileReadCacheLen = uint16_t(bytesReadIntoCache);
    fileReadCacheValid = true;
    return fileReadCacheLen;
  }
}

void resetFileReadCache()
{
  fileReadCacheValid = false;
  fileReadCacheStart = 0;
  fileReadCacheLen = 0;
}

byte readfile(byte nbytes, unsigned long p)
{
  if (nbytes == 0) {
    return 0;
  } 

  const unsigned long requestEnd = p + nbytes;
  if (!fileReadCacheValid ||
      p < fileReadCacheStart ||
      requestEnd > (fileReadCacheStart + fileReadCacheLen))
  {
    if (refillFileReadCache(p) == 0) {
      return 0;
    }
  }

  unsigned long cacheOffset = p - fileReadCacheStart;
  uint16_t available = fileReadCacheLen - uint16_t(cacheOffset);
  if (available < nbytes) {
    if (refillFileReadCache(p) == 0) {
      return 0;
    }
    cacheOffset = 0;
    available = fileReadCacheLen;
  }

  const byte copied = (available < nbytes) ? byte(available) : nbytes;
  memcpy(filebuffer, fileReadCache + cacheOffset, copied);
  return copied;
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
    outWord = word(filebuffer[1], filebuffer[0]);
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
    outLong = ((unsigned long) word(filebuffer[2], filebuffer[1]) << 8) | filebuffer[0];
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
    outLong = ((unsigned long)word(filebuffer[3], filebuffer[2]) << 16) | word(filebuffer[1], filebuffer[0]);
    return true;
  }
  return false;
}


