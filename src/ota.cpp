#include "ota.h"

#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecureBearSSL.h>
#include <Updater.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "logging.h"
#include "text_utils.h"
#include "version.h"

OtaService otaService;

namespace {

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

  char apiUrl[160];
  snprintf(
      apiUrl,
      sizeof(apiUrl),
      "https://api.github.com/repos/%s/%s/releases/latest",
      OTA_GITHUB_OWNER,
      OTA_GITHUB_REPO);

  BearSSL::WiFiClientSecure client;
  // GitHub's server certificate chain can change. v0.1.1 therefore pins the
  // OTA origin/repository but uses BearSSL insecure mode. See README security
  // notes and the future signed-firmware hardening path.
  client.setInsecure();
  client.setTimeout(12);

  HTTPClient http;
  http.setTimeout(12000);
  http.setReuse(false);
  http.useHTTP10(true);
  http.setUserAgent(String(PROJECT_NAME) + "/" + FW_VERSION);

  if (!http.begin(client, apiUrl)) {
    fail("GitHub API initialization failed");
    return false;
  }
  http.addHeader("Accept", "application/vnd.github+json");

  const int code = http.GET();
  if (code != HTTP_CODE_OK) {
    char message[96];
    snprintf(message, sizeof(message), "GitHub release check returned HTTP %d", code);
    http.end();
    fail(message);
    return false;
  }

  const int responseSize = http.getSize();
  if (responseSize > 32768) {
    http.end();
    fail("GitHub release metadata is unexpectedly large");
    return false;
  }

  JsonDocument filter;
  filter["tag_name"] = true;
  JsonObject assetFilter = filter["assets"][0].to<JsonObject>();
  assetFilter["name"] = true;
  assetFilter["browser_download_url"] = true;
  assetFilter["size"] = true;

  JsonDocument doc;
  const DeserializationError err = deserializeJson(
      doc,
      http.getStream(),
      DeserializationOption::Filter(filter),
      DeserializationOption::NestingLimit(4));
  http.end();

  if (err) {
    fail("GitHub release metadata is invalid JSON");
    return false;
  }

  const char* latest = doc["tag_name"] | "";
  int latestMajor = 0, latestMinor = 0, latestPatch = 0;
  if (!parseSemver(latest, latestMajor, latestMinor, latestPatch)) {
    fail("Latest GitHub tag is not a semantic version");
    return false;
  }
  copyText(status_.latestVersion, latest);

  JsonArrayConst assets = doc["assets"].as<JsonArrayConst>();
  for (JsonObjectConst asset : assets) {
    const char* name = asset["name"] | "";
    if (strcmp(name, OTA_ASSET_NAME) != 0) continue;

    const char* url = asset["browser_download_url"] | "";
    if (!isAllowedFirmwareUrl(url)) {
      fail("Release asset URL does not match the configured OTA channel");
      return false;
    }

    copyText(status_.firmwareUrl, url);
    status_.firmwareSize = asset["size"] | 0U;
    break;
  }

  if (status_.firmwareUrl[0] == '\0') {
    fail("firmware.bin is missing from the latest release");
    return false;
  }

  if (status_.firmwareSize > 0 &&
      status_.firmwareSize > static_cast<uint32_t>(ESP.getFreeSketchSpace())) {
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
      status_.firmwareUrl[0] == '\0') {
    fail("No checked OTA update is ready to install");
    return false;
  }

  if (!isAllowedFirmwareUrl(status_.firmwareUrl)) {
    fail("Firmware URL is outside the configured OTA channel");
    return false;
  }

  if (status_.firmwareSize > 0 &&
      status_.firmwareSize > static_cast<uint32_t>(ESP.getFreeSketchSpace())) {
    fail("Firmware image does not fit the available OTA flash space");
    return false;
  }

  appLog.info("OTA", "Firmware update started");

  // Resolve the GitHub release download redirect ourselves. GitHub release
  // assets are served through a short-lived signed URL on
  // release-assets.githubusercontent.com. Handling the redirect explicitly
  // lets us open a completely fresh TLS connection for the CDN and, more
  // importantly, report the real HTTP status code instead of ESPhttpUpdate's
  // generic -104 / Wrong HTTP Code result.
  BearSSL::WiFiClientSecure redirectClient;
  redirectClient.setInsecure();
  redirectClient.setTimeout(30);

  HTTPClient redirectHttp;
  redirectHttp.setReuse(false);
  redirectHttp.setTimeout(30000);
  redirectHttp.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);
  redirectHttp.useHTTP10(true);
  redirectHttp.setUserAgent(String(PROJECT_NAME) + "/" + FW_VERSION);

  if (!redirectHttp.begin(redirectClient, status_.firmwareUrl)) {
    fail("Firmware redirect request initialization failed");
    return false;
  }

  const int redirectCode = redirectHttp.GET();
  if (redirectCode <= 0) {
    char message[96];
    snprintf(message, sizeof(message), "Firmware redirect request failed: %d", redirectCode);
    redirectHttp.end();
    fail(message);
    return false;
  }

  {
    char message[72];
    snprintf(message, sizeof(message), "GitHub asset response HTTP %d", redirectCode);
    appLog.info("OTA", message);
  }

  String downloadUrl;
  if (redirectCode == HTTP_CODE_OK) {
    // GitHub may theoretically serve the asset directly. Preserve the
    // original URL in that case, but close this request and reopen it below
    // so the actual firmware transfer still gets a clean connection.
    downloadUrl = status_.firmwareUrl;
  } else if (redirectCode == HTTP_CODE_MOVED_PERMANENTLY ||
             redirectCode == HTTP_CODE_FOUND ||
             redirectCode == HTTP_CODE_SEE_OTHER ||
             redirectCode == HTTP_CODE_TEMPORARY_REDIRECT ||
             redirectCode == 308) {
    downloadUrl = redirectHttp.getLocation();
  } else {
    char message[96];
    snprintf(message, sizeof(message), "GitHub asset request returned HTTP %d", redirectCode);
    redirectHttp.end();
    fail(message);
    return false;
  }
  redirectHttp.end();

  if (downloadUrl.length() == 0) {
    fail("GitHub asset redirect did not provide a Location header");
    return false;
  }

  const bool directGithubUrl = downloadUrl.startsWith("https://github.com/");
  const bool releaseAssetUrl =
      downloadUrl.startsWith("https://release-assets.githubusercontent.com/");
  if (!directGithubUrl && !releaseAssetUrl) {
    fail("GitHub redirected firmware to an unexpected host");
    return false;
  }

  if (releaseAssetUrl) {
    appLog.info("OTA", "GitHub release asset redirect resolved");
  }

  BearSSL::WiFiClientSecure firmwareClient;
  firmwareClient.setInsecure();
  firmwareClient.setTimeout(30);

  HTTPClient firmwareHttp;
  firmwareHttp.setReuse(false);
  firmwareHttp.setTimeout(30000);
  firmwareHttp.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);
  firmwareHttp.useHTTP10(true);
  firmwareHttp.setUserAgent(String(PROJECT_NAME) + "/" + FW_VERSION);

  if (!firmwareHttp.begin(firmwareClient, downloadUrl)) {
    fail("Firmware CDN request initialization failed");
    return false;
  }
  firmwareHttp.addHeader("Accept", "application/octet-stream");

  const int firmwareCode = firmwareHttp.GET();
  if (firmwareCode != HTTP_CODE_OK) {
    char message[96];
    snprintf(message, sizeof(message), "Firmware download returned HTTP %d", firmwareCode);
    firmwareHttp.end();
    fail(message);
    return false;
  }

  const int contentLength = firmwareHttp.getSize();
  if (contentLength <= 0) {
    firmwareHttp.end();
    fail("Firmware download did not report a valid Content-Length");
    return false;
  }

  const uint32_t imageSize = static_cast<uint32_t>(contentLength);
  if (status_.firmwareSize > 0 && imageSize != status_.firmwareSize) {
    char message[96];
    snprintf(
        message,
        sizeof(message),
        "Firmware size mismatch: expected %lu, received %lu",
        static_cast<unsigned long>(status_.firmwareSize),
        static_cast<unsigned long>(imageSize));
    firmwareHttp.end();
    fail(message);
    return false;
  }

  if (imageSize > static_cast<uint32_t>(ESP.getFreeSketchSpace())) {
    firmwareHttp.end();
    fail("Firmware image does not fit the available OTA flash space");
    return false;
  }

  WiFiClient* stream = firmwareHttp.getStreamPtr();
  if (!stream) {
    firmwareHttp.end();
    fail("Firmware download stream is unavailable");
    return false;
  }

  uint8_t header[4] = {0, 0, 0, 0};
  if (stream->peekBytes(header, sizeof(header)) != sizeof(header) || header[0] != 0xE9) {
    firmwareHttp.end();
    fail("Downloaded file is not a valid ESP8266 firmware image");
    return false;
  }

  if (!Update.begin(imageSize, U_FLASH)) {
    char message[96];
    snprintf(message, sizeof(message), "Flash initialization failed: %u", Update.getError());
    firmwareHttp.end();
    fail(message);
    return false;
  }

  appLog.info("OTA", "Firmware download accepted; flashing started");
  const size_t written = Update.writeStream(*stream);
  if (written != imageSize) {
    char message[96];
    snprintf(
        message,
        sizeof(message),
        "Firmware write incomplete: %lu/%lu bytes (error %u)",
        static_cast<unsigned long>(written),
        static_cast<unsigned long>(imageSize),
        Update.getError());
    (void)Update.end();
    firmwareHttp.end();
    fail(message);
    return false;
  }

  if (!Update.end() || !Update.isFinished()) {
    char message[96];
    snprintf(message, sizeof(message), "Firmware finalization failed: %u", Update.getError());
    firmwareHttp.end();
    fail(message);
    return false;
  }

  firmwareHttp.end();
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

  char expectedPrefix[160];
  snprintf(
      expectedPrefix,
      sizeof(expectedPrefix),
      "https://github.com/%s/%s/releases/download/",
      OTA_GITHUB_OWNER,
      OTA_GITHUB_REPO);

  if (strncmp(url, expectedPrefix, strlen(expectedPrefix)) != 0) {
    return false;
  }

  const size_t len = strlen(url);
  const size_t assetLen = strlen(OTA_ASSET_NAME);
  return len >= assetLen &&
         strcmp(url + len - assetLen, OTA_ASSET_NAME) == 0;
}
