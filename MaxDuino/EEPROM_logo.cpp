#include "configs.h"
#include "EEPROM_logo.h"
#include "EEPROM_wrappers.h"

#ifdef RECORD_EEPROM_LOGO
// This build variant will just write the 'hardcoded' logo to eeprom
// to save firmware space; and you can then flash a standard image configured
// to load the image from eeprom
#include "Logos.h"
void write_logo_to_eeprom()
{
#if defined(OLED1306_128_64) || defined(video64text32)
  // logo is 128 x 64
  byte const J=8;
#else
  // logo is 128 x 32
  byte const J=4;
#endif
  for(int j=0;j<J;j++)
  {
    for(int i=0;i<128;i++)
    {
#if !defined(EEPROM_LOGO_COMPRESS)
      EEPROM_put(j*128+i, pgm_read_byte(logo+j*128+i));
#else
    #if defined(OLED1306_128_64) || defined(video64text32)
      // logo compression. for each block of 2x2 pixels, only store one pixel value,
      // for a factor4 reduction in size in eeprom (= 256 bytes instead of 1024 bytes)
      if (i%2 == 0 && j%2 == 0)
      {
        // count the number of pixels in a 2x2 area with i,j at the top left
        byte left, right;
        left = pgm_read_byte(logo+j*128+i);
        right = pgm_read_byte(logo+j*128+i+1);
        byte v=0;
        for(byte nb=0;nb<4;nb++)
        {
          byte cnt = ((left >> (nb*2))&0x01) + ((left >> (nb*2+1))&0x01) + ((right >> (nb*2))&0x01) + ((right >> (nb*2+1))&0x01);
          v >>= 1;
          if (cnt >= 2) {
            v |= 0x80;
          }
        }
        left = pgm_read_byte(logo+(j+1)*128+i);
        right = pgm_read_byte(logo+(j+1)*128+i+1);
        for(byte nb=0;nb<4;nb++)
        {
          byte cnt = ((left >> (nb*2))&0x01) + ((left >> (nb*2+1))&0x01) + ((right >> (nb*2))&0x01) + ((right >> (nb*2+1))&0x01);
          v >>= 1;
          if (cnt >= 2) {
            v |= 0x80;
          }
        }
        EEPROM_put((j/2)*64+i/2,v);
      } 
    #else
      // for 128x32 logos, only store the even vertical slices
      // for a factor2 reduction in size in eeprom (= 256 bytes instead of 512 bytes)
      if (i%2 == 0)
      {
        EEPROM_put(j*64+i/2, pgm_read_byte(logo+j*128+i));
      }
    #endif
  #endif 
    }
  }

  EEPROM_commit();
}        
#endif

#if defined(RECORD_EEPROM_LOGO_FROM_SDCARD)
// This build variant will let you browse bitmaps and select one to view it and write to eeprom
#include "file_utils.h"
#include "Display.h"
#include "buttons.h"
#include "buffer.h"  // using the audio buffer as temporary space for loading .bmp and also for doing compression

bool handle_load_logo_file()
{
  // gets called when user clicks on a file
  // check to see if it is a logo file (bitmap, etc)
  // returns true if this was a logo file (regardless of whether it was written to eeprom or not)
  // so that it doesn't get handled like an audio file
  // Also: this is a whole separate event loop, like Menu.  This function exits when you click the STOP button.

  if (strcasecmp_P(filenameExt, PSTR("bmp")) != 0)
    return false; // not a .bmp file

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
  return true;
}

void read_display_sdcard_logo(byte invert, bool eeprom_write, bool compress)
{
  // we read the bitmap and display it, and (if flags are set) simultaneously write
  // to EEPROM, either compressed or uncompressed
  // If compression is enabled, what we display on the screen is consistent with the 'after compression' result
  // (but you can toggle this if you want to see the original image without compression)
 
  #if defined(OLED1306_128_64) || defined(video64text32)
    const byte J = 8;  // 8 LINES
  #else
    const byte J = 4;  // 4 LINES
  #endif
  byte v;
  volatile byte * tmp;
  for(byte i=0;i<128;i++)
  {
    // load a vertical strip from the BMP into the playbuffer
    // Why vertical? because one byte for the oled controller represents 8 vertical bits.
    // starting at the top left,
    // and (if compression is enabled) we compress vertical slices.
    // This is made a bit more complicated by the fact that the .BMP is ordered horizontally
    // rather than vertically, and bottom-to-top rather than top-to-bottom
    tmp = wbuffer[0];
    if ((!compress) || (i%2==0))
    {
      for(byte j=0;j<J;j++)
      { 
        v = 0;
        for(byte bit=0; bit<8; bit++)
        {
          readfile(1, 0x3E + (i/8) + (128/8)*bit + 128*(J-j));
          v <<= 1;
          v += (filebuffer[0] >> (7-(i & 7))) & 0x01;
        }
        v ^= invert;
        *tmp++ = v;
      }
    }
    // now modify the vertical strip that we have read, in-place, to match
    // what it would look like after compression and decompression
    tmp = wbuffer[0];
    if (compress)
    {
      for(byte j=0;j<J;j+=2)
      {
        byte compressed = 0;
        for(byte shift=0; shift<1; shift++)
        {
          v = *tmp;
          for (byte bit=7; bit>0; bit-=2)
          {
            byte pixel = (v>>bit)&0x01;

            // compressed is what we will write to EEPROM
            compressed <<= 1;
            compressed += pixel;

            // v is what we will write to the screen
            v = (v&~(1<<(bit-1))) + (pixel<<(bit-1));
          }
          *tmp++ = v;
          setXY(i, j);
          SendByte(v);
          setXY(i+1, j);
          SendByte(v);
        }
        if (eeprom_write)
        {
          EEPROM_put((j/2)*64+(i/2), compressed);
        }
      }
    }
    else
    {
      for(byte j=0;j<J;j++)
      { 
        if (eeprom_write)
        {
          EEPROM_put(j*128+i, *tmp);
        }
        setXY(i, j);
        SendByte(*tmp);
        tmp++;
      }
    }
  }
  EEPROM_commit();
}

#endif

#if defined(LOAD_EEPROM_LOGO) && defined(EEPROM_LOGO_COMPRESS)
byte EEPROM_get_compressed(int x, int y)
{
#if !defined(COMPRESS_REPEAT_ROW)
  // repeat row also means repeat column (now)
  // If not repeating columns, then alternate columns (and rows) are blank
  if (x%2 == 0)
  {
    return 0;
  }
#endif
#if defined(OLED1306_128_64) || defined(video64text32)
  byte t=0;
  byte r;
  EEPROM_get((y/2)*64+x/2, r);
  if (y%2 == 1)
  {
    r >>= 4;
  }
  for(byte ib=0;ib<4;ib++)
  {
    if (bitRead (r,ib))
    {
    #ifdef COMPRESS_REPEAT_ROW
      t |= (3 << (ib*2));
    #else
      t |= (1 << (ib*2));
    #endif
    }
  }
#else // 128x32 logo
  // for 128x32 logos, only store the even vertical slices
  // for a factor2 reduction in size in eeprom (= 256 bytes instead of 512 bytes).
  // Columns are repeated.
  byte t;
  EEPROM_get(y*64+x/2, t);
#endif
  return(t);
}
#endif
