#include "configs.h"

#ifdef EEPROM_LOGO_BMP_LOADER
#include "Arduino.h"
#include "EEPROM_bmp_loader.h"
#include "EEPROM_wrappers.h"
#include "file_utils.h"
#include "Display.h"
#include "buttons.h"

static void read_display_sdcard_logo(byte invert, bool eeprom_write, bool compress)
{
  // we read the bitmap and display it, and (if flags are set) simultaneously write
  // to EEPROM, either compressed or uncompressed
  // If compression is enabled, what we display on the screen is consistent with the 'after compression' result
  // (but you can toggle this if you want to see the original image without compression)
 
  if (eeprom_write)
    EEPROM_write_logo_begin();

  #if defined(OLED1306_128_64) || defined(video64text32)
    const byte J = 8;  // 8 LINES
  #else
    const byte J = 4;  // 4 LINES
  #endif
  byte v;
  byte tmp[J];
  for(byte i=0;i<128;i++)
  {
    // load a vertical strip from the BMP into the tmp buffer
    // Why vertical? because one byte for the oled controller represents 8 vertical bits.
    // starting at the top left,
    // and (if compression is enabled) we compress vertical slices.
    // This is made a bit more complicated by the fact that the .BMP is ordered horizontally
    // rather than vertically, and bottom-to-top rather than top-to-bottom
    if ((!compress) || (i%2==0))
    {
      for(byte j=0;j<J;j++)
      { 
        v = 0;
        for(byte bit=0; bit<8; bit++)
        {
          readfile(1, 0x3E + (i>>3) + (128>>3)*bit + 128*(J-1-j));
          v <<= 1;
          v += (filebuffer[0] >> (7-(i & 7))) & 0x01;
        }
        v ^= invert;
        tmp[j] = v;
      }
    }
    // now modify the vertical strip that we have read, in-place, to match
    // what it would look like after compression and decompression
    if (compress)
    {
      for(byte j=0;j<J;j+=2)
      {
        byte compressed = 0;
        for(byte shift=0; shift<1; shift++)
        {
          v = tmp[j+shift];
          for (byte bit=7; bit>0; bit-=2)
          {
            byte pixel = (v>>bit)&0x01;

            // compressed is what we will write to EEPROM
            compressed <<= 1;
            compressed += pixel;

            // v is what we will write to the screen
            v = (v&~(1<<(bit-1))) + (pixel<<(bit-1));
          }
          setByteXY(i, j+shift);
          SendByte(v);
          setByteXY(i+1, j+shift);
          SendByte(v);
        }
        if (eeprom_write && i%2==0)
        {
          EEPROM_write_logo_byte((j/2)*64+(i/2), compressed);
        }
      }
    }
    else
    {
      for(byte j=0;j<J;j++)
      { 
        if (eeprom_write)
        {
          EEPROM_write_logo_byte(j*128+i, tmp[j]);
        }
        setByteXY(i, j);
        SendByte(tmp[j]);
      }
    }
  }

if (eeprom_write)
    EEPROM_write_logo_end();
}


bool EEPROM_bmp_load_file()
{
  // gets called when user clicks on a file
  // check to see if it is a logo file (bitmap, etc)
  // returns true if this was a logo file (regardless of whether it was written to eeprom or not)
  // so that it doesn't get handled like an audio file
  // Also: this is a whole separate event loop, like Menu.  This function exits when you click the STOP button.

// #if defined(OLED1306_128_64) || defined(video64text32)
//   // 128x64
//   if ((!strcasecmp_P(filenameExt, PSTR("bmp"))) && (filesize==1086))
// #else
//   // 128x32
//   if ((!strcasecmp_P(filenameExt, PSTR("bmp"))) && (filesize==574))
// #endif
//   {
//     readfile(2, 0);
//     if (filebuffer[0] == 'B' && filebuffer[1] == 'M') {
//       read_display_sdcard_logo();
//       return true;
//     }
//   }

//   return false;
  byte invert = 0;
  #if defined(EEPROM_LOGO_COMPRESS)
  bool compress = true;
  #else
  const bool compress = false;
  #endif
  clear_display();
  read_display_sdcard_logo(invert, false, compress);
  while(!button_stop() || lastbtn)
  {
    if (button_play())
    {
      printtextF(PSTR("WRITING EEPROM.."),0);
      read_display_sdcard_logo(invert, true, compress);
      printtextF(PSTR("SAVED TO EEPROM"),0);
      delay(1500);
      clear_display();
      return true;
    }
    if (button_up())
    {
      invert ^= 0xff;
      read_display_sdcard_logo(invert, false, compress);
    }
  #if defined(EEPROM_LOGO_COMPRESS)
    if (button_down())
    {
      compress = !compress;
      read_display_sdcard_logo(invert, false, compress);
    }
  #endif
    checkLastButton();
  }
  debounce(button_stop);
  clear_display();
  return true;
}


#endif //EEPROM_LOGO_BMP_LOADER
