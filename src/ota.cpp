#include "ota.h"

#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecureBearSSL.h>
#include <Updater.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "logging.h"
#include "text_utils.h"
#include "version.h"

OtaService otaService;

namespace {

constexpr size_t kMaxManifestBytes = 2048;

void buildRawUrl(char* out, size_t outSize, const char* fileName) {
  if (!out || outSize == 0) return;
  snprintf(
      out,
      outSize,
      "https://raw.githubusercontent.com/%s/%s/%s/%s",
      OTA_GITHUB_OWNER,
      OTA_GITHUB_REPO,
      OTA_BRANCH,
      fileName);
}

bool isSha256Hex(const char* value) {
  if (!value || strlen(value) != 64) return false;
  for (size_t i = 0; i < 64; ++i) {
    if (!isxdigit(static_cast<unsigned char>(value[i]))) return false;
  }
  return true;
}

}  // namespace

void OtaService::begin() {
  status_ = Status();
  copyText(status_.currentVersion, FW_VERSION);
}

void OtaService::fail(const char* message) {
  status_.ok = false;
  copyText(status_.lastError, message ? message : "Unknown OTA error");
  appLog.warn("OTA", status_.lastError);
}

bool OtaService::check() {
  status_.checked = true;
  status_.ok = false;
  status_.updateAvailable = false;
  status_.latestVersion[0] = '\0';
  status_.firmwareUrl[0] = '\0';
  status_.firmwareSize = 0;
  status_.lastError[0] = '\0';
  copyText(status_.currentVersion, FW_VERSION);

  if (WiFi.status() != WL_CONNECTED) {
    fail("WLAN is not connected");
    return false;
  }

  char manifestBase[192];
  buildRawUrl(manifestBase, sizeof(manifestBase), OTA_MANIFEST_NAME);

  // A short cache-buster avoids receiving an older manifest from an
  // intermediate cache immediately after a new tagged release was published.
  String manifestUrl(manifestBase);
  manifestUrl += F("?cb=");
  manifestUrl += String(millis());

  BearSSL::WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(15);

  HTTPClient http;
  http.setTimeout(15000);
  http.setReuse(false);
  http.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);
  http.useHTTP10(true);
  http.setUserAgent(String(PROJECT_NAME) + "/" + FW_VERSION);

  if (!http.begin(client, manifestUrl)) {
    fail("OTA manifest request initialization failed");
    return false;
  }
  http.addHeader("Accept", "application/json");
  http.addHeader("Cache-Control", "no-cache");

  const int code = http.GET();
  if (code != HTTP_CODE_OK) {
    char message[96];
    snprintf(message, sizeof(message), "OTA manifest returned HTTP %d", code);
    http.end();
    fail(message);
    return false;
  }

  const int responseSize = http.getSize();
  if (responseSize > static_cast<int>(kMaxManifestBytes)) {
    http.end();
    fail("OTA manifest is unexpectedly large");
    return false;
  }

  JsonDocument filter;
  filter["version"] = true;
  filter["firmware"] = true;
  filter["size"] = true;
  filter["sha256"] = true;

  JsonDocument doc;
  const DeserializationError err = deserializeJson(
      doc,
      http.getStream(),
      DeserializationOption::Filter(filter),
      DeserializationOption::NestingLimit(2));
  http.end();

  if (err) {
    fail("OTA manifest is invalid JSON");
    return false;
  }

  const char* latest = doc["version"] | "";
  const char* firmwareName = doc["firmware"] | "";
  const char* sha256 = doc["sha256"] | "";
  const uint32_t firmwareSize = doc["size"] | 0U;

  int latestMajor = 0, latestMinor = 0, latestPatch = 0;
  if (!parseSemver(latest, latestMajor, latestMinor, latestPatch)) {
    fail("OTA manifest version is not a semantic version");
    return false;
  }
  if (strcmp(firmwareName, OTA_ASSET_NAME) != 0) {
    fail("OTA manifest firmware name is invalid");
    return false;
  }
  if (firmwareSize == 0) {
    fail("OTA manifest firmware size is invalid");
    return false;
  }
  if (!isSha256Hex(sha256)) {
    fail("OTA manifest SHA-256 is invalid");
    return false;
  }

  copyText(status_.latestVersion, latest);
  status_.firmwareSize = firmwareSize;

  char firmwareBase[192];
  buildRawUrl(firmwareBase, sizeof(firmwareBase), OTA_ASSET_NAME);
  String firmwareUrl(firmwareBase);
  firmwareUrl += F("?v=");
  firmwareUrl += status_.latestVersion;
  if (firmwareUrl.length() >= sizeof(status_.firmwareUrl)) {
    fail("OTA firmware URL is unexpectedly long");
    return false;
  }
  copyText(status_.firmwareUrl, firmwareUrl.c_str());

  if (!isAllowedFirmwareUrl(status_.firmwareUrl)) {
    fail("Firmware URL is outside the configured OTA channel");
    return false;
  }

  if (status_.firmwareSize > static_cast<uint32_t>(ESP.getFreeSketchSpace())) {
    fail("Firmware image does not fit the available OTA flash space");
    return false;
  }

  status_.updateAvailable = compareSemver(FW_VERSION, status_.latestVersion) < 0;
  status_.ok = true;
  status_.lastError[0] = '\0';

  if (status_.updateAvailable) {
    char message[80];
    snprintf(message, sizeof(message), "Update available: %s", status_.latestVersion);
    appLog.info("OTA", message);
  } else {
    appLog.info("OTA", "Firmware is up to date");
  }

  return true;
}

bool OtaService::install() {
  if (WiFi.status() != WL_CONNECTED) {
    fail("WLAN is not connected");
    return false;
  }

  if (!status_.checked || !status_.ok || !status_.updateAvailable ||
      status_.firmwareUrl[0] == '\0' || status_.firmwareSize == 0) {
    fail("No checked OTA update is ready to install");
    return false;
  }

  if (!isAllowedFirmwareUrl(status_.firmwareUrl)) {
    fail("Firmware URL is outside the configured OTA channel");
    return false;
  }

  if (status_.firmwareSize > static_cast<uint32_t>(ESP.getFreeSketchSpace())) {
    fail("Firmware image does not fit the available OTA flash space");
    return false;
  }

  appLog.info("OTA", "Firmware update started");

  BearSSL::WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(30);

  HTTPClient http;
  http.setReuse(false);
  http.setTimeout(30000);
  http.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);
  http.useHTTP10(true);
  http.setUserAgent(String(PROJECT_NAME) + "/" + FW_VERSION);

  if (!http.begin(client, status_.firmwareUrl)) {
    fail("OTA firmware request initialization failed");
    return false;
  }
  http.addHeader("Accept", "application/octet-stream");
  http.addHeader("Cache-Control", "no-cache");

  const int code = http.GET();
  if (code != HTTP_CODE_OK) {
    char message[96];
    snprintf(message, sizeof(message), "OTA firmware returned HTTP %d", code);
    http.end();
    fail(message);
    return false;
  }

  const int contentLength = http.getSize();
  if (contentLength > 0 && static_cast<uint32_t>(contentLength) != status_.firmwareSize) {
    char message[96];
    snprintf(
        message,
        sizeof(message),
        "Firmware size mismatch: expected %lu, received %lu",
        static_cast<unsigned long>(status_.firmwareSize),
        static_cast<unsigned long>(contentLength));
    http.end();
    fail(message);
    return false;
  }

  WiFiClient* stream = http.getStreamPtr();
  if (!stream) {
    http.end();
    fail("OTA firmware stream is unavailable");
    return false;
  }

  uint8_t header[4] = {0, 0, 0, 0};
  stream->setTimeout(5000);
  if (stream->readBytes(header, sizeof(header)) != sizeof(header) || header[0] != 0xE9) {
    http.end();
    fail("Downloaded file is not a valid ESP8266 firmware image");
    return false;
  }

  if (!Update.begin(status_.firmwareSize, U_FLASH)) {
    char message[96];
    snprintf(message, sizeof(message), "Flash initialization failed: %u", Update.getError());
    http.end();
    fail(message);
    return false;
  }

  appLog.info("OTA", "Firmware download accepted; flashing started");
  size_t written = Update.write(header, sizeof(header));
  if (written == sizeof(header)) {
    written += Update.writeStream(*stream);
  }
  if (written != status_.firmwareSize) {
    char message[96];
    snprintf(
        message,
        sizeof(message),
        "Firmware write incomplete: %lu/%lu bytes (error %u)",
        static_cast<unsigned long>(written),
        static_cast<unsigned long>(status_.firmwareSize),
        Update.getError());
    (void)Update.end();
    http.end();
    fail(message);
    return false;
  }

  if (!Update.end() || !Update.isFinished()) {
    char message[96];
    snprintf(message, sizeof(message), "Firmware finalization failed: %u", Update.getError());
    http.end();
    fail(message);
    return false;
  }

  http.end();
  status_.lastError[0] = '\0';
  status_.ok = true;
  appLog.info("OTA", "Download and flash completed");
  appLog.info("OTA", "Update completed; reboot required");
  return true;
}

bool OtaService::parseSemver(const char* value, int& major, int& minor, int& patch) {
  if (!value || !*value) return false;
  const char* p = value;
  if (*p == 'v' || *p == 'V') ++p;

  char* end = nullptr;
  long a = strtol(p, &end, 10);
  if (end == p || *end != '.') return false;

  p = end + 1;
  long b = strtol(p, &end, 10);
  if (end == p || *end != '.') return false;

  p = end + 1;
  long c = strtol(p, &end, 10);
  if (end == p || *end != '\0') return false;

  if (a < 0 || b < 0 || c < 0 || a > 32767 || b > 32767 || c > 32767) {
    return false;
  }

  major = static_cast<int>(a);
  minor = static_cast<int>(b);
  patch = static_cast<int>(c);
  return true;
}

int OtaService::compareSemver(const char* a, const char* b) {
  int am = 0, an = 0, ap = 0;
  int bm = 0, bn = 0, bp = 0;

  const bool aOk = parseSemver(a, am, an, ap);
  const bool bOk = parseSemver(b, bm, bn, bp);

  if (!aOk && bOk) return -1;
  if (aOk && !bOk) return 1;
  if (!aOk && !bOk) return 0;

  if (am != bm) return am < bm ? -1 : 1;
  if (an != bn) return an < bn ? -1 : 1;
  if (ap != bp) return ap < bp ? -1 : 1;
  return 0;
}

bool OtaService::isAllowedFirmwareUrl(const char* url) {
  if (!url || !*url) return false;

  char expected[192];
  buildRawUrl(expected, sizeof(expected), OTA_ASSET_NAME);
  const size_t expectedLen = strlen(expected);

  if (strncmp(url, expected, expectedLen) != 0) return false;
  return url[expectedLen] == '\0' || url[expectedLen] == '?';
}
