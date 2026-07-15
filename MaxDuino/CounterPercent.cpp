#include "configs.h"
#include "Arduino.h"
#include "compat.h"
#include "Display.h"
#include "utils.h"
#include "file_utils.h"

byte currpct;
unsigned int lcdsegs = 0;

static unsigned long timeDiff2 = 0;
static byte newpct = 0;

void lcdTime() {
// on a 16-column screen, this function redraws just this part:
//
// Playing 95%  000
//              ^^^
//
// Not all characters will be redrawn - essentially just the ones that
// need to change - e.g. if countervalue is now a multiple of ten,
// it must mean previously it was one less than a multiple of ten,
// so redraw the tens digit and overwrite the ones digit as '0'.
// It makes for more complex logic, which takes up more space, 
// and there is no definite benefit (slightly less i2c interference
// perhaps - but for one update per second, some of those (e.g. transition
// 99 -> 100) need to update all the digits anyway - so for robustness this
// MUST work worst case.).  This function should probably be greatly simplified.
//
    if (millis() - timeDiff2 > 1000) {   // check switch every second 
    timeDiff2 = millis();           // get current millisecond count

    #ifdef LCDSCREEN16x2

      if (lcdsegs % 10 != 0) {
        // ultima cifra 1,2,3,4,5,6,7,8,9
        ultoa(lcdsegs%10,PlayBytes,10);
        lcd.setCursor(15,0);
        lcd.print(PlayBytes);
      }
      else if (lcdsegs % CNTRBASE != 0) {
        // es 10,20,30,40,50,60,70,80,90,110,120,..
        ultoa(lcdsegs%CNTRBASE,PlayBytes,10);
        lcd.setCursor(14,0);
        lcd.print(PlayBytes);
      }
      else if (lcdsegs % (CNTRBASE*10) != 0) {
        // es 100,200,300,400,500,600,700,800,900,1100,..
        ultoa(lcdsegs%(CNTRBASE*10)/CNTRBASE*100,PlayBytes,10);
        lcd.setCursor(13,0);
        lcd.print(PlayBytes);
      } 
      else {
        // es 000,1000,2000,...
        lcd.setCursor(13,0);
        lcd.print(F("000"));
      }

      lcdsegs++;
    #endif

    #ifdef OLED1306
      #ifdef XY2force
        input[0] = '0'+((lcdsegs % (CNTRBASE*10))/CNTRBASE);
        input[1] = '0'+((lcdsegs % CNTRBASE)/10);
        input[2] = '0'+(lcdsegs % 10);
        input[3] = 0;
        sendStrXY((char *)input,13,0);
        lcdsegs++;

      #else // not XY2force

        if (lcdsegs % 10 != 0) {
          // ultima cifra 1,2,3,4,5,6,7,8,9
          setXY(15,0);
          sendChar(48+lcdsegs%10);
        }
        else if (lcdsegs % CNTRBASE != 0) {
          // es 10,20,30,40,50,60,70,80,90,110,120,..
          setXY(14,0);
          sendChar(48+(lcdsegs%CNTRBASE)/10);
          sendChar('0');
        } else if (lcdsegs % (CNTRBASE*10) != 0) {
          // es 100,200,300,400,500,600,700,800,900,1100,..
          setXY(13,0);
          sendChar(48+(lcdsegs % (CNTRBASE*10))/CNTRBASE);
          sendChar('0');
          sendChar('0');
        } else {
          // es 000,1000,2000,...
          setXY(13,0);
          sendChar('0');
          sendChar('0');
          sendChar('0');
        }

        lcdsegs++; 

      #endif
    #endif

    #ifdef P8544
      if (lcdsegs % 10 != 0) {
        // ultima cifra 1,2,3,4,5,6,7,8,9
        ultoa(lcdsegs%10,PlayBytes,10);
        lcd.setCursor(13,3);
        lcd.print(PlayBytes);
      }
      else if (lcdsegs % CNTRBASE != 0) {
        // es 10,20,30,40,50,60,70,80,90,110,120,..
        ultoa(lcdsegs%CNTRBASE,PlayBytes,10);
        lcd.setCursor(12,3);
        lcd.print(PlayBytes);
      }
      else if (lcdsegs % (CNTRBASE*10) != 0) {
        // es 100,200,300,400,500,600,700,800,900,1100,..
        ultoa(lcdsegs%(CNTRBASE*10)/CNTRBASE*100,PlayBytes,10);
        lcd.setCursor(11,3);
        lcd.print(PlayBytes);
      }
      else {
        // es 000,1000,2000,...
        lcd.setCursor(11,3);
        lcd.print('0');
        lcd.print('0');
        lcd.print('0');
      }
    #endif
  }
}

void lcdPercent() {
  // on a 16-column screen, this function redraws just this part:
  //
  // Playing 95%  000
  //         ^^^^
  //
  // These four characters will all be redrawn.  Alignment looks like this:
  // 0-9:     9% 
  // 10-99:  99%
  // 100%:   100%
  //
  // Note that setting currpct=100 (prior to calling lcdPercent) seems to be a hack
  // used in a few places in MaxDuino processing, but it's not clear why this would be
  // necessary, and in any case this function just sets to 0 before using it anyway.

  newpct=(100 * bytesRead)/filesize;

  if (newpct != currpct || currpct==255) {
    #ifdef LCDSCREEN16x2            
      lcd.setCursor(8,0);
      lcd.print(newpct);lcd.print('%');
    #endif             

    #ifdef OLED1306
      #ifdef XY2force
        ultoa(newpct, PlayBytes, 10);
        strcat_P(PlayBytes, PSTR("%"));
        sendStrXY(PlayBytes, 8, 0);
                                          
      #else // not XY2force
        if (newpct <10) {
          setXY(8,0);
          sendChar(' ');
          sendChar(48+newpct%10);
        }
        else if (newpct <100) {
          setXY(8,0);
          sendChar(48+newpct/10);
          sendChar(48+newpct%10);
        }
        else {
          setXY(8,0);
          sendChar('1');
          sendChar('0');
          sendChar('0');
        }

        sendChar('%');             
      #endif                   
    #endif

    #ifdef P8544
      lcd.setCursor(0,3);
      lcd.print(newpct);lcd.print('%');
    #endif
    
    currpct = newpct;
  }
}
