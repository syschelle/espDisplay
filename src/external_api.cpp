#include "external_api.h"

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ctype.h>
#include <math.h>
#include <new>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "logging.h"
#include "settings.h"
#include "text_utils.h"
#include "version.h"

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

bool startsWithIgnoreCase(const char* text, const char* prefix) {
  if (!text || !prefix) return false;
  while (*prefix) {
    if (!*text) return false;
    if (tolower(static_cast<unsigned char>(*text)) !=
        tolower(static_cast<unsigned char>(*prefix))) return false;
    ++text;
    ++prefix;
  }
  return true;
}

bool containsIgnoreCase(const char* text, const char* needle) {
  if (!text || !needle || !*needle) return false;
  const size_t needleLength = strlen(needle);
  for (const char* p = text; *p; ++p) {
    size_t i = 0;
    while (i < needleLength && p[i] &&
           tolower(static_cast<unsigned char>(p[i])) ==
               tolower(static_cast<unsigned char>(needle[i]))) {
      ++i;
    }
    if (i == needleLength) return true;
  }
  return false;
}

}  // namespace

void ExternalApiService::begin() {
  values_ = ExternalValues();
  forcePoll_ = true;
  suspended_ = false;
  lastFinishedMs_ = 0;
  abortRequest();
}

void ExternalApiService::suspend() {
  suspended_ = true;
  if (requestInProgress()) {
    appLog.info("API", "External API request paused for OTA");
  }
  abortRequest();
}

void ExternalApiService::resume() {
  suspended_ = false;
  forcePoll_ = true;
}

void ExternalApiService::forcePoll() {
  forcePoll_ = true;
}

void ExternalApiService::clearCache() {
  abortRequest();
  values_ = ExternalValues();
  forcePoll_ = true;
  lastFinishedMs_ = 0;
}

void ExternalApiService::tick(const AppSettings& cfg, bool wifiConnected) {
  updateStale(cfg.apiPollSeconds);

  if (suspended_) {
    if (requestInProgress()) abortRequest();
    return;
  }

  if (!configured(cfg)) {
    if (requestInProgress()) abortRequest();
    values_.lastRequestOk = false;
    values_.stale = true;
    values_.lastHttpStatus = 0;
    copyText(values_.lastError, "API host not configured");
    return;
  }

  if (requestInProgress()) {
    serviceRequest(cfg);
    return;
  }

  const uint32_t intervalMs = static_cast<uint32_t>(cfg.apiPollSeconds) * 1000UL;
  const bool due = forcePoll_ || lastFinishedMs_ == 0 ||
                   static_cast<uint32_t>(millis() - lastFinishedMs_) >= intervalMs;
  if (!due) return;

  forcePoll_ = false;

  if (!wifiConnected) {
    values_.lastAttemptMs = millis();
    lastFinishedMs_ = millis();
    markFailure(0, "WLAN unavailable");
    return;
  }

  startRequest(cfg);
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

void ExternalApiService::releaseBodyBuffer() {
  if (bodyBuffer_) {
    delete[] bodyBuffer_;
    bodyBuffer_ = nullptr;
  }
  bodyCapacity_ = 0;
}

bool ExternalApiService::ensureBodyCapacity(size_t requiredBytes) {
  if (requiredBytes > MAX_EXTERNAL_API_BYTES + 1) return false;
  if (bodyBuffer_ && bodyCapacity_ >= requiredBytes) return true;

  size_t nextCapacity = bodyCapacity_ > 0 ? bodyCapacity_ : 1025;
  while (nextCapacity < requiredBytes && nextCapacity < MAX_EXTERNAL_API_BYTES + 1) {
    const size_t doubled = nextCapacity * 2;
    nextCapacity = doubled > MAX_EXTERNAL_API_BYTES + 1
                       ? MAX_EXTERNAL_API_BYTES + 1
                       : doubled;
  }
  if (nextCapacity < requiredBytes) return false;

  char* next = new (std::nothrow) char[nextCapacity];
  if (!next) return false;

  if (bodyBuffer_ && bodyLength_ > 0) {
    memcpy(next, bodyBuffer_, bodyLength_);
  }
  delete[] bodyBuffer_;
  bodyBuffer_ = next;
  bodyCapacity_ = nextCapacity;
  if (bodyLength_ < bodyCapacity_) bodyBuffer_[bodyLength_] = '\0';
  return true;
}

void ExternalApiService::resetRequestBuffers() {
  headerLength_ = 0;
  bodyLength_ = 0;
  chunkLineLength_ = 0;
  contentLength_ = -1;
  chunkRemaining_ = 0;
  chunkCrlfSeen_ = 0;
  chunked_ = false;
  sawHeaderTerminator_ = false;
  chunkState_ = ChunkState::Size;
  headerBuffer_[0] = '\0';
  releaseBodyBuffer();
  chunkLine_[0] = '\0';
}

void ExternalApiService::abortRequest() {
  if (client_) client_.abort();
  requestState_ = RequestState::Idle;
  resetRequestBuffers();
}

void ExternalApiService::startRequest(const AppSettings& cfg) {
  abortRequest();
  resetRequestBuffers();

  values_.lastAttemptMs = millis();
  requestStartedMs_ = values_.lastAttemptMs;
  lastActivityMs_ = requestStartedMs_;

  // WiFiClient::connect() is still synchronous in the ESP8266 Arduino core.
  // Keep only this unavoidable phase tightly bounded; all waiting for HTTP
  // headers/body after the TCP connection is cooperative and non-blocking.
  client_.setTimeout(API_CONNECT_TIMEOUT_MS);
  if (!client_.connect(cfg.apiHost, cfg.apiPort)) {
    failRequest(0, "TCP connection failed");
    return;
  }
  client_.setNoDelay(true);

  char request[320];
  const int requestLength = snprintf(
      request,
      sizeof(request),
      "GET %s HTTP/1.1\r\nHost: %s:%u\r\nAccept: application/json\r\nAccept-Encoding: identity\r\nConnection: close\r\nUser-Agent: espDisplay/%s\r\n\r\n",
      EXTERNAL_API_PATH,
      cfg.apiHost,
      static_cast<unsigned>(cfg.apiPort),
      FW_VERSION);
  if (requestLength <= 0 || static_cast<size_t>(requestLength) >= sizeof(request)) {
    failRequest(0, "API request header is too large");
    return;
  }

  const size_t written = client_.write(reinterpret_cast<const uint8_t*>(request),
                                       static_cast<size_t>(requestLength));
  if (written != static_cast<size_t>(requestLength)) {
    failRequest(0, "API request could not be sent completely");
    return;
  }

  requestState_ = RequestState::Headers;
  lastActivityMs_ = millis();
}

void ExternalApiService::serviceRequest(const AppSettings& cfg) {
  const uint32_t now = millis();
  if (static_cast<uint32_t>(now - requestStartedMs_) > API_RESPONSE_TIMEOUT_MS) {
    failRequest(0, "API response timeout");
    return;
  }
  if (static_cast<uint32_t>(now - lastActivityMs_) > API_INACTIVITY_TIMEOUT_MS) {
    failRequest(0, "API response stalled");
    return;
  }

  size_t budget = API_READ_BUDGET_BYTES;
  while (budget > 0 && client_.available() > 0 && requestInProgress()) {
    const int value = client_.read();
    if (value < 0) break;
    --budget;
    lastActivityMs_ = millis();
    if (!consumeByte(static_cast<uint8_t>(value))) return;
  }

  if (!requestInProgress()) return;

  if (!client_.connected() && client_.available() == 0) {
    if (requestState_ == RequestState::Body && !chunked_ && contentLength_ < 0 && bodyLength_ > 0) {
      finishResponse();
      return;
    }

    if (requestState_ == RequestState::Body && contentLength_ >= 0 &&
        bodyLength_ == static_cast<size_t>(contentLength_)) {
      finishResponse();
      return;
    }

    failRequest(values_.lastHttpStatus, "API connection closed before response completed");
  }

  (void)cfg;
}

bool ExternalApiService::consumeByte(uint8_t value) {
  if (requestState_ == RequestState::Headers) {
    if (headerLength_ >= sizeof(headerBuffer_) - 1) {
      failRequest(0, "API response headers are too large");
      return false;
    }

    headerBuffer_[headerLength_++] = static_cast<char>(value);
    headerBuffer_[headerLength_] = '\0';

    if (headerLength_ >= 4 &&
        headerBuffer_[headerLength_ - 4] == '\r' &&
        headerBuffer_[headerLength_ - 3] == '\n' &&
        headerBuffer_[headerLength_ - 2] == '\r' &&
        headerBuffer_[headerLength_ - 1] == '\n') {
      sawHeaderTerminator_ = true;
      if (!parseHeaders()) return false;
      requestState_ = RequestState::Body;
      if (!chunked_ && contentLength_ == 0) return finishResponse();
    }
    return true;
  }

  return consumeBodyByte(value);
}

bool ExternalApiService::parseHeaders() {
  if (!sawHeaderTerminator_) return false;

  char* line = headerBuffer_;
  char* lineEnd = strstr(line, "\r\n");
  if (!lineEnd) {
    failRequest(0, "Invalid HTTP response status line");
    return false;
  }
  *lineEnd = '\0';

  int status = 0;
  if (sscanf(line, "HTTP/%*u.%*u %d", &status) != 1) {
    failRequest(0, "Invalid HTTP response status line");
    return false;
  }
  values_.lastHttpStatus = static_cast<int16_t>(status);
  if (status != 200) {
    char message[96];
    snprintf(message, sizeof(message), "Unexpected HTTP status %d", status);
    failRequest(status, message);
    return false;
  }

  char* cursor = lineEnd + 2;
  while (*cursor) {
    char* end = strstr(cursor, "\r\n");
    if (!end) break;
    if (end == cursor) break;
    *end = '\0';

    if (startsWithIgnoreCase(cursor, "Content-Length:")) {
      const char* value = cursor + strlen("Content-Length:");
      while (*value == ' ' || *value == '\t') ++value;
      char* parseEnd = nullptr;
      const unsigned long parsed = strtoul(value, &parseEnd, 10);
      if (parseEnd == value || parsed > MAX_EXTERNAL_API_BYTES) {
        failRequest(status, "Response exceeds configured JSON size limit");
        return false;
      }
      contentLength_ = static_cast<int32_t>(parsed);
    } else if (startsWithIgnoreCase(cursor, "Transfer-Encoding:") &&
               containsIgnoreCase(cursor, "chunked")) {
      chunked_ = true;
      contentLength_ = -1;
    }

    cursor = end + 2;
  }

  if (contentLength_ >= 0) {
    const size_t required = static_cast<size_t>(contentLength_) + 1;
    if (!ensureBodyCapacity(required)) {
      failRequest(status, "Insufficient heap for API response buffer");
      return false;
    }
  }

  return true;
}

bool ExternalApiService::consumeBodyByte(uint8_t value) {
  if (chunked_) return consumeChunkedByte(value);

  if (bodyLength_ >= MAX_EXTERNAL_API_BYTES) {
    failRequest(values_.lastHttpStatus, "Response exceeds configured JSON size limit");
    return false;
  }

  if (!ensureBodyCapacity(bodyLength_ + 2)) {
    failRequest(values_.lastHttpStatus, "Insufficient heap for API response buffer");
    return false;
  }

  bodyBuffer_[bodyLength_++] = static_cast<char>(value);
  bodyBuffer_[bodyLength_] = '\0';

  if (contentLength_ >= 0 && bodyLength_ == static_cast<size_t>(contentLength_)) {
    return finishResponse();
  }
  return true;
}

bool ExternalApiService::consumeChunkedByte(uint8_t value) {
  if (chunkState_ == ChunkState::Size) {
    if (value == '\r') return true;
    if (value == '\n') {
      chunkLine_[chunkLineLength_] = '\0';
      char* extension = strchr(chunkLine_, ';');
      if (extension) *extension = '\0';
      char* end = nullptr;
      const unsigned long size = strtoul(chunkLine_, &end, 16);
      if (end == chunkLine_ || *end != '\0') {
        failRequest(values_.lastHttpStatus, "Invalid chunked API response");
        return false;
      }
      chunkLineLength_ = 0;
      chunkRemaining_ = static_cast<size_t>(size);
      if (chunkRemaining_ == 0) return finishResponse();
      if (bodyLength_ + chunkRemaining_ > MAX_EXTERNAL_API_BYTES) {
        failRequest(values_.lastHttpStatus, "Response exceeds configured JSON size limit");
        return false;
      }
      chunkState_ = ChunkState::Data;
      return true;
    }

    if (chunkLineLength_ >= sizeof(chunkLine_) - 1) {
      failRequest(values_.lastHttpStatus, "Invalid chunk size line");
      return false;
    }
    chunkLine_[chunkLineLength_++] = static_cast<char>(value);
    return true;
  }

  if (chunkState_ == ChunkState::Data) {
    if (bodyLength_ >= MAX_EXTERNAL_API_BYTES || chunkRemaining_ == 0) {
      failRequest(values_.lastHttpStatus, "Invalid chunked API response");
      return false;
    }
    if (!ensureBodyCapacity(bodyLength_ + 2)) {
      failRequest(values_.lastHttpStatus, "Insufficient heap for API response buffer");
      return false;
    }
    bodyBuffer_[bodyLength_++] = static_cast<char>(value);
    bodyBuffer_[bodyLength_] = '\0';
    --chunkRemaining_;
    if (chunkRemaining_ == 0) {
      chunkState_ = ChunkState::DataCrlf;
      chunkCrlfSeen_ = 0;
    }
    return true;
  }

  // Every non-final chunk must be followed by CRLF before the next size line.
  if (chunkCrlfSeen_ == 0 && value == '\r') {
    chunkCrlfSeen_ = 1;
    return true;
  }
  if (chunkCrlfSeen_ == 1 && value == '\n') {
    chunkCrlfSeen_ = 0;
    chunkState_ = ChunkState::Size;
    return true;
  }

  failRequest(values_.lastHttpStatus, "Invalid chunk delimiter");
  return false;
}

bool ExternalApiService::finishResponse() {
  if (!bodyBuffer_ || bodyLength_ == 0) {
    failRequest(values_.lastHttpStatus, "API response body is empty");
    return false;
  }
  bodyBuffer_[bodyLength_] = '\0';

  ExternalValues next = ExternalValues();
  char parseError[96] = "";
  if (!parsePayload(bodyBuffer_, bodyLength_, next, parseError, sizeof(parseError))) {
    failRequest(values_.lastHttpStatus, parseError);
    return false;
  }

  const bool wasOk = values_.lastRequestOk;
  next.valid = true;
  next.lastRequestOk = true;
  next.stale = false;
  next.lastAttemptMs = values_.lastAttemptMs;
  next.lastSuccessMs = millis();
  next.lastHttpStatus = values_.lastHttpStatus;
  next.lastError[0] = '\0';

  const time_t now = time(nullptr);
  next.lastSuccessEpoch = now > 1700000000 ? now : 0;

  values_ = next;
  lastFinishedMs_ = millis();
  client_.stop(50);
  requestState_ = RequestState::Idle;
  resetRequestBuffers();

  if (!wasOk) appLog.info("API", "External API connection is healthy");
  return true;
}

void ExternalApiService::failRequest(int httpStatus, const char* message) {
  client_.abort();
  requestState_ = RequestState::Idle;
  lastFinishedMs_ = millis();
  resetRequestBuffers();
  markFailure(httpStatus, message);
}

bool ExternalApiService::parsePayload(
    const char* payload,
    size_t payloadLength,
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
      payloadLength,
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

  // Deliberately keep all last-known-good numeric values and metadata in RAM.
  // A slow or failed refresh marks them stale but never clears the display cache.
  values_.lastRequestOk = false;
  values_.stale = true;
  values_.lastHttpStatus = static_cast<int16_t>(httpStatus);
  copyText(values_.lastError, message ? message : "Unknown API error");

  if (shouldLog) appLog.warn("API", values_.lastError);
}
