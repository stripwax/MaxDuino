#include "configs.h"
#include "constants.h"
#include "compat.h"
#include "processing_state.h"
#include "file_utils.h"
#include "ayplay.h"
#include "casProcessing.h"
#include "MaxProcessing.h"
#ifdef Use_TRS80
#include "trs80cas.h"
#endif
#ifdef Use_CG
#include "cg.h"
#endif
#ifdef Use_MZF
  #include "mzf.h"
#endif
#ifdef Use_MTX
  #include "mtx.h"
#endif
#ifdef Use_CAQ
  #include "caq.h"
#endif
#ifdef Use_c64
  #include "c64tap.h"
#endif

#ifdef CLI
#define LOG_FILETYPE(x) fprintf(stderr, "Identified: " #x "\n")
#else
#define LOG_FILETYPE(x)
#endif

void checkForEXT() {
  //Check for .xxx file extension as these have no header

#ifdef Use_CAS
  casduino = CASDUINO_FILETYPE::NONE;
#endif

  if (!strcasecmp_P(filenameExt, PSTR("tap"))) {
#ifdef Use_c64
    if (readfile(20, bytesRead) == 20 && c64tap_is_header(filebuffer, filesize)) {
      LOG_FILETYPE("C64 .TAP");
      c64tap_init();
      return;
    }
#endif
    currentTask=TASK::PROCESSID;
    readfile(1,bytesRead);
    if (filebuffer[0] == 0x1A) {
      LOG_FILETYPE("Jupiter Ace .TAP");
      currentID=BLOCKID::JTAP;    
    }   
    #ifdef tapORIC
      //readfile(1,bytesRead);
      else if (filebuffer[0] == 0x16) {
        LOG_FILETYPE("Oric .TAP");
        currentID=BLOCKID::ORIC;
      }
    #endif
    else {
      LOG_FILETYPE("ZX Spectrum .TAP");
      currentID=BLOCKID::TAP;
    }
  }
  else if (!strcasecmp_P(filenameExt, PSTR("p"))) {
    LOG_FILETYPE("ZX80/81 .P");
    currentTask=TASK::PROCESSID;
    currentID=BLOCKID::ZXP;
  }
  else if (!strcasecmp_P(filenameExt, PSTR("o"))) {
    LOG_FILETYPE("ZX80/81 .O");
    currentTask=TASK::PROCESSID;
    currentID=BLOCKID::ZXO;
  }


#ifdef Use_CAQ
else if (!strcasecmp_P(filenameExt, PSTR("caq"))) {
  LOG_FILETYPE("Aquarius .CAQ");
  caq_init();
}
#endif
#ifdef AYPLAY  
  else if (!strcasecmp_P(filenameExt, PSTR("ay"))) {
    LOG_FILETYPE(".AY FILE");
    currentTask=TASK::GETAYHEADER;
    currentID=BLOCKID::AYO;
    AYPASS_hdrptr = AYPASS_STEP::HDRSTART;
  }
#endif
#ifdef Use_UEF
  else if (!strcasecmp_P(filenameExt, PSTR("uef"))) {
    LOG_FILETYPE("BBC .UEF");
    currentTask=TASK::GETUEFHEADER;
    currentID=BLOCKID::UEF;
  }

#endif
#ifdef Use_MZF
  else if (!strcasecmp_P(filenameExt, PSTR("mzf")) ||
           !strcasecmp_P(filenameExt, PSTR("mzt")) ||
           !strcasecmp_P(filenameExt, PSTR("m12"))) {
    // Sharp MZ series tape image (MZF/MZT/M12).
    // Uses PWM encoding: long pulse = 1, short pulse = 0.
    // MZT/M12 reuse the same 128-byte header layout and playback timings as MZF.
    // Initialises internal MZF playback state and then runs through TASK::PROCESSID.
    LOG_FILETYPE("Sharp MZ");
    mzf_init();
  }
#endif

#ifdef Use_MTX
  else if (!strcasecmp_P(filenameExt, PSTR("mtx"))) {
    LOG_FILETYPE("Memotech MTX");
    mtx_init();
  }
#endif
#ifdef Use_CG
  else if ((!strcasecmp_P(filenameExt, PSTR("cgc")) || !strcasecmp_P(filenameExt, PSTR("cas"))) && cgcas_detect_and_init()) {
    LOG_FILETYPE("Color Genie");
    return;
  }
#endif
#ifdef Use_CAS
  else if (!strcasecmp_P(filenameExt, PSTR("cas")) || !strcasecmp_P(filenameExt, PSTR("c10"))) {
#ifdef Use_TRS80
    if (trs80cas_detect_and_init()) {
      LOG_FILETYPE("TRS-80 (Model 1/2/3/4)");
      return;
    }
#endif
    #if defined(Use_DRAGON)
      readfile(1,bytesRead);
      if (filebuffer[0] == 0x55) {
        LOG_FILETYPE("Dragon 32/64 or Tandy CoCo or TRS-80 MC10 .CAS");
        invert=true;
        casduino = CASDUINO_FILETYPE::DRAGONMODE;
        cas_period=249;
      }
      else
    #endif
    {
      LOG_FILETYPE("MSX .CAS");
      casduino = CASDUINO_FILETYPE::CASDUINO;
      invert=false;
    }
  }
#endif
#ifdef ID11CDTspeedup  
  else if (!strcasecmp_P(filenameExt, PSTR("cdt"))) {
    LOG_FILETYPE("Amstrad .CDT");
    AMScdt = true;
  }
#endif
  else
  {
    LOG_FILETYPE(".TZX/.TSX");
  }
}

