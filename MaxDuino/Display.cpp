#include "configs.h"
#include "compat.h"
#include "Arduino.h"
#include "Fonts.h"
#include "Logos.h"
#include "Display.h"
#include "i2c.h"
#include "current_settings.h"
#include "EEPROM_wrappers.h"

// general text-printing buffer shared by most routines, it's long enough for one line of text plus one NUL terminator
char fline[17];

#ifdef LCDSCREEN16x2

  LiquidCrystal_I2C lcd(LCD_I2C_ADDR,16,2); // set the LCD address, and configure for a 16 chars and 2 line display

#elif defined(OLED1306)

  byte HEX_CHAR(byte nibble){
    // optimised for smallest firmware size
    if ((nibble + 7) & 16) nibble += 7;
    return nibble + '0';
  }

  //==========================================================//
  // Used to send commands to the display.
  void sendcommand(unsigned char com)
  {
    mx_i2c_start(OLED_address); //begin transmitting
    mx_i2c_write(0x80); //command mode
    mx_i2c_write(com);
    mx_i2c_end(); // stop transmitting
  }

  //==========================================================//
  // Actually this sends a byte, not a char to draw in the display.
  // Display's chars uses 8 byte font
  void SendByte(unsigned char data)
  {
    mx_i2c_start(OLED_address); //begin transmitting
    mx_i2c_write(0x40); //data mode
    mx_i2c_write(data);
    mx_i2c_end(); // stop transmitting
  }

  //==========================================================//
  // Prints a display char (not just a byte)
  // being multiples of 8. This means we have 16 COLS (0-15)
  // and 8 ROWS (0-7).
  void sendChar(unsigned char data)
  {
    mx_i2c_start(OLED_address);
    mx_i2c_write(0x40);
    for(byte i=0;i<8;i++) {
      mx_i2c_write(pgm_read_byte(myFont[data-0x20]+i));
    }
    mx_i2c_end();
  } 

  //==========================================================//
  // Set the cursor position in a 16 COL * 2 ROW map.
  void setXY(unsigned char col,unsigned char row)
  {
    mx_i2c_start(OLED_address);
    mx_i2c_write(0x80);  
    mx_i2c_write(0xb0+(row)); //set page address (row)
    mx_i2c_write(0x80); //command mode
    #ifdef OLED1106_1_3            
      //mx_i2c_write(0x02+(8*col&0x0f)); //set low col address
      //mx_i2c_write(0x02+(8*col %16)); //set low col address
      mx_i2c_write( 0x02+(8*(col %2)) ); //set low col address
    #else
      //mx_i2c_write(0x00+(8*col&0x0f)); //set low col address
      //mx_i2c_write(0x00+(8*col %16)); //set low col address
      mx_i2c_write( 0x00+(8*(col %2)) ); //set low col address
    #endif
    mx_i2c_write(0x80); 
    //mx_i2c_write(0x10+((8*col>>4)&0x0f)); //set high col address
    mx_i2c_write(0x10+((col /2) %16)); //set high col address 
    mx_i2c_end();         
  }   

  #if defined(EEPROM_LOGO_BMP_LOADER)
  //==========================================================//
  // Set the address to a pixel X and pixelrow row
  void setByteXY(unsigned char x, unsigned char row)
  {
    mx_i2c_start(OLED_address);
    mx_i2c_write(0x80);  
    mx_i2c_write(0xb0+(row)); //set page address (row)
    mx_i2c_write(0x80); //command mode
    #ifdef OLED1106_1_3
    x+=2;
    #endif
    mx_i2c_write(0x00+(x%16)); //set low col address
    mx_i2c_write(0x80); 
    mx_i2c_write(0x10+(x/16)); //set high col address 
    mx_i2c_end();         
  }
  #endif

  //==========================================================//
  // Prints a string regardless the cursor position.
  void sendStr(const __FlashStringHelper *string)
  {
    // copy string from flash to ram
    strncpy_P(fline, (PGM_P)string, 16);
    fline[16]='\0';
    sendStr(fline);
  }

  void sendStr(const char *string)
  {
    unsigned char i=0;
    while(*string)
    {
      for(i=0;i<8;i++) {
        SendByte(pgm_read_byte(myFont[*string-0x20]+i));
      }
      string++;
    }
 
 /*
      const char *stringC=string;
      
      while(*stringC) {
        mx_i2c_start(OLED_address);
        mx_i2c_write(0x40);
        for(byte i=0;i<8;i++){
          mx_i2c_write(pgm_read_byte(myFont[*stringC-0x20]+i));
        }
        mx_i2c_end();
        stringC++;
      } 
 */   
  }

  //==========================================================//
  // Prints a string in coordinates X Y, being multiples of 8.
  // This means we have 16 COLS (0-15) and 8 ROWS (0-7).
  void sendStrXY(const __FlashStringHelper *string, byte X, byte Y)
  {
    strncpy_P(fline, (PGM_P)string, 16);
    fline[16]='\0';
    sendStrXY(fline, X, Y);
  }
  
  void sendStrXY(const char *string, byte X, byte Y)
  {
    #ifdef XY
      setXY(X,Y);
      unsigned char i=0;
      while(*string)
      {
        #ifdef OLED1306_128_64
          for(i=0;i<8;i++)  SendByte(pgm_read_byte(myFont[*string-0x20]+i));
        #else
          for(i=0;i<4;i++)  SendByte(pgm_read_byte(myFont[*string-0x20]+i));    
        #endif
        string++;
      }
    #endif
  
    #if defined(XY2) && not defined(DoubleFont)
      const char *stringL=string;
      byte nib, dbl;

      setXY(X,Y);
      while(*stringL) {
        mx_i2c_start(OLED_address);
        mx_i2c_write(0x40);

        for(byte i=0;i<8;i++){
          nib=(pgm_read_byte(myFont[*stringL-0x20]+i)) & 0x0F;
          dbl=pgm_read_byte(DFONT+nib);
          mx_i2c_write(dbl);
        }

        mx_i2c_end();
        stringL++;
      }
    
      setXY(X,Y+1);
      while(*string){      
        mx_i2c_start(OLED_address);
        mx_i2c_write(0x40);           
        
        for(byte i=0;i<8;i++){
          nib=(pgm_read_byte(myFont[*string-0x20]+i))/16;
          dbl=pgm_read_byte(DFONT+nib);
          mx_i2c_write(dbl);
        }

        mx_i2c_end();
        string++;
      }
    
    #endif // defined(XY2) && not defined(DoubleFont)

  #if defined(XY2) && defined(DoubleFont)
    const char *stringL=string;
  
    setXY(X,Y);
    while(*stringL) {
      mx_i2c_start(OLED_address);
      mx_i2c_write(0x40); 
    
      for(byte i=0;i<8;i++){
        byte ril=(pgm_read_byte(myFont[*stringL-0x20]+i));
        mx_i2c_write(ril);
      }
      mx_i2c_end();
      stringL++;
    }
  
    setXY (X,Y+1);
    while(*string) {
      mx_i2c_start(OLED_address);
      mx_i2c_write(0x40); 
      for(byte i=0;i<8;i++){
        byte rih=(pgm_read_byte(myFont[*string-0x20]+i+8));
        mx_i2c_write(rih);
      }
      mx_i2c_end();
      string++;
    }
  
    #endif // defined(XY2) && defined(DoubleFont)
  }

  //==========================================================//
  // Resets display depending on the actual mode.
  void reset_display(void)
  {
    displayOff();
    clear_display();  
    #if defined(video64text32)     // back to 128x32
      sendcommand(0xA8);            //SSD1306_SETMULTIPLEX     
      sendcommand(0x1f);            //--1/48 duty, NEW!!! Feb 23, 2013: 128x32 OLED: 0x01f,  128x64 OLED 0x03f     
      sendcommand(0xDA);           //0xDA
      sendcommand(0x02);           //COMSCANDEC /* com pin HW config, sequential com pin config (bit 4), disable left/right remap (bit 5) */      
    #endif
    displayOn();
  }

  //==========================================================//
  // Turns display on.
  void displayOn(void)
  {
    sendcommand(0xaf);    //display on
  }

  //==========================================================//
  // Turns display off.
  void displayOff(void)
  {
    sendcommand(0xae);    //display off
  }

  //==========================================================//
  // Clears the display by sending 0 to all the screen map.
  void clear_display(void)
  {
    unsigned char i,k;
    #if defined(OLED1306_128_64) || defined(video64text32)
      for(k=0;k<8;k++) // 8 LINES
    #else
      for(k=0;k<4;k++) // 4 LINES  
    #endif
    { 
      setXY(0,k);    
      {
        for(i=128;i>0;i--)
        {
          SendByte(0);         //clear all COL
        }
      }
    }
  }

#if defined(HAS_LOGO)
  #if defined(LOGO_FADE_IN)
  inline byte attract_mask(word screen_address, byte frame)
  {
    // compute some bitwise mask that converges at 255 for all bits but
    // could otherwise be any kind of pattern based on frame number and screen address
    // screen_address in 0..1023
    // frame in 0..128
    if (frame<64)
    {
      if((screen_address&0xff)<128)
        return frame*2 <= (screen_address&0xff)?0:(screen_address&0x01)?170:85;
      else
        return frame*2 <= (255-(screen_address&0xff))?0:(screen_address&0x01)?170:85;
    }
    else
    {
      if((screen_address&0xff)<128)
        return (frame-64)*2 <= (screen_address&0xff)?(screen_address&0x01)?170:85:255;
      else
        return (frame-64)*2 <= (255-(screen_address&0xff))?(screen_address&0x01)?170:85:255;
    }
  }
  #endif

  #if defined(LOAD_EEPROM_LOGO) || defined(LOAD_EEPROM_LOGO_MEM_FALLBACK)
  inline byte load_eeprom_logo_byte(byte i, byte j)
  {
    byte t;
    #if not defined(EEPROM_LOGO_COMPRESS)
      EEPROM_read_logo_byte(j*128+i, t);
    #elif defined(EEPROM_LOGO_COMPRESS)
      if (i%2 == 0){
        t=0;
        #ifdef OLED1306_128_64
          if (j%2 == 0) {
            byte ril=0;
            byte ib=0;
            EEPROM_read_logo_byte((j/2)*64+i/2, ril);

            for(ib=0;ib<4;ib++) {
              if (bitRead (ril,ib)) {
                t |= (1 << ib*2);
                #ifdef COMPRESS_REPEAT_ROW
                  t |= (1 << (ib*2)+1);
                #endif
              }
            }
          } else {
            byte rih=0;
            byte ic=0;
            EEPROM_read_logo_byte((j/2)*64+i/2, rih);

            for(ic=4;ic<8;ic++) {
              if (bitRead (rih,ic)) {
                t |= (1 << (ic-4)*2);
                #ifdef COMPRESS_REPEAT_ROW
                  t |= (1 << ((ic-4)*2)+1);
                #endif
              }
            }
          }
        #else
          EEPROM_read_logo_byte(j*64+i/2, t);
        #endif
      }
    #endif
    return t;
  }
  #endif

  #if defined(LOAD_MEM_LOGO) || defined(LOAD_EEPROM_LOGO_MEM_FALLBACK)
  inline byte load_mem_logo_byte(byte i, byte j)
  {
    return pgm_read_byte(logo+j*128+i);
  }
  #endif

  void load_logo()
  {
    // read logo from eeprom and write to screen
    #if defined(LOAD_EEPROM_LOGO_MEM_FALLBACK)
    bool found_eeprom_logo = EEPROM_check_logo_valid();
    Serial.print("found_eeprom_logo=");
    Serial.println(found_eeprom_logo);
    #endif

    byte t;
    #if defined(LOGO_FADE_IN)
    unsigned long start_time = millis();
    for(byte frame=0; frame<=128;)
    {
    #endif

    #if defined(OLED1306_128_64) || defined(video64text32)
      for(byte j=0;j<8;j++) {
    #else
      for(byte j=0;j<4;j++) {
    #endif
      setXY(0,j);
      for(byte i=0;i<128;i++)     // show 128* 32 Logo
      {
        #if defined(LOAD_EEPROM_LOGO_MEM_FALLBACK)
          if (found_eeprom_logo)
            t = load_eeprom_logo_byte(i, j);
          else
            t = load_mem_logo_byte(i, j);
        #elif defined(LOAD_EEPROM_LOGO)
          t = load_eeprom_logo_byte(i, j);
        #else
          t = load_mem_logo_byte(i, j);
        #endif

        #if defined(LOGO_FADE_IN)
        byte mask = attract_mask(j*128+i, frame);
        t &= mask;
        #endif
        SendByte(t);
      }  
    }

    #if defined(LOGO_FADE_IN)
    // LOGO_FADE_IN defines the number of milliseconds for the animation
    const unsigned long millis_per_frame = (LOGO_FADE_IN/128);
    byte next_frame = ((millis() - start_time) / millis_per_frame); // if we could do 64 fps the animation takes 2 seconds
    if (next_frame == frame)
    {
      delay(millis_per_frame);
      next_frame = frame+1;
    } 
    if (frame < 128 && next_frame > 128)
      // always make sure we hit final frame
      next_frame = 128;
    frame = next_frame;
    }  
    #endif
  }
#endif // HAS_LOGO

  #if defined(RECORD_EEPROM_LOGO)
  void record_eeprom_logo()
  {
    // read logo from firmware and write to eeprom
    EEPROM_write_logo_begin();
    #if defined(OLED1306_128_64) || defined(video64text32)
      for(byte j=0;j<8;j++) {
    #else
      for(byte j=0;j<4;j++) {
    #endif
      setXY(0,j);
      for(byte i=0;i<128;i++)     // show 128* 32 Logo
      {
        #if not defined(EEPROM_LOGO_COMPRESS)
          EEPROM_write_logo_byte(j*128+i, pgm_read_byte(logo+j*128+i));
        #else
          if (i%2 == 0){
            #ifdef OLED1306_128_64
              if (j%2 == 0){
                byte nl=0;
                byte rnl=0;
                byte nb=0;
                rnl = pgm_read_byte(logo+j*128+i);
                for(nb=0;nb<4;nb++) {
                  if (bitRead (rnl,nb*2)) {
                    nl |= (1 << nb);
                  }
                }
                byte nh=0;
                byte rnh=0;
                byte nc=0;
                rnh = pgm_read_byte(logo+(j+1)*128+i);
                for(nc=0;nc<4;nc++) {
                  if (bitRead (rnh,nc*2)) {
                    nh |= (1 << nc);
                  }
                }

                EEPROM_write_logo_byte((j/2)*64+i/2,nl+nh*16);
              } 

            #else
              EEPROM_write_logo_byte(j*64+i/2, pgm_read_byte(logo+j*128+i));
            #endif
          }
        #endif   
      }  
    }
    EEPROM_write_logo_end();
  }
  #endif

  //==========================================================//
  // Inits oled and draws logo at startup
  void init_OLED(void)
  {
    mx_i2c_init();

    /*
    sequence := { direct_value | escape_sequence }
    direct_value := 0..254
    escape_sequence := value_255 | sequence_end | delay | adr | cs | not_used 
    value_255 := 255 255
    sequence_end = 255 254
    delay := 255 0..127
    adr := 255 0x0e0 .. 0x0ef 
    cs := 255 0x0d0 .. 0x0df 
    not_used := 255 101..254
    */  
    sendcommand(0xAE);             //DISPLAYOFF

     //sendcommand(0xD5);  // SSD1306_SETDISPLAYCLOCKDIV
     //sendcommand(0x80);  // the suggested ratio 0x80, 4 higher bits for oscilator frecuency, and 4 lower for divide ratio

    // sendcommand(0xD3);  // SSD1306_SETDISPLAYOFFSET
    // sendcommand(0x0);  // no offset

    #if defined(OLED1306_128_64) || defined(video64text32) 
      sendcommand(0xA8);            //SSD1306_SETMULTIPLEX      
      sendcommand(0x3f);            //--1/48 duty, NEW!!!  128x64 OLED: 0x3f
    #else
      sendcommand(0xA8);            //SSD1306_SETMULTIPLEX     
      sendcommand(0x1f);            //--1/48 duty, NEW!!!  128x32 OLED: 0x1f
    #endif
      
    sendcommand(0x8D);            //CHARGEPUMP, 0x14 to enable and 0x10 to turn off
    sendcommand(0x14);
    
    #ifdef OLED_ROTATE180    
      sendcommand(0xA0 | 0x1);      //SEGREMAP   //Rotate screen 180 deg 0xA1
      sendcommand(0xC8);            //COMSCANDEC  Rotate screen 180 Deg /* c0: scan dir normal, c8: reverse */
    #else    
      sendcommand(0xA0);            //SEGREMAP   //no rotation
      sendcommand(0xC0);            //COMSCANDEC  no reverse /* c0: scan dir normal */
    #endif
      
    //COMSCANDEC /* com pin HW config, sequential com pin config (bit 4), disable left/right remap (bit 5) */
    #if defined(OLED1306_128_64) || defined(video64text32)             
      sendcommand(0xDA);
      sendcommand(0x12);            //0x12 for 128x64 mode, default value                           
    #else                           
      sendcommand(0xDA);
      sendcommand(0x02);            //0x02 for 128x32 mode
    #endif

    #if defined(OLED_SETCONTRAS)
      sendcommand(0x81);                    //SETCONTRAS
      sendcommand(OLED_SETCONTRAS);         // value inside userconfig.h
    #elif defined(OLED1106_1_3)
      sendcommand(0x81);                    //SETCONTRAS
      sendcommand(0x27);                    // default 0x80 : (SMALL 0x00, LARGE 0xFF)
    #else
      sendcommand(0x81);                    //SETCONTRAS
      sendcommand(0x47);                    // default 0x80 : (SMALL 0x00, LARGE 0xFF)
    #endif
      
    #if defined(XY2) 
      sendcommand(0x20);            //Set Memory Addressing Mode
      sendcommand(0x00);            //Set Memory Addressing Mode ab Horizontal addressing mode
    #else
      sendcommand(0x20);            //Set Memory Addressing Mode
      sendcommand(0x02);            //Set Memory Addressing Mode ab Page addressing mode      
    #endif              

     //sendcommand(0xd9); // SSD1306_SETPRECHARGE
     //sendcommand(0xF1);

    // sendcommand(0xDB); // SSD1306_SETVCOMDETECT
    // sendcommand(0x40);

    // sendcommand(0xA4); // SSD1306_DISPLAYALLON_RESUME
    // sendcommand(0xA6); // SSD1306_NORMALDISPLAY
    // sendcommand(0x2E); // SSD1306_DEACTIVATE_SCROLL

    #if defined(HAS_LOGO)
      sendcommand(0xAF);    //display on
      load_logo();
    #endif

    #if defined(RECORD_EEPROM_LOGO)
    record_eeprom_logo();
    #endif
  }

  
//==========================================================//
//END #if defined(OLED1306)

#elif defined(P8544)

  pcd8544 lcd(dc_pin, reset_pin, cs_pin);

  void bitmap2(uint8_t bdata[], uint8_t rows, uint8_t columns)
  {
    uint8_t row, column;
    uint16_t  i; // de tipo word para poder leer mas de 256 bytes
    uint8_t toprow = 0;
    uint8_t startcolumn = 0;
    for (row = 0, i = 0; row < rows; row++) {
      lcd.gotoRc(row+toprow, startcolumn);
      for (column = 0; column < columns; column++) {
        lcd.data(pgm_read_byte(&bdata[i++]));
      }
    }
  }

  void P8544_splash (void)
  {
    lcd.gotoRc(0, 0);
    bitmap2(logo, 6,84);
    delay(2000); 
    lcd.clear();
  }

#endif // defined(P8544)


byte scrollPos = 0;                 //Stores scrolling text position
//unsigned long scrollTime = millis() + scrollWait;
unsigned long scrollTime = 0;

void scrollText(char* text, bool is_dir){
  if(millis()<scrollTime)
    return;

  #ifdef LCDSCREEN16x2
  //Text scrolling routine.  Setup for 16x2 screen so will only display 16 chars
  byte i=0;
  byte p=scrollPos;
  if(is_dir) {
    fline[0]='>';
    i++;
  }
  for(;i<16;i++,p++)
  {
    if(p<strlen(text)) 
    {
      fline[i]=text[p];
    } else {
      fline[i]='\0';
    }
  }
  fline[16]='\0';
  printtext(fline,1);
  #endif

  #ifdef OLED1306
  //Text scrolling routine.  Setup for 16x2 screen so will only display 16 chars
  byte i=0;
  byte p=scrollPos;
  if(is_dir) {
    fline[0]='>';
    i++;
  }
  for(;i<16;i++,p++)
  {
    if(p<strlen(text)) 
    {
      fline[i]=text[p];
    } else {
      fline[i]='\0';
    }
  }
  fline[16]='\0';
  printtext(fline,lineaxy);
  #endif

  #ifdef P8544
  //Text scrolling routine.  Setup for P8544 screen so will only display 14 chars
  byte i=0;
  byte p=scrollPos;
  if(is_dir) {
    fline[0]='>';
    i++;
  }
  for(;i<14;i++,p++)
  {
    if(p<strlen(text)) 
    {
      fline[i]=text[p];
    } else {
      fline[i]='\0';
    }
  }
  fline[14]='\0';
  printtext(fline,1);
  #endif

  scrollTime = millis();
  scrollTime += (scrollPos? scrollSpeed : scrollWait);
  scrollPos = (scrollPos+1) %strlen(text);
}

void scrollText(char* text, bool is_dir, byte scroll_pos) {
  // this variant resets the scroll position and timer, so printing is immediate
  scrollPos = scroll_pos;
  scrollTime = 0;
  scrollText(text, is_dir);
}

void printtext2F(const char* text, byte l) {  //Print text to screen. 
  
  #ifdef SERIALSCREEN
  Serial.println(reinterpret_cast <const __FlashStringHelper *> (text));
  #endif
  
  #ifdef LCDSCREEN16x2
    lcd.setCursor(0,l);
    char x = 0;
    while (char ch=pgm_read_byte(text+x)) {
      lcd.print(ch);
      x++;
    }
  #endif

  #ifdef OLED1306
    #ifdef XY2
      sendStrXY(reinterpret_cast<const __FlashStringHelper *> (text),0,l);
    #endif
     
    #ifdef XY 
      setXY(0,l);
      char x = 0;
      while (char ch=pgm_read_byte(text+x)) {
        sendChar(ch);
        x++;
      }
    #endif
  #endif

  #ifdef P8544
    strncpy_P(fline, text, 14);
    lcd.setCursor(0,l);
    lcd.print(fline);
  #endif 
   
}

void printtextF(const char* text, byte l) {  //Print text to screen. 
  
  #ifdef SERIALSCREEN
    Serial.println(reinterpret_cast <const __FlashStringHelper *> (text));
  #endif
  
  #ifdef LCDSCREEN16x2
    lcd.setCursor(0,l);
    char x = 0;
    while (char ch=pgm_read_byte(text+x)) {
      lcd.print(ch);
      x++;
    }
    for(; x<16; x++) {
      lcd.print(' ');
    }
  #endif

  #ifdef OLED1306
    #ifdef XY2
      strncpy_P(fline, text, 16);
      for(byte i=strlen(fline);i<16;i++) {
        fline[i]=0x20;
      }
      sendStrXY(fline,0,l);
    #endif
     
    #ifdef XY 
      setXY(0,l);
      char x = 0;
      while (char ch=pgm_read_byte(text+x)) {
        sendChar(ch);
        x++;
      }
      for(; x<16; x++) {
        sendChar(' ');
      }
     #endif
  #endif

  #ifdef P8544
    strncpy_P(fline, text, 14);
    for(byte i=strlen(fline);i<14;i++) {
      fline[i]=0x20;
    }
    lcd.setCursor(0,l);
    lcd.print(fline);
  #endif 
   
}

void printtext(char* text, byte l) {  //Print text to screen. 
  
  #ifdef SERIALSCREEN
    Serial.println(text);
  #endif
  
  #ifdef LCDSCREEN16x2
    lcd.setCursor(0,l);
    char ch;
    bool end=false;
    for(byte i=0;i<16;i++) {
      if(!end)
        if(text[i]=='\0')
          end=true;
      if(!end)
        ch=text[i];
      else
        ch=' ';
      lcd.print(ch); 
    }
  #endif

  #ifdef OLED1306
    #ifdef XY2
      bool end=false;
      for(byte i=0;i<16;i++) {
        if(!end)
          if(text[i]=='\0')
            end=true;
        if(!end)
          fline[i]=text[i];
        else
          fline[i]=' ';
      }    
      sendStrXY(fline,0,l);
    #endif
    
    #ifdef XY
      setXY(0,l); 
      char ch;
      bool end=false;
      for(byte i=0;i<16;i++) {
        if(!end)
          if(text[i]=='\0')
            end=true;
        if(!end)
          ch=text[i];
        else
          ch=' ';
        sendChar(ch);
      }       
    #endif
  #endif

  #ifdef P8544
    bool end=false;
    for(byte i=0;i<16;i++) {
      if(!end)
        if(text[i]=='\0')
          end=true;
      if(!end)
        fline[i]=text[i];
      else
        fline[i]=' ';
    }  
    lcd.setCursor(0,l);
    lcd.print(fline);
  #endif 
}

#if defined(OLED1306)
void OledStatusLine() {
  #ifdef XY
    setXY(4,2);
    sendStr(F("ID:   BLK:"));
    #ifdef OLED1306_128_64
      setXY(0,7);
      ultoa(BAUDRATE,fline,10);
      sendStr(fline);

      #ifndef NO_MOTOR       
        setXY(5,7);
        if(mselectMask) {
          sendStr(F(" M:ON"));
        } else {
          sendStr(F("m:off"));
        }
      #endif    

      setXY(11,7); 
      if (TSXCONTROLzxpolarityUEFSWITCHPARITY) {
        sendStr(F(" %^ON"));
      } else {
        sendStr(F("%^off"));
      }

    #else // OLED1306_128_64 not defined
      setXY(0,3);
      ultoa(BAUDRATE,fline,10);
      sendStr(fline);

      #ifndef NO_MOTOR        
        setXY(5,3);
        if(mselectMask) {
          sendStr(F(" M:ON"));
        } else {
          sendStr(F("m:off"));
        }
      #endif    
      setXY(11,3); 
      if (TSXCONTROLzxpolarityUEFSWITCHPARITY) {
        sendStr(F(" %^ON"));
      } else {
        sendStr(F("%^off"));
      }
    #endif
  #endif
  
  #ifdef XY2                        // Y with double value
    #ifdef OLED1306_128_64          // 8 rows supported
      sendStrXY(F("ID:   BLK:"),4,4);        
      ultoa(BAUDRATE,fline,10);
      sendStrXY(fline,0,6);
      #ifndef NO_MOTOR       
        if(mselectMask) {
          sendStrXY(F(" M:ON"),5,6);
        } else {
          sendStrXY(F("m:off"),5,6);
        }
      #endif      
      if (TSXCONTROLzxpolarityUEFSWITCHPARITY) {
        sendStrXY(F(" %^ON"),11,6);
      } else {
        sendStrXY(F("%^off"),11,6);
      }
    #endif      
  #endif  

}
#endif // defined(OLED1306)

void scrollTextReset()
{
  scrollTime=millis()+scrollWait;
  scrollPos=0;
}
