#include "configs.h"
#include "Arduino.h"
#include "buttons.h"
#include "pinSetup.h"

bool lastbtn=true;

#if defined(BUTTON_ADC)

enum class ADCButtonId : byte {
  None,
  Up,
  Down,
  Root,
  Stop,
  Play
};

static int readButtonADC()
{
  #if defined(BUTTON_ADC_USE_MILLIVOLTS)
    return analogReadMilliVolts(btnADC);
  #else
    return analogRead(btnADC);
  #endif
}

static ADCButtonId decodeButtonADC(int sensorValue)
{
  if(sensorValue >= btnADCPlayLow) {
    return ADCButtonId::Play;
  }
  if(sensorValue >= btnADCStopLow) {
    return ADCButtonId::Stop;
  }
  if(sensorValue >= btnADCRootLow) {
    return ADCButtonId::Root;
  }
  if(sensorValue >= btnADCDownLow) {
    return ADCButtonId::Down;
  }
  if(sensorValue >= btnADCUpLow) {
    return ADCButtonId::Up;
  }
  return ADCButtonId::None;
}

static ADCButtonId readDecodedButtonADC()
{
  // Cache one ADC sample briefly so a poll cycle classifies all button_*()
  // calls from the same reading rather than from slightly different conversions.
  static unsigned long cachedAt = 0;
  static ADCButtonId cachedButton = ADCButtonId::None;

  const unsigned long now = micros();
  if ((unsigned long)(now - cachedAt) > 2000UL) {
    cachedButton = decodeButtonADC(readButtonADC());
    cachedAt = now;
  }
  return cachedButton;
}

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

  #if defined(ESP32)
  // ESP32 has additional options for setting ADC range
  analogSetPinAttenuation(btnADC, ADC_11db);
  analogSetClockDiv(255);
  #endif
}

  // todo - use isr to capture buttons?

bool button_any() {
  return (readDecodedButtonADC() != ADCButtonId::None);
}

bool button_play()
{
  return (readDecodedButtonADC() == ADCButtonId::Play);
}

bool button_stop()
{
  return (readDecodedButtonADC() == ADCButtonId::Stop);
}

bool button_root()
{
  return (readDecodedButtonADC() == ADCButtonId::Root);
}

bool button_down()
{
  return (readDecodedButtonADC() == ADCButtonId::Down);
}

bool button_up()
{
  return (readDecodedButtonADC() == ADCButtonId::Up);
}

#ifdef REC_TZX
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

#ifdef REC_TZX
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
