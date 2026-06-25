#include "configs.h"
#include "Arduino.h"
#include "buttons.h"
#include "pinSetup.h"

bool lastbtn=true;

#if defined(BUTTON_ADC)

#if defined(ESP32_XTENSA)
// using a lower-level ADC driver for performance
#include <driver/adc.h>
#include <esp_adc_cal.h>
// Global calibration characteristics structure
static esp_adc_cal_characteristics_t adc_chars;
// Thread-safe volatile global variable to hold the latest reading
volatile int latest_voltage_mv = 0; 

void adcTaskCore0(void * parameter)
{
  // 1. Initialize ADC on Core 0
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_12);
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_12, 1100, &adc_chars);

  while(1) {
      int raw_reading = adc1_get_raw(ADC1_CHANNEL_0);
      // convert to millivolts using system calibration and store the result in shared variable
      // (32-bit reads/writes are atomic on ESP32)
      latest_voltage_mv = esp_adc_cal_raw_to_voltage(raw_reading, &adc_chars);
      // Delay to allow Core 0 to handle other tasks (avoid watchdog crashes)
      vTaskDelay(pdMS_TO_TICKS(10)); 
  }
}

void setup_buttons(void)
{
  // Execute the ADC reads on the other core, to avoid interrupting the TimerCounter
  // Create the task, pin it to Core 0, and allocate a stack size
  xTaskCreatePinnedToCore(
    adcTaskCore0,       // Function to implement the task
    "ADC_Task",         // Name of the task
    4096,               // Stack size in words
    NULL,               // Task input parameter
    1,                  // Priority of the task
    NULL,               // Task handle
    0                   // Core ID (0)
  );
}

static int readButtonADC()
{
  return latest_voltage_mv;
}

#else
void setup_buttons(void)
{
  pinMode(btnADC, INPUT);
  #if !defined(ESP8266)
  // analogReadResolution is only defined on certain platforms (including SAMD21 and ESP32, but not including ESP8266)
  // For some devices, the resolution is 10 bits by default (which is why we set other platforms to also use 10 bits
  // so that all the same code works on all the devices)
  // ESP8266 defaults to 10 bits anyway
  analogReadResolution(10);
  #endif

  #if defined(ARDUINO_ESP32C3_DEV) || defined(CONFIG_IDF_TARGET_ESP32C3)
  // ESP32-C3 has additional options for setting ADC range
  analogSetAttenuation(ADC_11db);
  analogSetClockDiv(255);
  #endif
}

static int readButtonADC()
{
  return analogRead(btnADC);
}
#endif

bool button_any() {
  int sensorValue = readButtonADC();
  return(sensorValue>=btnADCUpLow);
}

bool button_play()
{
  int sensorValue = readButtonADC();
  return(sensorValue>=btnADCPlayLow);
}

bool button_stop()
{
  int sensorValue = readButtonADC();
  return(sensorValue>=btnADCStopLow && sensorValue<btnADCPlayLow);
}

bool button_root()
{
  int sensorValue = readButtonADC();
  return(sensorValue>=btnADCRootLow && sensorValue<btnADCStopLow);
}

bool button_down()
{
  int sensorValue = readButtonADC();
  return(sensorValue>=btnADCDownLow && sensorValue<btnADCRootLow);
}

bool button_up()
{
  int sensorValue = readButtonADC();
  return(sensorValue>=btnADCUpLow && sensorValue<btnADCDownLow);
}

#ifdef Use_Rec
bool button_rec()
{
  // Record is a dedicated digital pin even when the rest of the UI uses ADC buttons.
  #ifdef btnRec
    return (digitalRead(btnRec) == LOW);
  #else
    return false;
  #endif
}
#endif

#else // !defined (BUTTON_ADC)

void setup_buttons(void)
{
  // 
}

bool button_any() {
  return (digitalRead(btnPlay) == LOW || 
  digitalRead(btnStop) == LOW ||
  digitalRead(btnUp)   == LOW ||
  digitalRead(btnDown) == LOW ||
  digitalRead(btnRoot) == LOW);
}

bool button_play() {
  return(digitalRead(btnPlay) == LOW);
}

bool button_stop() {
  return(digitalRead(btnStop) == LOW);
}

bool button_root() {
  return(digitalRead(btnRoot) == LOW);
}

bool button_down() {
  return(digitalRead(btnDown) == LOW);
}

bool button_up() {
  return(digitalRead(btnUp) == LOW);
}

#ifdef Use_Rec
bool button_rec() {
  #ifdef btnRec
    return (digitalRead(btnRec) == LOW);
  #else
    return false;
  #endif
}
#endif

#endif // defined(BUTTON_ADC)


// common:
void debounce(bool (*button_fn)()) {
  while(button_fn()) {
    //prevent button repeats by waiting until the button is released.
    delay(50);
  }
}

void debouncemax(bool (*button_fn)())
{
  //prevent button repeats by waiting until the button is released.
  // return true or false, depending whether we hit the timeout (true) or key was released (false) before timeout occurred
  for(byte i=4; i>0; i--)
  {
    if (!button_fn()) break;
    delay(50);
  }
}

void checkLastButton()
{
  if(!button_down() && !button_up() && !button_play() && !button_stop()) lastbtn=false; 
  delay(50);
}
