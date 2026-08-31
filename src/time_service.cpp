#include "time_service.h"

#include <ESP8266WiFi.h>
#include <time.h>

#include "config.h"
#include "logging.h"

TimeService timeService;

namespace {

bool clockLooksValid() {
  return time(nullptr) > 1700000000;
}

}  // namespace

void TimeService::begin() {
  settimeofday_cb([this](bool fromSntp) {
    onSntpUpdate(fromSntp);
  });
}

void TimeService::startSync(const AppSettings& cfg) {
  if (syncPending_) return;

  configTime(cfg.timezone, cfg.ntpServer);
  syncPending_ = true;
  syncStartedMs_ = millis();
  lastAttemptMs_ = millis();

  char message[112];
  snprintf(message, sizeof(message), "Synchronization started using %s", cfg.ntpServer);
  appLog.info("NTP", message);
}

bool TimeService::bootSynchronize(const AppSettings& cfg, bool wifiConnected) {
  if (!wifiConnected) {
    appLog.warn("NTP", "Boot synchronization skipped because WLAN is unavailable");
    return false;
  }

  appLog.info("NTP", "Boot synchronization requested");
  startSync(cfg);

  const uint32_t started = millis();
  while (!synchronized_ &&
         static_cast<uint32_t>(millis() - started) < NTP_BOOT_SYNC_TIMEOUT_MS) {
    delay(100);
    yield();
  }

  if (synchronized_) {
    appLog.info("NTP", "Boot synchronization successful");
    return true;
  }

  syncPending_ = false;
  appLog.warn("NTP", "Boot synchronization timed out; startup continues");
  return false;
}

void TimeService::tick(const AppSettings& cfg, bool wifiConnected) {
  if (!wifiConnected) return;

  if (syncPending_ &&
      static_cast<uint32_t>(millis() - syncStartedMs_) > NTP_BOOT_SYNC_TIMEOUT_MS) {
    syncPending_ = false;
    appLog.warn("NTP", "Synchronization attempt timed out");
  }

  if (!synchronized_) {
    if (!syncPending_ &&
        (lastAttemptMs_ == 0 ||
         static_cast<uint32_t>(millis() - lastAttemptMs_) >= NTP_RETRY_INTERVAL_MS)) {
      startSync(cfg);
    }
    return;
  }

  if (!syncPending_ &&
      static_cast<uint32_t>(millis() - lastAttemptMs_) >= NTP_RESYNC_INTERVAL_MS) {
    startSync(cfg);
  }
}

void TimeService::forceResync(const AppSettings& cfg, bool wifiConnected) {
  if (!wifiConnected) return;
  syncPending_ = false;
  startSync(cfg);
}

void TimeService::onSntpUpdate(bool fromSntp) {
  if (!fromSntp) return;

  synchronized_ = clockLooksValid();
  syncPending_ = false;
  if (synchronized_) {
    lastSyncEpoch_ = time(nullptr);
    lastAttemptMs_ = millis();
    appLog.info("NTP", "Synchronization successful");
  }
}

bool TimeService::getLocalTm(struct tm& out) const {
  if (!clockLooksValid()) return false;
  const time_t now = time(nullptr);
  localtime_r(&now, &out);
  return true;
}

String TimeService::localTimeString() const {
  struct tm value;
  if (!getLocalTm(value)) return String();

  char buffer[32];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &value);
  return String(buffer);
}

String TimeService::lastSyncString() const {
  if (lastSyncEpoch_ <= 0) return String();

  struct tm value;
  localtime_r(&lastSyncEpoch_, &value);
  char buffer[32];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &value);
  return String(buffer);
}
