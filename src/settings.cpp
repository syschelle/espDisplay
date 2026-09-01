#include "settings.h"

#include <EEPROM.h>
#include <ctype.h>
#include <string.h>

#include "config.h"
#include "logging.h"
#include "text_utils.h"
#include "validation.h"

SettingsManager settingsManager;
AppSettings settings;

namespace {

bool isSafePrintable(const char* value, size_t maxLen) {
  if (!value) return false;
  const size_t len = strlen(value);
  if (len == 0 || len > maxLen) return false;
  for (size_t i = 0; i < len; ++i) {
    const unsigned char c = static_cast<unsigned char>(value[i]);
    if (c < 32 || c == 127) return false;
  }
  return true;
}

bool isValidDeviceName(const char* value) {
  return isSafePrintable(value, 32);
}

bool isValidLanguage(const char* value) {
  return value && (strcmp(value, "de") == 0 || strcmp(value, "en") == 0);
}

bool isValidTheme(const char* value) {
  return value && (strcmp(value, "light") == 0 || strcmp(value, "dark") == 0);
}

bool displayPinsValid(uint8_t clk, uint8_t dio) {
  return clk != dio && isSelectableDisplayGpio(clk) && isSelectableDisplayGpio(dio);
}

#pragma pack(push, 1)
struct PersistedSettingsHeader {
  uint32_t magic;
  uint16_t schema;
  uint16_t payloadSize;
};

// Exact v0.1.0 payload layout. Keep this definition frozen so an OTA migration
// can preserve Wi-Fi/API/NTP/display settings from the already-installed build.
struct PersistedSettingsPayloadV1 {
  char deviceName[33];
  char language[3];
  char theme[6];
  char wifiSsid[33];
  char wifiPassword[65];
  char apiHost[65];
  uint16_t apiPollSeconds;
  char ntpServer[65];
  char timezone[65];
  uint8_t displayEnabled;
  uint8_t displayBrightness;
  uint8_t selectedMetric;
  uint8_t displayMode;
  uint16_t displayUpdateMs;
  uint16_t alternateSeconds;
};

struct PersistedSettingsRecordV1 {
  uint32_t magic;
  uint16_t schema;
  uint16_t payloadSize;
  PersistedSettingsPayloadV1 payload;
  uint32_t crc32;
};

struct PersistedSettingsPayloadV2 {
  // Keep all v1 fields in the same order and append new fields only.
  char deviceName[33];
  char language[3];
  char theme[6];
  char wifiSsid[33];
  char wifiPassword[65];
  char apiHost[65];
  uint16_t apiPollSeconds;
  char ntpServer[65];
  char timezone[65];
  uint8_t displayEnabled;
  uint8_t displayBrightness;
  uint8_t selectedMetric;
  uint8_t displayMode;
  uint16_t displayUpdateMs;
  uint16_t alternateSeconds;
  uint8_t displayClkGpio;
  uint8_t displayDioGpio;
};

struct PersistedSettingsRecordV2 {
  uint32_t magic;
  uint16_t schema;
  uint16_t payloadSize;
  PersistedSettingsPayloadV2 payload;
  uint32_t crc32;
};

struct PersistedSettingsPayloadV3 {
  // Keep the complete v2 payload as an exact prefix and append new fields.
  char deviceName[33];
  char language[3];
  char theme[6];
  char wifiSsid[33];
  char wifiPassword[65];
  char apiHost[65];
  uint16_t apiPollSeconds;
  char ntpServer[65];
  char timezone[65];
  uint8_t displayEnabled;
  uint8_t displayBrightness;
  uint8_t selectedMetric;
  uint8_t displayMode;
  uint16_t displayUpdateMs;
  uint16_t alternateSeconds;
  uint8_t displayClkGpio;
  uint8_t displayDioGpio;
  uint16_t apiPort;
};

struct PersistedSettingsRecordV3 {
  uint32_t magic;
  uint16_t schema;
  uint16_t payloadSize;
  PersistedSettingsPayloadV3 payload;
  uint32_t crc32;
};

struct PersistedSettingsPayloadV4 {
  // v0.1.16 widens displayUpdateMs to 32 bits so intervals up to 200 seconds
  // can be represented without overflow. Older payload layouts remain frozen
  // above and are migrated explicitly when loaded.
  char deviceName[33];
  char language[3];
  char theme[6];
  char wifiSsid[33];
  char wifiPassword[65];
  char apiHost[65];
  uint16_t apiPollSeconds;
  char ntpServer[65];
  char timezone[65];
  uint8_t displayEnabled;
  uint8_t displayBrightness;
  uint8_t selectedMetric;
  uint8_t displayMode;
  uint32_t displayUpdateMs;
  uint16_t alternateSeconds;
  uint8_t displayClkGpio;
  uint8_t displayDioGpio;
  uint16_t apiPort;
};

struct PersistedSettingsRecordV4 {
  uint32_t magic;
  uint16_t schema;
  uint16_t payloadSize;
  PersistedSettingsPayloadV4 payload;
  uint32_t crc32;
};
#pragma pack(pop)

static_assert(sizeof(PersistedSettingsRecordV4) <= EEPROM_SETTINGS_BYTES,
              "EEPROM settings record exceeds configured storage size");

uint32_t crc32Update(uint32_t crc, const uint8_t* data, size_t length) {
  crc = ~crc;
  for (size_t i = 0; i < length; ++i) {
    crc ^= data[i];
    for (uint8_t bit = 0; bit < 8; ++bit) {
      const uint32_t mask = -(crc & 1U);
      crc = (crc >> 1U) ^ (0xEDB88320UL & mask);
    }
  }
  return ~crc;
}

template <typename Payload>
uint32_t payloadCrc(uint16_t schema, uint16_t payloadSize, const Payload& payload) {
  uint32_t crc = 0;
  crc = crc32Update(crc, reinterpret_cast<const uint8_t*>(&schema), sizeof(schema));
  crc = crc32Update(crc, reinterpret_cast<const uint8_t*>(&payloadSize), sizeof(payloadSize));
  crc = crc32Update(crc, reinterpret_cast<const uint8_t*>(&payload), sizeof(payload));
  return crc;
}

bool recordValid(const PersistedSettingsRecordV1& record) {
  if (record.magic != EEPROM_SETTINGS_MAGIC) return false;
  if (record.schema != LEGACY_SETTINGS_SCHEMA_VERSION) return false;
  if (record.payloadSize != sizeof(PersistedSettingsPayloadV1)) return false;
  return record.crc32 == payloadCrc(record.schema, record.payloadSize, record.payload);
}

bool recordValid(const PersistedSettingsRecordV2& record) {
  if (record.magic != EEPROM_SETTINGS_MAGIC) return false;
  if (record.schema != DISPLAY_GPIO_SETTINGS_SCHEMA_VERSION) return false;
  if (record.payloadSize != sizeof(PersistedSettingsPayloadV2)) return false;
  return record.crc32 == payloadCrc(record.schema, record.payloadSize, record.payload);
}

bool recordValid(const PersistedSettingsRecordV3& record) {
  if (record.magic != EEPROM_SETTINGS_MAGIC) return false;
  if (record.schema != PREVIOUS_SETTINGS_SCHEMA_VERSION) return false;
  if (record.payloadSize != sizeof(PersistedSettingsPayloadV3)) return false;
  return record.crc32 == payloadCrc(record.schema, record.payloadSize, record.payload);
}

bool recordValid(const PersistedSettingsRecordV4& record) {
  if (record.magic != EEPROM_SETTINGS_MAGIC) return false;
  if (record.schema != SETTINGS_SCHEMA_VERSION) return false;
  if (record.payloadSize != sizeof(PersistedSettingsPayloadV4)) return false;
  return record.crc32 == payloadCrc(record.schema, record.payloadSize, record.payload);
}

void fromCommonPayload(const PersistedSettingsPayloadV1& source, AppSettings& target) {
  copyText(target.deviceName, source.deviceName);
  copyText(target.language, source.language);
  copyText(target.theme, source.theme);
  copyText(target.wifiSsid, source.wifiSsid);
  copyText(target.wifiPassword, source.wifiPassword);
  copyText(target.apiHost, source.apiHost);
  target.apiPollSeconds = source.apiPollSeconds;
  copyText(target.ntpServer, source.ntpServer);
  copyText(target.timezone, source.timezone);
  target.displayEnabled = source.displayEnabled != 0;
  target.displayBrightness = source.displayBrightness;

  if (source.selectedMetric <= static_cast<uint8_t>(MetricId::AirPm25)) {
    target.selectedMetric = static_cast<MetricId>(source.selectedMetric);
  }
  if (source.displayMode <= static_cast<uint8_t>(DisplayMode::Alternate)) {
    target.displayMode = static_cast<DisplayMode>(source.displayMode);
  }

  target.displayUpdateMs = source.displayUpdateMs;
  target.alternateSeconds = source.alternateSeconds;
}

void fromPayload(const PersistedSettingsPayloadV1& source, AppSettings& target) {
  fromCommonPayload(source, target);
  target.apiPort = DEFAULT_API_PORT;
  target.displayClkGpio = DEFAULT_TM1637_CLK_GPIO;
  target.displayDioGpio = DEFAULT_TM1637_DIO_GPIO;

  // Rename only the old factory default. A user-defined device name is kept.
  if (strcmp(target.deviceName, LEGACY_DEFAULT_DEVICE_NAME) == 0) {
    copyText(target.deviceName, DEFAULT_DEVICE_NAME);
  }
}

void fromPayload(const PersistedSettingsPayloadV2& source, AppSettings& target) {
  // The prefix is byte-for-byte compatible with v1.
  PersistedSettingsPayloadV1 common{};
  memcpy(&common, &source, sizeof(common));
  fromCommonPayload(common, target);
  target.apiPort = DEFAULT_API_PORT;
  target.displayClkGpio = source.displayClkGpio;
  target.displayDioGpio = source.displayDioGpio;
}

void fromPayload(const PersistedSettingsPayloadV3& source, AppSettings& target) {
  PersistedSettingsPayloadV2 previous{};
  memcpy(&previous, &source, sizeof(previous));
  fromPayload(previous, target);
  target.apiPort = source.apiPort;
}

void fromPayload(const PersistedSettingsPayloadV4& source, AppSettings& target) {
  copyText(target.deviceName, source.deviceName);
  copyText(target.language, source.language);
  copyText(target.theme, source.theme);
  copyText(target.wifiSsid, source.wifiSsid);
  copyText(target.wifiPassword, source.wifiPassword);
  copyText(target.apiHost, source.apiHost);
  target.apiPollSeconds = source.apiPollSeconds;
  copyText(target.ntpServer, source.ntpServer);
  copyText(target.timezone, source.timezone);
  target.displayEnabled = source.displayEnabled != 0;
  target.displayBrightness = source.displayBrightness;
  if (source.selectedMetric <= static_cast<uint8_t>(MetricId::AirPm25)) {
    target.selectedMetric = static_cast<MetricId>(source.selectedMetric);
  }
  if (source.displayMode <= static_cast<uint8_t>(DisplayMode::Alternate)) {
    target.displayMode = static_cast<DisplayMode>(source.displayMode);
  }
  target.displayUpdateMs = source.displayUpdateMs;
  target.alternateSeconds = source.alternateSeconds;
  target.displayClkGpio = source.displayClkGpio;
  target.displayDioGpio = source.displayDioGpio;
  target.apiPort = source.apiPort;
}

void toPayload(const AppSettings& source, PersistedSettingsPayloadV4& target) {
  memset(&target, 0, sizeof(target));
  copyText(target.deviceName, source.deviceName);
  copyText(target.language, source.language);
  copyText(target.theme, source.theme);
  copyText(target.wifiSsid, source.wifiSsid);
  copyText(target.wifiPassword, source.wifiPassword);
  copyText(target.apiHost, source.apiHost);
  target.apiPollSeconds = source.apiPollSeconds;
  copyText(target.ntpServer, source.ntpServer);
  copyText(target.timezone, source.timezone);
  target.displayEnabled = source.displayEnabled ? 1U : 0U;
  target.displayBrightness = source.displayBrightness;
  target.selectedMetric = static_cast<uint8_t>(source.selectedMetric);
  target.displayMode = static_cast<uint8_t>(source.displayMode);
  target.displayUpdateMs = source.displayUpdateMs;
  target.alternateSeconds = source.alternateSeconds;
  target.displayClkGpio = source.displayClkGpio;
  target.displayDioGpio = source.displayDioGpio;
  target.apiPort = source.apiPort;
}

}  // namespace

void SettingsManager::setError(const char* value) {
  copyText(lastError_, value ? value : "");
}

bool SettingsManager::begin() {
  EEPROM.begin(EEPROM_SETTINGS_BYTES);
  ready_ = true;
  setError("");
  appLog.info("SETTINGS", "EEPROM settings storage ready");
  return true;
}

bool SettingsManager::load(AppSettings& out) {
  out = AppSettings();

  if (!ready_) {
    validateAndNormalize(out);
    setError("Settings storage unavailable");
    return false;
  }

  PersistedSettingsHeader header{};
  EEPROM.get(0, header);

  if (header.magic != EEPROM_SETTINGS_MAGIC) {
    validateAndNormalize(out);
    setError("");
    appLog.info("SETTINGS", "No valid EEPROM settings found; defaults are active");
    return true;
  }

  bool migrate = false;
  uint16_t sourceSchema = header.schema;

  if (header.schema == SETTINGS_SCHEMA_VERSION &&
      header.payloadSize == sizeof(PersistedSettingsPayloadV4)) {
    PersistedSettingsRecordV4 record{};
    EEPROM.get(0, record);
    if (!recordValid(record)) {
      validateAndNormalize(out);
      setError("Stored settings failed CRC validation; defaults are active");
      appLog.warn("SETTINGS", lastError_);
      return true;
    }
    fromPayload(record.payload, out);
  } else if (header.schema == PREVIOUS_SETTINGS_SCHEMA_VERSION &&
             header.payloadSize == sizeof(PersistedSettingsPayloadV3)) {
    PersistedSettingsRecordV3 previous{};
    EEPROM.get(0, previous);
    if (!recordValid(previous)) {
      validateAndNormalize(out);
      setError("Previous settings failed CRC validation; defaults are active");
      appLog.warn("SETTINGS", lastError_);
      return true;
    }
    fromPayload(previous.payload, out);
    migrate = true;
  } else if (header.schema == DISPLAY_GPIO_SETTINGS_SCHEMA_VERSION &&
             header.payloadSize == sizeof(PersistedSettingsPayloadV2)) {
    PersistedSettingsRecordV2 previous{};
    EEPROM.get(0, previous);
    if (!recordValid(previous)) {
      validateAndNormalize(out);
      setError("Previous settings failed CRC validation; defaults are active");
      appLog.warn("SETTINGS", lastError_);
      return true;
    }
    fromPayload(previous.payload, out);
    migrate = true;
  } else if (header.schema == LEGACY_SETTINGS_SCHEMA_VERSION &&
             header.payloadSize == sizeof(PersistedSettingsPayloadV1)) {
    PersistedSettingsRecordV1 legacy{};
    EEPROM.get(0, legacy);
    if (!recordValid(legacy)) {
      validateAndNormalize(out);
      setError("Legacy settings failed CRC validation; defaults are active");
      appLog.warn("SETTINGS", lastError_);
      return true;
    }
    fromPayload(legacy.payload, out);
    migrate = true;
  } else {
    validateAndNormalize(out);
    setError("Unsupported settings schema; defaults are active");
    appLog.warn("SETTINGS", lastError_);
    return true;
  }

  const bool normalized = validateAndNormalize(out);
  if (!normalized) {
    appLog.warn("SETTINGS", "Stored settings required normalization");
  }

  if (migrate) {
    if (!save(out)) {
      appLog.warn("SETTINGS", "Settings loaded but migration write failed");
    } else {
      char message[80];
      snprintf(message, sizeof(message), "Settings migrated from schema %u to schema %u",
               static_cast<unsigned>(sourceSchema), static_cast<unsigned>(SETTINGS_SCHEMA_VERSION));
      appLog.info("SETTINGS", message);
    }
    setError("");
    return true;
  }

  if (!normalized) {
    setError("Settings contained invalid values; defaults/clamps applied");
    appLog.warn("SETTINGS", lastError_);
  } else {
    setError("");
  }

  return true;
}

bool SettingsManager::save(const AppSettings& input) {
  if (!ready_) {
    setError("Settings storage unavailable");
    appLog.error("SETTINGS", "Cannot save settings because EEPROM storage is unavailable");
    return false;
  }

  AppSettings normalized = input;
  if (!validateAndNormalize(normalized)) {
    setError("Settings validation failed");
    return false;
  }

  PersistedSettingsRecordV4 record{};
  record.magic = EEPROM_SETTINGS_MAGIC;
  record.schema = SETTINGS_SCHEMA_VERSION;
  record.payloadSize = sizeof(PersistedSettingsPayloadV4);
  toPayload(normalized, record.payload);
  record.crc32 = payloadCrc(record.schema, record.payloadSize, record.payload);

  EEPROM.put(0, record);
  if (!EEPROM.commit()) {
    setError("EEPROM commit failed");
    appLog.error("SETTINGS", lastError_);
    return false;
  }

  PersistedSettingsRecordV4 verify{};
  EEPROM.get(0, verify);
  if (!recordValid(verify) || verify.crc32 != record.crc32) {
    setError("EEPROM verification failed");
    appLog.error("SETTINGS", lastError_);
    return false;
  }

  setError("");
  return true;
}

bool SettingsManager::factoryReset() {
  if (!ready_) {
    setError("Settings storage unavailable");
    return false;
  }

  for (size_t i = 0; i < EEPROM_SETTINGS_BYTES; ++i) {
    EEPROM.write(static_cast<int>(i), 0xFF);
  }

  if (!EEPROM.commit()) {
    setError("EEPROM reset failed");
    appLog.error("SETTINGS", lastError_);
    return false;
  }

  setError("");
  appLog.info("SETTINGS", "Persistent settings cleared");
  return true;
}

bool SettingsManager::validateAndNormalize(AppSettings& s) {
  bool valid = true;

  if (!isValidDeviceName(s.deviceName)) {
    copyText(s.deviceName, DEFAULT_DEVICE_NAME);
    valid = false;
  }

  if (!isValidLanguage(s.language)) {
    copyText(s.language, "de");
    valid = false;
  }

  if (!isValidTheme(s.theme)) {
    copyText(s.theme, "light");
    valid = false;
  }

  if (strlen(s.wifiSsid) > 32) {
    s.wifiSsid[32] = '\0';
    valid = false;
  }

  if (strlen(s.wifiPassword) > 64) {
    s.wifiPassword[64] = '\0';
    valid = false;
  }

  if (s.apiHost[0] != '\0' && !isValidHost(s.apiHost)) {
    s.apiHost[0] = '\0';
    valid = false;
  }

  if (s.apiPort == 0) {
    s.apiPort = DEFAULT_API_PORT;
    valid = false;
  }

  s.apiPollSeconds = constrain(
      s.apiPollSeconds,
      static_cast<uint16_t>(MIN_API_POLL_SECONDS),
      static_cast<uint16_t>(MAX_API_POLL_SECONDS));

  if (!isValidHost(s.ntpServer)) {
    copyText(s.ntpServer, DEFAULT_NTP_SERVER);
    valid = false;
  }

  if (!isSafePrintable(s.timezone, 64)) {
    copyText(s.timezone, DEFAULT_TIMEZONE);
    valid = false;
  }

  if (s.displayBrightness > 7) {
    s.displayBrightness = 7;
    valid = false;
  }
  if (!displayPinsValid(s.displayClkGpio, s.displayDioGpio)) {
    s.displayClkGpio = DEFAULT_TM1637_CLK_GPIO;
    s.displayDioGpio = DEFAULT_TM1637_DIO_GPIO;
    valid = false;
  }
  s.displayUpdateMs = constrain(
      s.displayUpdateMs,
      MIN_DISPLAY_UPDATE_MS,
      MAX_DISPLAY_UPDATE_MS);
  s.alternateSeconds = constrain(
      s.alternateSeconds,
      static_cast<uint16_t>(MIN_ALTERNATE_SECONDS),
      static_cast<uint16_t>(MAX_ALTERNATE_SECONDS));

  return valid;
}

bool SettingsManager::isValidHost(const char* value) {
  return isValidHostValue(value);
}

const char* SettingsManager::metricToString(MetricId metric) {
  switch (metric) {
    case MetricId::SolarW: return "current_solar_production_w";
    case MetricId::GridW: return "current_grid_power_w";
    case MetricId::GridImportW: return "current_grid_import_w";
    case MetricId::GridExportW: return "current_grid_export_w";
    case MetricId::ConsumptionW: return "current_total_consumption_w";
    case MetricId::DailySolarKwh: return "daily_solar_production_kwh";
    case MetricId::DailyGridImportKwh: return "daily_grid_import_kwh";
    case MetricId::DailyGridExportKwh: return "daily_grid_export_kwh";
    case MetricId::TotalSolarKwh: return "total_solar_production_kwh";
    case MetricId::TotalGridImportKwh: return "total_grid_import_kwh";
    case MetricId::TotalGridExportKwh: return "total_grid_export_kwh";
    case MetricId::AirTemperatureC: return "air_temperature_c";
    case MetricId::AirHumidityPercent: return "air_humidity_percent";
    case MetricId::AirDewPointC: return "air_dew_point_c";
    case MetricId::AirPm10: return "air_pm10";
    case MetricId::AirPm25: return "air_pm25";
    default: return "current_grid_import_w";
  }
}

bool SettingsManager::metricFromString(const char* value, MetricId& metric) {
  if (!value) return false;
  for (uint8_t i = 0; i <= static_cast<uint8_t>(MetricId::AirPm25); ++i) {
    MetricId candidate = static_cast<MetricId>(i);
    if (strcmp(value, metricToString(candidate)) == 0) {
      metric = candidate;
      return true;
    }
  }
  return false;
}

const char* SettingsManager::modeToString(DisplayMode mode) {
  switch (mode) {
    case DisplayMode::Clock: return "clock";
    case DisplayMode::Alternate: return "alternate";
    default: return "metric";
  }
}

bool SettingsManager::modeFromString(const char* value, DisplayMode& mode) {
  if (!value) return false;
  if (strcmp(value, "metric") == 0) {
    mode = DisplayMode::Metric;
    return true;
  }
  if (strcmp(value, "clock") == 0) {
    mode = DisplayMode::Clock;
    return true;
  }
  if (strcmp(value, "alternate") == 0) {
    mode = DisplayMode::Alternate;
    return true;
  }
  return false;
}
