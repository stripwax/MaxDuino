// CLI entry point for MaxDuino WAV export
#include "configs.h"
#include "Arduino.h"
#include "isr.h"
#include "MaxProcessing.h"
#include "file_utils.h"
#include "buffer.h"
#include "processing_state.h"
#include "current_settings.h"
#include "TimerCounter.h"
#include "Display.h"
#include "MaxDuino.h"
#include "utils.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

// This is our pseduo pin output. WRITE_HIGH and WRITE_LOW are the only things that change this
byte cli_output_value=0;

// --- Globals required by compiled source files ---
block_type block = 0;

// --- Stubs for functions called from compiled code ---
void printtext2F(const char*, byte) {}
void printtextF(const char*, byte) {}
bool button_stop() { return false; }
void block_mem_oled() {}

void seekFile() {}

void stopFile() {
  UniStop();
  start = 0;
}

// --- WAV writing ---
static unsigned long wavSampleCount = 0;
static FILE *wavOut = nullptr;

static void wavWriteSample(unsigned char sample) {
  fputc(sample, wavOut);
  wavSampleCount++;
}

static void wavWriteHeader(FILE *f, unsigned long sampleRate, unsigned long totalSamples) {
  unsigned long dataSize = totalSamples;
  unsigned long fileSize = 36 + dataSize;
  unsigned short numChannels = 1;
  unsigned short bitsPerSample = 8;
  unsigned short blockAlign = numChannels * bitsPerSample / 8;
  unsigned long byteRate = sampleRate * blockAlign;

  // RIFF header
  fwrite("RIFF", 1, 4, f);
  unsigned char buf[4];
  buf[0]=fileSize&0xff; buf[1]=(fileSize>>8)&0xff; buf[2]=(fileSize>>16)&0xff; buf[3]=(fileSize>>24)&0xff;
  fwrite(buf, 1, 4, f);
  fwrite("WAVE", 1, 4, f);

  // fmt chunk
  fwrite("fmt ", 1, 4, f);
  unsigned long chunkSize = 16;
  buf[0]=chunkSize&0xff; buf[1]=(chunkSize>>8)&0xff; buf[2]=(chunkSize>>16)&0xff; buf[3]=(chunkSize>>24)&0xff;
  fwrite(buf, 1, 4, f);
  unsigned short fmt = 1; // PCM
  buf[0]=fmt&0xff; buf[1]=(fmt>>8)&0xff;
  fwrite(buf, 1, 2, f);
  buf[0]=numChannels&0xff; buf[1]=(numChannels>>8)&0xff;
  fwrite(buf, 1, 2, f);
  buf[0]=sampleRate&0xff; buf[1]=(sampleRate>>8)&0xff; buf[2]=(sampleRate>>16)&0xff; buf[3]=(sampleRate>>24)&0xff;
  fwrite(buf, 1, 4, f);
  buf[0]=byteRate&0xff; buf[1]=(byteRate>>8)&0xff; buf[2]=(byteRate>>16)&0xff; buf[3]=(byteRate>>24)&0xff;
  fwrite(buf, 1, 4, f);
  buf[0]=blockAlign&0xff; buf[1]=(blockAlign>>8)&0xff;
  fwrite(buf, 1, 2, f);
  buf[0]=bitsPerSample&0xff; buf[1]=(bitsPerSample>>8)&0xff;
  fwrite(buf, 1, 2, f);

  // data chunk
  fwrite("data", 1, 4, f);
  buf[0]=dataSize&0xff; buf[1]=(dataSize>>8)&0xff; buf[2]=(dataSize>>16)&0xff; buf[3]=(dataSize>>24)&0xff;
  fwrite(buf, 1, 4, f);
}

// --- Main ---
int main(int argc, char **argv) {
  const char *inputPath = nullptr;
  const char *outputPath = nullptr;
  unsigned long sampleRate = 44100;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-i") == 0 && i+1 < argc) inputPath = argv[++i];
    else if (strcmp(argv[i], "-o") == 0 && i+1 < argc) outputPath = argv[++i];
    else if (strcmp(argv[i], "-s") == 0 && i+1 < argc) sampleRate = strtoul(argv[++i], nullptr, 10);
  }

  if (!inputPath || !outputPath) {
    fprintf(stderr, "Usage: maxduino -i <input> -o <output> [-s <sample_rate>]\n");
    return 1;
  }

  // Open input file
  if (!entry.open(inputPath, O_RDONLY)) {
    fprintf(stderr, "Cannot open input: %s\n", inputPath);
    return 1;
  }

  // Set up globals for the processing code
  filesize = entry.fileSize();
  const char *baseName = strrchr(inputPath, '/');
  if (!baseName) baseName = strrchr(inputPath, '\\');
  if (baseName) baseName++; else baseName = inputPath;
  strncpy(fileName, baseName, filenameLength);
  fileName[filenameLength] = '\0';
  filenameExt = strrchr(fileName, '.');
  if (filenameExt) filenameExt++; else filenameExt = "";
  start = 1;

  fprintf(stderr, "Converting: %s (%lu bytes)\n", inputPath, filesize);
  UniSetup();
  UniPlay();

  // Open output WAV file
  wavOut = fopen(outputPath, "wb");
  if (!wavOut) {
    fprintf(stderr, "Cannot open output: %s\n", outputPath);
    entry.close();
    return 1;
  }

  // Write placeholder header (we'll seek back and fix the size later)
  wavWriteHeader(wavOut, sampleRate, 0);

  // Main conversion loop
  unsigned long accumulatedUs = 0;
  unsigned long iterations = 0;

  auto emitSilence = [&](unsigned long durationUs) {
    unsigned long samples = (durationUs * sampleRate) / 1000000;
    for (unsigned long s = 0; s < samples; s++) {
      wavWriteSample(64); // silence = midpoint
    }
    accumulatedUs += durationUs;
  };

  unsigned long periodUs = Timer.getCurrentMicroseconds();
  if (periodUs == 0) periodUs = 1;
  unsigned long samples = (periodUs * sampleRate + 500000) / 1000000;

  while (!isStopped) {
    UniLoop();

    if (samples==0)
    {
      isrCallback();
      periodUs = Timer.getCurrentMicroseconds();
      if (periodUs == 0) periodUs = 1;
      samples = (periodUs * sampleRate + 500000) / 1000000;

      // Handle pauses: ForcePauseAfter0 and ID2A set pauseOn=true.
      // In CLI mode, emit silence instead of waiting for user input.
      if (pauseOn) {
        unsigned long pauseUs = 5000000UL; // default 5 seconds for forcepause
        emitSilence(pauseUs);
        pauseOn = false;
        fprintf(stderr, "Pause: %.1fs silence\n", pauseUs / 1000000.0);
      }
    }
    else
    {
      samples--;
    }


    // Output samples at the current cli_output_value level
    unsigned char level = cli_output_value ? 192 : 64;
    wavWriteSample(level);

    accumulatedUs += (sampleRate + 500000) / 1000000;
    iterations++;

    // Safety: if we've been running for an absurdly long time, break
    if (accumulatedUs > 600UL * 1000000UL) {
      fprintf(stderr, "Timeout: exceeded 10 minutes\n");
      break;
    }
  }

  // Drain remaining periods from the read buffer after processing stopped.
  // Temporarily clear isStopped so the ISR can advance readpos through remaining data.
  {
    unsigned long drainCount = 0;
    isStopped = false;
    buffsize_t startReadpos = readpos;
    while (drainCount < buffsize) {
      isrCallback();
      unsigned long periodUs = Timer.getCurrentMicroseconds();
      if (periodUs == 0) periodUs = 1;
      unsigned long samples = (periodUs * sampleRate + 500000) / 1000000;
      if (samples == 0) samples = 1;
      unsigned char level = cli_output_value ? 192 : 64;
      for (unsigned long s = 0; s < samples; s++) {
        wavWriteSample(level);
      }
      accumulatedUs += periodUs;
      drainCount++;
      // Stop when ISR wraps the buffer (readpos went backwards or wrapped)
      if (drainCount > 1 && readpos < startReadpos) break;
    }
    isStopped = true;
  }

  // Seek back and write the correct data size
  unsigned long dataSize = wavSampleCount;
  fseek(wavOut, 40, SEEK_SET);
  unsigned char buf[4];
  buf[0]=dataSize&0xff; buf[1]=(dataSize>>8)&0xff; buf[2]=(dataSize>>16)&0xff; buf[3]=(dataSize>>24)&0xff;
  fwrite(buf, 1, 4, wavOut);

  // Fix RIFF chunk size
  unsigned long fileSize = 36 + dataSize;
  fseek(wavOut, 4, SEEK_SET);
  buf[0]=fileSize&0xff; buf[1]=(fileSize>>8)&0xff; buf[2]=(fileSize>>16)&0xff; buf[3]=(fileSize>>24)&0xff;
  fwrite(buf, 1, 4, wavOut);

  fclose(wavOut);
  entry.close();

  fprintf(stderr, "Output: %s (%.1f seconds)\n", outputPath, (double)dataSize / sampleRate);
  return 0;
}
