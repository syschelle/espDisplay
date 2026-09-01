#include "web.h"

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "display.h"
#include "external_api.h"
#include "index_html.h"
#include "java_script.h"
#include "logging.h"
#include "network.h"
#include "ota.h"
#include "settings.h"
#include "style_css.h"
#include "text_utils.h"
#include "time_service.h"
#include "version.h"

WebService webService;

namespace {

String jsonEscape(const char* value) {
  String out;
  if (!value) return out;
  out.reserve(strlen(value) + 8);
  for (const char* p = value; *p; ++p) {
    const unsigned char c = static_cast<unsigned char>(*p);
    switch (c) {
      case '"': out += F("\\\""); break;
      case '\\': out += F("\\\\"); break;
      case '\b': out += F("\\b"); break;
      case '\f': out += F("\\f"); break;
      case '\n': out += F("\\n"); break;
      case '\r': out += F("\\r"); break;
      case '\t': out += F("\\t"); break;
      default:
        if (c < 0x20) {
          char encoded[7];
          snprintf(encoded, sizeof(encoded), "\\u%04x", c);
          out += encoded;
        } else {
          out += static_cast<char>(c);
        }
    }
  }
  return out;
}

void appendQuoted(String& out, const char* value) {
  out += '"';
  out += jsonEscape(value);
  out += '"';
}

void appendNumeric(String& out, const NumericValue& value, uint8_t decimals = 4) {
  if (!value.valid || !isfinite(value.value)) {
    out += F("null");
    return;
  }
  out += String(value.value, decimals);
}

void appendNullableAge(String& out, uint32_t thenMs) {
  if (thenMs == 0) {
    out += F("null");
    return;
  }
  out += String((millis() - thenMs) / 1000UL);
}

bool changed(const char* a, const char* b) {
  return strcmp(a ? a : "", b ? b : "") != 0;
}

bool jsonStringTo(char* dest, size_t destSize, JsonVariantConst value) {
  if (!value.is<const char*>()) return false;
  const char* source = value.as<const char*>();
  if (!source || strlen(source) >= destSize) return false;
  copyText(dest, destSize, source);
  return true;
}

}  // namespace

WebService::WebService() : server_(80) {}

void WebService::begin() {
  registerRoutes();
  server_.begin();
  appLog.info("WEB", "HTTP server started on port 80");
}

void WebService::tick() {
  server_.handleClient();

  if (restartScheduled_ && static_cast<int32_t>(millis() - restartAtMs_) >= 0) {
    delay(50);
    ESP.restart();
  }
}

void WebService::scheduleRestart(uint32_t delayMs) {
  restartScheduled_ = true;
  restartAtMs_ = millis() + delayMs;
}

void WebService::registerRoutes() {
  server_.on("/", HTTP_GET, [this]() {
    sendProgmemAsset("text/html; charset=utf-8", INDEX_HTML, sizeof(INDEX_HTML) - 1);
  });
  server_.on("/style.css", HTTP_GET, [this]() {
    sendProgmemAsset("text/css; charset=utf-8", STYLE_CSS, sizeof(STYLE_CSS) - 1);
  });
  server_.on("/script.js", HTTP_GET, [this]() {
    sendProgmemAsset("application/javascript; charset=utf-8", JAVA_SCRIPT, sizeof(JAVA_SCRIPT) - 1);
  });
  server_.on("/favicon.ico", HTTP_GET, [this]() { server_.send(204, "text/plain", ""); });

  server_.on("/api/settings", HTTP_GET, [this]() { handleSettingsGet(); });
  server_.on("/api/state", HTTP_GET, [this]() { handleStateGet(); });
  server_.on("/api/log", HTTP_GET, [this]() { handleLogGet(); });
  server_.on("/api/settings/display", HTTP_POST, [this]() { handleDisplaySettingsPost(); });
  server_.on("/api/settings/system", HTTP_POST, [this]() { handleSystemSettingsPost(); });
  server_.on("/api/log/clear", HTTP_POST, [this]() { handleLogClearPost(); });
  server_.on("/api/ota/session", HTTP_POST, [this]() { handleOtaSession(); });
  server_.on(
      "/api/ota/upload",
      HTTP_POST,
      [this]() { handleOtaUploadComplete(); },
      [this]() { handleOtaUploadChunk(); });
  server_.on("/api/factory-reset", HTTP_POST, [this]() { handleFactoryReset(); });

  server_.onNotFound([this]() { sendError(404, "Not found"); });
}


void WebService::sendProgmemAsset(const char* contentType, PGM_P content, size_t contentLength) {
  // Large PROGMEM responses can time out when ESP8266WebServer tries to send the
  // whole asset in one operation. Chunking keeps each TCP write small and avoids
  // advertising a Content-Length that cannot be completed after a short send.
  static constexpr size_t kChunkSize = 1024;

  server_.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate");
  server_.sendHeader("Pragma", "no-cache");
  server_.sendHeader("Expires", "0");

  if (!server_.chunkedResponseModeStart(200, contentType)) {
    // HTTP/1.0 clients cannot use chunked transfer encoding. Keep a compatible
    // fallback; normal browsers use HTTP/1.1 and therefore take the path below.
    server_.send_P(200, contentType, content, contentLength);
    return;
  }

  size_t offset = 0;
  while (offset < contentLength) {
    const size_t remaining = contentLength - offset;
    const size_t chunk = remaining < kChunkSize ? remaining : kChunkSize;
    server_.sendContent_P(content + offset, chunk);
    offset += chunk;
    yield();
  }
  server_.chunkedResponseFinalize();
}

void WebService::sendJson(int status, const String& json) {
  server_.sendHeader("Cache-Control", "no-store");
  server_.send(status, "application/json; charset=utf-8", json);
}

void WebService::sendError(int status, const char* message) {
  String out;
  out.reserve(128);
  out += F("{\"ok\":false,\"error\":");
  appendQuoted(out, message ? message : "Unknown error");
  out += '}';
  sendJson(status, out);
}

bool WebService::parseBody(JsonDocument& doc, size_t maxBytes) {
  if (!server_.hasArg("plain")) {
    sendError(400, "Missing JSON body");
    return false;
  }
  const String& body = server_.arg("plain");
  if (body.length() == 0 || body.length() > maxBytes) {
    sendError(413, "JSON body is empty or too large");
    return false;
  }
  const DeserializationError error = deserializeJson(doc, body, DeserializationOption::NestingLimit(4));
  if (error || !doc.is<JsonObject>()) {
    sendError(400, "Invalid JSON body");
    return false;
  }
  return true;
}

void WebService::handleSettingsGet() {
  String out;
  out.reserve(850);
  out += F("{\"ok\":true,\"deviceName\":"); appendQuoted(out, settings.deviceName);
  out += F(",\"language\":"); appendQuoted(out, settings.language);
  out += F(",\"theme\":"); appendQuoted(out, settings.theme);
  out += F(",\"wifi\":{\"ssid\":"); appendQuoted(out, settings.wifiSsid);
  out += F(",\"passwordConfigured\":"); out += settings.wifiPassword[0] ? F("true") : F("false");
  out += F("},\"api\":{\"host\":"); appendQuoted(out, settings.apiHost);
  out += F(",\"port\":"); out += String(settings.apiPort);
  out += F(",\"pollSeconds\":"); out += String(settings.apiPollSeconds);
  out += F("},\"ntp\":{\"server\":"); appendQuoted(out, settings.ntpServer);
  out += F(",\"timezone\":"); appendQuoted(out, settings.timezone);
  out += F("},\"display\":{\"enabled\":"); out += settings.displayEnabled ? F("true") : F("false");
  out += F(",\"brightness\":"); out += String(settings.displayBrightness);
  out += F(",\"clkGpio\":"); out += String(settings.displayClkGpio);
  out += F(",\"dioGpio\":"); out += String(settings.displayDioGpio);
  out += F(",\"mode\":"); appendQuoted(out, SettingsManager::modeToString(settings.displayMode));
  out += F(",\"updateMs\":"); out += String(settings.displayUpdateMs);
  out += F(",\"alternateSeconds\":"); out += String(settings.alternateSeconds);
  out += F(",\"apiValueDisplayMs\":"); out += String(settings.apiValueDisplayMs);
  out += F("}}");
  sendJson(200, out);
}

void WebService::handleStateGet() {
  const ExternalValues& v = externalApiService.values();
  String out;
  out.reserve(2200);
  out += F("{\"ok\":true,\"deviceName\":"); appendQuoted(out, settings.deviceName);
  out += F(",\"firmwareVersion\":"); appendQuoted(out, FW_VERSION);

  out += F(",\"system\":{\"uptimeSeconds\":"); out += String(millis() / 1000UL);
  out += F(",\"freeHeap\":"); out += String(ESP.getFreeHeap()); out += '}';

  out += F(",\"wifi\":{\"connected\":"); out += networkService.connected() ? F("true") : F("false");
  out += F(",\"apActive\":"); out += networkService.apActive() ? F("true") : F("false");
  out += F(",\"ip\":"); appendQuoted(out, networkService.ipAddress().c_str());
  out += F(",\"apIp\":"); appendQuoted(out, networkService.apIpAddress().c_str());
  out += F(",\"rssi\":"); out += String(networkService.rssi());
  out += F(",\"hostname\":"); appendQuoted(out, networkService.hostname().c_str()); out += '}';

  out += F(",\"ntp\":{\"synchronized\":"); out += timeService.synchronized() ? F("true") : F("false");
  out += F(",\"localTime\":"); appendQuoted(out, timeService.localTimeString().c_str());
  out += F(",\"lastSync\":"); appendQuoted(out, timeService.lastSyncString().c_str());
  out += F(",\"server\":"); appendQuoted(out, settings.ntpServer);
  out += F(",\"timezone\":"); appendQuoted(out, settings.timezone); out += '}';

  out += F(",\"api\":{\"configured\":"); out += externalApiService.configured(settings) ? F("true") : F("false");
  out += F(",\"connected\":"); out += v.lastRequestOk ? F("true") : F("false");
  out += F(",\"requestInProgress\":"); out += externalApiService.requestInProgress() ? F("true") : F("false");
  out += F(",\"valid\":"); out += v.valid ? F("true") : F("false");
  out += F(",\"stale\":"); out += v.stale ? F("true") : F("false");
  out += F(",\"endpoint\":");
  if (settings.apiHost[0]) {
    String endpoint = F("http://");
    endpoint += settings.apiHost;
    endpoint += ':';
    endpoint += String(settings.apiPort);
    endpoint += EXTERNAL_API_PATH;
    appendQuoted(out, endpoint.c_str());
  } else appendQuoted(out, "");
  out += F(",\"httpStatus\":"); out += String(v.lastHttpStatus);
  out += F(",\"lastError\":"); appendQuoted(out, v.lastError);
  out += F(",\"dataAgeSeconds\":"); appendNullableAge(out, v.lastSuccessMs);
  out += F(",\"lastAttemptAgeSeconds\":"); appendNullableAge(out, v.lastAttemptMs);
  out += '}';

  out += F(",\"display\":{\"enabled\":"); out += settings.displayEnabled ? F("true") : F("false");
  out += F(",\"brightness\":"); out += String(settings.displayBrightness);
  out += F(",\"clkGpio\":"); out += String(settings.displayClkGpio);
  out += F(",\"dioGpio\":"); out += String(settings.displayDioGpio);
  out += F(",\"mode\":"); appendQuoted(out, SettingsManager::modeToString(settings.displayMode));
  out += F(",\"updateMs\":"); out += String(settings.displayUpdateMs);
  out += F(",\"alternateSeconds\":"); out += String(settings.alternateSeconds);
  out += F(",\"apiValueDisplayMs\":"); out += String(settings.apiValueDisplayMs);
  out += F(",\"rendered\":"); appendQuoted(out, displayService.lastRenderedText());
  out += '}';

  out += F(",\"external\":{\"timezone\":"); appendQuoted(out, v.remoteTimezone);
  out += F(",\"lastMeasurementAt\":"); appendQuoted(out, v.lastMeasurementAt);
  out += F(",\"temperatureC\":"); appendNumeric(out, v.temperatureC);
  out += F("}}");

  sendJson(200, out);
}

void WebService::handleLogGet() {
  String out;
  out.reserve(512 + appLog.count() * 96);
  out += F("{\"ok\":true,\"lines\":[");
  for (size_t i = 0; i < appLog.count(); ++i) {
    if (i) out += ',';
    appendQuoted(out, appLog.lineAtChronological(i));
  }
  out += F("]}");
  sendJson(200, out);
}

void WebService::handleDisplaySettingsPost() {
  JsonDocument doc;
  if (!parseBody(doc)) return;

  AppSettings next = settings;
  JsonObjectConst root = doc.as<JsonObjectConst>();

  if (!root["enabled"].is<bool>()) {
    sendError(400, "Missing or invalid field: enabled");
    return;
  }
  if (!root["brightness"].is<int>()) {
    sendError(400, "Missing or invalid field: brightness");
    return;
  }
  if (!root["mode"].is<const char*>()) {
    sendError(400, "Missing or invalid field: mode");
    return;
  }
  if (!root["updateMs"].is<int>()) {
    sendError(400, "Missing or invalid field: updateMs");
    return;
  }

  DisplayMode mode;
  if (!SettingsManager::modeFromString(root["mode"].as<const char*>(), mode)) {
    sendError(400, "Unknown display mode");
    return;
  }

  // Keep current values when an older cached UI omits the fields. This makes
  // the settings endpoint backward-compatible while the new no-cache/chunked
  // asset delivery ensures current browsers receive the complete UI script.
  const int brightness = root["brightness"].as<int>();
  const int clkGpio = root["clkGpio"].is<int>() ? root["clkGpio"].as<int>() : settings.displayClkGpio;
  const int dioGpio = root["dioGpio"].is<int>() ? root["dioGpio"].as<int>() : settings.displayDioGpio;
  const uint32_t updateMs = root["updateMs"].as<uint32_t>();
  const int alternateSeconds = root["alternateSeconds"].is<int>()
                                   ? root["alternateSeconds"].as<int>()
                                   : settings.alternateSeconds;
  const uint32_t apiValueDisplayMs = root["apiValueDisplayMs"].is<uint32_t>()
                                         ? root["apiValueDisplayMs"].as<uint32_t>()
                                         : settings.apiValueDisplayMs;

  if (brightness < 0 || brightness > 7) {
    sendError(400, "Brightness must be between 0 and 7");
    return;
  }
  if (clkGpio < 0 || clkGpio > 16 || dioGpio < 0 || dioGpio > 16 ||
      !isSelectableDisplayGpio(static_cast<uint8_t>(clkGpio)) ||
      !isSelectableDisplayGpio(static_cast<uint8_t>(dioGpio))) {
    sendError(400, "CLK/DIO GPIO is not supported");
    return;
  }
  if (clkGpio == dioGpio) {
    sendError(400, "CLK and DIO must use different GPIOs");
    return;
  }
  if (updateMs < MIN_DISPLAY_UPDATE_MS || updateMs > MAX_DISPLAY_UPDATE_MS) {
    sendError(400, "Display update interval is outside the supported range");
    return;
  }
  if (alternateSeconds < MIN_ALTERNATE_SECONDS || alternateSeconds > MAX_ALTERNATE_SECONDS) {
    sendError(400, "Display alternate interval is outside the supported range");
    return;
  }
  if (apiValueDisplayMs < MIN_API_VALUE_DISPLAY_MS ||
      apiValueDisplayMs > MAX_API_VALUE_DISPLAY_MS) {
    sendError(400, "API value display duration is outside the supported range");
    return;
  }

  const bool pinChanged = settings.displayClkGpio != static_cast<uint8_t>(clkGpio) ||
                          settings.displayDioGpio != static_cast<uint8_t>(dioGpio);

  next.displayEnabled = root["enabled"].as<bool>();
  next.displayBrightness = static_cast<uint8_t>(brightness);
  next.displayClkGpio = static_cast<uint8_t>(clkGpio);
  next.displayDioGpio = static_cast<uint8_t>(dioGpio);
  next.displayMode = mode;
  next.displayUpdateMs = updateMs;
  next.alternateSeconds = static_cast<uint16_t>(alternateSeconds);
  next.apiValueDisplayMs = apiValueDisplayMs;

  if (!settingsManager.save(next)) {
    sendError(500, settingsManager.lastError());
    return;
  }
  settings = next;

  if (pinChanged) {
    char message[96];
    snprintf(message, sizeof(message), "Pin mapping changed to CLK GPIO%u / DIO GPIO%u; restart scheduled",
             settings.displayClkGpio, settings.displayDioGpio);
    appLog.info("DISPLAY", message);
    sendJson(200, F("{\"ok\":true,\"restartScheduled\":true}"));
    scheduleRestart(1200);
    return;
  }

  displayService.applySettings(settings);
  sendJson(200, F("{\"ok\":true,\"restartScheduled\":false}"));
}

void WebService::handleSystemSettingsPost() {
  JsonDocument doc;
  if (!parseBody(doc)) return;
  JsonObjectConst root = doc.as<JsonObjectConst>();

  AppSettings next = settings;
  if (!jsonStringTo(next.deviceName, sizeof(next.deviceName), root["deviceName"]) ||
      !jsonStringTo(next.language, sizeof(next.language), root["language"]) ||
      !jsonStringTo(next.theme, sizeof(next.theme), root["theme"]) ||
      !jsonStringTo(next.wifiSsid, sizeof(next.wifiSsid), root["wifiSsid"]) ||
      !jsonStringTo(next.apiHost, sizeof(next.apiHost), root["apiHost"]) ||
      !jsonStringTo(next.ntpServer, sizeof(next.ntpServer), root["ntpServer"]) ||
      !jsonStringTo(next.timezone, sizeof(next.timezone), root["timezone"]) ||
      !root["apiPort"].is<int>() ||
      !root["apiPollSeconds"].is<int>()) {
    sendError(400, "Missing or invalid system settings");
    return;
  }

  const bool wifiSsidChanged = changed(settings.wifiSsid, next.wifiSsid);
  if (!root["wifiPassword"].isNull()) {
    if (!jsonStringTo(next.wifiPassword, sizeof(next.wifiPassword), root["wifiPassword"])) {
      sendError(400, "Invalid Wi-Fi password");
      return;
    }
  } else if (wifiSsidChanged) {
    // Never carry a password from one SSID to a newly selected network.
    next.wifiPassword[0] = '\0';
  }
  if (next.wifiSsid[0] == '\0') next.wifiPassword[0] = '\0';

  const int apiPort = root["apiPort"].as<int>();
  if (apiPort < 1 || apiPort > 65535) {
    sendError(400, "API port must be between 1 and 65535");
    return;
  }
  next.apiPort = static_cast<uint16_t>(apiPort);

  const int pollSeconds = root["apiPollSeconds"].as<int>();
  if (pollSeconds < MIN_API_POLL_SECONDS || pollSeconds > MAX_API_POLL_SECONDS) {
    sendError(400, "API polling interval is outside the supported range");
    return;
  }
  next.apiPollSeconds = static_cast<uint16_t>(pollSeconds);

  AppSettings normalized = next;
  if (!SettingsManager::validateAndNormalize(normalized)) {
    sendError(400, "One or more settings are invalid");
    return;
  }

  const bool networkChanged = changed(settings.deviceName, normalized.deviceName) ||
                              changed(settings.wifiSsid, normalized.wifiSsid) ||
                              changed(settings.wifiPassword, normalized.wifiPassword);
  const bool ntpChanged = changed(settings.ntpServer, normalized.ntpServer) ||
                          changed(settings.timezone, normalized.timezone);
  const bool apiHostChanged = changed(settings.apiHost, normalized.apiHost);
  const bool apiEndpointChanged = apiHostChanged || settings.apiPort != normalized.apiPort;
  const bool apiChanged = apiEndpointChanged || settings.apiPollSeconds != normalized.apiPollSeconds;

  if (!settingsManager.save(normalized)) {
    sendError(500, settingsManager.lastError());
    return;
  }
  settings = normalized;

  if (ntpChanged) timeService.forceResync(settings, networkService.connected());
  if (apiEndpointChanged) externalApiService.clearCache();
  else if (apiChanged) externalApiService.forcePoll();

  String out = F("{\"ok\":true,\"restartScheduled\":");
  out += networkChanged ? F("true") : F("false");
  out += '}';
  sendJson(200, out);
  if (networkChanged) scheduleRestart();
}

void WebService::handleLogClearPost() {
  appLog.clear();
  appLog.info("LOG", "System log cleared");
  sendJson(200, F("{\"ok\":true}"));
}

void WebService::handleOtaSession() {
  const uint32_t a = ESP.random();
  const uint32_t b = ESP.random();
  snprintf(
      otaUploadToken_,
      sizeof(otaUploadToken_),
      "%08lx%08lx",
      static_cast<unsigned long>(a),
      static_cast<unsigned long>(b));
  otaUploadTokenExpiresMs_ = millis() + 60000UL;

  String out;
  out.reserve(96);
  out += F("{\"ok\":true,\"token\":");
  appendQuoted(out, otaUploadToken_);
  out += F(",\"expiresSeconds\":60}");
  sendJson(200, out);
}

void WebService::handleOtaUploadChunk() {
  HTTPUpload& upload = server_.upload();

  if (upload.status == UPLOAD_FILE_START) {
    otaUploadRequestSeen_ = true;
    otaUploadFailed_ = false;
    otaUploadCompleted_ = false;

    if (upload.name != "firmware" || upload.filename != "firmware.bin") {
      otaUploadFailed_ = true;
      otaService.abortUpload("Unexpected OTA upload field or filename");
      return;
    }

    const String token = server_.arg("token");
    const bool tokenFresh = otaUploadToken_[0] != '\0' &&
                            static_cast<int32_t>(otaUploadTokenExpiresMs_ - millis()) > 0;
    if (!tokenFresh || token != otaUploadToken_) {
      otaUploadFailed_ = true;
      otaUploadToken_[0] = '\0';
      otaUploadTokenExpiresMs_ = 0;
      otaService.abortUpload("OTA upload session is invalid or expired");
      return;
    }
    // One-time session: consume it before flash initialization.
    otaUploadToken_[0] = '\0';
    otaUploadTokenExpiresMs_ = 0;

    const String targetVersion = server_.arg("version");
    const String expectedSizeText = server_.arg("size");
    char* end = nullptr;
    const unsigned long parsedSize = strtoul(expectedSizeText.c_str(), &end, 10);
    if (targetVersion.length() == 0 || expectedSizeText.length() == 0 ||
        end == expectedSizeText.c_str() || *end != '\0' || parsedSize == 0) {
      otaUploadFailed_ = true;
      otaService.abortUpload("Invalid browser OTA metadata");
      return;
    }

    // No external TCP/API traffic is allowed while flash is being written.
    // GitHub HTTPS is handled by the browser, so the ESP8266 needs only enough
    // RAM for the local upload and the Update buffer.
    externalApiService.suspend();
    delay(25);
    yield();

    if (!otaService.beginUpload(targetVersion.c_str(), static_cast<uint32_t>(parsedSize))) {
      otaUploadFailed_ = true;
      return;
    }
    return;
  }

  if (upload.status == UPLOAD_FILE_WRITE) {
    if (otaUploadFailed_) return;
    if (!otaService.writeChunk(upload.buf, upload.currentSize)) {
      otaUploadFailed_ = true;
    }
    return;
  }

  if (upload.status == UPLOAD_FILE_END) {
    if (!otaUploadFailed_) {
      otaUploadCompleted_ = otaService.finishUpload(static_cast<uint32_t>(upload.totalSize));
      otaUploadFailed_ = !otaUploadCompleted_;
    }
    return;
  }

  if (upload.status == UPLOAD_FILE_ABORTED) {
    otaUploadFailed_ = true;
    otaUploadCompleted_ = false;
    otaUploadToken_[0] = '\0';
    otaUploadTokenExpiresMs_ = 0;
    otaService.abortUpload("Browser OTA upload was aborted");
    externalApiService.resume();
  }
}

void WebService::handleOtaUploadComplete() {
  if (!otaUploadRequestSeen_) {
    externalApiService.resume();
    sendError(400, "No OTA firmware file was uploaded");
    return;
  }

  otaUploadRequestSeen_ = false;

  if (otaUploadFailed_ || !otaUploadCompleted_) {
    otaUploadFailed_ = false;
    otaUploadCompleted_ = false;
    externalApiService.resume();
    sendError(500, otaService.status().lastError[0]
                       ? otaService.status().lastError
                       : "OTA upload failed");
    return;
  }

  otaUploadFailed_ = false;
  otaUploadCompleted_ = false;
  // Do not resume the external API after a successful flash. The ESP8266 is
  // about to reboot and starting another socket would only waste time/RAM.
  sendJson(200, F("{\"ok\":true,\"message\":\"Update completed; rebooting\"}"));
  scheduleRestart(1800UL);
}

void WebService::handleFactoryReset() {
  JsonDocument doc;
  if (!parseBody(doc, 256)) return;
  const char* confirm = doc["confirm"] | "";
  if (strcmp(confirm, "RESET") != 0) {
    sendError(400, "Factory reset confirmation is missing");
    return;
  }
  if (!settingsManager.factoryReset()) {
    sendError(500, settingsManager.lastError());
    return;
  }
  sendJson(200, F("{\"ok\":true,\"message\":\"Factory reset complete; rebooting\"}"));
  scheduleRestart(1500UL);
}
