#include "network.h"

#include <ESP8266WiFi.h>
#include <ctype.h>

#include "config.h"
#include "logging.h"

NetworkService networkService;

namespace {

String sanitizedHostname(const char* source) {
  String result;
  result.reserve(32);
  if (!source) return String(DEFAULT_DEVICE_NAME);

  for (size_t i = 0; source[i] != '\0' && result.length() < 32; ++i) {
    const char c = source[i];
    if (isalnum(static_cast<unsigned char>(c)) || c == '-') {
      result += static_cast<char>(tolower(static_cast<unsigned char>(c)));
    } else if (c == ' ' || c == '_' || c == '.') {
      result += '-';
    }
  }

  while (result.startsWith("-")) result.remove(0, 1);
  while (result.endsWith("-")) result.remove(result.length() - 1);

  if (result.length() == 0) {
    result = DEFAULT_DEVICE_NAME;
    result.toLowerCase();
  }
  return result;
}

}  // namespace

void NetworkService::configureHostname(const char* deviceName) {
  const String host = sanitizedHostname(deviceName);
  WiFi.hostname(host);
}

void NetworkService::begin(const AppSettings& cfg) {
  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);
  configureHostname(cfg.deviceName);

  if (cfg.wifiSsid[0] == '\0') {
    WiFi.mode(WIFI_AP);
    startFallbackAp();
    appLog.warn("WIFI", "No WLAN configured; fallback access point started");
    lastConnected_ = false;
    disconnectedSinceMs_ = millis();
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(cfg.wifiSsid, cfg.wifiPassword);

  char message[96];
  snprintf(message, sizeof(message), "Connecting to SSID '%s'", cfg.wifiSsid);
  appLog.info("WIFI", message);

  const uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED &&
         static_cast<uint32_t>(millis() - start) < WIFI_BOOT_CONNECT_TIMEOUT_MS) {
    delay(100);
    yield();
  }

  if (WiFi.status() == WL_CONNECTED) {
    lastConnected_ = true;
    connectedSinceMs_ = millis();
    disconnectedSinceMs_ = 0;
    snprintf(message, sizeof(message), "Connected, IP %s", WiFi.localIP().toString().c_str());
    appLog.info("WIFI", message);
    return;
  }

  WiFi.mode(WIFI_AP_STA);
  startFallbackAp();
  disconnectedSinceMs_ = millis();
  lastConnected_ = false;
  appLog.warn("WIFI", "Connection timeout; fallback AP active while STA reconnect remains enabled");
}

void NetworkService::tick(const AppSettings& cfg) {
  const bool nowConnected = connected();

  if (nowConnected != lastConnected_) {
    lastConnected_ = nowConnected;
    if (nowConnected) {
      connectedSinceMs_ = millis();
      disconnectedSinceMs_ = 0;
      lastReconnectAttemptMs_ = 0;

      char message[96];
      snprintf(message, sizeof(message), "Connected, IP %s", WiFi.localIP().toString().c_str());
      appLog.info("WIFI", message);
    } else {
      disconnectedSinceMs_ = millis();
      appLog.warn("WIFI", "Connection lost");
    }
  }

  if (nowConnected) {
    if (apActive_ &&
        static_cast<uint32_t>(millis() - connectedSinceMs_) >= WIFI_AP_STOP_DELAY_MS) {
      stopFallbackAp();
    }
    return;
  }

  if (cfg.wifiSsid[0] == '\0') {
    if (!apActive_) startFallbackAp();
    return;
  }

  if (disconnectedSinceMs_ == 0) {
    disconnectedSinceMs_ = millis();
  }

  if (!apActive_ &&
      static_cast<uint32_t>(millis() - disconnectedSinceMs_) >= WIFI_FALLBACK_AP_AFTER_MS) {
    WiFi.mode(WIFI_AP_STA);
    startFallbackAp();
  }

  if (lastReconnectAttemptMs_ == 0 ||
      static_cast<uint32_t>(millis() - lastReconnectAttemptMs_) >= WIFI_RECONNECT_INTERVAL_MS) {
    lastReconnectAttemptMs_ = millis();
    WiFi.reconnect();
  }
}

bool NetworkService::connected() const {
  return WiFi.status() == WL_CONNECTED;
}

int32_t NetworkService::rssi() const {
  return connected() ? WiFi.RSSI() : 0;
}

String NetworkService::ipAddress() const {
  return connected() ? WiFi.localIP().toString() : String();
}

String NetworkService::apIpAddress() const {
  return apActive_ ? WiFi.softAPIP().toString() : String();
}

String NetworkService::hostname() const {
  return WiFi.hostname();
}

void NetworkService::startFallbackAp() {
  if (apActive_) return;

  if (!WiFi.softAP(FALLBACK_AP_SSID, FALLBACK_AP_PASSWORD)) {
    appLog.error("WIFI", "Fallback access point could not be started");
    return;
  }

  apActive_ = true;
  char message[96];
  snprintf(message, sizeof(message), "Fallback AP '%s' at %s", FALLBACK_AP_SSID, WiFi.softAPIP().toString().c_str());
  appLog.info("WIFI", message);
}

void NetworkService::stopFallbackAp() {
  if (!apActive_) return;
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  apActive_ = false;
  appLog.info("WIFI", "Fallback AP stopped after stable station connection");
}
