#include "ota.h"

#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
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

  BearSSL::WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(30);

  // GitHub release assets redirect from github.com to a signed CDN URL.
  // ESP8266HTTPClient can fail while following such redirects when the
  // connection is reused. Build the HTTPClient explicitly and disable reuse
  // so every redirect is handled over a fresh connection.
  HTTPClient downloadHttp;
  downloadHttp.setReuse(false);
  downloadHttp.setTimeout(30000);
  downloadHttp.setRedirectLimit(5);
  downloadHttp.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
  downloadHttp.useHTTP10(true);
  if (!downloadHttp.begin(client, status_.firmwareUrl)) {
    fail("Firmware download initialization failed");
    return false;
  }

  ESPhttpUpdate.rebootOnUpdate(false);
  ESPhttpUpdate.setClientTimeout(30000);
  ESPhttpUpdate.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
  ESPhttpUpdate.setLedPin(-1);
  ESPhttpUpdate.onStart([]() {
    appLog.info("OTA", "Download started");
  });
  ESPhttpUpdate.onEnd([]() {
    appLog.info("OTA", "Download and flash completed");
  });
  ESPhttpUpdate.onError([](int error) {
    char message[96];
    snprintf(message, sizeof(message), "Updater error %d", error);
    appLog.error("OTA", message);
  });

  const HTTPUpdateResult result = ESPhttpUpdate.update(downloadHttp, FW_VERSION);

  if (result == HTTP_UPDATE_OK) {
    appLog.info("OTA", "Update completed; reboot required");
    return true;
  }

  if (result == HTTP_UPDATE_NO_UPDATES) {
    fail("Updater reported no newer firmware");
    return false;
  }

  const String detail = ESPhttpUpdate.getLastErrorString();
  char message[96];
  snprintf(message, sizeof(message), "Update failed: %s", detail.c_str());
  fail(message);
  return false;
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
