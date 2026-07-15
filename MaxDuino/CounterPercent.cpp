#include "configs.h"
#include "Arduino.h"
#include "compat.h"
#include "Display.h"
#include "utils.h"
#include "file_utils.h"

byte currpct;
unsigned int counter = 0;

static unsigned long timeDiff2 = 0;
static byte newpct = 0;

void lcdTime() {
// on a 16-column screen, this function redraws just this part:
//
// Playing 95%  000
//              ^^^
//
// All three characters will be redrawn even if value unchanged
// (This is a recent change to simplify logic and reduce firmware size)
// However this will only be redrawn once per second in any case.
//
    if (millis() - timeDiff2 > 1000) {   // check switch every second 
    timeDiff2 = millis();           // get current millisecond count

    #ifdef LCDSCREEN16x2

      if (counter % 10 != 0) {
        // ultima cifra 1,2,3,4,5,6,7,8,9
        ultoa(counter%10,PlayBytes,10);
        lcd.setCursor(15,0);
        lcd.print(PlayBytes);
      }
      else if (counter % CNTRBASE != 0) {
        // es 10,20,30,40,50,60,70,80,90,110,120,..
        ultoa(counter%CNTRBASE,PlayBytes,10);
        lcd.setCursor(14,0);
        lcd.print(PlayBytes);
      }
      else if (counter % (CNTRBASE*10) != 0) {
        // es 100,200,300,400,500,600,700,800,900,1100,..
        ultoa(counter%(CNTRBASE*10)/CNTRBASE*100,PlayBytes,10);
        lcd.setCursor(13,0);
        lcd.print(PlayBytes);
      } 
      else {
        // es 000,1000,2000,...
        lcd.setCursor(13,0);
        lcd.print(F("000"));
      }

      counter++;
    #endif

    #ifdef OLED1306
      #ifdef XY2force
        input[0] = '0'+((counter % (CNTRBASE*10))/CNTRBASE);
        input[1] = '0'+((counter % CNTRBASE)/10);
        input[2] = '0'+(counter % 10);
        input[3] = 0;
        sendStrXY((char *)input,13,0);
        counter++;

      #else // not XY2force

        setXY(13,0);
        sendChar('0'+((counter % (CNTRBASE*10))/CNTRBASE));
        sendChar('0'+((counter % CNTRBASE)/10));
        sendChar('0'+(counter % 10));
        counter++; 

      #endif
    #endif

    #ifdef P8544
      if (counter % 10 != 0) {
        // ultima cifra 1,2,3,4,5,6,7,8,9
        ultoa(counter%10,PlayBytes,10);
        lcd.setCursor(13,3);
        lcd.print(PlayBytes);
      }
      else if (counter % CNTRBASE != 0) {
        // es 10,20,30,40,50,60,70,80,90,110,120,..
        ultoa(counter%CNTRBASE,PlayBytes,10);
        lcd.setCursor(12,3);
        lcd.print(PlayBytes);
      }
      else if (counter % (CNTRBASE*10) != 0) {
        // es 100,200,300,400,500,600,700,800,900,1100,..
        ultoa(counter%(CNTRBASE*10)/CNTRBASE*100,PlayBytes,10);
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
  // These four characters will all be redrawn.  Alignment looks like this (recently changed)
  // 0-9:    0% 
  // 10-99:  99%
  // 100%:   100%
  //
  // Characters only redrawn when pct changes. To 'force' a redraw set currpct to 255

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
        setXY(8,0);
        if (newpct>=100)
          sendChar('1');
        if (newpct>=10)
          sendChar('0'+(newpct/10)%10);
        sendChar('0'+(newpct%10));
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
