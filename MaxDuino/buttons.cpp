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
      #if defined(DEBUG_ADC)
      Serial.println(latest_voltage_mv);
      #endif
  }
}

void setup_buttons(void)
{
  #if defined(DEBUG_ADC)
  Serial.begin(9600);
  #endif

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

#elif defined(ESP32_RISCV)

  // Uses analogReadMilliVolts, rate-limited to every 50 ms.
  // The IDF's adc2_get_raw (called internally) wraps the conversion
  // in portENTER_CRITICAL, which clears mstatus.MIE on single-core
  // RISC-V for ~25 µs.  This can starve the timer ISR, but at 50 ms
  // intervals the glitch duty cycle is negligible (~0.05 %).
  void setup_buttons(void)
  {
    #if defined(DEBUG_ADC)
    Serial.begin(9600);
    #endif
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
    analogSetClockDiv(128);
  }

  static int readButtonADC()
  {
    static unsigned long last = 0;
    static int adc_value = 0;
    unsigned long now = millis();
    if (now - last >= 50) {
      last = now;
      adc_value = (int)analogReadMilliVolts(btnADC);
      #if defined(DEBUG_ADC)
      Serial.println(adc_value);
      #endif
    }
    return adc_value;
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

  #if defined(DEBUG_ADC)
  Serial.begin(9600);
  #endif
}

static int readButtonADC()
{
  int adc_value = analogRead(btnADC);
  #if defined(DEBUG_ADC)
  Serial.println(adc_value);
  #endif
  return adc_value;
}

#endif


bool button_any() {
  int sensorValue = readButtonADC();
  return(sensorValue>=btnADCUpLow);
}

bool button_play()
{
  int sensorValue = readButtonADC();
#ifdef RECORD
  return(sensorValue>=btnADCPlayLow && sensorValue<btnADCRecLow);
#else
  return(sensorValue>=btnADCPlayLow);
#endif
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

#ifdef RECORD
bool button_rec()
{
  int sensorValue = readButtonADC();
  return(sensorValue>=btnADCRecLow);
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

#ifdef RECORD
bool button_rec() {
  return (digitalRead(btnRec) == LOW);
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
