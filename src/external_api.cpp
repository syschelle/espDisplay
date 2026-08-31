#include "external_api.h"

#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <math.h>

#include "config.h"
#include "logging.h"
#include "settings.h"
#include "text_utils.h"

ExternalApiService externalApiService;

namespace {

bool readNumber(JsonVariantConst source, NumericValue& target) {
  target.valid = false;
  target.value = NAN;

  if (source.isNull() || !source.is<float>()) {
    return false;
  }

  const float value = source.as<float>();
  if (!isfinite(value)) {
    return false;
  }

  target.valid = true;
  target.value = value;
  return true;
}

bool readBool(JsonVariantConst source, bool fallback = false) {
  return source.is<bool>() ? source.as<bool>() : fallback;
}

int32_t readInt(JsonVariantConst source, int32_t fallback = -1) {
  return source.is<int32_t>() ? source.as<int32_t>() : fallback;
}

void copyJsonString(JsonVariantConst source, char* dest, size_t destSize) {
  if (destSize == 0) return;
  dest[0] = '\0';
  if (!source.is<const char*>()) return;
  const char* text = source.as<const char*>();
  if (!text) return;
  copyText(dest, destSize, text);
}

}  // namespace

void ExternalApiService::begin() {
  values_ = ExternalValues();
  forcePoll_ = true;
}

void ExternalApiService::forcePoll() {
  forcePoll_ = true;
}

void ExternalApiService::clearCache() {
  values_ = ExternalValues();
  forcePoll_ = true;
}

void ExternalApiService::tick(const AppSettings& cfg, bool wifiConnected) {
  if (!configured(cfg)) {
    values_.lastRequestOk = false;
    values_.stale = true;
    values_.lastHttpStatus = 0;
    copyText(values_.lastError, "API host not configured");
    return;
  }

  updateStale(cfg.apiPollSeconds);

  const uint32_t intervalMs = static_cast<uint32_t>(cfg.apiPollSeconds) * 1000UL;
  if (!forcePoll_ && values_.lastAttemptMs != 0 &&
      static_cast<uint32_t>(millis() - values_.lastAttemptMs) < intervalMs) {
    return;
  }

  forcePoll_ = false;

  if (!wifiConnected) {
    values_.lastAttemptMs = millis();
    markFailure(0, "WLAN unavailable");
    return;
  }

  poll(cfg);
}

void ExternalApiService::updateStale(uint16_t pollSeconds) {
  if (!values_.valid || values_.lastSuccessMs == 0) {
    values_.stale = true;
    return;
  }

  const uint32_t calculatedStaleMs = static_cast<uint32_t>(pollSeconds) * 2000UL;
  const uint32_t staleAfterMs = calculatedStaleMs > 30000UL ? calculatedStaleMs : 30000UL;
  if (static_cast<uint32_t>(millis() - values_.lastSuccessMs) > staleAfterMs) {
    values_.stale = true;
  }
}

bool ExternalApiService::poll(const AppSettings& cfg) {
  values_.lastAttemptMs = millis();

  char url[128];
  snprintf(url, sizeof(url), "http://%s%s", cfg.apiHost, EXTERNAL_API_PATH);

  WiFiClient client;
  HTTPClient http;
  http.setTimeout(API_HTTP_TIMEOUT_MS);
  http.setReuse(false);
  http.useHTTP10(true);

  if (!http.begin(client, url)) {
    markFailure(0, "HTTP initialization failed");
    return false;
  }

  const int code = http.GET();
  values_.lastHttpStatus = static_cast<int16_t>(code);

  if (code <= 0) {
    const String detail = http.errorToString(code);
    char message[96];
    snprintf(message, sizeof(message), "Request failed: %s", detail.c_str());
    http.end();
    markFailure(code, message);
    return false;
  }

  if (code != HTTP_CODE_OK) {
    char message[96];
    snprintf(message, sizeof(message), "Unexpected HTTP status %d", code);
    http.end();
    markFailure(code, message);
    return false;
  }

  const int length = http.getSize();
  if (length > static_cast<int>(MAX_EXTERNAL_API_BYTES)) {
    http.end();
    markFailure(code, "Response exceeds configured JSON size limit");
    return false;
  }

  String payload = http.getString();
  http.end();

  if (payload.length() == 0) {
    markFailure(code, "Empty response");
    return false;
  }

  if (payload.length() > MAX_EXTERNAL_API_BYTES) {
    markFailure(code, "Response exceeds configured JSON size limit");
    return false;
  }

  ExternalValues next = ExternalValues();
  char parseError[96] = "";
  if (!parsePayload(payload, next, parseError, sizeof(parseError))) {
    markFailure(code, parseError);
    return false;
  }

  const bool wasOk = values_.lastRequestOk;

  next.valid = true;
  next.lastRequestOk = true;
  next.stale = false;
  next.lastAttemptMs = values_.lastAttemptMs;
  next.lastSuccessMs = millis();
  next.lastHttpStatus = code;
  next.lastError[0] = '\0';

  const time_t now = time(nullptr);
  next.lastSuccessEpoch = now > 1700000000 ? now : 0;

  values_ = next;

  if (!wasOk) {
    appLog.info("API", "External API connection is healthy");
  }
  return true;
}

bool ExternalApiService::parsePayload(
    const String& payload,
    ExternalValues& next,
    char* error,
    size_t errorLen) {
  JsonDocument filter;
  filter["timestamp_utc"] = true;
  filter["local_date"] = true;
  filter["timezone"] = true;
  filter["last_measurement_at"] = true;
  filter["current_solar_production_w"] = true;
  filter["current_grid_power_w"] = true;
  filter["current_grid_import_w"] = true;
  filter["current_grid_export_w"] = true;
  filter["current_total_consumption_w"] = true;
  filter["daily_solar_production_kwh"] = true;
  filter["daily_grid_import_kwh"] = true;
  filter["daily_grid_export_kwh"] = true;
  filter["total_solar_production_kwh"] = true;
  filter["total_grid_import_kwh"] = true;
  filter["total_grid_export_kwh"] = true;

  JsonObject airFilter = filter["air_sensor"].to<JsonObject>();
  airFilter["enabled"] = true;
  airFilter["configured"] = true;
  airFilter["ok"] = true;
  airFilter["cached"] = true;
  airFilter["temperature_c"] = true;
  airFilter["humidity_percent"] = true;
  airFilter["dew_point_c"] = true;
  airFilter["pressure_hpa"] = true;
  airFilter["pressure_sea_level_hpa"] = true;
  airFilter["sds_p1"] = true;
  airFilter["sds_p2"] = true;
  airFilter["age_seconds"] = true;
  airFilter["software_version"] = true;
  airFilter["last_success_at"] = true;
  airFilter["last_error"] = true;
  airFilter["weather_underground_last_ok"] = true;
  airFilter["weather_underground_last_status"] = true;
  airFilter["weather_underground_last_response"] = true;
  airFilter["weather_underground_last_at"] = true;

  JsonDocument doc;
  const DeserializationError parse = deserializeJson(
      doc,
      payload,
      DeserializationOption::Filter(filter),
      DeserializationOption::NestingLimit(4));

  if (parse) {
    snprintf(error, errorLen, "Invalid JSON: %s", parse.c_str());
    return false;
  }

  if (!doc.is<JsonObject>()) {
    snprintf(error, errorLen, "JSON root is not an object");
    return false;
  }

  JsonObjectConst root = doc.as<JsonObjectConst>();
  copyJsonString(root["timestamp_utc"], next.timestampUtc, sizeof(next.timestampUtc));
  copyJsonString(root["local_date"], next.localDate, sizeof(next.localDate));
  copyJsonString(root["timezone"], next.remoteTimezone, sizeof(next.remoteTimezone));
  copyJsonString(root["last_measurement_at"], next.lastMeasurementAt, sizeof(next.lastMeasurementAt));

  uint8_t recognized = 0;
  recognized += readNumber(root["current_solar_production_w"], next.currentSolarProductionW) ? 1 : 0;
  recognized += readNumber(root["current_grid_power_w"], next.currentGridPowerW) ? 1 : 0;
  recognized += readNumber(root["current_grid_import_w"], next.currentGridImportW) ? 1 : 0;
  recognized += readNumber(root["current_grid_export_w"], next.currentGridExportW) ? 1 : 0;
  recognized += readNumber(root["current_total_consumption_w"], next.currentTotalConsumptionW) ? 1 : 0;
  recognized += readNumber(root["daily_solar_production_kwh"], next.dailySolarProductionKwh) ? 1 : 0;
  recognized += readNumber(root["daily_grid_import_kwh"], next.dailyGridImportKwh) ? 1 : 0;
  recognized += readNumber(root["daily_grid_export_kwh"], next.dailyGridExportKwh) ? 1 : 0;
  recognized += readNumber(root["total_solar_production_kwh"], next.totalSolarProductionKwh) ? 1 : 0;
  recognized += readNumber(root["total_grid_import_kwh"], next.totalGridImportKwh) ? 1 : 0;
  recognized += readNumber(root["total_grid_export_kwh"], next.totalGridExportKwh) ? 1 : 0;

  JsonVariantConst airVariant = root["air_sensor"];
  if (airVariant.is<JsonObjectConst>()) {
    next.air.present = true;
    JsonObjectConst air = airVariant.as<JsonObjectConst>();

    next.air.enabled = readBool(air["enabled"]);
    next.air.configured = readBool(air["configured"]);
    next.air.ok = readBool(air["ok"]);
    next.air.cached = readBool(air["cached"]);

    recognized += readNumber(air["temperature_c"], next.air.temperatureC) ? 1 : 0;
    recognized += readNumber(air["humidity_percent"], next.air.humidityPercent) ? 1 : 0;
    recognized += readNumber(air["dew_point_c"], next.air.dewPointC) ? 1 : 0;
    recognized += readNumber(air["pressure_hpa"], next.air.pressureHpa) ? 1 : 0;
    recognized += readNumber(air["pressure_sea_level_hpa"], next.air.pressureSeaLevelHpa) ? 1 : 0;
    recognized += readNumber(air["sds_p1"], next.air.pm10) ? 1 : 0;
    recognized += readNumber(air["sds_p2"], next.air.pm25) ? 1 : 0;

    next.air.ageSeconds = readInt(air["age_seconds"], -1);
    copyJsonString(air["software_version"], next.air.softwareVersion, sizeof(next.air.softwareVersion));
    copyJsonString(air["last_success_at"], next.air.lastSuccessAt, sizeof(next.air.lastSuccessAt));
    copyJsonString(air["last_error"], next.air.lastError, sizeof(next.air.lastError));

    next.air.weatherUndergroundLastOk = readBool(air["weather_underground_last_ok"]);
    next.air.weatherUndergroundLastStatus =
        static_cast<int16_t>(readInt(air["weather_underground_last_status"], 0));
    copyJsonString(
        air["weather_underground_last_response"],
        next.air.weatherUndergroundLastResponse,
        sizeof(next.air.weatherUndergroundLastResponse));
    copyJsonString(
        air["weather_underground_last_at"],
        next.air.weatherUndergroundLastAt,
        sizeof(next.air.weatherUndergroundLastAt));
  }

  if (recognized == 0) {
    snprintf(error, errorLen, "JSON contains no supported numeric values");
    return false;
  }

  error[0] = '\0';
  return true;
}

void ExternalApiService::markFailure(int httpStatus, const char* message) {
  const bool shouldLog = values_.lastRequestOk || values_.lastError[0] == '\0' ||
                         strncmp(values_.lastError, message ? message : "", sizeof(values_.lastError)) != 0;

  values_.lastRequestOk = false;
  values_.stale = true;
  values_.lastHttpStatus = static_cast<int16_t>(httpStatus);
  copyText(values_.lastError, message ? message : "Unknown API error");

  if (shouldLog) {
    appLog.warn("API", values_.lastError);
  }
}
