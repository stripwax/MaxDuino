#include "configs.h"
#include "compat.h"
#include "oric.h"
#include "processing_state.h"
#include "current_settings.h"
#include "MaxProcessing.h"
#include "file_utils.h"

#ifdef tapORIC
#ifdef ORICSPEEDUP
PROGMEM const uint16_t oric_pulse_length_turbo[]={ORICTURBOZEROLOWPULSE, ORICTURBOZEROHIGHPULSE, ORICTURBOONEPULSE, ORICTURBOONEPULSE};
#endif
PROGMEM const uint16_t oric_pulse_length[]={ORICZEROLOWPULSE, ORICZEROHIGHPULSE, ORICONEPULSE, ORICONEPULSE};

static void OricBitWrite() {
  // we break a byte transmission into:
  // 1 start bit
  // 8 data bits
  // 1 parity bit
  // 3 stop bits
  // = total 13 full waves

  byte bit;
  if (currentBit == 13) {
    //Start Bit = a zero
    //we emit low pulse or high pulse depending on pass (0 or 1)
    bit = 0;
  } else if (currentBit == 4) { // Paridad inversa i.e. Impar
    //Parity bit (uses bitChecksum)/  If bitChecksum is 1 we emit zero pulse otherwise 1 pulse
    //we emit low pulse or high pulse depending on pass (0 or 1)
    bit = bitChecksum^1;
  } else if (currentBit < 4) {
    //Stop Bit = a one (x3)
    bit = 1;
  } else {
    bit = currentByte&0x01;
  }

  #ifdef ORICSPEEDUP
  const uint16_t * const pulse_table = BAUDRATE <= 2400 ? oric_pulse_length : oric_pulse_length_turbo;
  #else
  const uint16_t * const pulse_table = oric_pulse_length;
  #endif
  currentPeriod = pgm_read_word(pulse_table+(bit ? pass+2 : pass));

  pass+=1;      //Data is played as 2 x pulses for a zero, and 2 pulses for a one
  if (pass==2) {
    if(currentBit-5<8) {
      bitChecksum ^= bit;
      currentByte >>= 1;                        //Shift along to the next bit
    }
    currentBit--;
    pass=0;
  }

  if ((currentBit==0) && (lastByte)) {
    count_r = 255; 
    if(ReadByte()) { 
      bytesRead += -1;                      //rewind a byte if we've not reached the end           
      currentBlockTask = BLOCKTASK::PAUSE;
    }else {
      currentTask = TASK::GETID;
    }
  }    
}

static void newByte(byte value)
{
  currentByte=value;
  currentBit=13;
  bitChecksum=0;
  lastByte=0;
}

static void readNewByte()
{
  ReadByte();
  newByte(outByte);
}

static void OricDataBlock() {
  //Convert byte from file into string of pulses.  One pulse per pass
  //This is only called when currentBit==0 (i.e. we get the next byte)
  if(ReadByte()) {            //Read in a byte
    currentByte = outByte;
    bytesToRead = (unsigned int)(bytesToRead-1);
    bitChecksum = 0;
    if(bytesToRead == 0) {                  //Check for end of data block
      lastByte = 1;
    }
  } else {                         // If we reached the EOF
    count_r =255;
    currentTask = TASK::GETID;
    return;
  }

  currentBit = 13;
  pass=0;
}

void tzx_process_blockid_oric() {
  if (currentBit > 0)
  {
    OricBitWrite();
  }
  else
  {
    switch(currentBlockTask) {            
      case BLOCKTASK::READPARAM: // currentBit = 0 y count_r = 255
      case BLOCKTASK::SYNC1:
        readNewByte();
        if (currentByte==0x16) {
          count_r=(byte)count_r-1;
          break;
        }
        // else
        currentBlockTask=BLOCKTASK::SYNC2;
        // and fall through to:
        
      case BLOCKTASK::SYNC2:   
        if((byte)count_r) {
          newByte(0x16);
          count_r=(byte)count_r-1;
        } else {
          newByte(0x24);
          count_r=9;
          lastByte=0;
          currentBlockTask=BLOCKTASK::NEWPARAM;
        }
        break;
                      
      case BLOCKTASK::NEWPARAM:            
        if ((byte)count_r) {
          readNewByte();
          if      ((byte)count_r == 5) bytesToRead = ((unsigned int)outByte)<<8;
          else if ((byte)count_r == 4) bytesToRead = (unsigned int)(bytesToRead + ((unsigned int)outByte)+1);
          else if ((byte)count_r == 3) bytesToRead = (unsigned int)(bytesToRead - (((unsigned int)outByte)<<8));
          else if ((byte)count_r == 2) bytesToRead = (unsigned int)(bytesToRead - ((unsigned int)outByte));
          count_r=(byte)count_r-1;
          break;
        }
        // else
        currentBlockTask=BLOCKTASK::NAME;
        // and fall through to:
              
      case BLOCKTASK::NAME:
        readNewByte();
        if (currentByte==0x00) {
          newByte(0x00);
          count_r=100;
          currentBlockTask=BLOCKTASK::GAP;
        }
        break;

      case BLOCKTASK::GAP:
        if((byte)count_r) {
          currentPeriod = ORICONEPULSE;
          count_r=(byte)count_r-1;
        } else {   
          currentBlockTask=BLOCKTASK::TDATA;
        }             
        break;

      case BLOCKTASK::TDATA:
        OricDataBlock();
        break;
              
      case BLOCKTASK::PAUSE:
        if((byte)count_r) {
          currentPeriod = ORICONEPULSE;
          count_r=(byte)count_r-1;
        } else {   
          count_r=100;
          currentBlockTask = BLOCKTASK::SYNC1;
          #ifdef MenuBLK2A
            if (!skip2A) ForcePauseAfter0();
          #endif
        }
        break;                
    }
  }
}

#endif // tapORIC
