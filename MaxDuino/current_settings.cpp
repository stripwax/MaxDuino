#include "configs.h"
#include "Arduino.h"
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
  #include <nvs.h>
  #include <nvs_flash.h>
  #define MAXDUINO_ESP32_SETTINGS 1
#endif
#include "current_settings.h"
#include "casProcessing.h"

#if defined(LOAD_EEPROM_SETTINGS) && (defined(ARDUINO_ARCH_RP2040) || defined(MAXDUINO_RP2040))
  #include <EEPROM.h>
#endif

word BAUDRATE = DEFAULT_BAUDRATE;
RecordFormat recordFormat = RecordFormat::TZX_ID15;
// TODO really the following should only be defined ifndef NO_MOTOR
// but the order of #includes is wrong and we only define NO_MOTOR later :-/
bool mselectMask = DEFAULT_MSELECTMASK;
bool TSXCONTROLzxpolarityUEFSWITCHPARITY = DEFAULT_TSXzxpUEF;
bool skip2A = DEFAULT_SKIP2A;

#ifdef LOAD_EEPROM_SETTINGS
  #ifdef MAXDUINO_ESP32_SETTINGS
    static constexpr const char *kSettingsNamespace = "maxduino";
    static constexpr const char *kSettingsBlobKey = "settings";
  #elif defined(ARDUINO_ARCH_RP2040) || defined(MAXDUINO_RP2040)
    #ifndef MAXDUINO_EEPROM_SIZE
      #define MAXDUINO_EEPROM_SIZE 256
    #endif
  #else
    #include "EEPROM_wrappers.h"

    #ifndef EEPROM_RECORD_FORMAT_BYTEPOS
      #define EEPROM_RECORD_FORMAT_BYTEPOS (EEPROM_CONFIG_BYTEPOS - 1)
    #endif

    #ifndef EEPROM_RECORD_MODE_BYTEPOS
      #define EEPROM_RECORD_MODE_BYTEPOS (EEPROM_CONFIG_BYTEPOS - 2)
    #endif

    #ifndef EEPROM_SETTINGS_MAGIC_BYTEPOS
      #define EEPROM_SETTINGS_MAGIC_BYTEPOS (EEPROM_CONFIG_BYTEPOS - 3)
    #endif
  #endif

static constexpr byte kSettingsMagic = 0xA5;
static constexpr byte kSettingsVersion = 1;

static bool isRecordFormatSupported(const RecordFormat format)
{
  switch (format) {
    case RecordFormat::TZX_ID15:
      return true;
    case RecordFormat::CAS_MSX:
      #if defined(REC_TZX) && defined(REC_CAS_MSX) && defined(Use_CAS)
        return true;
      #else
        return false;
      #endif
    case RecordFormat::ZX_SPECTRUM:
      #ifdef REC_TZX
        return true;
      #else
        return false;
      #endif
    case RecordFormat::SHARP_MZF:
      #if defined(REC_TZX) && defined(Use_MZF)
        return true;
      #else
        return false;
      #endif
  }

  return false;
}

static byte encodeRecordFormat()
{
  return static_cast<byte>(recordFormat);
}

static bool decodeRecordFormat(const byte raw, RecordFormat &format)
{
  switch (raw) {
    case static_cast<byte>(RecordFormat::TZX_ID15):
      format = RecordFormat::TZX_ID15;
      return true;
    case static_cast<byte>(RecordFormat::CAS_MSX):
      format = RecordFormat::CAS_MSX;
      return true;
    case static_cast<byte>(RecordFormat::ZX_SPECTRUM):
      format = RecordFormat::ZX_SPECTRUM;
      return true;
    case static_cast<byte>(RecordFormat::SHARP_MZF):
      format = RecordFormat::SHARP_MZF;
      return true;
    default:
      return false;
  }
}

static byte encodeRecordMode()
{
  return 0;
}

static bool decodeRecordMode(const byte raw, bool &mode)
{
  switch (raw) {
    case 0:
      mode = false;
      return true;
    case 1:
      mode = true;
      return true;
    default:
      return false;
  }
}

static byte buildSettingsByte()
{
  byte settings = 0;

  switch(BAUDRATE) {
    case 1200:
      settings |= 1;
      break;
    case 2400:
      settings |= 2;
      break;
    case 3150:
      settings |= 4;
      break;
    case 3600:
      settings |= 8;
      break;
    case 3850:
      settings |= 16;
      break;
  }

  #ifndef NO_MOTOR
    if (mselectMask) settings |= 128;
  #endif

  if (TSXCONTROLzxpolarityUEFSWITCHPARITY) settings |= 64;

  #ifdef MenuBLK2A
    if (skip2A) settings |= 32;
  #endif

  return settings;
}

static void applySettingsByte(const byte settings)
{
  #ifndef NO_MOTOR
    mselectMask = bitRead(settings, 7);
  #endif

  TSXCONTROLzxpolarityUEFSWITCHPARITY = bitRead(settings, 6);

  #ifdef MenuBLK2A
    skip2A = bitRead(settings, 5);
  #endif

  if (bitRead(settings, 0)) {
    BAUDRATE = 1200;
  }
  if (bitRead(settings, 1)) {
    BAUDRATE = 2400;
  }
  if (bitRead(settings, 2)) {
    BAUDRATE = 3150;
  }
  if (bitRead(settings, 3)) {
    BAUDRATE = 3600;
  }
  if (bitRead(settings, 4)) {
    BAUDRATE = 3850;
  }
}

#if defined(MAXDUINO_ESP32_SETTINGS) || defined(ARDUINO_ARCH_RP2040) || defined(MAXDUINO_RP2040)
struct PersistedSettings {
  byte magic;
  byte version;
  byte settings;
  byte recordFormat;
  byte recordMode;
  byte checksum;
};

static byte checksumPersistedSettings(const PersistedSettings &data)
{
  return static_cast<byte>(data.magic ^ data.version ^ data.settings ^
                           data.recordFormat ^ data.recordMode ^ 0x5A);
}

  #ifdef MAXDUINO_ESP32_SETTINGS
static bool openSettingsNvs(nvs_handle_t &handle, const nvs_open_mode mode)
{
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    if (nvs_flash_erase() != ESP_OK) return false;
    err = nvs_flash_init();
  }
  if (err != ESP_OK) return false;

  return nvs_open(kSettingsNamespace, mode, &handle) == ESP_OK;
}

static bool readPersistedSettings(PersistedSettings &data)
{
  nvs_handle_t handle;
  if (!openSettingsNvs(handle, NVS_READONLY)) return false;

  size_t size = sizeof(PersistedSettings);
  const esp_err_t err = nvs_get_blob(handle, kSettingsBlobKey, &data, &size);
  nvs_close(handle);

  return err == ESP_OK &&
         size == sizeof(PersistedSettings) &&
         data.magic == kSettingsMagic &&
         data.version == kSettingsVersion &&
         data.checksum == checksumPersistedSettings(data);
}

static bool writePersistedSettings(const byte settings)
{
  PersistedSettings data;
  data.magic = kSettingsMagic;
  data.version = kSettingsVersion;
  data.settings = settings;
  data.recordFormat = encodeRecordFormat();
  data.recordMode = encodeRecordMode();
  data.checksum = checksumPersistedSettings(data);

  nvs_handle_t handle;
  if (!openSettingsNvs(handle, NVS_READWRITE)) return false;

  esp_err_t err = nvs_set_blob(handle, kSettingsBlobKey, &data, sizeof(PersistedSettings));
  if (err == ESP_OK) {
    err = nvs_commit(handle);
  }
  nvs_close(handle);

  PersistedSettings verify;
  return err == ESP_OK &&
         readPersistedSettings(verify) &&
         memcmp(&data, &verify, sizeof(PersistedSettings)) == 0;
}
  #else
static bool readPersistedSettings(PersistedSettings &data)
{
  EEPROM.begin(MAXDUINO_EEPROM_SIZE);
  EEPROM.get(0, data);
  EEPROM.end();

  return data.magic == kSettingsMagic &&
         data.version == kSettingsVersion &&
         data.checksum == checksumPersistedSettings(data);
}

static bool writePersistedSettings(const byte settings)
{
  PersistedSettings data;
  data.magic = kSettingsMagic;
  data.version = kSettingsVersion;
  data.settings = settings;
  data.recordFormat = encodeRecordFormat();
  data.recordMode = encodeRecordMode();
  data.checksum = checksumPersistedSettings(data);

  EEPROM.begin(MAXDUINO_EEPROM_SIZE);
  EEPROM.put(0, data);
  const bool writeOk = EEPROM.end();

  PersistedSettings verify;
  const bool verifyOk = writeOk && readPersistedSettings(verify) &&
                        memcmp(&data, &verify, sizeof(PersistedSettings)) == 0;
  return verifyOk;
}
  #endif
#endif

void updateEEPROM()
{
    const byte settings = buildSettingsByte();

    #if defined(MAXDUINO_ESP32_SETTINGS) || defined(ARDUINO_ARCH_RP2040) || defined(MAXDUINO_RP2040)
      writePersistedSettings(settings);
    #else
      EEPROM_put(EEPROM_CONFIG_BYTEPOS, settings);
      EEPROM_put(EEPROM_RECORD_FORMAT_BYTEPOS, encodeRecordFormat());
      // Clear the retired legacy record-mode flag so it can't override the new
      // single Record Type setting on later boots.
      EEPROM_put(EEPROM_RECORD_MODE_BYTEPOS, encodeRecordMode());
      EEPROM_put(EEPROM_SETTINGS_MAGIC_BYTEPOS, kSettingsMagic);
    #endif

    #if defined(Use_CAS) && !defined(MAXDUINO_ESP32_SETTINGS)
      setCASBaud();
    #endif
}

void loadEEPROM()
{
    byte settings = 0;
    byte rawRecordFormat = 0xFF;
    byte rawRecordMode = 0xFF;

    #if defined(MAXDUINO_ESP32_SETTINGS) || defined(ARDUINO_ARCH_RP2040) || defined(MAXDUINO_RP2040)
      PersistedSettings data;
      if (!readPersistedSettings(data)) return;
      settings = data.settings;
      rawRecordFormat = data.recordFormat;
      rawRecordMode = data.recordMode;
    #else
      byte magic = 0xFF;
      EEPROM_get(EEPROM_SETTINGS_MAGIC_BYTEPOS, magic);
      if (magic != kSettingsMagic) return;

      EEPROM_get(EEPROM_CONFIG_BYTEPOS, settings);
      EEPROM_get(EEPROM_RECORD_FORMAT_BYTEPOS, rawRecordFormat);
      EEPROM_get(EEPROM_RECORD_MODE_BYTEPOS, rawRecordMode);
    #endif

    applySettingsByte(settings);

    RecordFormat loadedRecordFormat;
    if (decodeRecordFormat(rawRecordFormat, loadedRecordFormat) &&
        isRecordFormatSupported(loadedRecordFormat)) {
      recordFormat = loadedRecordFormat;
    }

    // Migrate the short-lived separate ZX Spectrum setting into the unified
    // Record Type enum for older EEPROM contents.
    bool loadedWeakZXRecordMode = false;
    if (recordFormat == RecordFormat::TZX_ID15 &&
        decodeRecordMode(rawRecordMode, loadedWeakZXRecordMode) &&
        loadedWeakZXRecordMode &&
        isRecordFormatSupported(RecordFormat::ZX_SPECTRUM)) {
      recordFormat = RecordFormat::ZX_SPECTRUM;
    }

    #if defined(Use_CAS) && !defined(MAXDUINO_ESP32_SETTINGS)
      setCASBaud();
    #endif
}
#endif
