#include "EEPROM_logo.h"
#include "EEPROM_wrappers.h"

#ifdef RECORD_EEPROM_LOGO
// This build variant will just write the 'hardcoded' logo to eeprom
// to save firmware space; and you can then flash a standard image configured
// to load the image from eeprom
void write_logo_to_eeprom()
{
#if defined(OLED1306_128_64) || defined(video64text32)
  // logo is 128 x 64
  for(int j=0;j<8;j++)
#else
  // logo is 128 x 32
  for(int j=0;j<4;j++)
#endif
  {
    for(int i=0;i<128;i++)     // show 128* 32 Logo
    {
#if not defined(EEPROM_LOGO_COMPRESS)
      EEPROM_put(j*128+i, pgm_read_byte(logo+j*128+i));
#else
      if (i%2 == 0)
      {
      #ifdef OLED1306_128_64
        if (j%2 == 0)
        {
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

          EEPROM_put((j/2)*64+i/2,nl+nh*16);
        } 
      #else
        // no compression for 128x32 logos. sorry.
        EEPROM_put(j*64+i/2, pgm_read_byte(logo+j*128+i));
      #endif
      }
    #endif 
    }
  }
}        

#elif defined(SDCARD_RECORD_EEPROM_LOGO)
// This build variant will try to load a logo from SDCARD and write to eeprom
// If logo in eeprom is the same, this will do nothing (to save write cycles on eeprom)
void write_sdcard_logo_to_eeprom()
{
  #if defined(OLED1306_128_64) || defined(video64text32)
    // logo is 128 x 64
  #else
    // logo is 128 x 32
  #endif
}

#ifdef EEPROM_LOGO_COMPRESS
void EEPROM_get_compressed(int i, int j)
{
  if (i%2 == 0)
  {
    t=0;
  #ifdef OLED1306_128_64
    if (j%2 == 0) {
      byte ril=0;
      byte ib=0;
      EEPROM_get((j/2)*64+i/2, ril);

      for(ib=0;ib<4;ib++)
      {
        if (bitRead (ril,ib))
        {
          t |= (1 << ib*2);
        #ifdef COMPRESS_REPEAT_ROW
          t |= (1 << (ib*2)+1);
        #endif
        }
      }
    } else {
      byte rih=0;
      byte ic=0;
      EEPROM_get((j/2)*64+i/2, rih);

      for(ic=4;ic<8;ic++)
      {
        if (bitRead (rih,ic))
        {
          t |= (1 << (ic-4)*2);
        #ifdef COMPRESS_REPEAT_ROW
          t |= (1 << ((ic-4)*2)+1);
        #endif
        }
      }
    }
  #else
    // no compression for 128x32 logos. sorry.
    EEPROM_get(j*64+i/2, t);
  #endif
  }
}

}
#endif


#endif